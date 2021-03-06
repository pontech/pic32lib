#ifndef STEPANDDIRECTION_H
#define STEPANDDIRECTION_H

#include <WProgram.h>
#include "Core.h"
#include "TokenParser.h"
#include "CircleBuffer.h"
#include "Vector.h"


#define fast_io
//#define debug_sigmoid
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
		s32 currentPosition_temp;
		noInterrupts();
		currentPosition_temp = currentPosition;
		interrupts();
		return currentPosition_temp;
    }
    void setCurrentPosition(s32 position) {
		noInterrupts();
        currentPosition = position;
		interrupts();
    }
	/// Set the destination position (in absolute steps)
    void setDestinationPosition(s32 position) {
        destinationPosition = position;
    }
	/// Update the destination position (in steps relative to current destinationPosition)
    void updateDestinationPosition(s32 position) {
        destinationPosition = destinationPosition + position;
    }
	/// Get the destination position (in absolute steps)
    s32 getDestinationPosition() {
        return destinationPosition;
    }
    void setConversion(Variant mx, Variant b = 0, us8 precision = 0) {
        if(mx != 0) {
            conversion_mx = mx;
            conversion_b = b;
            conversion_p = pow(10, precision);
        }
#ifdef debug_stp
		Serial.print("setConversion m=");
        Serial.print(conversion_mx.toString());
		Serial.print(", b=");
        Serial.print(conversion_b.toString());
		Serial.print(", p=");
        Serial.println(conversion_p.toString());
#endif
    }
    void setOffset(Variant b) {
        conversion_b = b;
     }
    void setLimits(Variant min, Variant max) {
        if(min != 0 && max != 0) {
            limit_min = min;
            limit_max = max;
            limit_enabled = true;
        }
        else {
			limit_max = Variant((s32)0x7fffffff, 0);
			limit_min = Variant((s32)0x80000000, 0);
            limit_enabled = false;
        }
    }

    Variant unitConversion(Variant units, bool *ok = 0) {
        if(limit_enabled) {
//            if(units < limit_min || units > limit_max) {
            double temp = units.toDouble();
            if(temp < limit_min.toDouble() || temp > limit_max.toDouble()) {
                if(ok != 0) {
                    *ok = false;
                }
                return 0;
            }
        }

        if(ok != 0) {
            *ok = true;
        }
//        Serial.println("In:" + units.toString());
//        units *= conversion_p;
        units *= conversion_mx;
        units += conversion_b;
        return units;
    }

private:
    volatile s32 currentPosition; ///< current step position (updated in the interrupt)
    s32 destinationPosition; ///< current step position (updated in with a move or moveto command)

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

////////////////////////////////////////////////
// BigEasy Kard Info
////////////////////////////////////////////////
// GPIO | REV C   | REV C1  | REV D   |
// -------------------------------------
//  0   | Step    | Step    | Step    |
//  1   | Dir     | Dir     | Dir     |
//  2   | !Enable | !Enable | !Enable |
//  3   | Sleep   | Sleep   | MS1     |
//  4   | MS3     | MS2     | MS2     |

public:
    StepAndDirection(us8 motor, us8 pin_step, us8 pin_direction, us8 pin_enable, us8 pin_sleep_ms1, us8 pin_ms3_ms2, Variant timebase, char *command_prefix = "stp0", char *kard_rev = "C" ) {
        StepAndDirection::motor = motor;

        StepAndDirection::pin_step = pin_step;
        StepAndDirection::pin_direction = pin_direction;
        StepAndDirection::pin_enable = pin_enable;
        StepAndDirection::pin_sleep_ms1 = pin_sleep_ms1;
        StepAndDirection::pin_ms3_ms2 = pin_ms3_ms2;

		StepAndDirection::kard_rev = kard_rev;
		StepAndDirection::command_prefix = command_prefix;
		StepAndDirection::timebase = timebase;
		
        pinMode(pin_step, OUTPUT);
        pinMode(pin_direction, OUTPUT);
        pinMode(pin_enable, OUTPUT);
        pinMode(pin_sleep_ms1, OUTPUT);
        pinMode(pin_ms3_ms2, OUTPUT);
		
		setEnabled(false); // true = enabled, false = disabled
		setMicrostepsPerStep(4); // Initial step mode

#ifdef fast_io
        stepPort = (p32_ioport *)portRegisters(digitalPinToPort(pin_step));
        stepBit = digitalPinToBitMask(pin_step);

        directionPort = (p32_ioport *)portRegisters(digitalPinToPort(pin_direction));
        directionBit = digitalPinToBitMask(pin_direction);

        homeSensorPort = 0;
        homeSensorPolarity = false;
		homeSensorPersistent = false;
#endif

        defaultConfig = new StepConfig();
        config = defaultConfig;

        interruptPeriod = setTimeBase(Variant(250, -6));
        currentSkip = 0;

        halt();

        // sigmoid defaults
        setSigmoid(Variant(1, 3), Variant(1, 4), Variant(25, 1), 3.0);
		overhead = Variant(20, -6);
    }
    StepAndDirection(us8 motor, us8 card, Variant timebase, char *command_prefix = "stp?", char *kard_rev = "C") {
        *this = StepAndDirection(motor, KardIO[card][0], KardIO[card][1], KardIO[card][2], KardIO[card][3], KardIO[card][4], timebase, command_prefix, kard_rev);
    }

	/** 
	* void setMicrostepsPerStep(us8 microsteps)
    *
	*************************************************
	* REV C/C1 Step Truth Table
	* REV C: MS1, MS2 pulled high (MS3 Controlled)
	* REV C1: MS1, MS3 pulled low (MS2 Controlled)
	*************************************************
	* MS1 MS2 MS3 Step Mode  Excitation 
	*  L   L   L  Full       2 Phase
	*  H   L   L  Half       1-2 Phase
	*  L   H   L  Quarter    W1-2 Phase
	*  H   H   L  Eighth     2W1-2 Phase
	*  H   H   H  Sixteenth  4W1-2 Phase
	*************************************************
    *
	*************************************************
	* REV D Step Truth Table
	*************************************************
	* MS1 MS2 Step Mode      Excitation
	*  L   L  Full           2 Phase
	*  H   L  Half           1-2 Phase
	*  L   H  Quarter        W1-2 Phase
	*  H   H  Sixteenth      4W1-2 Phase
	*************************************************/

	bool setMicrostepsPerStep(us8 microsteps) {
		bool sucessful = true;
		if( strncmp(kard_rev, "C", 1) == 0) {
			switch(microsteps) {
				case 1: // REV C1
				case 8: // REV C
					digitalWrite(pin_ms3_ms2, LOW);
					break;
				case 4:  // REV C1
				case 16: // REV C
					digitalWrite(pin_ms3_ms2, HIGH);
					break;
				default:
					sucessful = false;
					break;
			}
		}
		else if( strcmp(kard_rev, "D") == 0) {
			switch(microsteps) {
				case 1:
					digitalWrite(pin_sleep_ms1, LOW);
					digitalWrite(pin_ms3_ms2, LOW);
					break;
				case 2:
					digitalWrite(pin_sleep_ms1, HIGH);
					digitalWrite(pin_ms3_ms2, LOW);
					break;
				case 4:
					digitalWrite(pin_sleep_ms1, LOW);
					digitalWrite(pin_ms3_ms2, HIGH);
					break;
				case 16:
					digitalWrite(pin_sleep_ms1, HIGH);
					digitalWrite(pin_ms3_ms2, HIGH);
					break;
				default:
					sucessful = false;
					break;
			}
		}
		if( sucessful ) this->microsteps = microsteps;
		return sucessful;
	}
	
    // todo: verify skip starts at 1
    inline void sharedInterrupt() {
#ifdef fast_io
		if ( stepPort->port.reg & stepBit ) {
			stepPort->lat.clr = stepBit;
			//return;
		}
#else
		if( digitalRead(pin_step, HIGH) {
			digitalWrite(pin_step, LOW);
			//return;
		}
#endif

        if(vector.steps == 0 && currentSkip <= 0 && running) {
            if(!buffer.isEmpty()) {
                vector = buffer.pop();
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
                running = false;
            }
        }

        if(vector.steps != 0 || currentSkip > 0) {
            if(currentSkip > 0) {
                currentSkip--;
            }
            else {
                currentSkip = vector.time; // (vector.time / timebase).toInt();
                step();
            }
        }
    }
    void unsharedInterrupt() {
        if(vector.steps == 0 && running) {
            if(!buffer.isEmpty()) {
                vector = buffer.pop();
                bool ok;
                uint32_t temp = vector.time; //setTimeBase(vector.time, &ok);
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
                    Serial.println(vector.time,DEC);
                    vector.steps = 0;
                    vector.time = 0; // Variant();
                    running = false;
                }
                return;
            }
            else {
                running = false;
            }
        }

        if(vector.steps != 0 && running) {
            step();
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

    // stp0 test 5e3 30e3 300 3 3200
    // stp0 test 20e3 40e3 1000 4 32000 // 10 rps w/16x
    // stp0 test 18e3 28e3 1000 3.5 32000 // 17.5 rps w/8x
    void modifiedSigmoid(Variant beginFreq, Variant endFreq, Variant accelSteps, float coefficient, s32 steps) {
        int points = 10;

        Variant beginPeriod(1, 0);
        beginPeriod /= beginFreq;

        Variant endPeriod(1, 0);
        endPeriod /= endFreq;

        if(steps < 0) {
            accelSteps *= Variant(-1, 0);
        }

#ifdef debug_sigmoid
        Serial.println("beginPeriod: " + beginPeriod.toString() + " sec");
        Serial.println("endPeriod  : " + endPeriod.toString() + " sec");
        Serial.println("accelSteps : " + accelSteps.toString() + "");
        Serial.println("coefficient: " + Variant(coefficient).toString() + "");
        Serial.println("steps      : " + String(steps, DEC) + " steps");
#endif

        // =(1 / (1 + (coefficient ^ (-point + 5))))
        Vector vectors[points + 1];
        Variant prev;
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
            Serial.print(" - ");
#endif
            if(i == 0) {
                vectors[i].steps = 0;
                vectors[i].time = 0;
            }
            else {
                vectors[i].steps = ((value - prev) * accelSteps).toInt();
                //vectors[i].time = (((endPeriod * value) + (beginPeriod * (Variant(1, 0) - value))) / timebase).toInt();
				vectors[i].time = ((Variant(1,0) / ((endFreq-beginFreq)*value + beginFreq))/ timebase).toInt();
            }

#ifdef debug_sigmoid
            Serial.print(vectors[i].steps);
            Serial.print(", ");
            Serial.print(vectors[i].time,DEC);
            Serial.println(" ");
#endif
            prev = value;
        }

        int totalSteps = 0;
        for(int i = 1; i <= points; i++) {
            totalSteps += vectors[i].steps;
            buffer.push(vectors[i]);
        }

        Vector flatVector(steps - (totalSteps * 2), (endPeriod/timebase).toInt()); // vectors[10].time); //endPeriod);

#ifdef debug_sigmoid
        Serial.print(flatVector.steps);
        Serial.print(", ");
        Serial.println(flatVector.time, DEC);
#endif

        buffer.push(flatVector);

        for(int i = points; i > 0; i--) {
            buffer.push(vectors[i]);
        }

#ifdef debug_sigmoid
        Serial.print("TotalSteps: ");
        Serial.println(totalSteps);
        Serial.println(vectors[points].time,DEC);
#endif
    }
    void start() {
        vector.steps = 0;
        vector.time = 0; //Variant();
        running = true;
    }
    void pause() {
        running = false;
    }
	/// Halt, wait for current vector to finish (could be a long long time).
    inline void halt() {
        running = false;
        buffer.clear();
    }
	/// Halt immediately, do not wait for current vector to finish. 
    inline void halt_immediatly() {
		noInterrupts();
		halt();
		vector.steps = 0;
		config->setDestinationPosition(config->getCurrentPosition());
		interrupts();
    }
    bool getHomeSensorStatus() {
        return (homeSensorPort == 0) ? true : false;
    }
    void setHomeSensorPersistent(int pin, bool desiredState = false, bool desiredPersistentDirection = false) {
		setHomeSensor(pin, desiredState);
		homeSensorPersistent = true;
		homeSensorPersistentDirection = desiredPersistentDirection;
	}
    void setHomeSensor(int pin, bool desiredState = false) {
		homeSensorPersistent = false;
        if(pin == 0) {
            homeSensorPort = (p32_ioport *)0;
            homeSensorBit  = 0;
        }
        else {
            homeSensorPort = (p32_ioport *)portRegisters(digitalPinToPort(pin));
            homeSensorBit = digitalPinToBitMask(pin);
            previousHomeState = (bool)(homeSensorPort->port.reg & homeSensorBit);
        }
        homeSensorPolarity = desiredState;
        //        Serial.println((us32)homeSensorPort, DEC);
        //        Serial.println((us32)homeSensorBit , DEC);
    }
    void chooseBestMove(s32 steps) {
#ifdef debug_stp
	Serial.print("chooseBestMove ");
	Serial.print(String(steps,DEC));
#endif

        if(steps == 0) {
#ifdef debug_stp
			Serial.println(" steps");
#endif
            return;
        }

		config->updateDestinationPosition(steps);
#ifdef debug_stp
	Serial.print(" -> ");
	Serial.print(fabs((double)steps), DEC);
	Serial.print(" >= ");
	Serial.print((sigSteps.toInt() * 2.5), DEC);
	Serial.print(" is ");	
#endif
        if(fabs((double)steps) >= (sigSteps.toInt() * 2.5)) {
#ifdef debug_stp
	Serial.println("true");
#endif
            modifiedSigmoid(sigLow, sigHigh, sigSteps, sigCoefficient.toFloat(), steps);
        }
        else {
#ifdef debug_stp
	Serial.println("false");
#endif
            Variant period(1, 0);
            period /= sigLow;
            buffer.push(Vector(steps, (period/timebase).toInt()));
        }
        start();
    }
    /// relative move (wrapper)
    bool move(Variant units) {
		return moveAbsRel(units, false);
    }
    /// absolute move (wrapper)
    bool moveTo(Variant units) {
		return moveAbsRel(units, true);
    }	
    /// combined absolute and relative move function
    bool moveAbsRel(Variant units, bool absolute) {
        bool ok;
#ifdef debug_stp
	Serial.print("moveAbsRel ");
	Serial.print(units.toString());
#endif
		
		units = config->unitConversion(units, &ok);
#ifdef debug_stp
		Serial.print("->");
		Serial.print(units.toString());
#endif
		if(!running && ok) {
			if(absolute) {
				units -= config->getCurrentPosition();
#ifdef debug_stp
				Serial.print("->");
				Serial.print(units.toString());
#endif
			}
			else {
#ifdef debug_stp
				Serial.print("->rel");
#endif
			}
#ifdef debug_stp
			Serial.println(" steps");
#endif
			// Don't move if we have persistent home sensors and we are sitting on the sensor
			if(homeSensorPersistent) {
				if( readHomeState() == homeSensorPolarity ) {
#ifdef debug_stp
					Serial.println("Sitting on home sensor.");
#endif
					if( homeSensorPersistentDirection == (units > 0) ) return false;
				}
			}
            chooseBestMove(units.toInt());
			return true;
        }
		else {		
#ifdef debug_stp
			Serial.println("no move");
#endif
			return false;
		}
	}
	void moveFreq(Variant units, Variant frequency) {
		bool ok;
#ifdef debug_stp
		Serial.print("moveFreq units=");
        Serial.print(units.toString());
#endif
        units = config->unitConversion(units, &ok);

        if(units.toInt() == 0) {
            return;
        }

		config->updateDestinationPosition(units.toInt());
		Variant period(1, 0);
		period /= frequency;

#ifdef debug_stp
		Serial.print(", steps=");
        Serial.print(units.toString());
        Serial.print(", frequency=");
        Serial.print(frequency.toString());
        Serial.print(", period=");
        Serial.print(period.toString());
        Serial.print(", time=");
        Serial.print(timebase.toString());
        Serial.print(", skip=");
        Serial.println((period / timebase).toString());
#endif
		buffer.push(Vector(units.toInt(), (period / timebase).toInt()));
        start();
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
    s32 getCurrentPosition() {
		if( config == 0 ) return 0;
		return config->getCurrentPosition();
    }
	s32 getDestinationPosition() {
		if( config == 0 ) return 0;
		return config->getDestinationPosition();
    }
	s32 getDeltaPosition() {
		if( config == 0 ) return 0;
			s32 temp = config->getDestinationPosition() - config->getCurrentPosition();
			if( temp == 0 )
				while(running);	// Wait for the interrupt to fire one last time before returning zero
		return temp;
    }
    void setCurrentPosition(s32 position) {
		if( config == 0 ) return;
        config->setCurrentPosition(position);
    }
	void setDestinationPosition(s32 position) {
		if( config == 0 ) return;
        config->setDestinationPosition(position);
	}
    void setConversion(Variant mx, Variant b = 0, us8 precision = 0) {
		if( config == 0 ) return;
        config->setConversion(mx, b, precision);
     }
    void setLimits(Variant min, Variant max) {
		if( config == 0 ) return;
        config->setLimits(min, max);
    }
    Variant unitConversion(Variant units, bool *ok = 0) {
		if( config == 0 ) return units;
        return config->unitConversion(units, ok);
    }
    bool isBusy() {
        return running;
    }
    bool isReady() {
        return !running;
    }
	/// Assert or de-assert the enable line for the stepper driver
    bool getEnabled() {
		return !digitalRead(pin_enable);
	}
    void setEnabled(bool enabled) {
		if( strncmp(kard_rev, "C", 1) == 0) { // REV C/C1
			digitalWrite(pin_enable, !enabled); // 0 = enabled
			digitalWrite(pin_sleep_ms1, enabled);  // 1 = enabled
		}
		else if( strcmp(kard_rev, "D") == 0) {
			digitalWrite(pin_enable, !enabled); // 0 = enabled
		}
	}
	

    void setSigmoid(Variant begin, Variant end, Variant accelSteps, Variant coefficient) {
        sigLow = begin;
        sigHigh = end;
        sigSteps = accelSteps;
        sigCoefficient = coefficient;
    }

	/**
	 * Parse commands received from the TokenParser
	 */
    void command(TokenParser &parser) {
#ifdef debug_stp
        parser.print("stepper parser command:\"");
        parser.print(parser.toString());
        parser.print("\" \"");
        parser.print(command_prefix);
        parser.println("\"");
#endif
        if(parser.startsWith(command_prefix)) {
            parser.save();
            parser.advanceTail(strlen(command_prefix) - 1);

            if(motor != parser.toVariant().toInt()) {
                parser.restore();
                return;
            }
            parser.nextToken();

#ifdef debug_stp
        parser.print("motor command:\"");
        parser.print(parser.toString());
        parser.println("\"");
#endif

			/// List of support commands
			if(parser.compare("pairs")) { /// pairs
                for(int i = 0; i < buffer.size; i++) {
                    if(!parser.nextToken()) {
                        break;
                    }
                    String token = parser.toString();
                    parser.println(String(i, DEC) + ": " + token);

                    int index = token.indexOf(",");

                    Vector temp;
                    temp.steps = token.substring(0, index).toInt();
					/// temp.time needs testing here
                    temp.time = (Variant::fromString(token.substring(++index)) / timebase).toInt();
                    buffer.push(temp);
                }
                parser.println("OK Pairs");
                start();
            }
            else if(parser.compare("enable")) { /// enable true|false (enable the stepper driver)
                parser.nextToken();
                setEnabled(parser.toVariant().toBool());
            }
            else if(parser.compare("base")) { /// base n
                parser.nextToken();
                setTimeBase(parser.toVariant());
            }
            else if(parser.compare("test")) { /// test n n n n n (generate a modifiedSigmoid)
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
                start();
            }
            else if(parser.compare("scp")) { /// scp (set current position)
                parser.nextToken();
                setCurrentPosition(parser.toVariant().toInt());
            }
            else if(parser.compare("rcp")) { /// rcp (read current position)
				parser.println(String(getCurrentPosition(), DEC) + " steps");
            }
            else if(parser.compare("move")) { /// move n (relative n steps)
                parser.nextToken();
                move(parser.toVariant());
            }
            else if(parser.compare("moveto")) { /// move n (absolute position n)
                parser.nextToken();
                moveTo(parser.toVariant());
            }
            else if(parser.compare("setsig")) { /// setsig sigLow sigHigh sigSteps sigCoefficient (set sigmoid properties)
                parser.nextToken();
                sigLow = parser.toVariant();
                parser.nextToken();
                sigHigh = parser.toVariant();
                parser.nextToken();
                sigSteps = parser.toVariant();
                parser.nextToken();
                sigCoefficient = parser.toVariant();
                parser.println("OK");
			}
            else if(parser.compare("getsig")) { /// getsig (get sigmoid properties)
                parser.print(sigLow.toString());
                parser.print(" ");
                parser.print(sigHigh.toString());
                parser.print(" ");
                parser.print(sigSteps.toString());
                parser.print(" ");
                parser.println(sigCoefficient.toString());
            }
            else if(parser.compare("units")) {
                parser.nextToken();
                parser.println(config->unitConversion(parser.toVariant()).toString());
            }
            else if(parser.compare("conv")) {
                parser.nextToken();
                Variant mx = parser.toVariant();

                parser.nextToken();
                Variant b = parser.toVariant();

                parser.nextToken();
                config->setConversion(mx, b, parser.toVariant().toInt());
            }
			/**
 			 * Here begins a list of PONTECH STP10x compatible commands
			 */
            else if(parser.compare("rsm")) { /// RSM (read step minimum delay)
				parser.println(sigLow.toString());
            }
            else if(parser.compare("rsd")) { /// RSD (read step delay)
				parser.println(sigHigh.toString());
            }
            else if(parser.compare("rss")) { /// RSS (read step delay)
				parser.println(sigSteps.toString());
            }
            else if(parser.compare("rsa")) { /// RSA (read step acceleration)
				parser.println(sigCoefficient.toString());
            }
            else if(parser.compare("sm")) { /// SM Hz (set minimum step delay in 1/Hz)
                parser.nextToken();
                sigLow = parser.toVariant();
                parser.println("OK");
			}
            else if(parser.compare("sd")) { /// SD Hz (set step delay in 1/Hz)
                parser.nextToken();
                sigHigh = parser.toVariant();
                parser.println("OK");
			}
            else if(parser.compare("ss")) { /// SS s (set number of steps (s) to accelerate over)
                parser.nextToken();
                sigSteps = parser.toVariant();
                parser.println("OK");
			}
            else if(parser.compare("sa")) { /// SC n (set acceleration sigmoid shape n)
                parser.nextToken();
                sigCoefficient = parser.toVariant();
                parser.println("OK");
			}
            else if(parser.compare("so")) { /// SO Stepper Off (disable driver)
                setEnabled(false);
                parser.println("OK");
            }
            else if(parser.compare("sp")) { /// SP Stepper Powered (enable driver)
                setEnabled(true);
                parser.println("OK");
            }
            else if(parser.compare("rc")) { /// RC (read current position)
				parser.println(String(getCurrentPosition(), DEC));
            }
            else if(parser.compare("rd")) { /// RD (read destination position)
				parser.println(String(getDestinationPosition(), DEC));
            }
            else if(parser.compare("rt")) { /// RT (read delta position)
				parser.println(String(getDeltaPosition(), DEC));
            }
            else if(parser.compare("rx")) { /// RX (read sign of delta)
				s32 temp = getDeltaPosition();
				if( temp > 0 )
					parser.println("+");
				else if( temp < 0 )
					parser.println("-");
				else
					parser.println("0");
            }
            else if(parser.compare("hm")) { /// HM n (set current position)
                parser.nextToken();
                setCurrentPosition(parser.toVariant().toInt());
                setDestinationPosition(parser.toVariant().toInt());
                parser.println("OK");
            }
            else if(parser.compare("h0")) { /// H0 (stop the motor from running)
                parser.nextToken();
				halt_immediatly();
                parser.println("OK");
            }
            else if(parser.compare("h+")) { /// H+ (absolute move the limit max)
                if(!moveTo(config->limit_max)) parser.print("N");
                parser.println("OK");
            }
            else if(parser.compare("h-")) { /// H- (absolute move the limit min)
                if(!moveTo(config->limit_min)) parser.print("N");
                parser.println("OK");
            }
            else if(parser.compare("ii")) { /// II n (Move relative to position n)
                parser.nextToken();
                if(!move(parser.toVariant())) parser.print("N");
                parser.println("OK");
            }
            else if(parser.compare("mi")) { /// MI n (Move absolute to position n)
                parser.nextToken();
                if(!moveTo(parser.toVariant())) parser.print("N");
                parser.println("OK");
            }
            else if(parser.compare("mf")) { /// MF m n (Move absolute to position m at frequency n, use only for testing)
                parser.nextToken();
                Variant units = parser.toVariant();
                parser.nextToken();
                Variant frequency = parser.toVariant();
                moveFreq(units, frequency);
                parser.println("OK");
            }
            else if(parser.compare("smsps")) { /// SMSPS n (Set microsteps per step, n = microsteps (1,2,4,8,16) per step)
                parser.nextToken();
                Variant new_microsteps = parser.toVariant();
                if(!setMicrostepsPerStep(new_microsteps.toInt())) parser.print("N");
                parser.println("OK");
            }
			else if(parser.compare("v?")) { /// V? (Return command set version)
                parser.println("STP100 V2.3");
            }
			else if(parser.compare("s?")) { /// V? (Return command set version)
                parser.println("STP100 V2.3");
                parser.print("REV: ");
                parser.println(kard_rev);
                parser.print("Enabled: ");
                parser.println(String((int)getEnabled(), DEC));
                parser.print("Current Position: ");
				parser.println(String(getCurrentPosition(), DEC));
                parser.print("Destination Position: ");
				parser.println(String(getDestinationPosition(), DEC));
                parser.print("Delta Position: ");
				parser.println(String(getDeltaPosition(), DEC));
				parser.print("Steps per step: ");
                parser.println(String((int)microsteps, DEC));
			}
        }
    }

    uint32_t interruptPeriod;

	inline bool readHomeState() {
		return (bool)(homeSensorPort->port.reg & homeSensorBit);
	}
	char * getKardRev() {
		return kard_rev;
	}
private:
    inline bool readHomeSensor() {
        if(homeSensorPort != 0) {
            bool currentHomeState = readHomeState();
            if(currentHomeState != previousHomeState) {
                previousHomeState = currentHomeState;
                return (currentHomeState == homeSensorPolarity);
            }
        }
        return false;
    }

    inline void step() {
        if(readHomeSensor()) {
            halt_immediatly();
			if( !homeSensorPersistent ) {
				homeSensorPort = 0;
			}
            return;
        }
#ifdef fast_io
        stepPort->lat.set = stepBit;
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
    }

    us8 motor;
    us8 pin_step;
    us8 pin_direction;
    us8 pin_enable;
    us8 pin_sleep_ms1;
	us8 pin_ms3_ms2;
	char *command_prefix;
	char *kard_rev;
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
#endif
    bool homeSensorPersistent; /// true = persistent single direction, false = one shot
    bool homeSensorPersistentDirection; /// homeSensorPersistentPolarity matches directionBit

	us8 microsteps;
	
    // sigmoid related
    Variant sigLow;
    Variant sigHigh;
    Variant sigSteps;
    Variant sigCoefficient;
	Variant overhead;
	Variant timebase;

    volatile bool running;
    bool previousHomeState;
    Vector vector; 	///< The currently running vector in the interrupt
    us32 currentSkip;	///< The amount of time (interrupt time base counts) that needs to be skipped before the next step
    CircleBuffer buffer;
};

#endif // STEPANDDIRECTION_H
