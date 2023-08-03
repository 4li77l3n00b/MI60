//
// Created by crazy on 2023/7/16.
//

#ifndef MI60_RGBFX_H
#define MI60_RGBFX_H

#endif //MI60_RGBFX_H

#include "common_inc.h"
#include "mi.h"

//RGBFXARGS : 0=fx_id, 1=bg_bri, 2-31=args

extern void (*fxArray[4])();

void bgFrame();
void fxMask();

void fxRainbow();
void fxSplash();
void fxRan();
