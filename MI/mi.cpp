#include "mi.h"

MI mi;

const uint16_t SectorByteLength[6] = {4*61, 9*61, 4*2*2, 2*2*61, 3*65*3, 32};

const uint16_t offset = 0;

void DelayUs(uint32_t _quarterus)
{
    for (int i = 0; i < _quarterus; i++)
        for (int j = 0; j < 13; j++)  // ToDo: tune this for different chips
            __NOP();
}
/*----------------------------------------------------------------------------------------------------------------------*/
void MI::ScanAndUpdate()
{
    //adc scan
    uint16_t ADC_value[4];
    uint16_t m;
    for (uint16_t i = 0; i < 16; i++) {
        HAL_GPIO_WritePin(GPIOD, i << 10, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOD, (i^0xF) << 10, GPIO_PIN_RESET);
        DelayUs(16);
        SCB_InvalidateDCache_by_Addr((uint32_t *)ADC_BUF, sizeof(ADC_BUF));
        for (int j = 0; j < 4; j++) ADC_value[j] = ADC_BUF[j];
        for (int k = 0; k < 4; k++) {
            m = ADC_BUF[k];
            switch (ADC_CONFIG[i * 4 + k].TRG_MODE) {
                case WOOT:
                    if (m > ADC_CONFIG[i*4+k].ACT_POINT) { //处于上死区
                        TRIGGERED[i*4+k] = false;
                        scanBuffer[i*4+k] = ADC_CONFIG[i*4+k].ACT_POINT;
                        keyBuffer[i*4+k] = false;
                        GOING[i*4+k] = false;
                    }
                    else if (m < ADC_CONFIG[i*4+k].ACT2_POINT) { //处于下死区
                        TRIGGERED[i*4+k] = false;
                        scanBuffer[i*4+k] = ADC_CONFIG[i*4+k].ACT2_POINT;
                        keyBuffer[i*4+k] = true;
                        GOING[i*4+k] = true;
                    }
                    if (TRIGGERED[i*4+k]) {
                        if (PENDING[i*4+k]) {
                            if (m - scanBuffer[i*4+k] >= ADC_CONFIG[i*4+k].LIFT_THRESHOLD) { //抬起距离大于触发行程
                                PENDING[i*4+k] = false;
                                keyBuffer[i*4+k] = false;
                            }
                            else if (scanBuffer[i*4+k] - m >= ADC_CONFIG[i*4+k].PRESS_THRESHOLD) { //按下距离大于触发行程
                                PENDING[i*4+k] = false;
                                keyBuffer[i*4+k] = true;
                            }
                        }
                        else { //已触发完，需判断下一次触发 （按键处于后触发状态）
                            if ((m > scanBuffer[i*4+k]) & ((abs(m - scanBuffer[i*4+k])) > offset) & GOING[i*4+k]) { //发生反向移动 //此处存在因adc抖动导致的系统误差，需实际测量后详细调定)
                                PENDING[i*4+k] = true; //进入下一次待触发状态
                                GOING[i*4+k] = ~GOING[i*4+k]; //反转移动方向
                            }
                            else scanBuffer[i*4+k] = m; //仍处于后触发状态，保持记录当前位置
                        }
                    }
                    if (!TRIGGERED[i*4+k] && (m >= ADC_CONFIG[i*4+k].ACT2_POINT && m <= ADC_CONFIG[i*4+k].ACT_POINT)) {
                        TRIGGERED[i*4+k] = true; //进入判定区
                        keyBuffer[i*4+k] = true; //按键触发
                        PENDING[i*4+k] = false; //等待触发
                        GOING[i*4+k] = ~GOING[i*4+k]; //反转移动方向（是否在向下走，仅用于指示后触发状态的行程方向）
                    }
                case APEX:
                    keyBuffer[i * 4 + k] = ADC_CONFIG[i * 4 + k].ACT_POINT - m > offset;
            }
        }
    }

}

void MI::Calibrate(uint8_t _keyID) {
    //adc scan
    uint16_t m;
    uint16_t ADC_value[4];
    for (uint16_t i = 0; i < 16; i++) {
        HAL_GPIO_WritePin(GPIOD, i << 10, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOD, (i ^ 0xF) << 10, GPIO_PIN_RESET);
        DelayUs(16);
        SCB_InvalidateDCache_by_Addr((uint32_t *)ADC_BUF, sizeof(ADC_BUF));
        for (int j = 0; j < 4; j++) ADC_value[j] = ADC_BUF[j];
        for (int k = 0; k < 4; k++) {
            if (i*4+k == _keyID) {
                m = ADC_value[k];
                if (!GOING[i * 4 + k] && abs(m - scanBuffer[i * 4 + k]) <= offset) {
                    ADC_CONFIG[_keyID].ZERO_POINT = m;
                    scanBuffer[i * 4 + k] = m;
                } else {
                    if (scanBuffer[i * 4 + k] - m >= offset) {
                        if (!GOING[i * 4 + k]) GOING[i * 4 + k] = true;
                        scanBuffer[i * 4 + k] = m;
                    }
                    else if (m - scanBuffer[i * 4 + k] >= offset) {
                        GOING[i * 4 + k] = false;
                        ADC_CONFIG[_keyID].MAX_POINT = scanBuffer[i * 4 + k];
                        isCalibrating = false;
                        ReturnState(0xFF);
                        Convert(&miConfig[_keyID], &ADC_CONFIG[_keyID]);
                    }
                }
            }
        }
    }
}

void MI::Press(MI::KeyCode_t _key)
{
    int index, bitIndex;

    if (_key < RESERVED)
    {
        index = _key / 8;
        bitIndex = (_key + 8) % 8;
    } else
    {
        index = _key / 8 + 1;
        bitIndex = _key % 8;
    }

    hidBuffer[index + 1] |= (1 << bitIndex);
}

void MI::Release(MI::KeyCode_t _key)
{
    int index, bitIndex;

    if (_key < RESERVED)
    {
        index = _key / 8;
        bitIndex = (_key + 8) % 8;
    } else
    {
        index = _key / 8 + 1;
        bitIndex = _key % 8;
    }

    hidBuffer[index + 1] &= ~(1 << bitIndex);
}

void MI::PostProcess() {
    memset(hidBuffer, 0, KEY_REPORT_SIZE);
    for (int i = 0; i < KEYNUM; i++) {
        bool key = keyBuffer[i];
        if (key) Press(keyMap[keyBuffer[60]?1:0][i]);
    }
    for (auto & _conf_key : ConflictingKeyMap) {
        if (keyBuffer[_conf_key[0]] && keyBuffer[_conf_key[1]]) {
            Release(keyMap[keyBuffer[60]?1:0][_conf_key[0]]);
            Release(keyMap[keyBuffer[60]?1:0][_conf_key[1]]);
        }
    }
}
//----------------------------------------------------------------------------------------------------------------------
void MI::SetRgbBufferByID(uint8_t _keyId, MI::Color_t _color, float _brightness)
{
    // To ensure there's no sequence zero bits, otherwise will case ws2812b protocol error.
    if (_color.b < 1)_color.b = 1;

    for (int i = 0; i < 8; i++)
    {
        rgbBuffer[_keyId][0][i] =
                ((uint8_t) ((float) _color.g * _brightness) >> brightnessPreDiv) & (0x80 >> i) ? WS_HIGH : WS_LOW;
        rgbBuffer[_keyId][1][i] =
                ((uint8_t) ((float) _color.r * _brightness) >> brightnessPreDiv) & (0x80 >> i) ? WS_HIGH : WS_LOW;
        rgbBuffer[_keyId][2][i] =
                ((uint8_t) ((float) _color.b * _brightness) >> brightnessPreDiv) & (0x80 >> i) ? WS_HIGH : WS_LOW;
    }
}

void MI::SyncLights()
{
    while (isRgbTxBusy);
    isRgbTxBusy = true;
    HAL_SPI_Transmit_DMA(&hspi2, (uint8_t*) rgbBuffer, LED_NUMBER * 3 * 8);
    while (isRgbTxBusy);
    isRgbTxBusy = true;
    HAL_SPI_Transmit_DMA(&hspi2, wsCommit, 64);
}
//----------------------------------------------------------------------------------------------------------------------
uint8_t* MI::GetHidReportBuffer(uint8_t _reportId)
{
    switch (_reportId)
    {
        case 1:
            hidBuffer[0] = 1;
            return hidBuffer;
        case 2:
            hidBuffer[KEY_REPORT_SIZE] = 2;
            return hidBuffer + KEY_REPORT_SIZE;
        default:
            return hidBuffer;
    }
}
//----------------------------------------------------------------------------------------------------------------------
bool MI::isFnPressed() {
    return keyBuffer[60];
}
//----------------------------------------------------------------------------------------------------------------------
// Read and Apply Parameters
inline void ReadSector(uint16_t addr, uint8_t usage) {
    W25qxx_ReadSector(sfBuffer,
                      addr,
                      0,
                      SectorByteLength[usage]);
}

void MI::InitAndIndex() {
    ReadSector(0, 0);
    for (int i = 0; i < KEYNUM-3; i++) {
        uint8to16(&sfBuffer[4*i+4], &sfBuffer[4*i+5], &ADC_CONFIG[i].ZERO_POINT);
        uint8to16(&sfBuffer[4*i+6], &sfBuffer[4*i+7], &ADC_CONFIG[i].MAX_POINT);
    }
}

void MI::CopyKeyArgs() {
    ReadSector(1, 1);
    for (int i = 0; i < KEYNUM-3; i++) {
        ADC_CONFIG[i].TRG_MODE = (sfBuffer[i*7] == 0 ? MI::APEX : MI::WOOT);
        uint8to16(&sfBuffer[i*7+1], &sfBuffer[i*7+2], &ADC_CONFIG[i].ACT_POINT);
        uint8to16(&sfBuffer[i*7+3], &sfBuffer[i*7+4], &ADC_CONFIG[i].ACT2_POINT);
        uint8to16(&sfBuffer[i*7+5], &sfBuffer[i*7+6], &ADC_CONFIG[i].LIFT_THRESHOLD);
        uint8to16(&sfBuffer[i*7+7], &sfBuffer[i*7+8], &ADC_CONFIG[i].PRESS_THRESHOLD);
    }
}

void MI::CopyConfKeyMap() {
    ReadSector(2, 2);
    memcpy(ConflictingKeyMap, sfBuffer, SectorByteLength[2]);
}

void MI::CopyKeyMap() {
    ReadSector(3, 3);
    memcpy(keyMap, sfBuffer, SectorByteLength[3]);
}

void MI::CopyRGBMap() {
    ReadSector(4, 4);
    memcpy(RGBMap, sfBuffer, SectorByteLength[4]);
}

void MI::CopyRGBFXArgs() {
    ReadSector(5, 5);
    memcpy(RGBFXArgs, sfBuffer, 32);
}

//----------------------------------------------------------------------------------------------------------------------
// Modify and Synchronize Parameters
void WriteSector(uint16_t addr, uint8_t usage) {
    W25qxx_EraseSector(addr);
    W25qxx_WriteSector(txBuffer,
                       addr,
                       0,
                       SectorByteLength[usage]);
}

void MI::SyncCalibration() {
    for (int i = 0; i < KEYNUM - 3; i++) {
        uint16to8(&ADC_CONFIG[i].ZERO_POINT, &txBuffer[4*i], &txBuffer[4*i+1]);
        uint16to8(&ADC_CONFIG[i].MAX_POINT, &txBuffer[4*i+2], &txBuffer[4*i+3]);
    }
    WriteSector(0, 0);
}

void MI::SyncKeyArgs() {
    for (int i = 0; i < KEYNUM - 3; i++) {
        txBuffer[7*i] = (ADC_CONFIG[i].TRG_MODE == WOOT)? 1:0;
        uint16to8(&ADC_CONFIG[i].ACT_POINT,  &txBuffer[7*i+1], &txBuffer[7*i+2]);
        uint16to8(&ADC_CONFIG[i].ACT2_POINT,  &txBuffer[7*i+3], &txBuffer[7*i+4]);
        uint16to8(&ADC_CONFIG[i].LIFT_THRESHOLD,  &txBuffer[7*i+5], &txBuffer[7*i+6]);
        uint16to8(&ADC_CONFIG[i].PRESS_THRESHOLD,  &txBuffer[7*i+7], &txBuffer[7*i+8]);
    }
    WriteSector(1, 1);
}

void MI::SyncConfKeyMap() {
    memcpy(txBuffer, ConflictingKeyMap, SectorByteLength[2]);
    WriteSector(2, 2);
}

void MI::SyncKeyMap() {
    memcpy(txBuffer, keyMap, SectorByteLength[3]);
    WriteSector(3, 3);
}

void MI::SyncRGBMap() {
    memcpy(txBuffer, RGBMap, SectorByteLength[2]);
    WriteSector(4, 4);
}

void MI::SyncRGBFXArgs() {
    memcpy(txBuffer, RGBFXArgs, SectorByteLength[2]);
    WriteSector(5, 5);
}
