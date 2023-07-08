#include "common_inc.h"
#include "MI/mi.h"
MI mi;

void KeyboardMain() {
    //Init config from W25Q16 Flash
    W25qxx_Init();
    W25qxx_ReadSector(sfBuffer, 0, 0, 248);
    mi.KeyAddr = sfBuffer[0];
    mi.KeyMapAddr = sfBuffer[1];
    mi.RGBMapAddr = sfBuffer[2];
    mi.RGBFXAddr = sfBuffer[3];
    //Fetch calibration result
    for (int i = 0; i < 61; i++) {
        mi.ADC_CONFIG[i].ZERO_POINT = (uint16_t)(sfBuffer[4*i+4]) | ((uint16_t)(sfBuffer[4*i+5]) << 8);
        mi.ADC_CONFIG[i].MAX_POINT = (uint16_t)(sfBuffer[4*i+6]) | ((uint16_t)(sfBuffer[4*i+7]) << 8);
    }
    //Fetch key config
    W25qxx_ReadSector(sfBuffer, mi.KeyAddr, 0, 427);
    for (int i = 0; i < 61; ++i) {
        mi.ADC_CONFIG[i].TRG_MODE = (sfBuffer[i*7] == 0 ? MI::APEX : MI::WOOT);
        mi.ADC_CONFIG[i].ACT_POINT = (uint16_t)(sfBuffer[i*7+1]) | ((uint16_t)(sfBuffer[i*7+2]) << 8);
        mi.ADC_CONFIG[i].LIFT_THRESHOLD = (uint16_t)(sfBuffer[i*7+3]) | ((uint16_t)(sfBuffer[i*7+4]) << 8);
        mi.ADC_CONFIG[i].PRESS_THRESHOLD = (uint16_t)(sfBuffer[i*7+5]) | ((uint16_t)(sfBuffer[i*7+6]) << 8);
    }
    //Fetch key map
    W25qxx_ReadSector(sfBuffer, mi.KeyMapAddr, 0, 244);
    for (int i = 0; i < 61; ++i) {
        mi.keyMap[0][i] = static_cast<MI::KeyCode_t>((uint16_t)(sfBuffer[2 * i]) | (((uint16_t)sfBuffer[2 * i + 1]) << 8));
        mi.keyMap[1][i] = static_cast<MI::KeyCode_t>((uint16_t)(sfBuffer[2 * i + 122]) | ((uint16_t)(sfBuffer[2 * i + 123]) << 8));
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
