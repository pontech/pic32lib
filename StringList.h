#ifndef STRINGLIST_H
#define STRINGLIST_H

#if defined(__PIC32MX__)
    /* chipKIT-specific code goes here */
#include <WProgram.h>
#endif
#include "chipKIT/WString.h"
#include "Core.h"
namespace chipKIT{
#define charactorSeperator '\r'

class StringList {
public:
    StringList() {}

    StringList(const String &string)
    {
        list = string;
        listSize = 0;
    }

    const void operator <<(const String &rhs)
    {
        if(list.length() > 0) {
            list += charactorSeperator;
        }
        list += rhs;
        listSize++;
    }

    us8 size()
    {
        return listSize;
    }

    String at(us8 index)
    {
        us8 count = 0;
        s16 tail = 0;
        s16 head = 0;

        do {
            tail = head;
            head = list.indexOf(charactorSeperator, tail);

            if(index == count++) {
                break;
            }
        }
        while(head++ != -1);

        if(head == -1) {
            head = list.length();
        }

        return list.substring(tail, head);
    }

    String join(const String &seperator)
    {
        String joinedString;

        s16 tail = 0;
        s16 head = 0;

        do {
            tail = head;
            head = list.indexOf(charactorSeperator, tail);

            if(joinedString.length() > 0) {
                joinedString += seperator;
            }

            joinedString += list.substring(tail, head);
        }
        while(head++ != -1);

        return joinedString;
    }

    String augment(const String &message)
    {
        String augmentedString;
        s16 tail = 0;
        s16 head = 0;

        do {
            tail = head;
            head = message.indexOf('%', tail);

            s16 temp = head;

            augmentedString += message.substring(tail, temp);

            // find digit width
            if(head != -1) {
                us8 charactor;
                do {
                    charactor = message.charAt(++head);
                }
                while('0' <= charactor && charactor <= '9');

                augmentedString += at(message.substring(temp + 1, head).toInt() - 1);
            }
        }
        while(head != -1);

        return augmentedString;
    }

private:
    String list;
    us8 listSize;
};
}
#endif // STRINGLIST_H
