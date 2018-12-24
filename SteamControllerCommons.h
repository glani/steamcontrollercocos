//
// Created by denis on 12/20/18.
//

#ifndef CCWARIO_STEAMCONTROLLERCOMMONS_H
#define CCWARIO_STEAMCONTROLLERCOMMONS_H

#include "base/ccMacros.h"

NS_CC_BEGIN
    typedef struct _SteamControllerValues {
        const static uint8_t YES = (uint8_t) 1;
        const static uint8_t NO = (uint8_t) 0;

        uint8_t ButtonA; // 0
        uint8_t ButtonX; // 1
        uint8_t ButtonB; // 2
        uint8_t ButtonY; // 3
        uint8_t ButtonRT; // 4
        uint8_t ButtonLT; // 5
        uint8_t ButtonRS; // 6
        uint8_t ButtonLS; // 7
        uint8_t ButtonDpadUp; // 8
        uint8_t ButtonDpadRight; // 9
        uint8_t ButtonDpadLeft; // 10
        uint8_t ButtonDpadDown; // 11
        uint8_t ButtonPrev; // 12
        uint8_t ButtonHome; // 13
        uint8_t ButtonNext; // 14
        uint8_t ButtonRGrip; // 15
        uint8_t ButtonLGrip; // 16
        uint8_t ButtonStick; // 17
        uint8_t ButtonRPad; // 18
        uint8_t TouchLFinger; // 19
        uint8_t TouchRFinger; // 20
        uint8_t Stick; // 21


        float LeftTrigger;
        float RightTrigger;

        float LeftPadX;
        float LeftPadY;

        float StickX;
        float StickY;
        float RightPadX;
        float RightPadY;
    } SteamControllerValues;

NS_CC_END

#endif //CCWARIO_STEAMCONTROLLERCOMMONS_H
