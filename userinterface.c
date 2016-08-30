#include "userinterface.h"
#include "camera.h"

static char PBState[2] = {1,1};
static uint32_t PBTime[2] = {0,0};
xQueueHandle ButtonQueue;

#define GPIO_HANDLER_PB1 gpio00_interrupt_handler

#ifndef _swap_int16_t
#define _swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }
#endif
#ifndef _swap_uint16_t
#define _swap_uint16_t(a, b) { uint16_t t = a; a = b; b = t; }
#endif

uint8_t _sharpmem_vcom, datapinmask, clkpinmask;

// LCD Dimensions
#define SHARPMEM_LCDWIDTH       (96)
#define SHARPMEM_LCDHEIGHT 		(96)

#define SHARPMEM_BIT_WRITECMD   (0x80)
#define SHARPMEM_BIT_VCOM       (0x40)
#define SHARPMEM_BIT_CLEAR      (0x20)
#define TOGGLE_VCOM             do { _sharpmem_vcom = _sharpmem_vcom ? 0x00 : SHARPMEM_BIT_VCOM; } while(0);

uint8_t sharpmem_buffer[(SHARPMEM_LCDWIDTH * SHARPMEM_LCDHEIGHT) / 8];

int buzzerInterval = 0;
bool AlarmIsOn = false;


void frc1_interrupt_handler(void)
{

	buzzerInterval++;
	if (buzzerInterval <= 2400)
	{
		gpio_toggle(GPIO_SPEAKER);
		//gpio_write(GPIO_LED, 1);
	}
	else
	{
		gpio_write(GPIO_SPEAKER,0);
		//gpio_write(GPIO_LED, 0);
		if (buzzerInterval >= 4800)
			buzzerInterval = 0;
	}
}

void Alarm_OFF()
{
	if (AlarmIsOn)
	{
		timer_set_interrupts(FRC1, false);
		timer_set_run(FRC1, false);
		AlarmIsOn = false;
		gpio_write(GPIO_SPEAKER,0);
		//gpio_write(GPIO_LED, 0);
	}
}
void Alarm_ON()
{

	if (!AlarmIsOn)
	{
		timer_set_interrupts(FRC1, false);
		timer_set_run(FRC1, false);
		_xt_isr_attach(INUM_TIMER_FRC1, frc1_interrupt_handler);
		timer_set_frequency(FRC1, 4800);
		timer_set_interrupts(FRC1, true);
		timer_set_run(FRC1, true);
		AlarmIsOn = true;
	}
}

void Display_SendByte(uint8_t data)
{
	uint8_t i = 0;

	// LCD expects LSB first
	for (i=0; i<8; i++)
	{
		// Make sure clock starts low
		//digitalWrite(_clk, LOW);
		gpio_write(14,0);

		if (data & 0x80)
			//digitalWrite(_mosi, HIGH);
			gpio_write(13,1);
		else
			//digitalWrite(_mosi, LOW);
			gpio_write(13,0);

		// Clock is active high
		//digitalWrite(_clk, HIGH);
		gpio_write(14,1);
		data <<= 1;
	}
	// Make sure clock ends low
	//digitalWrite(_clk, LOW);
	gpio_write(14,0);
}

void Display_sendbyteLSB(uint8_t data)
{
	uint8_t i = 0;

	// LCD expects LSB first
	for (i=0; i<8; i++)
	{
		// Make sure clock starts low
		//digitalWrite(_clk, LOW);
		gpio_write(14,0);
		if (data & 0x01)
			//digitalWrite(_mosi, HIGH);
			gpio_write(13,1);
		else
			//digitalWrite(_mosi, LOW);
			gpio_write(13,0);
		// Clock is active high
		//digitalWrite(_clk, HIGH);
		gpio_write(14,1);
		data >>= 1;
	}
	// Make sure clock ends low
	//digitalWrite(_clk, LOW);
	gpio_write(14,0);
}

void Display_Clear()
{
	memset(sharpmem_buffer, 0xff, (SHARPMEM_LCDWIDTH * SHARPMEM_LCDHEIGHT) / 8);
	// Send the clear screen command rather than doing a HW refresh (quicker)
	Display_ChipSelect(1);
	Display_SendByte(_sharpmem_vcom | SHARPMEM_BIT_CLEAR);
	Display_sendbyteLSB(0x00);
	TOGGLE_VCOM;
	Display_ChipSelect(0);
}

void Display_ChipSelect(bool Value)
{
	if(Value == 1)
		GP16O |= 1; // Set GPIO_16
	else
		GP16O &= ~1; // clear GPIO_16a
}

void Display_refresh(void)
{
	uint16_t i, totalbytes, currentline, oldline;
	totalbytes = (SHARPMEM_LCDWIDTH * SHARPMEM_LCDHEIGHT) / 8;

	// Send the write command
	Display_ChipSelect(1);
	Display_SendByte(SHARPMEM_BIT_WRITECMD | _sharpmem_vcom);
	TOGGLE_VCOM;

	// Send the address for line 1
	oldline = currentline = 1;
	Display_sendbyteLSB(currentline);

	// Send image buffer
	for (i=0; i<totalbytes; i++)
	{
		Display_sendbyteLSB(sharpmem_buffer[i]);
		currentline = ((i+1)/(SHARPMEM_LCDWIDTH/8)) + 1;
		if(currentline != oldline)
		{
			// Send end of line and address bytes
			Display_sendbyteLSB(0x00);
			if (currentline <= SHARPMEM_LCDHEIGHT)
			{
				Display_sendbyteLSB(currentline);
			}
			oldline = currentline;
		}
	}

	// Send another trailing 8 bits for the last line
	Display_sendbyteLSB(0x00);
	Display_ChipSelect(0);
}


//Debounce button events, and generate button press
//events.  short button presses are > 200ms and < 1 second
void GPIO_HANDLER(char ButtonNumber, char ButtonState)
{

	uint32_t now = xTaskGetTickCountFromISR();//get current tick count
	//calculate delta time between last interrupt and now
	//transform to ms
	uint32_t DeltaT = (now - PBTime[(int)ButtonNumber]) * portTICK_RATE_MS;


	char EventButtonPressed = 0;//clear our button event

	if (ButtonState == 0)//if button is down
	{
		//debounce button press by ignoring anything that
		//is less than 20ms in duration
		if( DeltaT >= 20)
		{
			//record time and button state
			PBTime[(int)ButtonNumber] = (uint32_t)now;
			PBState[(int)ButtonNumber] = ButtonState;
		}
	}
	else
	{//button is up

		//button is up and previously down
		if(	PBState[(int)ButtonNumber] == 0 )
		{
			//get current time for debounce
			PBTime[(int)ButtonNumber] = (uint32_t)now;
			PBState[(int)ButtonNumber] = ButtonState;

			//if button is held for more than 20ms
			if (DeltaT >= 20)
			{
				if (ButtonNumber == 1)//pushbutton #1
				{
					if (DeltaT < 400)//short press
						EventButtonPressed = 1;
					else//long press
						EventButtonPressed = 2;
				}
			}
		}
	}

	//generate button press event and stuff in the message queue
	if (EventButtonPressed > 0)
		xQueueSendToBackFromISR(ButtonQueue, &EventButtonPressed, NULL);

}


//push button #1 event handler, using
//gpio #0
void GPIO_HANDLER_PB1()
{
	//get the button state UP = 1, Down = 0
	//then do something about the button event
	GPIO_HANDLER(1, (char)gpio_read(0));
}

//send button events to client
void ButtonTask(void *pvParameters)
{

	while(1)
	{
		char button_ts = 0;
		xQueueReceive(ButtonQueue, &button_ts,portMAX_DELAY);//pull button message from queue

		if (button_ts > 0)
		{

		}

	}

}
