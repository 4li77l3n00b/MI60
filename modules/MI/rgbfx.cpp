//
// Created by crazy on 2023/7/16.
//

#include "rgbfx.h"
static void nop_() {}

const uint8_t fxKeyMap[5][14] = {
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,
        14,15,16,17,18,19,20,21,22,23,24,25,26,27,
        28,29,30,31,32,33,34,35,36,37,38,39,39,40,
        41,41,42,43,44,45,46,47,48,49,50,51,51,52,
        53,54,55,55,56,57,58,59,60,60,61,62,63,64

};

uint16_t frameCnt[5][14] = {0};

void (*fxArray[4])() = {nop_, fxRainbow,fxSplash,fxRan};

void bgFrame() {
    for (int i = 0; i < 61; i++)
        mi.SetRgbBufferByID(i, mi.RGBMap[mi.isFnPressed()][i], mi.RGBFXArgs[1]);
}

void fxMask() {
    bgFrame();
    fxArray[mi.RGBFXArgs[0]]();
    if (mi.isCapsLocked) mi.SetRgbBufferByID(28, {255,255,255});
    if (mi.isScrollLocked) mi.SetRgbBufferByID(27, {255,255,255});
}

void fxRainbow() {
    
}

void fxSplash() {

}

void fxRan() {

}