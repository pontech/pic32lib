#ifndef DETECTEDGE_H
#define DETECTEDGE_H

#include <WProgram.h>
#include "Core.h"

class DetectEdge {
public:
    DetectEdge(us8 pin = 0, bool activeLow = true)
    {
        pPin = pin;
        pActiveLow = activeLow;
        pPreviousState = activeLow;
        pRisingFlag = false;
        pFallingFlag = false;

        pTime = 0;
        pPreviousTime = 0;
    }

    void scan()
    {
        bool input = digitalRead(pPin);
        bool stateChanged = false;

        if(pActiveLow) {
            input ^= 1;
        }

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

    void clear()
    {
        scan();
        pRisingFlag = false;
        pFallingFlag = false;
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
        return pActiveLow ? !digitalRead(pPin) : digitalRead(pPin);
    }

    bool isFallen()
    {
        return pActiveLow ? digitalRead(pPin) : !digitalRead(pPin);
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
    bool pActiveLow;
    bool pPreviousState;
    bool pRisingFlag;
    bool pFallingFlag;
    unsigned long pTime;
    unsigned long pPreviousTime;
};

#endif // DETECTEDGE_H
