//#include <WProgram.h>
class DetectEdge {
public:
    DetectEdge(uint8_t pin = 0, bool activeLow = true, uint16_t deBounce_ms = 0)
    {
        pPin = pin;
        pActiveLow = activeLow;
        pPreviousState = activeLow;
        pPreviousStateBounce = activeLow;
        pRisingFlag = false;
        pFallingFlag = false;

        pTime = 0;
        pPreviousTime = 0;
        pDebounceTime_ms = deBounce_ms;
        pRisingWait = false;
        pFallingWait = false;
    }

    void scan()
    {
        bool input = digitalRead(pPin);
        bool stateChanged = false;

        if(pActiveLow) {
            input ^= 1;
        }

        if(input == 0 && pPreviousState != 0) {
            pFallingWait = true;
            pChangeTime = millis();
            stateChanged = true;
        }
        else if(input == 1 && pPreviousState != 1) {
            pRisingWait = true;
            pChangeTime = millis();
            stateChanged = true;
        }
        if(pRisingWait || pFallingWait )
        {
            if(CompareTime(pChangeTime, millis()) >= pDebounceTime_ms)
            {
                if(pFallingWait && pPreviousStateBounce && input == 0)
                {
                    pRisingFlag = false;
                    pFallingFlag = true;
                    pPreviousStateBounce = false;
                } 
                else if(pRisingWait && !pPreviousStateBounce && input == 1)
                {
                    pRisingFlag = true;
                    pFallingFlag = false;
                    pPreviousStateBounce = true;
                }
                pRisingWait = false;
                pFallingWait = false;
            }
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
        bool state = pRisingFlag;
        pRisingFlag = false;
        return state;
    }

    bool falling()
    {
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
    uint8_t pPin;
    bool pActiveLow;
    bool pPreviousState;
    bool pPreviousStateBounce;
    bool pRisingFlag;
    bool pFallingFlag;
    unsigned long pTime;
    unsigned long pPreviousTime;
    uint32_t pChangeTime;
    uint16_t pDebounceTime_ms;
    bool pRisingWait;
    bool pFallingWait;

    uint32_t CompareTime(uint32_t start, uint32_t stop)
    {
        if(start > stop) //accout for rollover
        {
//            Serial.println((0xFFFFFFFF - start) + stop,DEC);
//            Serial.println("boundry");
            return (0xFFFFFFFF - start) + stop;
        }
        else
        {
//            Serial.println(stop - start,DEC);
            return stop - start;
        }
    }
};
