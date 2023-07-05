#include "common_inc.h"
#include "MI/mi.h"
#include "spi_flash.h"
MI mi;

void KeyboardMain() {
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
