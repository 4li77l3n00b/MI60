#ifndef LOOP_H
#define LOOP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint-gcc.h"
#include "stm32h7xx_hal.h"
#include "main.h"
#include "stm32h7xx_hal_tim.h"
#include "usbd_custom_hid_if.h"
#include "usbd_customhid.h"
#include "usb_device.h"
#include "w25qxx.h"

void KeyboardMain();
void OnTimerCallBack();

#ifdef __cplusplus
}
#endif
#endif