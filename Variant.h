#ifndef VARIANT_H
#define VARIANT_H

#include <WProgram.h>
#include "Core.h"

class Variant {
public:
    Variant(s32 newValue = 0, s8 newExp = 0) {
        value = newValue;
        exp = newExp;
    }

    Variant(const char *text) {
        Variant var = fromString(text);
        this->value = var.value;
        this->exp = var.exp;
    }

    bool operator==(const Variant &other)
    {
        if(this->value == other.value && this->exp == other.exp) {
            return true;
        }
        return false;
    }

    bool operator!=(const Variant &other)
    {
        if(this->value != other.value || this->exp != other.exp) {
            return true;
        }
        return false;
    }

    bool operator<(const Variant &other)
    {
        int minimum = this->exp;
        if(minimum < other.exp) {
            minimum = other.exp;
        }

        s32 temp1 = this->value * pow(10, this->exp - minimum);
        s32 temp2 = other.value * pow(10, other.exp - minimum);

        if(temp1 < temp2) {
            return true;
        }
        return false;
    }

    bool operator<=(const Variant &other)
    {
        if(*this == other) {
            return true;
        }
        return *this < other;
    }

    bool operator>(const Variant &other)
    {
        int minimum = this->exp;
        if(minimum < other.exp) {
            minimum = other.exp;
        }

        s32 temp1 = this->value * pow(10, this->exp - minimum);
        s32 temp2 = other.value * pow(10, other.exp - minimum);

        if(temp1 > temp2) {
            return true;
        }
        return false;
    }

    bool operator>=(const Variant &other)
    {
        if(*this == other) {
            return true;
        }
        return *this > other;
    }

    Variant operator+(Variant other) {
        Variant result;

        if(abs(this->exp - other.exp) < 10) {
            result.exp = this->exp < other.exp ? this->exp : other.exp;
            result.value = this->value * pow(10, this->exp - result.exp);
            result.value += other.value * pow(10, other.exp - result.exp);
        }

        while(result.value % 10 == 0) {
            result.value /= 10;
            result.exp++;
        }
        return result;
    }

    Variant operator-(Variant other) {
        Variant result;

        if(*this == other) {
            return result;
        }

        if(abs(this->exp - other.exp) < 10) {
            result.exp = this->exp < other.exp ? this->exp : other.exp;
            result.value = this->value * pow(10, this->exp - result.exp);
            result.value -= other.value * pow(10, other.exp - result.exp);
        }

        if(result.value == 0) {
            result.exp = 0;
            result.value = 0;
            return result;
        }

        while(result.value % 10 == 0) {
            result.value /= 10;
            result.exp++;
        }
        return result;
    }

    Variant operator*(Variant other) {
        Variant result = *this;
        if(result.value != 0 && other.value != 0) {
            result.value *= other.value;
            result.exp += other.exp;

            while(result.value % 10 == 0) {
                result.value /= 10;
                result.exp++;
            }
        }
        return result;
    }

    Variant operator/(Variant other) {
        Variant result = *this;
        if(result.value != 0 && other.value != 0) {
            result.value *= 1000;
            result.value /= other.value;
            result.exp -= other.exp + 3;

            while(result.value % 10 == 0) {
                result.value /= 10;
                result.exp++;
            }
        }
        return result;
    }

    Variant operator=(Variant other) {
        this->value = other.value;
        this->exp = other.exp;
        return *this;
    }

    Variant operator+=(Variant other) {
        Variant result = *this + other;
        *this = result;
        return *this;
    }

    Variant operator-=(Variant other) {
        Variant result = *this - other;
        *this = result;
        return *this;
    }

    Variant operator*=(Variant other) {
        Variant result = *this * other;
        *this = result;
        return *this;
    }

    Variant operator/=(Variant other) {
        Variant result = *this / other;
        *this = result;
        return *this;
    }

    static Variant fromString(String string)
    {
        Variant var;

        string = string.toLowerCase();
        us8 index = 0;
        us8 length = string.length();
        bool negative = false;

        if(string.startsWith("-")) {
//            Serial.println("Is Negative");
            index += 1;
            negative = true;
        }

        if(string.startsWith("0x")) { // Hexadecimal
//            Serial.println("Is Hex");
            index += 2;
            do {
                var.value <<= 4;
                var.value |= Variant::hexCharToNibble(string.charAt(index++));
            }
            while(index < length);
        }
        else if(string.startsWith("0b")) { // Binary
//            Serial.println("Is Bin");
            index += 2;
        }
        else if(string.indexOf("e") != -1) { // Exp.Notation
//            Serial.println("Is Exp.Notation");
            int pos = string.indexOf("e", index);
            var.value = (s32)(string.substring(index, pos).toInt());
            pos++;
            var.exp = (s8)(string.substring(pos, length).toInt());

            if(var.value != 0) {
                while(var.value % 10 == 0) {
                    var.value /= 10;
                    var.exp++;
                }
            }
        }
        else if(string.indexOf(".") != -1) { // Real Number
//            Serial.println("Is Real");
            int pos = string.indexOf(".", index);
            var.value = string.substring(index, pos).toInt();

            index = pos + 1;
            while(index < length) {
                var.value *= 10;
                var.value += (string.charAt(index++) - 0x30);
                var.exp--;
            }
        }
        else { // Whole Number
//            Serial.println("Is Whole");
            s32 base;
            while(index < length) {
//                Serial.println(string.charAt(index) - 0x30, DEC);
                base = pow(10, length - index - 1);
                var.value += (string.charAt(index) - 0x30) * base;
                index++;
            }
        }

        if(negative) {
            var.value *= -1;
        }

        if(var.value == 0) {
            if(string == "true") {
                var.value = 1;
            }
        }

        return var;
    }

    bool toBool()
    {
        return (value > 0) ? true : false;
    }

    s32 toInt()
    {
        s32 temp = value;
        return temp *= pow(10, exp);
    }

    float toFloat()
    {
        float temp = value;
        return temp *= pow(10, exp);
    }

    double toDouble()
    {
        double temp = value;
        return temp *= pow(10, exp);
    }

    String toString()
    {
        return String(value, DEC) + "e" + String(exp, DEC);
    }

private:
//    if(parser.compare("test1")) {
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

    static inline us8 hexCharToNibble(us8 c)
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
