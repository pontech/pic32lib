#ifndef DETECTEDGE_H
#define DETECTEDGE_H

#include <WProgram.h>
#include "Core.h"

class DetectEdge {
public:
    DetectEdge(us8 newPin = 0, bool defaultPinState = false)
    {
        pPin = newPin;
        pPreviousState = defaultPinState;
        pRisingFlag = false;
        pFallingFlag = false;

        pTime = 0;
        pPreviousTime = 0;
    }

    void setup(us8 newPin, bool defaultPinState = false)
    {
        pPin = newPin;
        pPreviousState = defaultPinState;
    }

    void scan()
    {
        bool input = digitalRead(pPin);
        bool stateChanged = false;

        if(input == 0 && pPreviousState != 0) {
            pRisingFlag = false;
            pFallingFlag = true;
            stateChanged = true;
        }
        else if(input == 1 && pPreviousState != 1) {
            pRisingFlag = true;
            pFallingFlag = false;
            stateChanged = true;
        }

        if(stateChanged) {
            pPreviousState = input;
            pPreviousTime = pTime;
            pTime = millis();
        }
    }

    bool rising()
    {
        scan();
        bool state = pRisingFlag;
        pRisingFlag = false;
        return state;
    }

    bool falling()
    {
        scan();
        bool state = pFallingFlag;
        pFallingFlag = false;
        return state;
    }

    bool isRisen()
    {
        return digitalRead(pPin);
    }

    bool isFallen()
    {
        return !digitalRead(pPin);
    }

    void setRising()
    {
        pRisingFlag = true;
        pFallingFlag = false;
    }

    void setFalling()
    {
        pRisingFlag = false;
        pFallingFlag = true;
    }

    unsigned long time()
    {
        return pTime;
    }

    unsigned long previousTime()
    {
        return pPreviousTime;
    }

private:
    us8 pPin;
    bool pPreviousState;
    bool pRisingFlag;
    bool pFallingFlag;
    unsigned long pTime;
    unsigned long pPreviousTime;
};

#endif // DETECTEDGE_H
