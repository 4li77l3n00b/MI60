#ifndef MI_H
#define MI_H

#include "common_inc.h"

void DelayUs(uint32_t _quarterus);

class MI
{
public:
    static const uint8_t KEYNUM = 64;
    static const uint8_t LED_NUMBER = 65;
    static const uint16_t KEY_REPORT_SIZE = 1 + 16;
    static const uint16_t RAW_REPORT_SIZE = 1 + 64;
    static const uint16_t HID_REPORT_SIZE = KEY_REPORT_SIZE + RAW_REPORT_SIZE;

    enum KeyCode_t : int16_t
    {
        /*------------------------- HID report data -------------------------*/
        LEFT_CTRL = -8,LEFT_SHIFT = -7,LEFT_ALT = -6,LEFT_GUI = -5,
        RIGHT_CTRL = -4,RIGHT_SHIFT = -3,RIGHT_ALT = -2,RIGHT_GUI = -1,

        RESERVED = 0,ERROR_ROLL_OVER,POST_FAIL,ERROR_UNDEFINED,
        A,B,C,D,E,F,G,H,I,J,K,L,M,
        N,O,P,Q,R,S,T,U,V,W,X,Y,Z,
        NUM_1/*1!*/,NUM_2/*2@*/,NUM_3/*3#*/,NUM_4/*4$*/,NUM_5/*5%*/,
        NUM_6/*6^*/,NUM_7/*7&*/,NUM_8/*8**/,NUM_9/*9(*/,NUM_0/*0)*/,
        ENTER,ESC,BACKSPACE,TAB,SPACE,
        MINUS/*-_*/,EQUAL/*=+*/,LEFT_U_BRACE/*[{*/,RIGHT_U_BRACE/*]}*/,
        BACKSLASH/*\|*/,NONE_US/**/,SEMI_COLON/*;:*/,QUOTE/*'"*/,
        GRAVE_ACCENT/*`~*/,COMMA/*,<*/,PERIOD/*.>*/,SLASH/*/?*/,
        CAP_LOCK,F1,F2,F3,F4,F5,F6,F7,F8,F9,F10,F11,F12,
        PRINT,SCROLL_LOCK,PAUSE,INSERT,HOME,PAGE_UP,DELETE,END,PAGE_DOWN,
        RIGHT_ARROW,LEFT_ARROW,DOWN_ARROW,UP_ARROW,PAD_NUM_LOCK,
        PAD_SLASH,PAD_ASTERISK,PAD_MINUS,PAD_PLUS,PAD_ENTER,
        PAD_NUM_1,PAD_NUM_2,PAD_NUM_3,PAD_NUM_4,PAD_NUM_5,
        PAD_NUM_6,PAD_NUM_7,PAD_NUM_8,PAD_NUM_9,PAD_NUM_0,
        PAD_PERIOD , NONUS_BACKSLASH,APPLICATION,POWER,PAD_EQUAL,
        F13,F14,F15,F16,F17,F18,F19,F20,F21,F22,F23,F24, EXECUTE,
        HELP,MENU,SELECT,STOP,AGAIN,UNDO,CUT,COPY,PASTE,FIND,MUTE,VOLUME_UP,VOLUME_DOWN,
        FN = 1000
        /*------------------------- HID report data -------------------------*/
    };
    struct Color_t
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };
    enum SpiWs2812Byte_t : uint8_t
    {
        WS_HIGH = 0xFE,
        WS_LOW = 0xE0
    };
    KeyCode_t keyMap[2][KEYNUM] = {
            {ESC,NUM_1,NUM_2,NUM_3,NUM_4,NUM_5,NUM_6,NUM_7,NUM_8,NUM_9,NUM_0,MINUS,EQUAL,BACKSPACE,
             TAB,Q,W,E,R,T,Y,U,I,O,P,LEFT_U_BRACE,RIGHT_U_BRACE,BACKSLASH,
             CAP_LOCK,A,S,D,F,G,H,J,K,L,SEMI_COLON,QUOTE,ENTER,
             LEFT_SHIFT,Z,X,C,V,B,N,M,COMMA,PERIOD,SLASH,RIGHT_SHIFT,
             LEFT_CTRL,LEFT_GUI,LEFT_ALT,SPACE,RIGHT_ALT,MENU,RIGHT_CTRL,FN},

            {GRAVE_ACCENT,F1,F2,F3,F4,F5,F6,F7,F8,F9,F10,F11,F12,BACKSLASH,
             TAB,Q,W,E,R,T,Y,U,I,O,P,HOME,PAGE_UP,DELETE,
             CAP_LOCK,A,S,D,F,G,H,J,K,L,END,PAGE_DOWN,ENTER,
             LEFT_SHIFT,Z,X,C,V,B,N,M,COMMA,PERIOD,UP_ARROW,RIGHT_SHIFT,
             LEFT_CTRL,LEFT_GUI,LEFT_ALT,SPACE,LEFT_ARROW,DOWN_ARROW,RIGHT_ARROW,FN}
    };

    Color_t RGBMap[3][LED_NUMBER] = {
            {{159,255,255},{159,255,255},{159,255,255},{159,255,255},
            {159,255,255},{159,255,255},{159,255,255},{159,255,255},
            {159,255,255},{159,255,255},{159,255,255},{159,255,255},
            {159,255,255},{159,255,255},{159,255,255},{159,255,255},
            {159,255,255},{159,255,255},{159,255,255},{159,255,255},
            {159,255,255},{159,255,255},{159,255,255},{159,255,255},
            {159,255,255},{159,255,255},{159,255,255},{159,255,255},
            {159,255,255},{159,255,255},{159,255,255},{159,255,255},
            {159,255,255},{159,255,255},{159,255,255},{159,255,255},
            {159,255,255},{159,255,255},{159,255,255},{159,255,255},
            {159,255,255},{159,255,255},{159,255,255},{159,255,255},
            {159,255,255},{159,255,255},{159,255,255},{159,255,255},
            {159,255,255},{159,255,255},{159,255,255},{159,255,255},
            {159,255,255},{159,255,255},{159,255,255},{159,255,255},
            {159,255,255},{159,255,255},{159,255,255},{159,255,255},
            {159,255,255},{159,255,255},{159,255,255},{159,255,255},
            {159,255,255}},
            {{159,255,255},{159,255,255},{159,255,255},{159,255,255},
                    {159,255,255},{159,255,255},{159,255,255},{159,255,255},
                    {159,255,255},{159,255,255},{159,255,255},{159,255,255},
                    {159,255,255},{159,255,255},{159,255,255},{159,255,255},
                    {159,255,255},{159,255,255},{159,255,255},{159,255,255},
                    {159,255,255},{159,255,255},{159,255,255},{159,255,255},
                    {159,255,255},{255,0,0},{255,0,0},{159,255,255},
                    {159,255,255},{159,255,255},{159,255,255},{159,255,255},
                    {159,255,255},{159,255,255},{159,255,255},{159,255,255},
                    {159,255,255},{159,255,255},{255,0,0},{255,0,0},
                    {159,255,255},{159,255,255},{159,255,255},{159,255,255},
                    {159,255,255},{159,255,255},{159,255,255},{159,255,255},
                    {159,255,255},{159,255,255},{159,255,255},{255,0,0},
                    {159,255,255},{159,255,255},{159,255,255},{159,255,255},
                    {159,255,255},{159,255,255},{159,255,255},{159,255,255},
                    {159,255,255},{255,0,0},{255,0,0},{255,0,0},
                    {159,255,255}}
    };
    volatile bool isRgbTxBusy;
    bool isCalibrating = false;
    bool isCapsLocked;
    bool isScrollLocked;
    bool isFnPresssed;

    enum Mode
    {
        WOOT,
        APEX
    };

    struct TravelConfig
    {
        Mode KeyMode;
        float_t ActPoint;
        float_t LiftTravel;
        float_t PressTravel;
    };

    struct ScanConfig
    {
        Mode TRG_MODE;
        uint16_t MAX_POINT;
        uint16_t ZERO_POINT;
        uint16_t ACT_POINT;
        uint16_t LIFT_THRESHOLD;
        uint16_t PRESS_THRESHOLD;
    };

    static void Convert(TravelConfig* _config, ScanConfig* _SCAN_CONFIG)
    {
        _SCAN_CONFIG->TRG_MODE = _config->KeyMode;
        float_t Scaler = (_SCAN_CONFIG->MAX_POINT - _SCAN_CONFIG->ZERO_POINT) / 4;
        _SCAN_CONFIG->ACT_POINT = _SCAN_CONFIG->MAX_POINT - _config->ActPoint * Scaler;
        _SCAN_CONFIG->LIFT_THRESHOLD = _config->LiftTravel * Scaler;
        _SCAN_CONFIG->PRESS_THRESHOLD = _config->PressTravel * Scaler;
    }

    void InitAndIndex();
    uint8_t CalibKeyID;
    void Calibrate(uint8_t _keyID);
    void SyncCalibration();

    void CopyKeyArgs();
    void SyncKeyArgs();

    void CopyKeyMap();
    void SyncKeyMap();

    uint16_t ConflictingKeyMap[4][2] = {{63,63},{63,63},{63,63},{63,63}};
    void CopyConfKeyMap();
    void SyncConfKeyMap();

    void CopyRGBMap();
    void SyncRGBMap();

    void CopyRGBFXArgs();
    void SyncRGBFXArgs();

    void ScanAndUpdate();
    void PostProcess();
    void SyncLights();
    uint8_t* GetHidReportBuffer(uint8_t _reportId);

    void SetRgbBufferByID(uint8_t _keyId, Color_t _color, float _brightness = 1);
    uint8_t RGBFXArgs[16];
    void Press(KeyCode_t _key);
    void Release(KeyCode_t _key);
    TravelConfig miConfig[KEYNUM]{};
    ScanConfig ADC_CONFIG[KEYNUM]{};
    uint8_t hidBuffer[HID_REPORT_SIZE]{};

private:
    bool PENDING[KEYNUM]{};
    bool TRIGGERED[KEYNUM]{};
    bool GOING[KEYNUM]{};
    uint16_t scanBuffer[KEYNUM]{};
    bool keyBuffer[KEYNUM]{};
    uint8_t rgbBuffer[LED_NUMBER][3][8]{};
    uint8_t wsCommit[64] = {0};
    uint8_t brightnessPreDiv = 2;
};

#endif