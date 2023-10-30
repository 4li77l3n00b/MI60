/**
 *                             _ooOoo_
 *                            o8888888o
 *                            88" . "88
 *                            (| -_- |)
 *                            O\  =  /O
 *                         ____/`---'\____
 *                       .'  \\|     |//  `.
 *                      /  \\|||  :  |||//  \
 *                     /  _||||| -:- |||||-  \
 *                     |   | \\\  -  /// |   |
 *                     | \_|  ''\---/''  |   |
 *                     \  .-\__  `-`  ___/-. /
 *                   ___`. .'  /--.--\  `. . __
 *                ."" '<  `.___\_<|>_/___.'  >'"".
 *               | | :  `- \`.;`\ _ /`;.`/ - ` : | |
 *               \  \ `-.   \_ __\ /__ _/   .-` /  /
 *          ======`-.____`-.___\_____/___.-`____.-'======
 *                             `=---='
 *          ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 *                     佛祖保佑        永无BUG
 *            佛曰:
 *                   写字楼里写字间，写字间里程序员；
 *                   程序人员写程序，又拿程序换酒钱。
 *                   酒醒只在网上坐，酒醉还来网下眠；
 *                   酒醉酒醒日复日，网上网下年复年。
 *                   但愿老死电脑间，不愿鞠躬老板前；
 *                   奔驰宝马贵者趣，公交自行程序员。
 *                   别人笑我忒疯癫，我笑自己命太贱；
 *                   不见满街漂亮妹，哪个归得程序员？
*/


#include "common_inc.h"
#include "modules/MI/mi.h"
#include "modules/MI/rgbfx.h"

//program main loop
void KeyboardMain() {
    //Init config from W25Q16 Flash todo:init w25q16
    //keyboard initialization
    mi.InitAndIndex();
    mi.CopyKeyArgs();
    mi.CopyConfKeyMap();
    mi.CopyKeyMap();
    mi.CopyRGBMap();
    mi.CopyRGBFXArgs();
    //ADC start
    HAL_ADC_Start_DMA(&hadc3, (uint32_t *) ADC_BUF, 2);
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *) (ADC_BUF + 2), 2);
    //8000HZ TIM5
    HAL_TIM_Base_Start_IT(&htim5);
    //RGB Frame Loop
    while (1) {
        HAL_Delay(10);
        fxMask();
        mi.SyncLights();
    }
}
//TIM5 interrupt callback
extern "C" void OnTimerCallBack()
{
    if (!mi.isCalibrating) {
        //HID keyboard polling
        mi.ScanAndUpdate();
        mi.PostProcess();
        USBD_CUSTOM_HID_SendReport(&hUsbDeviceHS,
                                   mi.GetHidReportBuffer(1),
                                   MI::KEY_REPORT_SIZE);
    } else {
        //HID calibration
        mi.Calibrate(mi.CalibKeyID);
    }
}
/*
 * function:HAL_SPI_TxCpltCallback
 * argv: hspi(struct)
 * description: RGB SPI status reset
 */
extern "C"
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef* hspi)
{
    if (hspi == &hspi2)
        mi.isRgbTxBusy = false;
}
/*****
 *
 * @param _op0
 * @param _op1 dest1
 * @param _op2 dest2
 */
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
//todo
extern "C"
void CapsLock(bool state) {
    mi.isCapsLocked = state;
}

extern "C"
void ScrollLock(bool state) {
    mi.isScrollLocked = state;
}
//driver interaction
extern "C"
void ReturnState(uint8_t _state) {
    mi.hidBuffer[mi.KEY_REPORT_SIZE + 1] = _state;
    USBD_CUSTOM_HID_SendReport(&hUsbDeviceHS,
                               mi.GetHidReportBuffer(2),
                               mi.RAW_REPORT_SIZE);
}
//sync ram to flash
extern "C" //0
void SyncAll() {
    mi.SyncCalibration();
    mi.SyncKeyArgs();
    mi.SyncConfKeyMap();
    mi.SyncKeyMap();
    mi.SyncRGBMap();
    mi.SyncRGBFXArgs();
}

extern "C" //1
void StartCalibration(uint8_t _keyID) {
    mi.CalibKeyID = _keyID;
    mi.isCalibrating = true;
}

extern "C" //2
void ChangeKeyArg(const uint8_t* _buf) {
    uint8_t _keyID = _buf[0];
    mi.miConfig[_keyID].KeyMode = (_buf[1] == 1)? MI::WOOT : MI::APEX;
    uint8tofloat((_buf+2), &(mi.miConfig[_keyID].ActPoint));
    uint8tofloat((_buf+6), &(mi.miConfig[_keyID].ActPoint2));
    uint8tofloat((_buf+10), &(mi.miConfig[_keyID].LiftTravel));
    uint8tofloat((_buf+14), &(mi.miConfig[_keyID].PressTravel));
    mi.Convert(&mi.miConfig[_keyID], &mi.ADC_CONFIG[_keyID]);
    ReturnState(0xFF);
}

extern "C" //3
void ChangeConfKeyMap(const uint8_t* _buf) {
    memcpy(mi.ConflictingKeyMap, _buf, 8);
    ReturnState(0xFF);
}

extern "C" //4
void ChangeKeyMap(const uint8_t* _buf) {
    uint8_t _layer = _buf[0];
    uint8_t _keyID = _buf[1];
    uint8to16(&_buf[2], &_buf[3], reinterpret_cast<uint16_t *>(&mi.keyMap[_layer][_keyID]));
    ReturnState(0xFF);
}

extern "C" //5
void ChangeRGBMap(const uint8_t* _buf) {
    uint8_t _layer = _buf[0];
    uint8_t _keyID = _buf[1];
    memcpy(&mi.RGBMap[_layer][_keyID], _buf + 2, 3);
    ReturnState(0xFF);
}

extern "C" //6
void ChangeRGBFXArg(const uint8_t* _buf) {
    memcpy(mi.RGBFXArgs, _buf + 1, 32);
    ReturnState(0xFF);
}




