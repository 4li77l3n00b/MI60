#include "mi.h"

inline void DelayUs(uint32_t _quarterus)
{
    for (int i = 0; i < _quarterus; i++)
        for (int j = 0; j < 13; j++)  // ToDo: tune this for different chips
            __NOP();
}

inline void uint16to8(const uint16_t* _op0, uint8_t* _op1, uint8_t* _op2) {
    *_op1 = (uint8_t)((*_op0 >> 8) & 0xFF);
    *_op2 = (uint8_t)((*_op0) & 0xFF);
}

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
            if (m <= ADC_CONFIG[i*4+k].ZERO_POINT) keyBuffer[i*4+k] = false;
            else {
                switch (ADC_CONFIG[i * 4 + k].TRG_MODE) {
                    case WOOT:
                        if (m < ADC_CONFIG[i*4+k].ACT_POINT) {
                            TRIGGERED[i*4+k] = false;
                            scanBuffer[i*4+k] = ADC_CONFIG[i*4+k].ACT_POINT;
                            keyBuffer[i*4+k] = false;
                        }
                        if (!TRIGGERED[i*4+k] && m >= ADC_CONFIG[i*4+k].ACT_POINT) {
                            TRIGGERED[i*4+k] = true;
                            keyBuffer[i*4+k] = true;
                            PENDING[i*4+k] = false;
                            GOING[i*4+k] = true;
                        }
                        if (TRIGGERED[i*4+k]) {
                            if (PENDING[i*4+k]) {
                                if (scanBuffer[i*4+k] - m >= ADC_CONFIG[i*4+k].LIFT_THRESHOLD) {
                                    PENDING[i*4+k] = false;
                                    keyBuffer[i*4+k] = false;
                                }
                                else if (m - scanBuffer[i*4+k] >= ADC_CONFIG[i*4+k].PRESS_THRESHOLD) {
                                    PENDING[i*4+k] = false;
                                    keyBuffer[i*4+k] = true;
                                }
                            }
                            else {
                                if ((m > scanBuffer[i*4+k]) ^ !GOING[i*4+k]) {
                                    PENDING[i*4+k] = true;
                                }
                                else scanBuffer[i*4+k] = m;
                            }
                        }
                    case APEX:
                        keyBuffer[i * 4 + k] = m > ADC_CONFIG[i * 4 + k].ACT_POINT;
                }
            }
        }
    }

}

void MI::Calibrate(MI::ScanConfig *_config) {
    memset(GOING, false, KEYNUM);
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
            if (i*4+k == NowCalibrating) {
                m = ADC_value[k];
                if (!GOING[i * 4 + k] && m == scanBuffer[i * 4 + k]) {
                    _config->ZERO_POINT = m;
                    scanBuffer[i * 4 + k] = m;
                } else {
                    if (m >= scanBuffer[i * 4 + k]) {
                        if (!GOING[i * 4 + k]) GOING[i * 4 + k] = true;
                        scanBuffer[i * 4 + k] = m;
                    } else {
                        GOING[i * 4 + k] = false;
                        _config->MAX_POINT = scanBuffer[i * 4 + k];
                        isCalibrating = false;
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
        if (key) Press(keyMap[keyBuffer[58]?1:0][i]);
    }
}

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

void MI::StoreCalibration() {
    W25qxx_EraseSector(0);
    w25qxx.Lock = 1;
    memset(sfBuffer, 0xFF, 4096);
    sfBuffer[0] = KeyAddr;
    sfBuffer[1] = KeyMapAddr;
    sfBuffer[2] = RGBFXAddr;
    sfBuffer[3] = RGBMapAddr;
    for (int i = 0; i < 61; i++) {
        sfBuffer[4*i+4] = (uint8_t)((ADC_CONFIG[i].ZERO_POINT >> 8) & 0xFF);
        sfBuffer[4*i+5] = (uint8_t)((ADC_CONFIG[i].ZERO_POINT) & 0xFF);
        sfBuffer[4*i+6] = (uint8_t)((ADC_CONFIG[i].MAX_POINT >> 8) & 0xFF);
        sfBuffer[4*i+7] = (uint8_t)((ADC_CONFIG[i].MAX_POINT) & 0xFF);
    }
    w25qxx.Lock = 0;
    W25qxx_WriteSector(sfBuffer, 0, 0, 248);
}

void MI::StoreKey(uint8_t key_addr) {
    W25qxx_EraseSector(key_addr);
    w25qxx.Lock = 1;
    for (uint8_t i = 0; i < 61; i++) {
        sfBuffer[6 * i] = (ADC_CONFIG[i].TRG_MODE == WOOT)? 1 : 0;
        sfBuffer[6 * i + 1] = (uint8_t) ((ADC_CONFIG[i].ACT_POINT >> 8) & 0xFF);
        sfBuffer[6 * i + 2] = (uint8_t) ((ADC_CONFIG[i].ACT_POINT) & 0xFF);
        sfBuffer[6 * i + 3] = (uint8_t) ((ADC_CONFIG[i].LIFT_THRESHOLD >> 8) & 0xFF);
        sfBuffer[6 * i + 4] = (uint8_t) ((ADC_CONFIG[i].LIFT_THRESHOLD) & 0xFF);
        sfBuffer[6 * i + 5] = (uint8_t) ((ADC_CONFIG[i].MAX_POINT >> 8) & 0xFF);
        sfBuffer[6 * i + 6] = (uint8_t) ((ADC_CONFIG[i].MAX_POINT) & 0xFF);
    }
    w25qxx.Lock = 0;
    W25qxx_WriteSector(sfBuffer, key_addr, 0, 427);
}

void MI::StoreKeyMap(uint8_t keymap_addr) {
    W25qxx_EraseSector(keymap_addr);
    w25qxx.Lock = 1;
    for (uint8_t i = 0; i < 61; i++) {
        uint16to8((const uint16_t *) keyMap[0][i], &sfBuffer[2 * i], &sfBuffer[2 * i + 1]);
        uint16to8((const uint16_t *) keyMap[1][i], &sfBuffer[2 * i + 122], &sfBuffer[2 * i + 123]);
    }
    w25qxx.Lock = 0;
    W25qxx_WriteSector(sfBuffer, keymap_addr, 0, 248);
}

void MI::StoreRGBMap(uint8_t rgbmap_addr) {
    W25qxx_EraseSector(rgbmap_addr);
    w25qxx.Lock = 1;
    for (uint8_t i = 0; i < 65; i++) {
        sfBuffer[3*i] = RGBMap[0][i].r;
        sfBuffer[3*i+1] = RGBMap[0][i].g;
        sfBuffer[3*i+2] = RGBMap[0][i].b;
        sfBuffer[3*i+195] = RGBMap[1][i].r;
        sfBuffer[3*i+196] = RGBMap[1][i].g;
        sfBuffer[3*i+197] = RGBMap[1][i].b;
    }
    w25qxx.Lock = 0;
    W25qxx_WriteSector(sfBuffer, rgbmap_addr, 0, 390);
}