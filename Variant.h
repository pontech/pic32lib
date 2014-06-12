#ifndef VARIANT_H
#define VARIANT_H

#include <WProgram.h>
#include "Core.h"

class Variant {
public:
    Variant(s32 newValue = 0, s8 newExp = 0) {
		fvalue = newValue * pow(10, newExp);
        reduce(this);
    }
    Variant(float newValue, unsigned int precision = 3) {
		fvalue = newValue;
        reduce(this);
    }
    Variant(double newValue, unsigned int precision = 6) {
		fvalue = newValue;
        reduce(this);
    }
    Variant(const char *text) {
        Variant var = fromString(text);
        this->fvalue = var.fvalue;
    }

    bool operator==(const Variant &other) {
        if(this->fvalue == other.fvalue) {
            return true;
        }
        return false;
    }
    bool operator!=(const Variant &other) {
        if(this->fvalue != other.fvalue) {
            return true;
        }
        return false;
    }
    bool operator<(const Variant &other) {
        if(this->fvalue < other.fvalue) {
            return true;
        }
        return false;
    }
    bool operator<=(const Variant &other) {
        if(*this == other) {
            return true;
        }
        return *this < other;
    }
    bool operator>(const Variant &other) {
        if(this->fvalue > other.fvalue) {
            return true;
        }
        return false;
    }
    bool operator>=(const Variant &other) {
        if(*this == other) {
            return true;
        }
        return *this > other;
    }
    Variant operator+(Variant other) {
        Variant result;
		result.fvalue = this->fvalue + other.fvalue;
        reduce(&result);
        return result;
    }
    Variant operator-(Variant other) {
        Variant result;
		result.fvalue = this->fvalue - other.fvalue;
        reduce(&result);
        return result;
    }
    Variant operator*(Variant other) {
        Variant result;
		result.fvalue = this->fvalue * other.fvalue;
		reduce(&result);
        return result;
    }
    // todo: add dynamic precision
    Variant operator/(Variant other) {
        Variant result = *this;
		result.fvalue = this->fvalue / other.fvalue;
		reduce(&result);
        return result;
    }
    Variant operator=(Variant other) {
        this->fvalue = other.fvalue;
        return *this;
    }
    Variant operator+=(Variant other) {
        *this = *this + other;
        return *this;
    }
    Variant operator-=(Variant other) {
        Variant result = *this - other;
        *this = result;
        return *this;
    }
    Variant operator*=(Variant other) {
        *this = *this * other;
        return *this;
    }
    Variant operator/=(Variant other) {
        *this = *this / other;
        return *this;
    }
    static Variant fromString(String string) {
        Variant var;
		us32 value = 0;
		s8 exp = 0;
		
        string = string.toLowerCase();
        us8 index = 0;
        us8 length = string.length();
        bool negative = false;

#ifdef debug_variant
		Serial.print("fromString \"");
		Serial.print(string);
		Serial.println("\"");
#endif

        if(string.startsWith("-")) {
#ifdef debug_variant
            Serial.println("Is Negative");
#endif
            index += 1;
            negative = true;
        }

		if(string == "true") {
#ifdef debug_variant
            Serial.println("Is boolean");
#endif
			value = 1;
		}
		else if(string == "false") {
#ifdef debug_variant
            Serial.println("Is boolean");
#endif
			value = 0;
		}
        else if(string.startsWith("0x")) { // Hexadecimal
#ifdef debug_variant
            Serial.println("Is Hex");
#endif
            index += 2;
            do {
                value <<= 4;
                value |= Variant::hexCharToNibble(string.charAt(index++));
            }
            while(index < length);
        }
        else if(string.startsWith("0b")) { // Binary
#ifdef debug_variant
            Serial.println("Is Bin");
#endif
            index += 2;
            do {
                value <<= 1;
                value |= Variant::binCharToNibble(string.charAt(index++));
            }
            while(index < length);
        }
        else if(string.indexOf("e") != -1) { /// Exp.Notation (does not *yet* handle floating point mantissa)
#ifdef debug_variant
            Serial.println("Is Exp.Notation");
#endif
            int pos = string.indexOf("e", index);
            value = (s32)(string.substring(index, pos).toInt());
            pos++;
            exp = (s8)(string.substring(pos, length).toInt());
        }
        else if(string.indexOf(".") != -1) { // Real(ish) Number
#ifdef debug_variant
            Serial.println("Is Real");
#endif
            int pos = string.indexOf(".", index);
            value = string.substring(index, pos).toInt();

            index = pos + 1;
            while(index < length) {
                value *= 10;
                value += (string.charAt(index++) - 0x30);
                exp--;
            }
        }
        else { // Whole Number
#ifdef debug_variant
            Serial.print("Is Whole \"");
#endif
            s32 base;
            while(index < length) {
#ifdef debug_variant
                Serial.print(string.charAt(index) - 0x30, DEC);
#endif
                base = pow(10, length - index - 1);
                value += (string.charAt(index) - 0x30) * base;
                index++;
            }
#ifdef debug_variant
			Serial.println("\"");
#endif
        }

        if(negative) {
            value *= -1;
        }

		
		var.fvalue = value * pow(10, exp);
#ifdef debug_variant
		Serial.print("value=");
        Serial.print(value, DEC);
		Serial.print(",exp=");
        Serial.print(exp, DEC);
		Serial.print(",fvalue=");
        Serial.println(var.toString());
#endif
        return var;
    }
    bool toBool() {
        return (fvalue > 0) ? true : false;
    }
    s32 toInt() {
        return (s32)fvalue;
    }
    float toFloat() {
        return fvalue;
    }
    double toDouble() {
        return fvalue;
    }
    String toString(char *format = "%1.3e") {
		char temp[50];
		sprintf(temp, format, fvalue);		
        return String(temp);
    }

private:
//    if(parser.compare("test1")) {
//        Serial.print("Whole: ");
//        Serial.println(Variant("1234").toInt());
//        Serial.println(Variant("4321").toInt());

//        Serial.print("Hex: ");
//        Serial.println(Variant("0x1234").toInt(), HEX);
//        Serial.println(Variant("0x4321").toInt(), HEX);

//        Serial.print("Real: ");
//        Serial.println(Variant("1.234").toFloat());
//        Serial.println(Variant("-432.1").toFloat());

//        Serial.print("Binary: ");
//        Serial.println(Variant("0b10101010").toInt());
//        Serial.println(Variant("0b01010101").toInt());

//        Serial.print("Bool: ");
//        Serial.println(Variant("true").toBool());
//        Serial.println(Variant("false").toBool());

//        float temp1 = 1.23456;
//        float temp2 = 12.3456;
//        float temp3 = 123.456;

//        Serial.println("temp1");
//        Serial.println(Variant::fromFloat(temp1, 4).toString());

//        Serial.println("temp2");
//        Serial.println(Variant::fromFloat(temp2, 4).toString());

//        Serial.println("temp3");
//        Serial.println(Variant::fromFloat(temp3, 4).toString());
//    }

//    if(parser.compare("test2")) {
//        parser.nextToken();
//        Variant left = Variant::fromString(parser.toString());

//        parser.nextToken();
//        Variant right = Variant::fromString(parser.toString());

//        Serial.print(left.toString());
//        Serial.print(" vs ");
//        Serial.println(right.toString());

//        Serial.print("left == right = ");
//        Serial.println(left == right);

//        Serial.print("left != right = ");
//        Serial.println(left != right);

//        Variant temp;

//        temp = left;
//        temp *= right;
//        Serial.print("left * right = ");
//        Serial.println(temp.toString());

//        temp = left;
//        temp /= right;
//        Serial.print("left / right = ");
//        Serial.println(temp.toString());

//        temp = left;
//        temp += right;
//        Serial.print("left + right = ");
//        Serial.println(temp.toString());

//        temp = left;
//        temp -= right;
//        Serial.print("left - right = ");
//        Serial.println(temp.toString());

//        Serial.print("left < right = ");
//        Serial.println(left < right);

//        Serial.print("left <= right = ");
//        Serial.println(left <= right);

//        Serial.print("left > right = ");
//        Serial.println(left > right);

//        Serial.print("left >= right = ");
//        Serial.println(left >= right);

//        Serial.println("left < 100e3");
//        Serial.println(left < "100e3");

//        Serial.println("right < 100e3");
//        Serial.println(right < "100e3");
//    }

    static inline void reduce(Variant *var) {
    }
    static inline us8 hexCharToNibble(us8 c) {
        if('0' <= c && c <= '9') {
            c -= 0x30;
        }
        else if('a' <= c && c <= 'f') {
            c -= 0x57;
        }
		else
			c = 0;
        return c & 0xf;
    }
    static inline us8 binCharToNibble(us8 c) {
        if('0' <= c && c <= '1') {
            c -= 0x30;
        }
		else
			c = 0;
        return c & 0x1;
    }

	double fvalue;
};

#endif // VARIANT_H
