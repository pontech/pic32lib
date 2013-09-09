#ifndef _CORE_H_
#define _CORE_H_

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
//typedef bool            us1;

typedef signed char     s8;
typedef signed short    s16;
typedef signed int      s32;
//typedef signed long     s64;

typedef unsigned char   us8;
typedef unsigned short  us16;
typedef unsigned int    us32;
//typedef unsigned long   us64;

#if defined(__PIC32MX__)
    /* chipKIT-specific code goes here */
#warning "PIC32 Arduino: don't know how to make reset(),nop(),clear_wdt(),us1,fptr,fptr_us8"
#endif
//typedef enum        {false = 0, true} us1;
typedef void        (*fptr)(void);
typedef void        (*fptr_us8)(us8);

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

