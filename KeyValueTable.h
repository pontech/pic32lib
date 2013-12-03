#ifndef KEYVALUETABLE_H
#define KEYVALUETABLE_H

#include "Core.h"
#include "TokenParser.h"
#include "StringList.h"

#define keySize 20
#define valueSize 40

class KeyValueTable {
    friend class Properties;

    typedef struct {
        char key[keySize];
        char value[valueSize];
    } KeyValueTableDetail;

public:
    KeyValueTable(us8 size = 10)
    {
        //kvTable = (KeyValueTableDetail*)malloc(size * sizeof(KeyValueTableDetail));
        tableSize = size;
    }

    void clear()
    {
        for(us8 i = 0; i < tableSize; i++) {
            remove(i);
        }
    }

    s8 index(String key)
    {
        return findIndex(key);
    }

    bool remove(s8 index)
    {
        if(0 <= index && index < tableSize) {
            for(us8 y = 0; y < keySize; y++) {
              kvTable[index].key[y] = 0;
            }

            for(us8 z = 0; z < valueSize; z++) {
              kvTable[index].value[z] = 0;
            }
            return true;
        }
        return false;
    }

    bool remove(String key)
    {
        return remove(findIndex(key));
    }

    String key(s8 index)
    {
        if(index < 0 || tableSize <= index) {
            return String("null");
        }
        return kvTable[index].key;
    }

    String value(s8 index)
    {
        if(index < 0 || tableSize <= index) {
            return String("null");
        }
        return kvTable[index].value;
    }

    String value(String key)
    {
        return value(findIndex(key));
    }

    s8 setValue(String key, String value, bool create = false)
    {
        s8 index = findIndex(key, create);

        if(kvTable[index].key[0] == 0) {
            if(key.length() < keySize) {
                for(us8 i = 0; i < key.length(); i++) {
                    kvTable[index].key[i] = key.charAt(i);
                }
            }
            else {
                return -1;
            }
        }

        setValue(index, value);

        return index;
    }

    void setValue(s8 index, String value)
    {
        if(0 <= index && index < tableSize) {
            if(value.length() < valueSize) {
                us8 i;
                for(i = 0; i < value.length(); i++) {
                    char *a = kvTable[index].value + i;
                    char b = value.charAt(i);
                    if(*a != b) {
                        *a = b;
                    }
                }

                if(i < valueSize) {
                    kvTable[index].value[i] = 0;
                }
            }
        }
    }

    void command(TokenParser &parser)
    {
        if(parser.compare("get")) {
            parser.nextToken();
            String key = parser.toString();

            parser.println(value(key));
        }
        else if(parser.compare("set")) {
            parser.nextToken();
            String key = parser.toString();

            parser.nextToken();
            String value = parser.toString();

            setValue(key, value);
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
        else if(parser.compare("dump")) {
            for(us8 i = 0; i < tableSize; i++) {
                if(kvTable[i].key[0] == 0) {
                    break;
                }

                StringList list;
                list << String(i, DEC);
                list << key(i);
                list << value(i);

                parser.println(list.augment("%1: %2 = %3"));
            }
        }
    }

private:
    us8 tableSize;
    KeyValueTableDetail kvTable[24];

    bool compare(String string, char* buffer, us8 maxLength)
    {
        us8 length = 0;
        while(buffer[length] != 0 && length < maxLength) {
            length++;
        }

        if(string.length() != length) {
          return false;
        }

        for(us8 i = 0; i < length; i++) {
            if(string.charAt(i) != buffer[i]) {
                return false;
            }
        }
        return true;
    }

    // finds empty slot or matching key
    s8 findIndex(String key, bool includeEmpty = false)
    {
        if(key.length() > 0) {
            for(s8 i = 0; i < tableSize; i++) {
                if(compare(key, kvTable[i].key, keySize)) {
                    return i;
                }
                if(includeEmpty) {
                    if(kvTable[i].key[0] == 0) {
                        return i;
                    }
                }
            }
        }
        return -1;
    }
};

#endif // KEYVALUETABLE_H
