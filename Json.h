#ifndef JSON_H
#define JSON_H

#include <WProgram.h>
#include "Core.h"
#include "TokenParser.h"
#include "StringList.h"

class Json {
public:
    typedef enum {
        NotJson = 0,
        Array,
        Object
    } JsonType;

    Json(String *string, s8 tail = -1, s8 head = -1)
    {
        jsonString = string;
        tailLimit = -1;
        headLimit = -1;
        jsonType = NotJson;
        jsonSize = 0;

        if(string == 0) {
            return;
        }

        us8 arrayLevel = 0;
        us8 objectLevel = 0;
        us8 index;

        if(tail == -1) {
            tail = 0;
        }

        if(head == -1) {
            head = jsonString->length();
        }

        index = tail;

        for(us8 i = 0; i < 10; i++) {
            positions[i] = -1;
        }

        while(index <= head) {
            s8 value = TokenParser::match(jsonString->charAt(index), "{}[],");

            if((value == 0) || (value == 2)) {
                if(arrayLevel == 0 && objectLevel == 0) {
                    jsonType = (value == 0) ? Object : Array;
                    positions[jsonSize++] = index;
                    tail = index;
                }
                if(value == 0) {
                    objectLevel++;
                }
                else {
                    arrayLevel++;
                }
            }
            else if((value == 1) || (value == 3)) {
                if(value == 1) {
                    objectLevel--;
                }
                else {
                    arrayLevel--;
                }
                if((arrayLevel == 0) && (objectLevel == 0)) {
                    head = index;
                    break;
                }
            }
            else if(value == 4) {
                bool valueFound = false;
                if(jsonType == Array) {
                    if((arrayLevel == 1) && (objectLevel == 0)) {
                        valueFound = true;
                    }
                }
                else {
                    if((objectLevel == 1) && (arrayLevel == 0)) {
                        valueFound = true;
                    }
                }

                if(valueFound) {
                    positions[jsonSize++] = index;
                }
            }

            index++;
        }

        if(jsonSize == 0) {
            positions[jsonSize++] = tail;
        }

        if((0 <= tail) && (tail <= head)) {
            tailLimit = tail;
            headLimit = head;
            positions[jsonSize] = head;
        }
    }

    JsonType type()
    {
        return jsonType;
    }

    us8 size()
    {
        return jsonSize;
    }

    String key(us8 index)
    {
        if(jsonType == Object) {
            if((index < jsonSize) && (positions[index] != -1)) {
                us8 length = positions[index + 1];
                us8 tail = 0;
                us8 head = 0;
                for(us8 i = positions[index]; i < length; i++) {
                    if(jsonString->charAt(i) == '"') {
                        tail = ++i;
                        break;
                    }
                }

                for(us8 i = tail; i < length; i++) {
                    if(jsonString->charAt(i) == '"') {
                        head = i;
                        break;
                    }
                }
                return jsonString->substring(tail, head);
            }
        }
        return "";
    }

    StringList keys()
    {
        StringList list;
        if(jsonType == Object) {
            for(us8 i = 0; i < jsonSize; i++) {
                list << key(i);
            }
        }
        return list;
    }

    Json value(int index)
    {
        if((index < jsonSize) && (positions[index] != -1)) {
            us8 tail = positions[index] + 1;
            us8 head = positions[index + 1] - 1;

            if(jsonType == Object) {
                for(us8 i = tail; i <= head; i++) {
                    if(jsonString->charAt(i) == ':') {
                        tail = i + 1;
                        break;
                    }
                }
            }

            if((jsonType == Array) || (jsonType == Object)) {
                for(us8 i = tail; i <= head; i++) {
                    if(TokenParser::match(jsonString->charAt(i), " ,\"") == -1) {
                        tail = i;
                        break;
                    }
                }

                for(us8 i = head; tail <= head;) {
                    if(TokenParser::match(jsonString->charAt(i), " ,\"") == -1) {
                        head = i;
                        break;
                    }
                    i--;
                }
                return Json(jsonString, tail, head);
            }
        }
        return Json(0);
    }

    Json value(const char *data)
    {
        String string(data);

        for(us8 i = 0; i < jsonSize; i++) {
            if(key(i) == string) {
                return value(i);
            }
        }
        return Json(0);
    }

    String toString()
    {
        s8 tail = tailLimit;
        s8 head = headLimit + 1;
        if((0 <= tailLimit) && (tailLimit < head)) {
            if(head <= jsonString->length()) {
                return jsonString->substring(tail, head);
            }
        }
        return "";
    }

private:
    String *jsonString;
    s8 tailLimit;
    s8 headLimit;

    JsonType jsonType;
    us8 jsonSize;
    s8 positions[10];
};

#endif // JSON_H
