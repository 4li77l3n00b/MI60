#include "common_inc.h"
#include "MI/mi.h"
#include "w25qxx.h"
MI mi;
uint8_t KeyMapAddr;
uint8_t KeyAddr;
uint8_t RGBMapAddr;
uint8_t RGBFXAddr;

void KeyboardMain() {
    //Init config from W25Q16 Flash
    W25qxx_Init();
    W25qxx_ReadSector(sfBuffer, 0, 0, 248);
    KeyAddr = sfBuffer[0];
    KeyMapAddr = sfBuffer[1];
    RGBMapAddr = sfBuffer[2];
    RGBFXAddr = sfBuffer[3];
    //Fetch calibration result
    for (int i = 0; i < 61; i++) {
        mi.ADC_CONFIG[i].ZERO_POINT = sfBuffer[4*i+4] | (sfBuffer[4*i+5] << 8);
        mi.ADC_CONFIG[i].MAX_POINT = sfBuffer[4*i+6] | (sfBuffer[4*i+7] << 8);
    }
    //Fetch key config
    W25qxx_ReadSector(sfBuffer, KeyAddr, 0, 427);
    for (int i = 0; i < 61; ++i) {
        mi.ADC_CONFIG[i].TRG_MODE = (sfBuffer[i*7] == 0 ? MI::APEX : MI::WOOT);
        mi.ADC_CONFIG[i].ACT_POINT = sfBuffer[i*7+1] | (sfBuffer[i*7+2] << 8);
        mi.ADC_CONFIG[i].LIFT_THRESHOLD = sfBuffer[i*7+3] | (sfBuffer[i*7+4] << 8);
        mi.ADC_CONFIG[i].PRESS_THRESHOLD = sfBuffer[i*7+5] | (sfBuffer[i*7+6] << 8);
    }
    //Fetch key map
    W25qxx_ReadSector(sfBuffer, KeyMapAddr, 0, 244);
    for (int i = 0; i < 61; ++i) {
        mi.keyMap[0][i] = static_cast<MI::KeyCode_t>(sfBuffer[2 * i] | (sfBuffer[2 * i + 1] << 8));
        mi.keyMap[1][i] = static_cast<MI::KeyCode_t>(sfBuffer[2 * i + 122] | (sfBuffer[2 * i + 123] << 8));
    }
    HAL_ADC_Start_DMA(&hadc3, (uint32_t *) ADC_BUF, 2);
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *) (ADC_BUF + 2), 2);
    HAL_TIM_Base_Start_IT(&htim5);
    while (1) {

    }
}

extern "C" void OnTimerCallBack(uint8_t* CNT)
{
    if (!mi.isCalibrating) {
        *CNT++;
        mi.ScanAndUpdate();
        mi.PostProcess();
        if (*CNT == 8)
        {
            USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS,
                                       mi.GetHidReportBuffer(1),
                                       MI::KEY_REPORT_SIZE);
            *CNT = 0;
        }
    } else {
        mi.Calibrate(mi.ADC_CONFIG);
    }
}

extern "C"
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef* hspi)
{
    mi.isRgbTxBusy = false;
}
