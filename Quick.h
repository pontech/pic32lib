#ifndef QUICK_H
#define QUICK_H

#include <WProgram.h>
#include <Wire.h>
#include "Core.h"
#include "TokenParser.h"

#warning "COMPILING FOR REV A"
#define led1  37
#define led2  81

// Kard 0
#define c0p0 68
#define c0p1 58
#define c0p2 62
#define c0p3 55
#define c0p4 82
#define c0p5 32

// Kard 1
#define c1p0 57
#define c1p1 56
#define c1p2 63
#define c1p3 54
#define c1p4 83
#define c1p5 31

// Kard 2
#define c2p0 86
#define c2p1 64
#define c2p2 5
#define c2p3 70
#define c2p4 84
#define c2p5 30

// Kard 3
#define c3p0 22
#define c3p1 76
#define c3p2 9
#define c3p3 2
#define c3p4 35
#define c3p5 52

// Kard 4
#define c4p0 23
#define c4p1 39
#define c4p2 8
#define c4p3 21
#define c4p4 34
#define c4p5 50

// Kard 5
#define c5p0 78
#define c5p1 79
#define c5p2 10
#define c5p3 20
#define c5p4 33
#define c5p5 85

// Kard Com (Turnaround definitions missing, look for them on another branch)
#define c6p0 71
#define c6p1 71
#define c6p2 71
#define c6p3 71
#define c6p4 71
#define c6p5 44

extern us8 KardIO[7][6]; // Moved definition to KardIO.h, include this file before Quick.h in your sketch

class Quick {
public:
    Quick()
    {
        Wire.begin();

        pinMode(led1, OUTPUT);
        pinMode(led2, OUTPUT);
    }

    void kardConfig(us8 c, us8 config)
    {
      for(us8 i = 0; i <= 5; i++) {
        digitalWrite(KardIO[c][i], LOW);
        pinMode(KardIO[c][i], (config & 0x01) ? INPUT : OUTPUT);
        config >>= 1;
      }
    }

    bool pin(us8 c, us8 p)
    {
        if(c <= 7 && p <= 6) {
            return digitalRead(KardIO[c][p]);
        }
        return false;
    }

    void pin(us8 c, us8 p, bool state)
    {
        if(c < 7 && p < 6) {
            digitalWrite(KardIO[c][p], state);
        }
    }

    us8 card(us8 c)
    {
        us8 value = 0;
        if(c <= 7) {
            for(int i = 6; i > 0;) {
                Serial.print(digitalRead(KardIO[c][i]), DEC);

                value <<= 1;
                value |= digitalRead(KardIO[c][i--]);
            }
            Serial.println();
        }
        return value;
    }

    void card(us8 c, us8 state)
    {
        if(c < 7) {
            for(int i = 0; i < 6; i++) {
                digitalWrite(KardIO[c][i], state & 1);
                state >>= 1;
            }
        }
    }

    void printEE(us8 address)
    {
      Wire.beginTransmission((int)address); // 0x53 quicK 0x50 Kard
      Wire.send(0xFA);
      Wire.endTransmission();
      Wire.requestFrom((int)address, 10);   // request 6 bytes from slave device #2

      while(Wire.available()) {    // slave may send less than requested
        us8 data = Wire.receive(); // receive a byte as character
        Serial.print(data, HEX);   // print the character
        Serial.print(".");
      }
      Serial.println();
    }

    void cardData(us8 c)
    {
        Serial.print("QuicK0: ");
        printEE(0x53);

        us8 kard;
        for(kard = 0; kard < 7; kard++) {
          pinMode(KardIO[kard][5], OUTPUT);
          digitalWrite(KardIO[kard][5], HIGH);
        }

        for(kard = 0; kard < 7; kard++) {
          Serial.print("KARD-");
          Serial.print(kard, DEC);
          Serial.print(": ");
          digitalWrite(KardIO[kard][5], LOW);

          printEE(0x50);
          digitalWrite(KardIO[kard][5], HIGH);
        }

        for(kard = 0; kard < 7; kard++) {
          pinMode(KardIO[kard][5], INPUT);
        }
    }

    void command(TokenParser &parser)
    {
        if(parser.compare("qc.ee")) {
            cardData(0);
        }
        else if(parser.compare("pin")) {
            parser.nextToken();
            us8 pinNumber = parser.toVariant().toInt();
            us8 cardNumber = 0;

            while(pinNumber >= 4) {
                pinNumber -= 4;
                cardNumber++;
            }

            Serial.println(cardNumber, DEC);
            Serial.println(pinNumber, DEC);
        }
        else if(parser.compare("c?p?", true)) {
            parser.save();
            parser.advanceTail(1);
            parser.reverseHead(2);
            us8 c = parser.toVariant().toInt();

            parser.restore();
            parser.advanceTail(3);
            us8 p = parser.toVariant().toInt();

            if(parser.nextToken()) {
                pin(c, p, parser.toVariant().toBool());
                Serial.println("OK");
            }
            else {
                Serial.println(pin(c, p) ? "ON" : "OFF");
            }
        }
        else if(parser.compare("c?", true)) {
            Serial.println(parser.toString());

            parser.advanceTail(1);
            us8 c = parser.toVariant().toInt();

            if(parser.nextToken()) {
                card(c, parser.toVariant().toInt());
                Serial.println("OK");
            }
            else {
                Serial.println(card(c), HEX);
            }
        }
        if(parser.compare("reset")) {
            Serial.println("Close serial terminal, resetting board in...");
            for(us8 sec = 5; sec >= 1; sec-- ) {
                Serial.print(sec, DEC);
                Serial.println(" seconds...");
                delay(1000);
            }
            executeSoftReset(ENTER_BOOTLOADER_ON_BOOT);
            // Example 7-1 from Section 7 of PIC32 Family Manual

            /* The following code illustrates a software Reset */
            // assume interrupts are disabled
            // assume the DMA controller is suspended
            // assume the device is locked

            /* perform a system unlock sequence */

            // starting critical sequence
            //SYSKEY = 0x00000000;  //write invalid key to force lock
            //SYSKEY = 0xAA996655;  //write key1 to SYSKEY
            //SYSKEY = 0x556699AA;  //write key2 to SYSKEY
            // OSCCON is now unlocked
            /* set SWRST bit to arm reset */
            //RSWRSTSET = 1;
            /* read RSWRST register to trigger reset */
            //unsigned int dummy;
            //dummy = RSWRST;
            /* prevent any unwanted code execution until reset occurs*/
            while(1);
        }
    }
};

#endif // QUICK_H
