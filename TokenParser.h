#ifndef TOKENPARSER_H
#define TOKENPARSER_H

#include <WProgram.h>
#include <Stream.h>
#include "Core.h"
#include "Variant.h"

class TokenParser {
public:
    typedef enum {
        InvalidToken = 0,
        ValidToken,
        JsonToken
    } TokenType;

    TokenParser(Stream* stream, us8 bufferSize = 100) {
        myStream = stream;
        buffer = (us8*)malloc(bufferSize + 1);
        reset();
        save();
        length = 0;
        stopCharactorFound = false;
        type = InvalidToken;
    }
    bool scan(us8 stopCharactor = '\r') {
        if(stopCharactorFound) {
            stopCharactorFound = false;
            length = 0;
        }

        if(myStream->available() > 0) {
            us8 data = myStream->read();

            if(data != '\\') {
                buffer[length++] = data;
            }

            if(data == stopCharactor) {
                stopCharactorFound = true;
                buffer[length - 1] = ' ';
                reset();
                save();
                nextToken();
            }
        }
        return stopCharactorFound;
    }
    us8 getLength() {
        return length;
    }
    us8 getTail() {
        return tail;
    }
    us8 getHead() {
        return head;
    }
    us8 remaining() {
        return length - head;
    }
    bool isJson() {
        return (type == JsonToken) ? true : false;
    }
    void reset() {
        head = 0;
        tail = 0;
    }
    void save() {
        savedHead = head;
        savedTail = tail;
    }
    void restore() {
        head = savedHead;
        tail = savedTail;
    }
    // todo: add bounds checking
    bool startsWith(const char* string, bool caseSensitive = false) {
        us8 i = 0;
        us8 index = tail;
#ifdef DEBUG_TOKEN_PARSER
        print(string);
        print("=");
        print(toString());
        print("=>");
#endif
        while(string[i] != 0) {
            if(string[i] != '?') {
                if(!characterCompare(buffer[index], string[i], caseSensitive)) {
#ifdef DEBUG_TOKEN_PARSER
                    println("false");
#endif
                    return false;
                }
            }
            i++;
            index++;
        }
#ifdef DEBUG_TOKEN_PARSER
        println("true");
#endif
        return true;
    }
    // Compares an entire string for equality
    bool compare(const char* string, bool caseSensitive = false) {
        us8 i = 0;
        us8 index = tail;
#ifdef DEBUG_TOKEN_PARSER
        print(string);
        print("=");
        print(toString());
        print("=>");
#endif
        while(string[i] != 0) {
            if(string[i] != '?') {
                if(!characterCompare(buffer[index], string[i], caseSensitive)) {
#ifdef DEBUG_TOKEN_PARSER
                    println("false");
#endif
                    return false;
                }
            }
            i++;
            index++;
        }

        if((head - tail) != i) {
#ifdef DEBUG_TOKEN_PARSER
            println("false (head - tail) != i");
#endif
            return false;
        }
#ifdef DEBUG_TOKEN_PARSER
        println("true");
#endif
        return true;
    }
    // todo: enforce consecutive characters
    bool contains(const char* string, bool caseSensitive = false) {
        us8 i = 0;
        us8 index = tail;
#ifdef DEBUG_TOKEN_PARSER
        print(string);
        print("=");
        print(toString());
        print("=>");
#endif
        while(string[i] != 0 && index < length) {
            if(characterCompare(buffer[index++], string[i], caseSensitive)) {
                i++;
                if(string[i] == 0) {
#ifdef DEBUG_TOKEN_PARSER
                    println("true");
#endif
                    return true;
                }
            }
        }
#ifdef DEBUG_TOKEN_PARSER
        println("false");
#endif
        return false;
    }
    String toString() {
        if(head > tail) {
            char array[(head - tail) + 1];
            us8 index = 0;
            for(us8 i = tail; i < head; i++) {
                array[index++] = buffer[i];
            }
            array[index] = 0;

            return String((const char*)array);
        }
        return String();
    }
        bool nextToken(s8 index = -1) {
        if((0 < index) && (index < length)) {
            head = index;
        }

        tail = head;
        for(us8 i = tail; tail < length; i++) {
            s8 result = match(buffer[i], " \n\t{");
            if(result == -1) {
                tail = i;
                break;
            }
            else if(result >= 2) {
                head = length - 1;
                type = JsonToken;
                return true;
            }
        }

        head = tail;
        for(us8 i = head; head < length; i++) {
            s8 result = match(buffer[i], " \n\t");
            if(result != -1) {
                head = i;
                type = ValidToken;
                return true;
            }
        }

        type = InvalidToken;
        return false;
    }
    bool advanceTail(us8 advance) {
        if(advance < (head - tail)) {
            tail += advance;
            return true;
        }
        return false;
    }
    bool reverseHead(us8 reverse) {
        if(reverse < (head - tail)) {
            head = head - reverse;
            return true;
        }
        return false;
    }
    void advanceHead(us8 advance) {
        us8 limit = length - head;
        if(advance < limit) {
            head += advance;
        }
        else {
            head += (limit - 1);
        }
    }
    us8 hexCharToNibble(us8 c) {
        // make upper case
        if('a' <= c && c <= 'z') {
            c -= 0x20;
        }

        if('0' <= c && c <= '9') {
            c -= 0x30;
        }
        else if('A' <= c && c <= 'F') {
            c -= 0x37;
        }
        return c & 0xf;
    }
    Variant toVariant() {
        return Variant::fromString(toString());
    }
    void print(const String &string) {
        myStream->print(string);
    }
    void println(const String &string) {
        myStream->println(string);
    }
    static s8 match(us8 charactor, const char* delimiters = 0) {
        us8 i = 0;
        while(delimiters[i] != 0) {
            if(charactor == delimiters[i]) {
                return i;
            }
            i++;
        }
        return -1;
    }

private:
    inline bool characterCompare(us8 char1, us8 char2, bool caseSensitive = true) {
        if(char1 == char2) {
            return true;
        }

        if(!caseSensitive) {
            if(uppercase(char1) == uppercase(char2)) {
                return true;
            }
        }
        return false;
    }
    inline us8 uppercase(us8 x) {
        // make upper case
        if ('a' <= x && x <= 'z') {
            x -= 0x20;
        }
        return x;
    }

    Stream* myStream;
    us8* buffer;
    us8 length;
    us8 head;
    us8 tail;
    us8 savedHead;
    us8 savedTail;
    bool stopCharactorFound;
    TokenType type;
};

#endif // TOKENPARSER_H
