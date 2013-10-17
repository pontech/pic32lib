#ifndef STEPANDDIRECTION2_H
#define STEPANDDIRECTION2_H

#include <WProgram.h>
#include "Core.h"
#include "TokenParser.h"
#include "CircleBuffer.h"
#include "Vector.h"

#define array_base_type 0
typedef bool us1;	// for some reason typedef for bool or boolean do not work

const uint32_t delay_5us   = CORE_TICK_RATE/1000*5;
const uint32_t delay_10us  = CORE_TICK_RATE/1000*10;
const uint32_t delay_25us  = CORE_TICK_RATE/1000*25;
const uint32_t delay_50us  = CORE_TICK_RATE/1000*50;
const uint32_t delay_125us = CORE_TICK_RATE/1000*125;
const uint32_t delay_250us = CORE_TICK_RATE/1000*250;
const uint32_t delay_1ms   = CORE_TICK_RATE/1000*1000;
const uint32_t delay_250ms = CORE_TICK_RATE/1000*250000;

class StepAndDirection2 {
public:

    StepAndDirection2(us8 motor, us8 pin_step, us8 pin_direction, us8 pin_enable, us8 pin_sleep) {
		StepAndDirection2::motor = motor;
		StepAndDirection2::pin_step = pin_step;
		StepAndDirection2::pin_direction = pin_direction;
		StepAndDirection2::pin_enable = pin_enable;
		StepAndDirection2::pin_sleep = pin_sleep;
		motor_position = 0;
		
		pinMode(pin_step,OUTPUT);
		pinMode(pin_direction,OUTPUT);
		pinMode(pin_enable,OUTPUT);
		pinMode(pin_sleep,OUTPUT);
		digitalWrite(pin_sleep,HIGH);
		digitalWrite(pin_enable,LOW);
		
        interruptPeriod = delay_5us;
		flag = false;
		topside = false;
    }

    uint32_t interrupt(uint32_t currentTime) {
        if(vector.steps<=0 & vector.currentSkip<=0 & flag) {
			if(!buffer.isEmpty()){
				Serial.println("Next");
				vector = buffer.pop();
				Serial.print(vector.steps);
				Serial.print(" ");
				Serial.println(vector.skip);
				if(buffer.isEmpty()){
					flag = false;
					Serial.println(buffer.isEmpty());
				}
			}
        }
		if(vector.steps > 0 || vector.currentSkip > 0) {
            if(vector.currentSkip > 0) {
                vector.currentSkip--;
            }
            else{ //if(!topside){
                vector.currentSkip = vector.skip;

                // step
                //us1 temp = digitalRead(pin_step);
                digitalWrite(pin_step, 1);
//				topside = true;
				us1 temp = digitalRead(pin_step);
				motor_position++;
				//digitalWrite(pin_step, 0);
                if(temp) {
                    vector.steps--;
                }
				digitalWrite(pin_step, 0);

				//digitalWrite(pin_step, 0);
				//motor_position[motor]++;
				//digitalWrite(pin_step, 1);
			}
//			else
//			{
//				digitalWrite(pin_step, 0);
//				topside = false;
//			}
        }

        return (currentTime + interruptPeriod);
    }

    void command(TokenParser &parser)
    {
		if(parser.startsWith("stp?")) {
			parser.save();
			parser.advanceTail(3);
			if( motor != parser.toVariant().toInt() - array_base_type) {
				parser.restore();
				goto done;
			}
			
            parser.nextToken();
            if(parser.compare("vector")) {
                vector.direction = 0;
                vector.currentSkip = 0;

                parser.nextToken();
                vector.steps = parser.toVariant().toInt();

                parser.nextToken();
                vector.skip = parser.toVariant().toInt();

                Serial.println("OK Vector");
            }
			if(parser.compare("lock")) {
				flag = false;
				Serial.println("OK Lock");
			}
			if(parser.compare("specific")) {
				parser.nextToken();
				int amount = parser.toVariant().toInt();
				for(int i = 0; i < amount; i++){
					Vector temp;
					parser.nextToken();
					temp.steps = parser.toVariant().toInt();

					parser.nextToken();
					temp.skip = parser.toVariant().toInt();
					buffer.push(temp);
				}
				Serial.println("OK Specific");
			}
			if(parser.compare("create")) {
				parser.nextToken();
				int start = parser.toVariant().toInt();
				parser.nextToken();
				int stop = parser.toVariant().toInt();
				parser.nextToken();
				int numsteps = parser.toVariant().toInt();
				parser.nextToken();
				int stepheight = parser.toVariant().toInt();
				Serial.println((start - stop)/stepheight);
				if((start - stop) > 0) {
					for(int i = 0; i <= (start - stop)/stepheight; i++){
						Vector temp;
						temp.steps = numsteps;
						temp.skip = start-((stepheight)*(i));
						buffer.push(temp);
					}
				}
				else if((stop - start) > 0) {
					for(int i = 0; i <= (stop - start)/stepheight; i++){
						Vector temp;
						temp.steps = numsteps;
						temp.skip = start+((stepheight)*(i));
						buffer.push(temp);
					}
				}
				//flag = true;
				Serial.println("OK Create");
			}
			if(parser.compare("trapezoidal")) {
				flag = false;
				parser.nextToken();
				int start = parser.toVariant().toInt();
				parser.nextToken();
				int stop = parser.toVariant().toInt();
				parser.nextToken();
				int numsteps = parser.toVariant().toInt();
				parser.nextToken();
				int stepheight = parser.toVariant().toInt();
				Serial.println((start - stop)/stepheight);
				if((start - stop) > 0) {
					for(int i = 0; i <= (start - stop)/stepheight; i++){
						Vector temp;
						temp.steps = numsteps;
						temp.skip = start-((stepheight)*(i));
						buffer.push(temp);
					}
				}
				parser.nextToken();
				int amount = parser.toVariant().toInt();
				for(int i = 0; i < amount; i++){
					Vector temp;
					parser.nextToken();
					temp.steps = parser.toVariant().toInt();

					parser.nextToken();
					temp.skip = parser.toVariant().toInt();
					buffer.push(temp);
				}
				parser.nextToken();
				start = parser.toVariant().toInt();
				parser.nextToken();
				stop = parser.toVariant().toInt();
				parser.nextToken();
				numsteps = parser.toVariant().toInt();
				parser.nextToken();
				stepheight = parser.toVariant().toInt();
				if((stop - start) > 0) {
					for(int i = 0; i <= (stop - start)/stepheight; i++){
						Vector temp;
						temp.steps = numsteps;
						temp.skip = start+((stepheight)*(i));
						buffer.push(temp);
					}
				}
				flag = true;
				Serial.println("OK Trapezoidal");
			}
			if(parser.compare("unlock")) {
				flag = true;
				Serial.println("OK Unlock");
			}
		}
		done:
			;
    }

private:
	us8 motor;
	us8 pin_step;
	us8 pin_direction;
	us8 pin_enable;
	us8 pin_sleep;
	us32 motor_position;

    Variant timeBase;
    uint32_t interruptPeriod;
    Vector vector;
	bool flag;
	bool topside;
	CircleBuffer buffer;
};

#endif // STEPANDDIRECTION2_H
