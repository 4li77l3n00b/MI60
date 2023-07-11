#include "common_inc.h"
#include "MI/mi.h"
MI mi;

void KeyboardMain() {
    //Init config from W25Q16 Flash
    W25qxx_Init();
    mi.InitAndIndex();
    mi.CopyKeyArgs();
    mi.CopyConfKeyMap();
    mi.CopyKeyMap();
    mi.CopyRGBMap();
    mi.CopyRGBFXArgs();
    HAL_ADC_Start_DMA(&hadc3, (uint32_t *) ADC_BUF, 2);
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *) (ADC_BUF + 2), 2);
    HAL_TIM_Base_Start_IT(&htim5);
    //RGB Frame Loop
    while (1) {
        DelayUs(500);

        mi.SyncLights();
    }
}

extern "C" void OnTimerCallBack()
{
    if (!mi.isCalibrating) {
        mi.ScanAndUpdate();
        mi.PostProcess();
        USBD_CUSTOM_HID_SendReport(&hUsbDeviceHS,
                                   mi.GetHidReportBuffer(1),
                                   MI::KEY_REPORT_SIZE);
    } else {
        mi.Calibrate(mi.CalibKeyID);
    }
}

extern "C"
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef* hspi)
{
    if (hspi == &hspi2)
        mi.isRgbTxBusy = false;
}

void uint16to8(const uint16_t* _op0, uint8_t* _op1, uint8_t* _op2) {
    *_op1 = (uint8_t)((*_op0 >> 8) & 0xFF);
    *_op2 = (uint8_t)((*_op0) & 0xFF);
}

void uint8to16(const uint8_t* _op1, const uint8_t* _op2, uint16_t* _dest) {
    *_dest = (uint16_t)*_op2 | ((uint16_t)*_op1 << 8);
}

void uint8tofloat(const uint8_t* addr, float_t* _dest) {
    memcpy(_dest, addr, 4);
}

extern "C"
void CapsLock(bool state) {
    mi.isCapsLocked = state;
}

extern "C"
void ScrollLock(bool state) {
    mi.isScrollLocked = state;
}

extern "C"
void ReturnState(uint8_t _state) {
    mi.hidBuffer[mi.KEY_REPORT_SIZE + 1] = _state;
    USBD_CUSTOM_HID_SendReport(&hUsbDeviceHS,
                               mi.GetHidReportBuffer(2),
                               mi.RAW_REPORT_SIZE);
}

extern "C"
void SyncAll() {
    mi.SyncKeyArgs();
    mi.SyncConfKeyMap();
    mi.SyncKeyMap();
    mi.SyncRGBMap();
    mi.SyncRGBFXArgs();
}

extern "C"
void StartCalibration(uint8_t _keyID) {
    mi.CalibKeyID = _keyID;
    mi.isCalibrating = true;
    ReturnState(0xFF);
}

extern  "C"
void ChangeKeyArg(const uint8_t* _buf) {
    uint8_t _keyID = _buf[0];
    mi.miConfig[_keyID].KeyMode = (_buf[1] == 1)? MI::WOOT : MI::APEX;
    uint8tofloat((_buf+2), &(mi.miConfig[_keyID].ActPoint));
    uint8tofloat((_buf+6), &(mi.miConfig[_keyID].LiftTravel));
    uint8tofloat((_buf+10), &(mi.miConfig[_keyID].PressTravel));
    mi.Convert(&mi.miConfig[_keyID], &mi.ADC_CONFIG[_keyID]);
    ReturnState(0xFF);
}

extern "C"
void ChangeConfKeyMap(const uint8_t* _buf) {
    memcpy(mi.ConflictingKeyMap, _buf, 8);
    ReturnState(0xFF);
}

extern "C"
void ChangeKeyMap(const uint8_t* _buf) {
    uint8_t _layer = _buf[0];
    uint8_t _keyID = _buf[1];
    uint8to16(&_buf[2], &_buf[3], reinterpret_cast<uint16_t *>(&mi.keyMap[_layer][_keyID]));
    ReturnState(0xFF);
}

extern "C"
void ChangeRGBMap(const uint8_t* _buf) {
    uint8_t _layer = _buf[0];
    uint8_t _keyID = _buf[1];
    memcpy(&mi.RGBMap[_layer][_keyID], _buf + 2, 3);
    ReturnState(0xFF);
}

extern "C"
void ChangeRGBFXArg(const uint8_t* _buf) {
    memcpy(mi.RGBFXArgs, _buf + 1, 16);
    ReturnState(0xFF);
}