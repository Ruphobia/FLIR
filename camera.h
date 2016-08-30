#ifndef CAMERA_H_
#define CAMERA_H_

#include "headers.h"

extern uint16_t CameraBuffer[4800];

void InitCamera();
void CaptureImage();
void ControlCreateSocketTask(void *pvParameters);

#endif
