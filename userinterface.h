#ifndef USERINTERFACE_H
#define USERINTERFACE_H

#include "headers.h"

void ButtonTask(void *pvParameters);
void Display_ChipSelect(bool Value);
void Display_Clear();
void Display_refresh(void);
void Display_sendbyteLSB(uint8_t data);
void Display_SendByte(uint8_t data);
void Alarm_OFF();
void Alarm_ON();
extern bool AlarmIsOn;

#endif
