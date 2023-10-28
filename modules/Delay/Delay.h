
#pragma once
#include "common_inc.h"

void DelayUs(uint32_t _quarterus)
{
    for (int i = 0; i < _quarterus; i++)
        for (int j = 0; j < 13; j++)  // ToDo: tune this for different chips
            __NOP();
}

