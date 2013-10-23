#ifndef STEPANDDIRECTION_H
#define STEPANDDIRECTION_H

#include <WProgram.h>
#include "Core.h"
#include "TokenParser.h"
#include "CircleBuffer.h"
#include "Vector.h"

#define array_base_type 0
typedef bool us1;	// for some reason typedef for bool or boolean do not work

// todo: control direction polarity when creating vectors
class StepAndDirection {
public:
    StepAndDirection(us8 motor, us8 pin_step, us8 pin_direction, us8 pin_enable, us8 pin_sleep) {
        StepAndDirection::motor = motor;
        StepAndDirection::pin_step = pin_step;
        StepAndDirection::pin_direction = pin_direction;
        StepAndDirection::pin_enable = pin_enable;
        StepAndDirection::pin_sleep = pin_sleep;

        pinMode(pin_step, OUTPUT);
        pinMode(pin_direction, OUTPUT);
        pinMode(pin_enable, OUTPUT);
        pinMode(pin_sleep, OUTPUT);
        digitalWrite(pin_enable, HIGH); // 0 = enabled
        digitalWrite(pin_sleep, HIGH);  // 1 = enabled

        stepPort = (p32_ioport *)portRegisters(digitalPinToPort(pin_step));
        stepBit = digitalPinToBitMask(pin_step);

        directionPort = (p32_ioport *)portRegisters(digitalPinToPort(pin_direction));
        directionBit = digitalPinToBitMask(pin_direction);

        interruptPeriod = setTimeBase(Variant(250, -6));
        currentPosition = 0;
        currentSkip = 0;
        stepsPerUnit = 1;
        halt();

        // sigmoid defaults
        setSigmoid(Variant(1, 3), Variant(1, 4), Variant(25, 1), 3);
    }

    StepAndDirection(us8 motor, us8 card) {
        *this = StepAndDirection(motor, KardIO[card][0], KardIO[card][1], KardIO[card][2], KardIO[card][3]);
    }

    // todo: verify skip starts at 1
    void sharedInterrupt(Variant timebase) {
        if(vector.steps == 0 && currentSkip <= 0 && flag) {
            if(!buffer.isEmpty()) {
                vector = buffer.pop();
                if(vector.steps > 0) {
//                    directionPort->lat.set = directionBit;
                    digitalWrite(pin_direction, HIGH);
                }
                else {
//                    directionPort->lat.clr = directionBit;
                    digitalWrite(pin_direction, LOW);
                }
            }
            else {
                flag = false;
            }
        }

        if(vector.steps != 0 || currentSkip > 0) {
            if(currentSkip > 0) {
                currentSkip--;
            }
            else {
                currentSkip = (vector.time / timebase).toInt();
                step();
            }
        }
    }

    void unsharedInterrupt() {
        if(vector.steps == 0 && flag) {
            if(!buffer.isEmpty()) {
                vector = buffer.pop();
                bool ok;
                uint32_t temp = setTimeBase(vector.time, &ok);
                if(ok) {
                    interruptPeriod = temp;
                    if(vector.steps > 0) {
//                        directionPort->lat.set = directionBit;
                        digitalWrite(pin_direction, HIGH);
                    }
                    else {
//                        directionPort->lat.clr = directionBit;
                        digitalWrite(pin_direction, LOW);
                    }
                }
                else {
                    Serial.println("Timebase Error");
                    Serial.println(vector.steps);
                    Serial.println(vector.time.toString());
                    vector.steps = 0;
                    vector.time = Variant();
                    flag = false;
                }
                return;
            }
            else {
                flag = false;
            }
        }

        if(vector.steps != 0) {
            step();
        }
    }

    // Quick240 setTimeBase Profiling
    // ~285us execution with const Variant declaration, 250, -6
    // ~320us execution with string Variant declaration, "250e-6"
    // ~285us during bounds checking using 2x string declaration's
    // ~700ns during bounds checking using 2x const declaration's
    // ~700ns const declaration, and *= operator
    // Changed bounds checking to const declaration
    // ~700ns execution with const, Variant(250, -6)
    // ~700ns consumed by 2x digtalWrite's
    uint32_t setTimeBase(Variant milliseconds, bool *ok = 0) {
        if(milliseconds >= Variant(5, -6) && milliseconds <= Variant(1, 0)) {
            Variant var(1, 6);
            var *= milliseconds;

            if(ok) {
                *ok = true;
            }
            return (uint32_t)(CORE_TICK_RATE / 1000 * var.toInt());
        }

        if(ok) {
            *ok = false;
        }

        return 0;
    }

    // stp0 test 5e3 30e3 300 3 3200
    // stp0 test 20e3 40e3 1000 4 32000 // 10 rps w/16x
    // stp0 test 18e3 28e3 1000 3.5 32000 // 17.5 rps w/8x
    void modifiedSigmoid(Variant begin, Variant end, Variant accelSteps, float coefficient, s32 steps) {
        int points = 10;

        Variant beginPeriod(1, 0);
        beginPeriod /= begin;

        Variant endPeriod(1, 0);
        endPeriod /= end;

        if(steps < 0) {
            accelSteps *= Variant(-1, 0);
        }

//        Serial.println("Begin: " + beginPeriod.toString() + " sec");
//        Serial.println("End  : " + endPeriod.toString() + " sec");

        // =(1 / (1 + (coefficient ^ (-point + 5))))

        Vector vectors[points + 1];
        Variant prev;
        for(int i = 0; i <= points; i++) {
            int exp = -i + 5;
            Variant base = (float)(1 + pow(coefficient, exp));
            Variant value(1, 0);
            value /= base;

//            Serial.print(i);
//            Serial.print(": ");
//            Serial.print(base.toString());
//            Serial.print("/");
//            Serial.print(value.toString());
//            Serial.print(" - ");

            if(i == 0) {
                vectors[i].steps = 0;
            }
            else {
                vectors[i].steps = ((value - prev) * accelSteps).toInt();
                vectors[i].time = ((endPeriod * value) + (beginPeriod * (Variant(1, 0) - value)));
            }

//            Serial.print(vectors[i].steps);
//            Serial.print(", ");
//            Serial.print(vectors[i].time.toString());
//            Serial.println(" ");

            prev = value;
        }

        int totalSteps = 0;
        for(int i = 1; i <= points; i++) {
            totalSteps += vectors[i].steps;
            buffer.push(vectors[i]);
        }

        Vector flatVector(steps - (totalSteps * 2), vectors[10].time); //endPeriod);

//        Serial.print(flatVector.steps);
//        Serial.print(", ");
//        Serial.println(flatVector.time.toString());

        buffer.push(flatVector);

        for(int i = points; i > 0; i--) {
            buffer.push(vectors[i]);
        }

//        Serial.print("TotalSteps: ");
//        Serial.println(totalSteps);
//        Serial.println(vectors[points].time.toString());
    }

    void start() {
        vector.steps = 0;
        vector.time = Variant();
        flag = true;
    }

    void pause() {
        flag = false;
    }

    void halt() {
        flag = false;
    }

    void setStepsPerUnit(us16 steps) {
        if(steps > 0) {
            stepsPerUnit = steps;
        }
    }

    void chooseBestMove(s32 units) {
        if(units == 0) {
            return;
        }

        if(abs(units) >= (sigSteps.toInt() * 3)) {
            modifiedSigmoid(sigLow, sigHigh, sigSteps, sigCoefficient, units);
        }
        else {
            Variant period(1, 0);
            period /= sigLow;
            buffer.push(Vector(units, period));
        }
        start();
    }

    // relative move
    void move(TokenParser &parser, s32 units) {
        units *= stepsPerUnit;

        parser.print(String(motor, DEC));
        parser.print(" move: ");
        parser.println(String(units, DEC));

        chooseBestMove(units);
    }

    // absolute move
    void moveTo(TokenParser &parser, s32 units) {
        units *= stepsPerUnit;
        units -= currentPosition;

        parser.print(String(motor, DEC));
        parser.print(" moveTo: ");
        parser.println(String(units, DEC));

        chooseBestMove(units);
    }

    bool isBusy() {
        return (!buffer.isEmpty() && flag);
    }

    void setCurrentPosition(s32 position) {
        currentPosition = position;
    }

    void setMaxSpeed(Variant maxSpeed) {

    }

    void setAcceleration(Variant acceleration) {

    }

    void setEnabled(bool enabled) {
        digitalWrite(pin_enable, !enabled);
    }

    void setSigmoid(Variant begin, Variant end, Variant accelSteps, float coefficient)
    {
        sigLow = begin;
        sigHigh = end;
        sigSteps = accelSteps;
        sigCoefficient = coefficient;
    }

    void command(TokenParser &parser) {
        if(parser.startsWith("stp?")) {
            parser.save();
            parser.advanceTail(3);

            if(motor != parser.toVariant().toInt()) {
                parser.restore();
                return;
            }

            parser.nextToken();
            if(parser.compare("pairs")) {
                for(int i = 0; i < buffer.size; i++) {
                    if(!parser.nextToken()) {
                        break;
                    }
                    String token = parser.toString();
                    parser.println(String(i, DEC) + ": " + token);

                    int index = token.indexOf(",");

                    Vector temp;
                    temp.steps = token.substring(0, index).toInt();
                    temp.time = Variant::fromString(token.substring(++index));
                    buffer.push(temp);
                }
                parser.println("OK Pairs");
                start();
            }
            else if(parser.compare("enable")) {
                parser.nextToken();
                setEnabled(parser.toVariant().toBool());
            }
            else if(parser.compare("base")) {
                parser.nextToken();
                setTimeBase(parser.toVariant());
            }
            else if(parser.compare("test")) {
                parser.nextToken();
                Variant begin = parser.toVariant();

                parser.nextToken();
                Variant end = parser.toVariant();

                parser.nextToken();
                Variant accelSteps = parser.toVariant();

                parser.nextToken();
                Variant c = parser.toVariant();

                parser.nextToken();
                Variant steps = parser.toVariant();

                modifiedSigmoid(begin, end, accelSteps, c.toFloat(), steps.toInt());
            }
            else if(parser.compare("scp")) {
                parser.nextToken();
                setCurrentPosition(parser.toVariant().toInt());
            }
            else if(parser.compare("rcp")) {
                parser.println(String(currentPosition, DEC) + " steps");
                parser.println(String((currentPosition / stepsPerUnit), DEC) + " units");
            }
            else if(parser.compare("move")) {
                parser.nextToken();
                move(parser, parser.toVariant().toInt());
            }
            else if(parser.compare("moveto")) {
                parser.nextToken();
                moveTo(parser, parser.toVariant().toInt());
            }
            else if(parser.compare("units")) {
                parser.nextToken();
                setStepsPerUnit(parser.toVariant().toInt());
                Serial.println(String(stepsPerUnit, DEC));
            }
            else if(parser.compare("setsig")) {
                parser.nextToken();
                sigLow = parser.toVariant();

                parser.nextToken();
                sigHigh = parser.toVariant();

                parser.nextToken();
                sigSteps = parser.toVariant();

                parser.nextToken();
                sigCoefficient = parser.toVariant().toFloat();

                Serial.println("OK");
            }
        }
    }

    uint32_t interruptPeriod;

private:
    inline void step() {
//        stepPort->lat.set = stepBit;
        digitalWrite(pin_step, HIGH);
        if(vector.steps > 0 ) {
            currentPosition++;
            vector.steps--;
        }
        else {
            currentPosition--;
            vector.steps++;
        }
        digitalWrite(pin_step, LOW);
//        stepPort->lat.clr = stepBit;
    }

    us8 motor;
    us8 pin_step;
    us8 pin_direction;
    us8 pin_enable;
    us8 pin_sleep;
    s32 currentPosition;
    us16 stepsPerUnit;

    // step pin
    p32_ioport *stepPort;
    unsigned int stepBit;

    // direction pin
    p32_ioport *directionPort;
    unsigned int directionBit;

    // sigmoid related
    Variant sigLow;
    Variant sigHigh;
    Variant sigSteps;
    float sigCoefficient;

    bool flag;
    Vector vector;
    us32 currentSkip;
    CircleBuffer buffer;
};

#endif // STEPANDDIRECTION_H
