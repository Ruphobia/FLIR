#include "userinterface.h"
#include "camera.h"

int buzzerInterval = 0;
bool AlarmIsOn = false;


void frc1_interrupt_handler(void)
{
	buzzerInterval++;
	if (buzzerInterval <= 2400)
	{
		gpio_toggle(GPIO_SPEAKER);
		gpio_write(GPIO_LED, 1);
	}
	else
	{
		gpio_write(GPIO_SPEAKER,0);
		gpio_write(GPIO_LED, 0);
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
		gpio_write(GPIO_LED, 0);
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



