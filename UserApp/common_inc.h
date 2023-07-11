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

void uint16to8(const uint16_t* _op0, uint8_t* _op1, uint8_t* _op2);
void uint8to16(const uint8_t* _op1, const uint8_t* _op2, uint16_t* _dest);

void ReturnState(uint8_t _state);

void CapsLock(bool state);
void ScrollLock(bool state);

void SyncAll();

void StartCalibration(uint8_t _keyID);

void ChangeKeyArg(const uint8_t* _buf);

void ChangeConfKeyMap(const uint8_t* _buf);

void ChangeKeyMap(const uint8_t* _buf);

void ChangeRGBMap(const uint8_t* _buf);

void ChangeRGBFXArg(const uint8_t* _buf);

#ifdef __cplusplus
}
#endif
#endif