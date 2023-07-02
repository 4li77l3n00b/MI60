#include "mi.h"

inline void DelayUs(uint32_t _quarterus)
{
    for (int i = 0; i < _quarterus; i++)
        for (int j = 0; j < 13; j++)  // ToDo: tune this for different chips
            __NOP();
}

void MI::ScanAndUpdate()
{
    //adc scan
    uint16_t m;
    for (uint16_t i = 0; i < 16; i++) {
        HAL_GPIO_WritePin(GPIOB, i << 10, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, (i^0xF) << 10, GPIO_PIN_RESET);
        DelayUs(1);
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
    for (uint16_t i = 0; i < 16; i++) {
        HAL_GPIO_WritePin(GPIOB, i << 10, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, (i ^ 0xF) << 10, GPIO_PIN_RESET);
        DelayUs(1);
        for (int k = 0; k < 4; k++) {
            m = ADC_BUF[k];
            if (!GOING[i*4+k] && m == scanBuffer[i*4+k]) {
                _config->ZERO_POINT = m;
                scanBuffer[i*4+k] = m;
            }
            else {
                if (m >= scanBuffer[i*4+k]) {
                    if (!GOING[i*4+k]) GOING[i*4+k] = true;
                    scanBuffer[i*4+k] = m;
                }
                else {
                    GOING[i*4+k] = false;
                    _config->MAX_POINT = scanBuffer[i*4+k];
                    isCalibrating = false;
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