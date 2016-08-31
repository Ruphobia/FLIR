#include "userinterface.h"
#include "camera.h"

static char PBState[2] = {1,1};
static uint32_t PBTime[2] = {0,0};
xQueueHandle ButtonQueue;

#define GPIO_HANDLER_PB1 gpio00_interrupt_handler

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
