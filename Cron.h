#ifndef CRON_H
#define CRON_H

#include <WProgram.h>
#include "Core.h"
#include "TokenParser.h"

#define cronSlots   32
#define invalidSlot 0xff

class Cron {
public:
    typedef enum {
        Ok = 0,
        Error,
        SlotsFull,
        NotFound
    } CronErrorCode;

    typedef enum {
        SingleThread = 0,
        MultiThread
    } CronOptions;

    typedef struct {
        fptr function;
        unsigned long yield;
        us16 temp;
        us16 temp2;
        us16 temp3;
        us16 counter;
    } CronDetail;

    typedef us8 CronId;
    typedef unsigned long (*ulong_fptr)();

    Cron(ulong_fptr timer = millis)
    {
        systemTimer = timer;
        currentSlot = 0;
        for(us8 i = 0; i < cronSlots; i++) {
            resetDetails(i);
        }
    }

    void scheduler(void)
    {
        if(systemTimer() >= slot[currentSlot].yield && slot[currentSlot].function != 0) {
            cronStatus |= 0x01;
            slot[currentSlot].yield = 0;
            ((fptr)(slot[currentSlot].function))();
            cronStatus &= ~0x01;

            if(slot[currentSlot].yield == 0) {
                resetDetails(currentSlot);
            }
        }
        currentSlot++;
    //    filter_max_value(currentSlots, CronSlots);
        if(currentSlot >= cronSlots) {
            currentSlot = 0;
        }
    }

    CronId add(fptr fpointer, CronOptions options = SingleThread, us32 yield = 0)
    {
        CronId id = invalidSlot;
        for(us8 i = 0; i < cronSlots; i++) {
            if(options == MultiThread) {
                if(slot[i].function == 0 && slot[i].yield == 0) {
                    id = i;
                    goto end;
                }
            }
            else {
                if(slot[i].function == fpointer) {
                    id = i;
                    goto end;
                }

                if(id == invalidSlot) {
                    if(slot[i].function == 0 && slot[i].yield == 0) {
                        id = i;
                    }
                }
            }
        }

        if(options == SingleThread && id != invalidSlot) {
            goto end;
        }

        return invalidSlot;

    end:
        resetDetails(id);
        slot[id].function = fpointer;
        slot[id].yield    = yield;
        return id;
    }

    CronDetail* self()
    {
        if(cronStatus & 0x01) {
            return &(slot[currentSlot]);
        }
        return 0;
    }

    CronDetail* at(us8 id)
    {
        if(id < cronSlots) {
            return &(slot[id]);
        }
        return 0;
    }

    CronErrorCode searchByFunction(fptr fpointer, CronId *id)
    {
        us8 x = 0;
        us8 i;
        for(i = 0; i < cronSlots; i++) {
            if(slot[i].function == fpointer) {
                if(id != 0) {
                    *id = i;
                }
                x++;
            }
        }

        if(x == 0) {
            return NotFound;
        }
        return Ok;
    }

    void stopAll()
    {
        us8 i;
        for(i = 0; i < cronSlots; i++) {
            resetDetails(i);
        }
        currentSlot = 0;
    }

    CronErrorCode stopById(CronId id)
    {
        if(id >= cronSlots) {
            return NotFound;
        }
        resetDetails(id);
        return Ok;
    }

    CronErrorCode stopByFunction(fptr fpointer)
    {
        us8 x = 0;
        us8 i;
        for(i = 0; i < cronSlots; i++) {
            if(slot[i].function == fpointer) {
                resetDetails(i);
                x++;
            }
        }

        if(x == 0) {
            return NotFound;
        }
        return Ok;
    }

    bool toggleFunction(fptr fpointer)
    {
        CronId id;
        if(searchByFunction(fpointer, &id) == Cron::Ok) {
            stopById(id);
        }
        else {
            add(fpointer, Cron::SingleThread, 0);
            return true;
        }
        return false;
    }

    void resetDetails(us8 i)
    {
        if(i < cronSlots) {
            slot[i].function    = 0;
            slot[i].yield       = 0;
            slot[i].temp        = 0;
            slot[i].temp2       = 0;
            slot[i].temp3       = 0;
            slot[i].counter     = 0;
        }
    }

    us8 usedSlots()
    {
        us8 x = 0;
        us8 i;
        for(i = 0; i < cronSlots; i++) {
            if(slot[i].yield != 0) {
                x++;
            }
        }
        return x;
    }

    void command(TokenParser *parser)
    {
        if(parser->compare("cron")) {
            parser->nextToken();
            if(parser->compare("status")) {
                String s((us16)usedSlots());
                s += "/";
                s += cronSlots;
                Serial.println(s);
            }
            if(parser->compare("stop")) {
                stopAll();
                Serial.println("OK");
            }
        }
    }

private:
    ulong_fptr systemTimer;
    us8 currentSlot;
    us8 cronStatus;
    CronDetail slot[cronSlots];
};

#endif // CRON_H
