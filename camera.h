#ifndef CAMERA_H_
#define CAMERA_H_

#include "headers.h"

extern FLIRBuffer CameraBuffer;
extern sys_mutex_t CameraBufferMutex;
void InitCamera();
void CaptureImage();
void ControlCreateSocketTask(void *pvParameters);
void MeasureBabyTask(void *pvParameters);

#endif
