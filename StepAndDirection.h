#ifndef STEPANDDIRECTION_H
#define STEPANDDIRECTION_H

#include <WProgram.h>
#include "Core.h"
#include "TokenParser.h"
#include "CircleBuffer.h"
#include "Vector.h"

#define fast_io
#define debug_sigmoid
#define array_base_type 0
typedef bool us1;	// for some reason typedef for bool or boolean do not work

class StepConfig {
    friend class StepAndDirection;

public:
    StepConfig() {
        currentPosition = 0;
        conversion_mx = 1;
        conversion_b = 0;
        conversion_p = 0;
        limit_enabled = false;

    }

    s32 getCurrentPosition() {
       return currentPosition;
    }

    void setCurrentPosition(s32 position) {
        currentPosition = position;
    }

    void setConversion(Variant mx, Variant b, us8 precision = 0) {
         if(mx != 0) {
             conversion_mx = mx;
             conversion_b = b;
             conversion_p = pow(10, precision);
         }
     }

    void setLimits(Variant min, Variant max) {
        if(min != 0 && max != 0) {
            limit_min = min;
            limit_max = max;
            limit_enabled = true;
        }
        else {
            limit_enabled = false;
        }
    }

    Variant unitConversion(Variant units) {
        if(limit_enabled) {
            if(units < limit_min || units > limit_max) {
                return 0;
            }
        }
//        Serial.println("In:" + units.toString());
//        units *= conversion_p;
        units *= conversion_mx;
        units += conversion_b;
        return units;
    }

private:
    s32 currentPosition;

    // conversion variables
    Variant conversion_mx;
    Variant conversion_b;
    Variant conversion_p;

    Variant speed_max;

    bool limit_enabled;
    Variant limit_min;
    Variant limit_max;
};

// todo: control direction polarity when creating vectors
class StepAndDirection {
public:
    StepAndDirection(us8 motor, us8 pin_step, us8 pin_direction, us8 pin_enable, us8 pin_sleep, us8 pin_ms2) {
        StepAndDirection::motor = motor;
#ifndef fast_io
        StepAndDirection::pin_step = pin_step;
        StepAndDirection::pin_direction = pin_direction;
#endif
        StepAndDirection::pin_enable = pin_enable;
        StepAndDirection::pin_sleep = pin_sleep;

        pinMode(pin_step, OUTPUT);
        pinMode(pin_direction, OUTPUT);
        pinMode(pin_enable, OUTPUT);
        pinMode(pin_sleep, OUTPUT);
        pinMode(pin_ms2, OUTPUT);
        digitalWrite(pin_enable, HIGH); // 0 = enabled
        digitalWrite(pin_sleep, HIGH);  // 1 = enabled
        digitalWrite(pin_ms2, HIGH);  // 0 = full step, 1 = quarter step

#ifdef fast_io
        stepPort = (p32_ioport *)portRegisters(digitalPinToPort(pin_step));
        stepBit = digitalPinToBitMask(pin_step);

        directionPort = (p32_ioport *)portRegisters(digitalPinToPort(pin_direction));
        directionBit = digitalPinToBitMask(pin_direction);

        homeSensorPort = 0;
        homeSensorPolarity = false;
#endif

        defaultConfig = new StepConfig();
        config = defaultConfig;

        interruptPeriod = setTimeBase(Variant(250, -6));

        halt();

        // sigmoid defaults
        setSigmoid(Variant(1, 3), Variant(1, 4), Variant(25, 1), 3);
    }

    StepAndDirection(us8 motor, us8 card) {
        *this = StepAndDirection(motor, KardIO[card][0], KardIO[card][1], KardIO[card][2], KardIO[card][3], KardIO[card][4]);
    }

    // todo: verify skip starts at 1
    void sharedInterrupt(Variant timebase) {
        static us32 skip = 0;
        static us32 currentSkip = 0;

        if(running) {
            if(vector.steps == 0 && currentSkip <= 0) {
                if(!buffer.isEmpty()) {
                    vector = buffer.pop();

                    skip = (vector.time / timebase).toInt();
                    currentSkip = skip;

                    if(vector.steps > 0) {
#ifdef fast_io
                        directionPort->lat.set = directionBit;
#else
                        digitalWrite(pin_direction, HIGH);
#endif
                    }
                    else {
#ifdef fast_io
                        directionPort->lat.clr = directionBit;
#else
                        digitalWrite(pin_direction, LOW);
#endif
                    }
                }
                else {
    //                running = false;
                    halt();
                }
            }

            if(vector.steps != 0 || currentSkip > 0) {
                if(currentSkip > 0) {
                    currentSkip--;
                }
                else {
                    currentSkip = skip;
                    step();
                }
            }
        }
    }

    void unsharedInterrupt() {
        if(running) {
            if(vector.steps == 0) {
                if(!buffer.isEmpty()) {
                    vector = buffer.pop();
                    bool ok;
                    uint32_t temp = setTimeBase(vector.time, &ok);
                    if(ok) {
                        interruptPeriod = temp;
                        if(vector.steps > 0) {
#ifdef fast_io
                            directionPort->lat.set = directionBit;
#else
                            digitalWrite(pin_direction, HIGH);
#endif
                        }
                        else {
#ifdef fast_io
                            directionPort->lat.clr = directionBit;
#else
                            digitalWrite(pin_direction, LOW);
#endif
                        }
                    }
                    else {
                        Serial.println("Timebase Error");
                        Serial.println(vector.steps);
                        Serial.println(vector.time.toString());
                        vector.steps = 0;
                        vector.time = Variant();
                        running = false;
                        halt();
                    }
                    return;
                }
                else {
//                    running = false;
                    halt();
                }
            }

            if(vector.steps != 0) {
                step();
            }
        }
    }

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

    void modifiedSigmoid(Variant begin_Hz, Variant end_Hz, Variant accelSteps, float coefficient, s32 steps, Variant move_time_s) {
        int points = 10;
        int rounds = move_time_s == 0 ? 1 : 16;

        Variant totalSteps;
        Variant totalTime;

        if(steps < 0) {
            accelSteps *= Variant(-1, 0);
        }

#ifdef debug_sigmoid
        Serial.println("Begin: " + begin_Hz.toString() + " sec");
        Serial.println("End  : " + end_Hz.toString() + " sec");
#endif

        // =(1 / (1 + (coefficient ^ (-point + 5))))
        Vector flatVector;
        Vector vectors[points + 1];
        Variant prev;
        Variant selected_Hz;

        selected_Hz = begin_Hz;
        selected_Hz += (end_Hz - begin_Hz) / 2;

        for(int r = 0; r < rounds; r++) {
            prev = 0;
            totalSteps = 0;
            totalTime = 0;

            for(int i = 0; i <= points; i++) {
                int exp = -i + 5;
                Variant base = Variant((float)(1 + pow(coefficient, exp)));
                Variant value(1, 0);
                value /= base;

#ifdef debug_sigmoid
                Serial.print(i);
                Serial.print(": ");
                Serial.print(base.toString());
                Serial.print("/");
                Serial.print(value.toString());
#endif
                if(i == 0) {
                    vectors[i].steps = 0;
                }
                else {
                    Variant temp1(1, 0);
                    temp1 /= selected_Hz;

                    Variant temp2(1, 0);
                    temp2 /= begin_Hz;

                    vectors[i].steps = ((value - prev) * accelSteps).toInt();
                    vectors[i].time = ((temp1 * value) + (temp2 * (Variant(1, 0) - value)));

                    totalSteps += vectors[i].steps;
                    totalTime += vectors[i].time * vectors[i].steps;
                }

#ifdef debug_sigmoid
                Serial.print("\t(");
                Serial.print(vectors[i].steps);
                Serial.print(" @ ");
                Serial.print(vectors[i].time.toString());
                Serial.println(")");
#endif
                prev = value;
            }

            totalTime *= 2;
            totalSteps *= 2;

            flatVector.steps = steps - totalSteps.toInt();
            flatVector.time = vectors[10].time;
            totalTime += flatVector.time * flatVector.steps;

#ifdef debug_sigmoid
//            Serial.print("Totals: ");
//            Serial.print(totalSteps.toString());
//            Serial.print(" @ ");
//            Serial.print(totalTime.toString());
//            Serial.print(" = ");
            totalTime *= Variant(1095, -3);
//            Serial.println(totalTime.toString());
#endif

            Serial.print("Round: " + String(r, DEC));

            if((totalTime.toDouble()) > (move_time_s.toDouble())) {
                Serial.print("(t +)");
                begin_Hz = selected_Hz;
                selected_Hz += (end_Hz - selected_Hz) / 2;
            }
            else {
                Serial.print("(f -)");
                end_Hz = selected_Hz;
                selected_Hz -= (begin_Hz - selected_Hz) / 2;
            }

//            Serial.print("\t(");
//            Serial.print(totalTime.toDouble());
//            Serial.print(" > ");
//            Serial.print(move_time_s.toDouble());
//            Serial.print(")\t");
//            Serial.print(begin_Hz.toString());
//            Serial.print(" ");
//            Serial.print(selected_Hz.toString());
//            Serial.print(" ");
//            Serial.print(end_Hz.toString());
//            Serial.println(" ");
        }

        // push vectors
        for(int i = 1; i <= points; i++) {
            buffer.push(vectors[i]);
        }
        buffer.push(flatVector);
        for(int i = points; i > 0; i--) {
            buffer.push(vectors[i]);
        }
    }

    void start() {
        vector.steps = 0;
        vector.time = Variant();
        running = true;
        time = millis();
    }

    void pause() {
        running = false;
    }

    void halt() {
        running = false;
        buffer.clear();
        Serial.println(millis() - time, DEC);
    }

    void setHomeSensor(int pin, bool desiredState = false) {
        if(pin == 0) {
            homeSensorPort = (p32_ioport *)0;
            homeSensorBit  = 0;
        }
        else {
            homeSensorPort = (p32_ioport *)portRegisters(digitalPinToPort(pin));
            homeSensorBit = digitalPinToBitMask(pin);
            previousState = (bool)(homeSensorPort->port.reg & homeSensorBit);
        }
        homeSensorPolarity = desiredState;
//        Serial.println((us32)homeSensorPort, DEC);
//        Serial.println((us32)homeSensorBit , DEC);
    }

    void chooseBestMove(s32 steps, Variant move_time_s = 0) {
        if(steps == 0) {
            return;
        }

        if(abs(steps) >= (sigSteps.toInt() * 2.5)) {
            modifiedSigmoid(sigLow, sigHigh, sigSteps, sigCoefficient, steps, move_time_s);
        }
        else {
            Variant period(1, 0);
            period /= sigLow;
            buffer.push(Vector(steps, period));
        }
//        start();
    }

    // relative move
    void move(Variant units, Variant move_time_s = 0) {
        units = config->unitConversion(units);

//        Serial.print(String(motor, DEC));
//        Serial.print(" move: ");
//        Serial.println(units.toString());

        chooseBestMove(units.toInt(), move_time_s);
    }

    // absolute move
    void moveTo(Variant units, Variant move_time_s = 0) {
        units = config->unitConversion(units);
        units -= config->getCurrentPosition();

//        Serial.print(String(motor, DEC));
//        Serial.print(" moveTo: ");
//        Serial.println(units.toString());

        chooseBestMove(units.toInt(), move_time_s);
    }

    StepConfig* getDefaultConfig() {
        return defaultConfig;
    }

    void setConfig(StepConfig *temp) {
        if(temp == 0) {
            config = defaultConfig;
        }
        else {
            config = temp;
        }
    }

    bool isBusy() {
        return running;
    }

    bool isReady() {
        return !running;
    }

    void setEnabled(bool enabled) {
        digitalWrite(pin_enable, !enabled);
        digitalWrite(pin_sleep ,  enabled);
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
                parser.println("OK Enable");
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

                parser.nextToken();
                Variant move_time_s = parser.toVariant();

                modifiedSigmoid(begin, end, accelSteps, c.toFloat(), steps.toInt(), move_time_s);
                start();
            }
            else if(parser.compare("scp")) {
                parser.nextToken();
                config->setCurrentPosition(parser.toVariant().toInt());
            }
            else if(parser.compare("rcp")) {
                if(config != 0) {
                    parser.println(String(config->getCurrentPosition(), DEC) + " steps");
                }
            }
            else if(parser.compare("move")) {
                parser.nextToken();
                Variant steps = parser.toVariant();
                parser.nextToken();
                Variant move_time_s = parser.toVariant();
                move(steps, move_time_s);
            }
            else if(parser.compare("moveto")) {
                parser.nextToken();
                Variant steps = parser.toVariant();
                parser.nextToken();
                Variant move_time_s = parser.toVariant();
                moveTo(steps, move_time_s);
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

                Serial.print("Sig: ");
                Serial.print(sigLow.toString());
                Serial.print(" ");
                Serial.print(sigHigh.toString());
                Serial.print(" ");
                Serial.print(sigSteps.toString());
                Serial.print(" ");
                Serial.println(parser.toVariant().toString());
                Serial.println("OK");
            }
            else if(parser.compare("units")) {
                parser.nextToken();
                Serial.println(config->unitConversion(parser.toVariant()).toString());
            }
            else if(parser.compare("conv")) {
                parser.nextToken();
                Variant mx = parser.toVariant();

                parser.nextToken();
                Variant b = parser.toVariant();

                parser.nextToken();
                config->setConversion(mx, b, parser.toVariant().toInt());
            }
        }
    }

    uint32_t interruptPeriod;

private:
    inline bool readHomeSensor() {
        if(homeSensorPort != 0) {
            bool input = (bool)(homeSensorPort->port.reg & homeSensorBit);
            if(input == 0 && previousState != 0) {
                previousState = 0;
                if(!homeSensorPolarity) {
                    return true;
                }
            }
            else if(input == 1 && previousState != 1) {
                previousState = 1;
                if(homeSensorPolarity) {
                    return true;
                }
            }
//            return (bool)(homeSensorPort->port.reg & homeSensorBit) == homeSensorPolarity;
        }
        return false;
    }

    inline void step() {
        if(readHomeSensor()) {
            halt();
            return;
        }
#ifdef fast_io
        stepPort->lat.set = stepBit;
        asm("nop\n nop\n nop\n nop\n nop\n nop\n nop\n");
#else
        digitalWrite(pin_step, HIGH);
#endif

        if(vector.steps > 0 ) {
            config->currentPosition++;
            vector.steps--;
        }
        else {
            config->currentPosition--;
            vector.steps++;
        }
#ifdef fast_io
        stepPort->lat.clr = stepBit;
#else
        digitalWrite(pin_step, LOW);
#endif
    }

    us8 motor;
    us8 pin_enable;
    us8 pin_sleep;
    StepConfig *config;
    StepConfig *defaultConfig;

#ifdef fast_io
    // step pin
    p32_ioport *stepPort;
    unsigned int stepBit;

    // direction pin
    p32_ioport *directionPort;
    unsigned int directionBit;

    // home sensor pin
    p32_ioport *homeSensorPort;
    unsigned int homeSensorBit;
    bool homeSensorPolarity;
#else
    us8 pin_step;
    us8 pin_direction;
#endif

    // sigmoid related
    Variant sigLow;
    Variant sigHigh;
    Variant sigSteps;
    float sigCoefficient;

    bool running;
    bool previousState;
    Vector vector;
//    us32 currentSkip;
    CircleBuffer buffer;

    // time motor
    int time;
};

#endif // STEPANDDIRECTION_H
