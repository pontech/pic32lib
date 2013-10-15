#ifndef STEPANDDIRECTION2_H
#define STEPANDDIRECTION2_H

#include <WProgram.h>
#include "Core.h"
#include "TokenParser.h"

typedef bool us1;	// for some reason typedef for bool or boolean do not work

class StepAndDirection2 {
public:
    typedef struct {
        us1 direction;
        int steps;
        int skip;
        int currentSkip;
    } Vector;

    StepAndDirection2() {
        interruptPeriod = (uint32_t)(CORE_TICK_RATE / 1000 * 1000 / 2); // 1ms
    }

    uint32_t interrupt(uint32_t currentTime) {
        if(vector.steps > 0) {
            if(vector.currentSkip > 0) {
                vector.currentSkip--;
            }
            else {
                vector.currentSkip = vector.skip;

                // step
                us1 temp = digitalRead(c1p0);
                if(temp) {
                    vector.steps--;
                }
                digitalWrite(c1p0, !temp);
            }
        }

        return (currentTime + interruptPeriod);
    }

    void command(TokenParser &parser)
    {
        if(parser.compare("stp")) {
            parser.nextToken();

            if(parser.compare("vector")) {
                vector.direction = 0;
                vector.currentSkip = 0;

                parser.nextToken();
                vector.steps = parser.toVariant().toInt();

                parser.nextToken();
                vector.skip = parser.toVariant().toInt();

                Serial.println("OK");
            }
        }
    }

private:
    Variant timeBase;
    uint32_t interruptPeriod;
    Vector vector;
};

#endif // STEPANDDIRECTION2_H
