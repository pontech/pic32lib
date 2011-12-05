#ifndef _CORE_H_
#define _CORE_H_

// todo: 2 re-impliment to integrate back into piclib
//#include "system/peripherals.h"

#if defined(BOOSTC)
    #warning "Compiling for Boost C"
    #include <system.h>

#elif defined(HITECH_PICC18)
    #warning "Compiling for Hitech PICC18"
    #include <htc.h>

#elif defined(MICROCHIP_C18)
    #warning "Compiling for Microchip C18"
    #include <p18cxxx.h>

#elif defined(MICROCHIP_C32)
    #warning "Compiling for Microchip C32"
    //#include <p18cxxx.h>

#elif defined(__PIC32MX__)
    #warning "Compiling for Microchip C32 (Arduino)"
    #include <plib.h>

#else
    #warning "Compiling for unknown compiler"
    #error
#endif

#if defined(BOOSTC) || defined(HITECH_PICC18) || defined(MICROCHIP_C18)
    typedef signed char     s8;
    typedef signed int      s16;
    typedef signed long     s32;

    typedef unsigned char   us8;
    typedef unsigned int    us16;
    typedef unsigned long   us32;

#elif defined(MICROCHIP_C32)
    typedef signed char     s8;
    typedef signed short    s16;
    typedef signed int      s32;
    typedef signed long     s64;

    typedef unsigned char   us8;
    typedef unsigned short  us16;
    typedef unsigned int    us32;
    typedef unsigned long   us64;
#elif defined(__PIC32MX__)
/*
CPU Type = 32MX795F512L
size of char = 1
size of short = 2
size of int = 4
size of long = 4
size of ull = 8
size of float = 4
size of double= 4
size of void* = 4
*/
//    typedef boolean         us1;

    typedef signed char     s8;
    typedef signed short    s16;
    typedef signed int      s32;
    typedef signed long     s64;

    typedef unsigned char   us8;
    typedef unsigned short  us16;
    typedef unsigned int    us32;
    typedef unsigned long   us64;
#endif

#if defined(BOOSTC)
    #define reset()     asm RESET;
    typedef bool        us1;
    typedef us16        fptr;
    typedef us16        fptr_us8;

#elif defined(HITECH_PICC18) || defined(MICROCHIP_C18) || defined(MICROCHIP_C32)
    #define reset()     RESET()
    #define nop()       asm("nop");
    #define clear_wdt()
    typedef enum        {false = 0, true} us1;
    typedef void        (*fptr)(void);
    typedef void        (*fptr_us8)(us8);

#elif defined(__PIC32MX__)
    #warning "PIC32 Arduino: don't know how to make reset(),nop(),clear_wdt(),us1,fptr,fptr_us8"
//    typedef enum        {false = 0, true} us1;
    typedef void        (*fptr)(void);
    typedef void        (*fptr_us8)(us8);
#else
    #warning "Compiling for unknown compiler"
    #error
#endif

typedef enum {
    data_out = 0,
    data_in  = 1
} data_direction;

typedef unsigned char bcd8;

typedef union {
    us8 value;
} uus8;

typedef struct {
    uus8 lo;
    uus8 hi;
} sus16;

typedef union {
    us16 value;
    sus16 parts;
} uus16;

typedef struct {
    uus16 lo;
    uus16 hi;
} sus32;

typedef union {
    us32 value;
    sus32 parts;
} uus32;

//uus32.value = 0x12345678
//uus32.parts.hi.value = 0x1234
//uus32.parts.lo.value = 0x5678

//uus32.parts.hi.parts.hi = 0x12
//uus32.parts.hi.parts.lo = 0x34
//uus32.parts.lo.parts.hi = 0x56
//uus32.parts.lo.parts.lo = 0x78

typedef struct {
    s16 value;
    s8 exp;
} e16;

typedef struct {
    s32 value;
    s8 exp;
} e32;

typedef uus32 lba;
typedef uus32 fat;

typedef struct packed_int {
  unsigned char b0 :1;
  unsigned char b1 :1;
  unsigned char b2 :1;
  unsigned char b3 :1;
  unsigned char b4 :1;
  unsigned char b5 :1;
  unsigned char b6 :1;
  unsigned char b7 :1;
  unsigned char b8 :1;
  unsigned char b9 :1;
  unsigned char b10 :1;
  unsigned char b11 :1;
  unsigned char b12 :1;
  unsigned char b13 :1;
  unsigned char b14 :1;
  unsigned char b15 :1;
} packed_int;

typedef union {
	unsigned int i;
	packed_int b;
} packed;

#endif
