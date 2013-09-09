#ifndef VARIANT_H
#define VARIANT_H

#if defined(__PIC32MX__)
    /* chipKIT-specific code goes here */
#include <WProgram.h>
#else
#include <math.h>
#endif
#include "Core.h"

class Variant {
public:
    Variant(String string)
    {
        string = string.toLowerCase();
        us8 index = 0;
        us8 length = string.length();
        bool negative = false;

        value = 0;
        exp = 0;

        if(string.startsWith("-")) {
//            Serial.println("Is Negative");
            index += 1;
            negative = true;
        }

        if(string.startsWith("0x")) { // Hexadecimal
//            Serial.println("Is Hex");
            index += 2;
            do {
                value <<= 4;
                value |= hexCharToNibble(string.charAt(index++));
            } while(index < length);
        }
        else if(string.startsWith("0b")) { // Binary
//            Serial.println("Is Bin");
            index += 2;
        }
        else if(string.indexOf("e") != -1) { // Exp.Notation
//            Serial.println("Is Exp.Notation");
        }
        else if(string.indexOf(".") != -1) { // Real Number
//            Serial.println("Is Real");
            s32 base = pow(10, (length - index - 2));
            bool ad = false; // after decimal, of course...
            while(index < length) {
                us8 data = string.charAt(index++);
                if(data == '.') {
                    ad = true;
                }
                else {
                    value += (data - 0x30) * base;
                    base /= 10;

                    if(ad) {
                        exp--;
                    }
                }
            }
        }
        else { // Whole Number
//            Serial.println("Is Whole");
            s32 base;
            while(index < length) {
//                Serial.println(string.charAt(index) - 0x30, DEC);
                base = pow(10, length - index - 1);
                value += (string.charAt(index) - 0x30) * base;
                index++;
            }
        }

        if(negative) {
            value *= -1;
        }

        if(value == 0) {
            if(string == "true") {
                value = 1;
            }
        }
    }

    bool toBool()
    {
        return (value > 0) ? true : false;
    }

    s32 toInt()
    {
        return value;
    }

    float toFloat()
    {
        float temp = value;
        temp *= pow(10, exp);
        return temp;
    }

private:
//    if(parser.compare("test")) {
//      Serial.print("Whole: ");
//      Serial.println(Variant("1234").toInt());
//      Serial.println(Variant("4321").toInt());

//      Serial.print("Hex: ");
//      Serial.println(Variant("0x1234").toInt(), HEX);
//      Serial.println(Variant("0x4321").toInt(), HEX);

//      Serial.print("Real: ");
//      Serial.println(Variant("1.234").toFloat());
//      Serial.println(Variant("-432.1").toFloat());

//      Serial.print("Binary: ");
//      Serial.println(Variant("0b10101010").toInt());
//      Serial.println(Variant("0b01010101").toInt());

//      Serial.print("Bool: ");
//      Serial.println(Variant("true").toBool());
//      Serial.println(Variant("false").toBool());
//    }

    us8 hexCharToNibble(us8 c)
    {
        if('0' <= c && c <= '9') {
            c -= 0x30;
        }
        else if('a' <= c && c <= 'f') {
            c -= 0x57;
        }
        return c & 0xf;
    }

    s32 value;
    s8 exp;
};

#endif // VARIANT_H
