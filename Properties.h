#ifndef PROPERTIES_H
#define PROPERTIES_H

#include "Core.h"
#include "KeyValueTable.h"
#include "TokenParser.h"
#include "Json.h"
#include "StringList.h"

class Properties : public KeyValueTable {
public:
    typedef void (*fptr_string)(String);

    typedef enum {
        NullProperty = 0,
        BoolProperty,
        NumberProperty,
        StringProperty,
        JsonProperty
    } PropertyType;

    typedef enum {
        RO = 0,
        RW
    } PropertyMode;

    typedef struct {
        PropertyType type;
        PropertyMode mode;
        fptr_string action;
        us8 changed;
    } PropertiesTableDetail;

    Properties(us8 size = 10)
        : KeyValueTable(size)
    {
        //propertiesTable = (PropertiesTableDetail*)malloc(size * sizeof(PropertiesTableDetail));

        for(us8 i = 0; i < tableSize; i++) {
            propertiesTable[i].type = NullProperty;
            propertiesTable[i].mode = RW;
            propertiesTable[i].action = 0;
            propertiesTable[i].changed = 0;
        }

        echoFunction = 0;
        batchMode = -1;
        executingAction = false;
    }

    s8 addBool(String key, bool value, fptr_string func = 0, PropertyMode mode = RW)
    {
        s8 index = setValue(key, value ? "true" : "false", true);
        if(index >= 0) {
            propertiesTable[index].type = BoolProperty;
            propertiesTable[index].mode = mode;
            if(func != 0) {
                propertiesTable[index].action = func;
            }
        }
        return index;
    }

    s8 addNumber(String key, String value, fptr_string func = 0, PropertyMode mode = RW)
    {
        s8 index = setValue(key, value, true);
        if(index >= 0) {
            propertiesTable[index].type = NumberProperty;
            propertiesTable[index].mode = mode;
            if(func != 0) {
                propertiesTable[index].action = func;
            }
        }
        return index;
    }

    s8 addString(String key, String value, fptr_string func = 0, PropertyMode mode = RW)
    {
        s8 index = setValue(key, value, true);
        if(index >= 0) {
            propertiesTable[index].type = StringProperty;
            propertiesTable[index].mode = mode;
            if(func != 0) {
                propertiesTable[index].action = func;
            }
        }
        return index;
    }

    s8 addJson(String key, String value, fptr_string func = 0, PropertyMode mode = RW)
    {
        s8 index = setValue(key, value, true);
        if(index >= 0) {
            propertiesTable[index].type = JsonProperty;
            propertiesTable[index].mode = mode;
            if(func != 0) {
                propertiesTable[index].action = func;
            }
        }
        return index;
    }

    void setBatchMode(bool mode) {
        if(mode) {
            batchMode = 0;
        }
        else {
            for(us8 i = 1; i <= batchMode; i++) {
                for(us8 p = 0; p < tableSize; p++) {
                    if(propertiesTable[p].changed == i) {
                        propertiesTable[p].changed = 0;
                        if(echoFunction != 0) {
                            echoFunction(jsonString(p, i, batchMode));
                        }
                        break;
                    }
                }
            }
            if(echoFunction != 0) {
                echoFunction("\r");
            }
            batchMode = -1;
        }
    }

    bool getBatchMode() {
        return batchMode;
    }

    bool update(String key, String value)
    {
        s8 index = findIndex(key);
        return update(index, value);
    }

    bool update(s8 index, String value)
    {
        if(0 <= index && index < tableSize) {
            setValue(index, value);

            if(batchMode != -1) {
                if(propertiesTable[index].changed == 0) {
                    batchMode++;
                    propertiesTable[index].changed = batchMode;
                }
            }
            else {
                if(echoFunction != 0) {
                    echoFunction(jsonString(index) + "\r");
                }
            }

            if((propertiesTable[index].action != 0) && (!executingAction)) {
                executingAction = true;
                propertiesTable[index].action(value);
                executingAction = false;
            }
            return true;
        }
        return false;
    }

    String jsonString(s8 index, us8 start = 1, us8 stop = 1)
    {
        if(0 <= index && index < tableSize) {
            StringList list;
            list << key(index);
            list << value(index);

            String temp;
            if(start == 1) {
                temp += "{";
            }

            temp += " \"%1\": ";

            switch(propertiesTable[index].type) {
            case NullProperty:
                temp += "null";
                break;

            case BoolProperty:
            case NumberProperty:
            case JsonProperty:
                temp += "%2";
                break;

            case StringProperty:
                temp += "\"%2\"";
            }

            if(start != stop) {
                temp += ",";
            }
            else {
                temp += " }";
            }

            return list.augment(temp);
        }
        return "";
    }

    void evaluate(TokenParser &parser)
    {
        if(parser.isJson()) {
            parser.advanceHead(parser.remaining());
            String string = parser.toString();
            Json json(&string);

            for(us8 i = 0; i < json.size(); i++) {
                s8 index = findIndex(json.key(i));

                if(0 <= index && index < tableSize) {
                    if(propertiesTable[index].mode == RW) {
                        String value = json.value(i).toString();
                        setValue(index, value);

                        if(propertiesTable[index].action != 0) {
                            propertiesTable[index].action(value);
                        }
                    }
                }
            }
        }
        else {
            command(parser);
        }
    }

    void setEchoFunction(fptr_string func)
    {
        echoFunction = func;
    }

    void command(TokenParser &parser)
    {
        if(parser.compare("get")) {
            parser.nextToken();
            s8 index = findIndex(parser.toString());
            parser.println(jsonString(index));
        }
        else if(parser.compare("set")) {
            parser.nextToken();
            s8 index = findIndex(parser.toString());
            parser.nextToken();
            update(index, parser.toString());
        }
        else if(parser.compare("keys")) {
            us8 count = 0;
            parser.print("[");
            for(s8 i = 0; i < tableSize; i++) {
                if(kvTable[i].key[0] != 0) {
                    if(count++ > 0) {
                        parser.print(",");
                    }
                    parser.print("\"");
                    String key(kvTable[i].key);
                    parser.print(key);
                    parser.print("\"");
                }
            }
            parser.println("]");
        }
        else if(parser.compare("properties")) {
            us8 length;
            for(length = 0; length < tableSize; length++) {
                if(kvTable[length].key[0] == 0) {
                    break;
                }
            }

            parser.print("{\n");
            for(us8 i = 0; i < length; i++) {
                StringList list;
                list << key(i);

                String mode = String((propertiesTable[i].mode == RW) ? "rw" : "r");
                if(propertiesTable[i].action != 0) {
                    mode += "x";
                }
                list << mode;

                switch(propertiesTable[i].type) {
                case NullProperty:
                    list << "null";
                    list << "null";
                    break;
                case BoolProperty:
                    list << "bool";
                    list << String((value(i) == "true") ? "true" : "false");
                    break;
                case NumberProperty:
                    list << "number";
                    list << value(i);
                    break;
                case StringProperty:
                    list << "string";
                    list << String("\"" + value(i) + "\"");
                    break;
                case JsonProperty:
                    list << "json";
                    list << value(i);
                }


                parser.print(list.augment("\t\"%1\": [\"%3\", \"%2\", %4]"));
                if(i < (length - 1)) {
                    parser.print(",");
                }
                parser.print("\n");
            }
            parser.println("}");
        }
    }

private:
    PropertiesTableDetail propertiesTable[50];
    fptr_string echoFunction;
    s8 batchMode;
    bool executingAction;
};

#endif // PROPERTIES_H
