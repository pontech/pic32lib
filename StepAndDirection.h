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
        digitalWrite(pin_enable, LOW); // 0 = enabled
        digitalWrite(pin_sleep, LOW);  // 1 = enabled
//        digitalWrite(c3p4, HIGH); // 8x microstepping
//        digitalWrite(c1p4, LOW); // 16x microstepping

        motor_position = 0;
        flag = false;
    }

    StepAndDirection(us8 motor, us8 card) {
        *this = StepAndDirection(motor, KardIO[card][0], KardIO[card][1], KardIO[card][2], KardIO[card][3]);
    }

    // todo: verify skip starts at 1
    void interrupt() {
        if(vector.steps == 0 && vector.currentSkip <= 0 && flag) {
            if(!buffer.isEmpty()) {
                vector = buffer.pop();
				if( vector.steps > 0 )
					digitalWrite(pin_direction, HIGH);
				else
					digitalWrite(pin_direction, LOW);
            }
            else {
                flag = false;
            }
        }
        if(vector.steps != 0 || vector.currentSkip > 0) {
            if(vector.currentSkip > 0) {
                vector.currentSkip--;
            }
            else {
                vector.currentSkip = vector.skip;
                digitalWrite(pin_step, 1);
				if( vector.steps > 0 ) {
					motor_position++;
					vector.steps--;
				}
				else {
					motor_position--;
					vector.steps++;
				}
                digitalWrite(pin_step, 0);
            }
        }
    }

    void stop() {
        flag = false;
    }

    void moveTo(TokenParser &parser, s32 units) {
        parser.print("moveTo: ");

        parser.println(String(units, DEC));
        buffer.push(Vector(15, 30));
        buffer.push(Vector(20, 20));
        buffer.push(Vector(20, 10));
        buffer.push(Vector(30, 5));
        buffer.push(Vector(50, 1));

        buffer.push(Vector(2930, 0));

        buffer.push(Vector(50, 1));
        buffer.push(Vector(30, 5));
        buffer.push(Vector(20, 10));
        buffer.push(Vector(20, 20));
        buffer.push(Vector(15, 30));
        flag = true;
    }

    void move(TokenParser &parser, s32 units) {
        parser.print("move: ");
        parser.println(String(units, DEC));
    }

    bool isBusy() {
        return (!buffer.isEmpty() && flag);
    }

    void setCurrentPosition(s32 position) {
        motor_position = position;
    }

    void setMaxSpeed(Variant maxSpeed) {

    }

    void setAcceleration(Variant acceleration) {

    }

    void setEnabled(bool enabled) {
        digitalWrite(pin_sleep, enabled);
    }

    void command(TokenParser &parser) {
        if(parser.startsWith("stp?")) {
            parser.save();
            parser.advanceTail(3);
            if(motor != parser.toVariant().toInt() - array_base_type) {
                parser.restore();
                goto done;
            }

            parser.nextToken();

            if(parser.compare("lock")) {
                flag = false;
                parser.println("OK Lock");
            }
            else if(parser.compare("unlock")) {
                flag = true;
                parser.println("OK Unlock");
            }
            else if(parser.compare("pairs")) {
                for(int i = 0; i < buffer.size; i++) {
                    if(!parser.nextToken()) {
                        break;
                    }
                    String token = parser.toString();
                    parser.println(String(i, DEC) + ": " + token);

                    int index = token.indexOf(",");

                    Vector temp;
                    temp.steps = token.substring(0, index).toInt();
                    temp.skip = token.substring(++index).toInt();
                    buffer.push(temp);
                }
                flag = true;
                parser.println("OK Pairs");
            }
            else if(parser.compare("create")) {
                parser.nextToken();
                int start = parser.toVariant().toInt();

                parser.nextToken();
                int stop = parser.toVariant().toInt();

                parser.nextToken();
                int numsteps = parser.toVariant().toInt();

                parser.nextToken();
                int stepheight = parser.toVariant().toInt();

                parser.println((start - stop)/stepheight);
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
                parser.println("OK Create");
            }
            else if(parser.compare("trapezoidal")) {
                flag = false;

                parser.nextToken();
                int start = parser.toVariant().toInt();

                parser.nextToken();
                int stop = parser.toVariant().toInt();

                parser.nextToken();
                int numsteps = parser.toVariant().toInt();

                parser.nextToken();
                int stepheight = parser.toVariant().toInt();

                parser.println((start - stop)/stepheight);
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
                parser.println("OK Trapezoidal");
            }
			else if(parser.compare("rcp")) {
                parser.print(String(motor_position, DEC));
                parser.print("\r\n");
				goto done;
            }
			else if(parser.compare("moveto")) {
			  parser.nextToken();
			  s32 pos = parser.toVariant().toInt();
			  parser.println(String(pos, DEC));
			  moveTo(parser, pos);
			}
			else if(parser.compare("pow")) {
			  Serial.println((pow(4,0.5)));
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
    s32 motor_position;

    bool flag;
    Vector vector;
    CircleBuffer buffer;
};

#endif // STEPANDDIRECTION_H
