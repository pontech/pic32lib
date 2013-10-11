#ifndef STEPANDDIRECTION_H
#define STEPANDDIRECTION_H

#include <WProgram.h>
#include "Core.h"
#include "TokenParser.h"

typedef bool us1;	// for some reason typedef for bool or boolean do not work

#define array_base_type 1

// Stepper Motors
// stepper count is used to let the stepper code know how many controllers to implement
#define MOTOR_COUNT				3
//#define MOTOR_ENABLE			1 // todo: 3 Define this if you want to use the motor enable pin (untested on pic32)

//////////////////////////////////////////////////////////////////////////////////////////
// PIC32 / chipKIT constants
//////////////////////////////////////////////////////////////////////////////////////////
const uint32_t delay_5us = CORE_TICK_RATE/1000*5;
const uint32_t delay_250us = CORE_TICK_RATE/1000*250;
const uint32_t delay_250ms = CORE_TICK_RATE/1000*250000;

// Change this number to control the fundamental unit of step delay
const uint32_t step_interrupt_period = delay_250us;

// OUTPUT: Step, Direction and Enable pins
#define step1_stp           led2
#define step1_dir           led1
#define step1_enable        c0p2
#define step2_stp           c1p0
#define step2_dir           c1p1
#define step2_enable        c1p2
#define step3_stp           c2p0
#define step3_dir           c2p1
#define step3_enable        c2p2

// INPUTS: +/- direction limit switches
#define step1_limit_minus   c3p0
#define step2_limit_minus   c3p1
#define step3_limit_minus   c3p2
#define step1_limit_plus    c3p0
#define step2_limit_plus    c3p1
#define step3_limit_plus    c3p2

us1 stepper_timer_enable = false; // this used to be to turn PIC18 timer interrupt on and off

//////////////////////////////////////////////////////////////////////////////////////////
// End of PIC32 / chipKIT constants
//////////////////////////////////////////////////////////////////////////////////////////

#ifdef GONE // from plib project.h file
#define setup_steppers() \
    step1_stp_tris = 0; \
    step2_stp_tris = 0; \
    step3_stp_tris = 0; \
    step1_dir_tris = 0; \
    step2_dir_tris = 0; \
    step3_dir_tris = 0; \
#ifdef MOTOR_ENABLE \
    step1_enable_tris = 0; \
    step2_enable_tris = 0; \
    step3_enable_tris = 0; \
#endif \
    step1_limit_minus_tris = 1; \
    step2_limit_minus_tris = 1; \
    step3_limit_minus_tris = 1; \
    step1_limit_plus_tris = 1; \
    step2_limit_plus_tris = 1; \
    step3_limit_plus_tris = 1; \
    step1_enable = 1; \
    step2_enable = 1; \
    step3_enable = 1; \
    step1_dpot_cs = 1; \
    step2_dpot_cs = 1; \
    step3_dpot_cs = 1; \
    step1_dpot_cs_tris = 0; \
    step2_dpot_cs_tris = 0; \
    step3_dpot_cs_tris = 0;


#define stepper_timer_enable        pl_T1CON_TMR1ON
#define stepper_timer_int_enable    pl_PIE1_TMR1IE
#define stepper_timer_int_flag      pl_PIR1_TMR1IF
#define stepper_timer_msb           pl_TMR1H
#define stepper_timer_lsb           pl_TMR1L

#define step1_stp_tris          pl_TRISC_1
#define step1_dir_tris          pl_TRISG_4
#define step2_stp_tris          pl_TRISA_4
#define step2_dir_tris          pl_TRISG_3
#define step3_stp_tris          pl_TRISF_7
#define step3_dir_tris          pl_TRISE_0

#ifdef MOTOR_ENABLE
#define step1_enable_tris       pl_TRISE_1
#define step2_enable_tris       pl_TRISD_6
#define step3_enable_tris       pl_TRISD_5
#endif

#define step1_limit_minus_tris  pl_TRISH_0
#define step2_limit_minus_tris  pl_TRISH_1
#define step3_limit_minus_tris  pl_TRISH_2
#define step1_limit_plus_tris   pl_TRISH_4
#define step2_limit_plus_tris   pl_TRISH_5
#define step3_limit_plus_tris   pl_TRISH_6

#define step1_dpot_cs_tris      pl_TRISH_3
#define step2_dpot_cs_tris      pl_TRISH_7
#define step3_dpot_cs_tris      pl_TRISJ_4

#define step1_stp           pl_PORTC_1
#define step1_dir           pl_PORTG_4
#define step1_enable        pl_PORTE_1
#define step2_stp           pl_PORTA_4
#define step2_dir           pl_PORTG_3
#define step2_enable        pl_PORTD_6
#define step3_stp           pl_PORTF_7
#define step3_dir           pl_PORTE_0
#define step3_enable        pl_PORTD_5

#define step1_limit_minus   pl_PORTH_0
#define step2_limit_minus   pl_PORTH_1
#define step3_limit_minus   pl_PORTH_2
#define step1_limit_plus    pl_PORTH_4
#define step2_limit_plus    pl_PORTH_5
#define step3_limit_plus    pl_PORTH_6

#define step1_dpot_cs       pl_PORTH_3
#define step2_dpot_cs       pl_PORTH_7
#define step3_dpot_cs       pl_PORTJ_4

#endif

typedef enum {
    motor_reverse = 0,
    motor_forward = 1
} motor_direction;

// todo: 2 temp in setDirection(void) is defined as uus32, should be uss32 (union of s32) (may be okay, producing compiler warning)
// todo: 2 parser will fail with spaces between command and value
// todo: 3 parsing hex values requires leading zeros and to know the size of the expected value to parse
// todo: 2 if sitting at position 2147483647 and an h+ is commanded the motor moves is the wrong direction
// todo: 2 stp0.hm2147483647 then stp0.ii1 doesn't work
// todo: 1 Check to see if the above bug is in the STP100 code (probably)
// todo: 3 if you hammer the board with ii commands you can confuse its destination, especially when changing directions, also accel and decel calc confusion

// todo: 3 BDn N = 0 to 255 Board Select
// MIn N = -2147483648 to 2147483647 Move Immediately to an absolute location
// todo: 3 MCn N = -2147483648 to 2147483647 Move on Cue to an absolute location (must be followed by CU command)
// IIn N = -2147483648 to 2147483647 Move Immediately relative to current position
// todo: 3 ICn N = -2147483648 to 2147483647 Move on Cue relative to current position cue (must be followed by CU command)
// todo: 3 CU None Cue a move
// RC None Read Current Position
// RD None Read Destination Position
// RT None Read delTa Position (DestinationPosition - CurrentPosition)
// RX None Read Direction Sign (Returns +, - or 0)
// HMn n = -2147483648 to 2147483647 Set new home position, current position = destination position = n
// HI None Halt Immediately, set destination position to current position and do not decelerate
// H0 (H - Zero) None Halt, Set speed to 0 and decelerate
// H+ None Move Clockwise forever
// H- None Move Counter-Clockwise forever
// SP None Stepper Always powered
// SO None Stepper Off when not moving (Remove power from stepper, no holding torque)
// todo: 4 SH None Set Half Step mode
// todo: 4 SF None Set Full Step mode
// todo: 4 SW None Set Wave Step mode
// todo: 3 TCn n = 3, 5, 6, or 8 Test if pin clear, on clear execute H0
// todo: 3 TSn n = 3, 5, 6, or 8 Test if pin set, on set execute H0
// todo: 3 TTn n = 0 or 1 Set Test Mode
// todo: 3 RSA None Read Step Acceleration/Deceleration Factor
// RSD None Read Step Delay
// todo: 3 RSM None Read Minimum Step Delay Factor
// todo: 3 SAn n = 1 - 255 Acceleration/Deceleration Factor
// SDn n = 6 to 65535 Step Delay (n x 2.04μs)
// todo: 3 SMn n = 0 to (65535 - SD) Minimum Step Delay Factor (Start Step Delay = StepDelay + MinimumStepDelayFactor) if = 0 then no acceleration

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// The following commands would be part of different sub system in new architecture
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// todo: 3 PCn N = 3, 5, 6, or 8 Pin clear on 10-pin header
// todo: 3 PSn N = 3, 5, 6, or 8 Pin set on 10-pin header
// todo: 3 RPn N = 3, 5, 6, 8 or None Read Pin on 10-pin header (If n is not specified, a four bit value is returned representing all pins)
// todo: 3 ADn N = 1 or 2 Get A/D value, the board will return a value between "0" to "255" which represents a voltage between 0 to 5 Volts.
// todo: 3 WRm n M = 0 to 255 N = 0 to 255 Write to internal RAM, m is the memory location n is the value to write
// todo: 3 WEm n M = 0 to 8190 N = 0 to 255 Write to external EEPROM, m is the memory location n is the value to write
// todo: 3 WSS None Write System Settings to EEPROM (current value of SD, SM, SA, SH, SF, SW, SP are stored to EEPROM)
// todo: 3 RRm M = 0 to 255 Read the contain of internal RAM, m is the memory location to read
// todo: 3 REm M = 0 to 8190 Read the contain of external EEPROM, m is the memory location to read
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

volatile s32 motor_decel_point[MOTOR_COUNT];
volatile s32 motor_destination[MOTOR_COUNT];
volatile s32 motor_save_dest[MOTOR_COUNT];
volatile s32 motor_position[MOTOR_COUNT];
volatile s32 motor_half_way[MOTOR_COUNT];

//us16 stp_delay_current = 0xC568;

//us1 fRunning;
//us1 fNewline;
//us1 fSequence;
//us1 fBoardEn;
//us1 fBoardZero;

//us1 fCue                    = false;
volatile us1 motor_fAccelerate[MOTOR_COUNT]     = {true,true,true};
volatile us1 motor_fMoving[MOTOR_COUNT]         = {false,false,false};
volatile us1 motor_fAtSpeed[MOTOR_COUNT]        = {false,false,false};
volatile us1 motor_fKeepOn[MOTOR_COUNT]                  = {false,false,false};
volatile us1 motor_fContinuous[MOTOR_COUNT]              = {false,false,false};
volatile us1 motor_fChangeDirection[MOTOR_COUNT]         = {false,false,false};
volatile us1 motor_fForward[MOTOR_COUNT]                 = {true,true,true};

//volatile uus16 motor_stp_delay_fixed = 0xE890; // 1 second for 2000 steps
//volatile uus16 motor_stp_delay_fixed = 0xFDA8; // 1 second for 20000 steps
//volatile uus16 motor_stp_delay_fixed = 0xFF6A; // 1 second for 80000 steps
volatile uus16 motor_stp_delay_fixed = {0xFF88}; // 1 second for 100000 steps
//volatile uus16 motor_stp_delay_fixed = 0xFFC4; // 1 second for 200000 steps
//volatile uus16 motor_stp_delay_fixed = 0xFFD8; // 1 second for 300000 steps
//volatile uus16 motor_stp_delay_fixed = 0xFFE8; // 1 second for 500000 steps
//volatile uus16 motor_stp_delay_fixed = 0xFFF4; // 1 second for 1000000 steps
volatile uus16 motor_accel_count     = {0x0000};
volatile uus16 motor_accel_mod       = {0x0010};

volatile uus16 motor_stp_delay_accel[MOTOR_COUNT]   = {0x0001,0x0001,0x0001};
volatile uus16 motor_stp_delay_minimum[MOTOR_COUNT] = {0xFFC0,0xFFC0,0xFFC0}; // 1 second for 2000 steps
volatile uus16 motor_stp_delay_target[MOTOR_COUNT]  = {0xFFC0,0xFFC0,0xFFC0}; // 1 second for 2000 steps
volatile uus16 motor_stp_delay_current[MOTOR_COUNT] = {0xFFC0,0xFFC0,0xFFC0}; // 1 second for 2000 steps
volatile uus16 motor_stp_delay_counter[MOTOR_COUNT] = {0x0000,0x0000,0x0000}; // 1 second for 2000 steps
volatile uus16 motor_stp_delay[MOTOR_COUNT]         = {0xFFF8,0xFFF8,0xFFF8};

///////////////////////////////////////////////////////////////////////////////////////////////////////
// This code cannot be generalized into an array due to reading and setting specific pins per motor
///////////////////////////////////////////////////////////////////////////////////////////////////////
void step_motor(us8 motor) {
	switch(motor) {
#if MOTOR_COUNT >= 1
		case 0:
			if (digitalRead(step1_dir) == (bool)motor_forward) {
				if( ~digitalRead(step1_limit_plus) ) {
					digitalWrite(step1_stp, 0);
					motor_position[motor]++;
					digitalWrite(step1_stp, 1);
				}
			}
			else {
				if( ~digitalRead(step1_limit_minus) ) {
					digitalWrite(step1_stp, 0);
					motor_position[motor]--;
					digitalWrite(step1_stp, 1);
				}
			}
			break;
#endif
#if MOTOR_COUNT >= 2
		case 1:
			if (digitalRead(step2_dir) == (bool)motor_forward) {
				if( ~digitalRead(step2_limit_plus) ) {
					digitalWrite(step2_stp, 0);
					motor_position[motor]++;
					digitalWrite(step2_stp, 1);
				}
			}
			else {
				if( ~digitalRead(step2_limit_minus) ) {
					digitalWrite(step2_stp, 0);
					motor_position[motor]--;
					digitalWrite(step2_stp, 1);
				}
			}
			break;
#endif
#if MOTOR_COUNT >= 3
		case 2:
			if (digitalRead(step3_dir) == (bool)motor_forward) {
				if( ~digitalRead(step3_limit_plus) ) {
					digitalWrite(step3_stp, 0);
					motor_position[motor]++;
					digitalWrite(step3_stp, 1);
				}
			}
			else {
				if( ~digitalRead(step3_limit_minus) ) {
					digitalWrite(step3_stp, 0);
					motor_position[motor]--;
					digitalWrite(step3_stp, 1);
				}
			}
			break;
#endif
	}
}
uint32_t stepper_interrupt(uint32_t currentTime) {
	us8 motor = 1;
#ifdef NEED_TO_PORT
	if (stepper_timer_int_flag) {
#endif
		motor_accel_count.value++;
		if ( motor_accel_count.value == motor_accel_mod.value ) {
			motor_accel_count.value = 0;
		}

		//digitalWrite(led1, !digitalRead(led1));

		us8 status = 0;
		for( motor = 0; motor < MOTOR_COUNT; motor++ ) {
			if ((motor_position[motor] == motor_destination[motor]) && (!motor_fContinuous[motor] || motor_fChangeDirection[motor])) {
				motor_fMoving[motor] = false;
				status = status | (1 << motor);
			}
			else {
				motor_stp_delay_counter[motor].value++;
				if( motor_stp_delay_counter[motor].value == 0 )  // then step
				{
					step_motor(motor);
					motor_stp_delay_counter[motor].value = motor_stp_delay_current[motor].value;

					if (!motor_fAtSpeed[motor]) {
						if (motor_fAccelerate[motor]) {
							motor_decel_point[motor]++;
							if( motor_decel_point[motor] >= motor_half_way[motor] ) {
								// fShortMove
								motor_fAccelerate[motor] = false;
								motor_stp_delay_target[motor].value = motor_stp_delay_minimum[motor].value;
							}
						}
						else {
							motor_decel_point[motor]--;
						}
					}
				}

				if ( motor_accel_count.value == 0x0000 ) {
					if (!motor_fAtSpeed[motor]) {
						if (motor_fAccelerate[motor]) {
							motor_stp_delay_current[motor].value += motor_stp_delay_accel[motor].value;
							if (motor_stp_delay_current[motor].value >= motor_stp_delay_target[motor].value) {
								motor_stp_delay_current[motor].value = motor_stp_delay_target[motor].value;
								motor_stp_delay_target[motor].value = motor_stp_delay_minimum[motor].value;
								motor_fAccelerate[motor] = false;
								motor_fAtSpeed[motor] = true;
							}
						}
						else {
							motor_stp_delay_current[motor].value -= motor_stp_delay_accel[motor].value;
							if (motor_stp_delay_current[motor].value <= motor_stp_delay_target[motor].value) {
								motor_stp_delay_current[motor].value = motor_stp_delay_target[motor].value;
								motor_fAccelerate[motor] = true;
								motor_fAtSpeed[motor] = true;
							}
						}
					}
				}
			}
		}

#ifdef NEED_TO_PORT
		stepper_timer_msb = motor_stp_delay_fixed.parts.hi.value;
		stepper_timer_lsb = motor_stp_delay_fixed.parts.lo.value;
#endif

		if (status == 0x07) {
			stepper_timer_enable = 0;
		}
#ifdef NEED_TO_PORT
		stepper_timer_int_flag = 0;
#endif

#ifdef NEED_TO_PORT
	}
#endif
	return (currentTime + step_interrupt_period);
}

class StepAndDirection {
public:
    StepAndDirection() {
	    attachCoreTimerService(stepper_interrupt);
    }
	void print_ok(TokenParser &parser, us8 status) {
		parser.print("OK");
		if(status > 0) {
			parser.print(".");
			//print_dec_us8(status);
		}
	}
	void print_nok(TokenParser &parser, us8 status) {
		parser.print("NOK");
		if(status > 0) {
			parser.print(".");
			//print_dec_us8(status);
		}
	}
	void print_cr(TokenParser &parser) {
	  parser.print("\r\n");
	}
	void print_dec_s32(TokenParser &parser, s32 value) {
	  parser.print(String(value));
	}
	void print_us16(TokenParser &parser, us16 value) {
	  parser.print(String(value));
	}
	void print_us32(TokenParser &parser, us32 value) {
	  parser.print(String(value));
	}
	void print_byte(TokenParser &parser, us8 value) {
	  parser.print(String(value));
	}
	void stepper_powered(us8 motor, us1 powered) {
#ifdef MOTOR_ENABLE
		switch(motor) {
#if MOTOR_COUNT >= 1
			case 0:
				digitalWrite(step1_enable, powered);
				break;
#endif
#if MOTOR_COUNT >= 2
			case 1:
				digitalWrite(step2_enable, powered);
				break;
#endif
#if MOTOR_COUNT >= 3
			case 2:
				digitalWrite(step3_enable, powered);
				break;
#endif
		}
#endif
	}
	void stepper_direction_set(us8 motor, motor_direction direction) {
		switch(motor) {
	#if MOTOR_COUNT >= 1
			case 0:
				digitalWrite(step1_dir,direction); // step1_dir = direction;
				break;
	#endif
	#if MOTOR_COUNT >= 2
			case 1:
				digitalWrite(step2_dir,direction); // step2_dir = direction;
				break;
	#endif
	#if MOTOR_COUNT >= 3
			case 2:
				digitalWrite(step3_dir,direction); // step3_dir = direction;
				break;
	#endif
		}
	}
	motor_direction stepper_direction_get(us8 motor) {
		motor_direction direction;
		switch(motor) {
	#if MOTOR_COUNT >= 1
			case 0:
				direction = (motor_direction)digitalRead(step1_dir);
				break;
	#endif
	#if MOTOR_COUNT >= 2
			case 1:
				direction = (motor_direction)digitalRead(step2_dir);
				break;
	#endif
	#if MOTOR_COUNT >= 3
			case 2:
				direction = (motor_direction)digitalRead(step3_dir);
				break;
	#endif
		}
		return direction;
	}
	us1 stepper_limit_minus_get(us8 motor) {
		us1 limit;
		switch(motor) {
	#if MOTOR_COUNT >= 1
			case 0:
				limit = (us1)digitalRead(step1_limit_minus);
				break;
	#endif
	#if MOTOR_COUNT >= 2
			case 1:
				limit = (us1)digitalRead(step2_limit_minus);
				break;
	#endif
	#if MOTOR_COUNT >= 3
			case 2:
				limit = (us1)digitalRead(step3_limit_minus);
				break;
	#endif
		}
		return !limit;
	}
	us1 stepper_limit_plus_get(us8 motor) {
		us1 limit;
		switch(motor) {
	#if MOTOR_COUNT >= 1
			case 0:
				limit = (us1)digitalRead(step1_limit_plus);
				break;
	#endif
	#if MOTOR_COUNT >= 2
			case 1:
				limit = (us1)digitalRead(step2_limit_plus);
				break;
	#endif
	#if MOTOR_COUNT >= 3
			case 2:
				limit = (us1)digitalRead(step3_limit_plus);
				break;
	#endif
		}
		return !limit;
	}
	us16 timer_from_freq(us16 freq_Hz) {
		// Don't forget to divide by 2, once for going high, once for going low...

		// For Timer 0 on the PIC18F67J50 with divid by 64 prescaler
		// 48MHz / 4 = 12MHz / 64 = 187.5kHz / 2 = 93.75kHz
		//       / 4(Fosc/4) / 64(prescale)  / 2 2 changes per cycle)
		// Min: 93.75kHz * (1) = 93.75kHz
		// Max: 93.75kHz * (2^16) = 1.4Hz
		// Freq = 93.75kHz * (0x10000 - tmr0)
		// tmr0 = 0x10000 - ( 93.75kHz / (freq_Hz) )

		if (freq_Hz != 0) {
			//return (us16)(0x10000L - (93750L / (long)freq_Hz));
			return (us16)(0x10000L - (6000000L / (long)freq_Hz));
		}
		// 0xb1e0 = 100Hz
		return 0; //0xd8f0;
	}
	/////////////////////////////////////////////////////////////////////////////
	// decelStop
	/////////////////////////////////////////////////////////////////////////////
	void decelStop(us8 motor) {	// H0
		us1 step_dir = 0;
		noInterrupts(); //stepper_timer_int_enable = 0;
		motor_fAtSpeed[motor] = false;
		motor_fAccelerate[motor] = false;
		motor_stp_delay_target[motor].value = motor_stp_delay_minimum[motor].value;
		if (stepper_direction_get(motor) == motor_forward)
			motor_destination[motor] = motor_position[motor] + motor_decel_point[motor];
		else
			motor_destination[motor] = motor_position[motor] - motor_decel_point[motor];
		interrupts(); //stepper_timer_int_enable = 1;
	}
	void debugStepperState(us8 motor) {
	#ifdef STP_DEBUG
		print("motor_fChangeDirection="); print_us8(motor_fChangeDirection[motor]); print_cr();
		print("motor_fContinuous="); print_us8(motor_fContinuous[motor]); print_cr();
		print("motor_fAccelerate="); print_us8(motor_fAccelerate[motor]); print_cr();
		print("motor_fAtSpeed="); print_us8(motor_fAtSpeed[motor]); print_cr();
		print("motor_fMoving="); print_us8(motor_fMoving[motor]); print_cr();
		print("stepper_direction_get="); print_us8(stepper_direction_get(motor)); print_cr();

		// u16
		print("motor_stp_delay_minimum="); print_us16(motor_stp_delay_minimum[motor].value); print_cr();
		print("motor_stp_delay_current="); print_us16(motor_stp_delay_current[motor].value); print_cr();
		print("motor_stp_delay_counter="); print_us16(motor_stp_delay_counter[motor].value); print_cr();
		print("motor_stp_delay_target="); print_us16(motor_stp_delay_target[motor].value); print_cr();
		print("motor_stp_delay="); print_us16(motor_stp_delay[motor].value); print_cr();

		// s32
		print("motor_decel_point="); print_us32(motor_decel_point[motor]); print_cr();
		print("motor_destination="); print_us32(motor_destination[motor]); print_cr();
		print("motor_save_dest="); print_us32(motor_save_dest[motor]); print_cr();
		print("motor_position="); print_us32(motor_position[motor]); print_cr();
		print("motor_half_way="); print_us32(motor_half_way[motor]); print_cr();
	#endif
	}
	/////////////////////////////////////////////////////////////////////////////
	// setDirection
	/////////////////////////////////////////////////////////////////////////////
	void setDirection(us8 motor) {
		s32 local_decel;
		s32 local_des;
		s32 local_pos;
		s32 temp;

#ifdef STP_DEBUG
		print("sd:");
#endif
		/////////////////////////////////////////////////////////////////////////////
		// Do we need to go forward or backwards?
		/////////////////////////////////////////////////////////////////////////////
		if (motor_destination[motor] > motor_position[motor]) {
			motor_fForward[motor] = motor_forward;
#ifdef STP_DEBUG
			print("fwd");
#endif
		}
		else {
			motor_fForward[motor] = motor_reverse;
#ifdef STP_DEBUG
			print("rev");
#endif
		}
#ifdef STP_DEBUG
		print(":");
#endif

		motor_half_way[motor] = 0x7ffffff;
		motor_fChangeDirection[motor] = false;
		motor_fContinuous[motor] = false;
		if (!motor_fMoving[motor])
		{
			motor_decel_point[motor] = 0;

#ifdef STP_DEBUG
			print("Not Moving:");
#endif
			motor_fMoving[motor] = true;
			if (motor_stp_delay_minimum[motor].value <= motor_stp_delay[motor].value) {
#ifdef STP_DEBUG
				print("sm<=sd:");
#endif
				motor_stp_delay_current[motor].value = motor_stp_delay_minimum[motor].value;
			}
			else {
#ifdef STP_DEBUG
				print("sm >sd:");
#endif
				motor_stp_delay_current[motor].value = 0;
			}
			motor_stp_delay_target[motor].value = motor_stp_delay[motor].value;
			motor_fAccelerate[motor] = true;
			motor_fAtSpeed[motor] = false;
		}
		else
		{
#ifdef STP_DEBUG
			print("Is  Moving:");
#endif
			if ((!motor_fAccelerate[motor]) && (!motor_fAtSpeed[motor])) // Decelerating
			{
#ifdef STP_DEBUG
				print("De:");
#endif
				motor_stp_delay_target[motor].value = motor_stp_delay[motor].value;
				motor_fAccelerate[motor] = true;
			}
			motor_direction step_dir = stepper_direction_get(motor);
			if ((step_dir == motor_forward && !motor_fForward[motor]) || ( (step_dir == motor_reverse) && motor_fForward[motor])) // Change Direction
			{
#ifdef STP_DEBUG
				print("ChDir:");
#endif
				motor_fChangeDirection[motor] = true;
				motor_save_dest[motor] = motor_destination[motor];
				decelStop(motor);
			}
		}

		if (!motor_fChangeDirection[motor])
		{
#ifdef STP_DEBUG
			print("!ChDir:");
			print_cr();
#endif
			// Bug causing values
			//SA=10 (acceleration factor) slopeSpeed
			//SD=700 (step delay)         stepDelay
			//SM=3000 (start step delay)  minSpeed

			////////////////////////////////////////////////////////////////////////
			// Determine half way point
			////////////////////////////////////////////////////////////////////////
			noInterrupts(); //stepper_timer_int_enable = 0;
			local_des = motor_destination[motor];
			local_pos = motor_position[motor];
			local_decel = motor_decel_point[motor];
			interrupts(); //stepper_timer_int_enable = 1;

			temp = local_des - local_pos;
#ifdef STP_DEBUG
			print("temp="); print_us32(temp); print_cr();
#endif

			if(temp < 0) {
				temp = -temp;
				if(temp == 0x80000000) {
					temp = 0x7fffffff;
				}
			}
#ifdef STP_DEBUG
			print("temp="); print_us32(temp); print_cr();
#endif
			temp /= 2;
#ifdef STP_DEBUG
			print("temp="); print_us32(temp); print_cr();
#endif
			motor_half_way[motor] = temp + local_decel;

			if (motor_fForward[motor])
				stepper_direction_set(motor, motor_forward);
			else
				stepper_direction_set(motor, motor_reverse);

			stepper_powered(motor, true);

			motor_stp_delay_counter[motor].value = motor_stp_delay_current[motor].value;

			debugStepperState(motor);

#ifdef STP_DEBUG
		print("local_des="); print_us32(local_des); print_cr();
		print("local_pos="); print_us32(local_pos); print_cr();
		print("local_decel="); print_us32(local_decel); print_cr();
		print("temp="); print_us32(temp); print_cr();
#endif
		interrupts(); //stepper_timer_int_enable = 1;
		stepper_timer_enable = 1;
		}
	}
	void move_immediate(us8 motor, s32 position) {
		noInterrupts(); //stepper_timer_int_enable = 0;
		motor_destination[motor] = position;
		setDirection(motor);
		interrupts(); //stepper_timer_int_enable = 1;
		stepper_timer_enable = 1;
	}
	void increment_immediate(us8 motor, s32 offset) {
		noInterrupts(); //stepper_timer_int_enable = 0;
		if( motor_fChangeDirection[motor] == true ) {
			motor_save_dest[motor] = motor_save_dest[motor] + offset;
		}
		else {
			motor_destination[motor] = motor_destination[motor] + offset;
		}
		setDirection(motor);
		interrupts(); //stepper_timer_int_enable = 1;
		stepper_timer_enable = 1;
	}
	/////////////////////////////////////////////////////////////////////////////
	// HI / absStop
	/////////////////////////////////////////////////////////////////////////////
	void halt_immediate(us8 motor) {
		noInterrupts(); //stepper_timer_int_enable = 0;
		motor_destination[motor] = motor_position[motor];
		motor_fContinuous[motor] = false;
		interrupts(); //stepper_timer_int_enable = 1;
		stepper_timer_enable = 1;
	}
	void home(us8 motor, s32 position) {
		noInterrupts(); //stepper_timer_int_enable = 0;
		motor_position[motor] = position;
		motor_destination[motor] = position;
		interrupts(); //stepper_timer_int_enable = 1;
		stepper_timer_enable = 1;
	}
	void stepper_init() {
		us8 motor;
	#ifdef STP_DEBUG
		print("stepper_init\r");
	#endif

		for( motor = 0; motor < MOTOR_COUNT; motor++ ){
	#ifdef STP_DEBUG
			print("motor_");
			print_us8(motor);
			print_cr();
	#endif
			motor_decel_point[motor]    = 0;
			motor_destination[motor]    = 0;
			motor_save_dest[motor]      = 0;
			motor_position[motor]       = 0;
			motor_half_way[motor]       = 0;

			motor_fAccelerate[motor]        = true;
			motor_fMoving[motor]            = false;
			motor_fAtSpeed[motor]           = false;
			motor_fKeepOn[motor]            = false;
			motor_fContinuous[motor]        = false;
			motor_fChangeDirection[motor]   = false;
			motor_fForward[motor]           = true;

			motor_stp_delay_accel[motor].value      = 0x0001;
			motor_stp_delay_minimum[motor].value    = 0xFF00;
			motor_stp_delay_target[motor].value     = 0xFF00;
			motor_stp_delay_current[motor].value    = 0xFF00;
			motor_stp_delay_counter[motor].value    = 0x0000;
			motor_stp_delay[motor].value            = 0xFFF8;
		}

#ifdef NEED_TO_PORT
		stepper_timer_msb = motor_stp_delay_fixed.parts.hi.value;
		stepper_timer_lsb = motor_stp_delay_fixed.parts.lo.value;
#endif
	}
	/////////////////////////////////////////////////////////////////////////////
	// checkPins
	/////////////////////////////////////////////////////////////////////////////
	void checkPins(void) {
		//digitalWrite(led2,!digitalRead(led2); //toggle pLED
		us1 fCheckTypeNew = true;
		us8 motor = 0;
		s32 local_des;
		s32 local_pos;

		for( motor = 0; motor < MOTOR_COUNT; motor++ ) {
			noInterrupts(); //stepper_timer_int_enable = 0;
			local_des = motor_destination[motor];
			local_pos = motor_position[motor];
			interrupts(); //stepper_timer_int_enable = 1;

			if (stepper_limit_minus_get(motor)) {
				if(fCheckTypeNew) {
					if( local_des < local_pos ) { // RX = '-'
						halt_immediate(motor);
					}
				}
				else {
					halt_immediate(motor);
				}
			}
			if (stepper_limit_plus_get(motor)) {
				if(fCheckTypeNew) {
					if( local_des > local_pos ) { // RX = '+'
						halt_immediate(motor);
					}
				}
				else {
					halt_immediate(motor);
				}
			}
		}
	}
	void loop(TokenParser &parser) {
		us8 motor = 0;
		s32 step_delta;
		s32 decel_local;

	/*
		if( in2_asserted ) {
			step1_dir = 0;
			step1_stp = 0;
			delay_us(1);
			step1_stp = 1;
			delay_us(200);
		}
		if( in3_asserted ) {
			step1_dir = 1;
			step1_stp = 0;
			delay_us(1);
			step1_stp = 1;
			delay_us(200);
		}
	*/

		///////////////////////////////////////////////////////////
		// Process digital in puts if needed
		///////////////////////////////////////////////////////////
		checkPins();
	/*
		parser.print("DES: ");
		noInterrupts(); //stepper_timer_int_enable = 0;
		step_delta = (us32)motor_destination[motor];
		interrupts(); //stepper_timer_int_enable = 1;

		print_dec_s32(parser, (us32)step_delta);
		parser.print(" ");

		print_cr(parser);
	*/
		for ( motor = 0; motor < MOTOR_COUNT; motor++ ) {
			if (!motor_fMoving[motor]) {
				// Should we fKeepOn the motor windings?
				stepper_powered(motor, motor_fKeepOn[motor]);

				if (motor_fChangeDirection[motor]) {
					motor_fChangeDirection[motor] = false;
					noInterrupts(); //stepper_timer_int_enable = 0;
					motor_destination[motor] = motor_save_dest[motor];
					interrupts(); //stepper_timer_int_enable = 1;
					if (motor_fContinuous[motor]) {
						setDirection(motor);
						motor_fContinuous[motor] = true;
					}
					else {
						setDirection(motor);
					}
				}
			}
			else {
				noInterrupts(); //stepper_timer_int_enable = 0;
				step_delta = motor_destination[motor] - motor_position[motor];
				decel_local = motor_decel_point[motor];
				interrupts(); //stepper_timer_int_enable = 1;
				if(step_delta < 0) step_delta = -step_delta;
				if (decel_local > step_delta) {
					if (!motor_fContinuous[motor]) {
						if (motor_fAtSpeed[motor])
							motor_fAtSpeed[motor] = false;
					}
				}
			}
		}
	}
	/////////////////////////////////////////////////////////////////////////////
	// readDirectionSign
	/////////////////////////////////////////////////////////////////////////////
	us8 readDirectionSign(us8 motor) {
		s32 destination;
		s32 position;
		destination = motor_destination[motor];
		position    = motor_position[motor];

		if(destination > position) {
			return '+';
		}
		else if(destination < position) {
			return '-';
		}
		else {
			return '0';
		}
	}
    void command(TokenParser &parser) {

		if(parser.startsWith("STP?")) {
			parser.advanceTail(3);
			us8 motor = parser.toVariant().toInt() - array_base_type;
			
			if( !(motor < MOTOR_COUNT) ) {
				print_nok(parser, 0);
				goto done;
			}
            parser.nextToken();
			if (parser.startsWith("V?")) {
				parser.print("STP100 V2.3");
				goto done;
			}
			else if (parser.startsWith("H+")) {
				parser.println(parser.toString());
				move_immediate(motor, 0x7fffffff);
				motor_fContinuous[motor] = true;
				goto ok;
			}
			else if (parser.startsWith("H-")) {
				parser.println(parser.toString());
				move_immediate(motor, -0x80000000);
				motor_fContinuous[motor] = true;
				goto ok;
			}
			else if (parser.startsWith("H0")) {
				parser.println(parser.toString());
				decelStop(motor);
				motor_fContinuous[motor] = false;
				goto ok;
			}
			else if (parser.startsWith("HA")) {
				parser.println(parser.toString());
				us8 i;
				for(i = 0; i < MOTOR_COUNT; i++) {
					halt_immediate(i);
				}
				goto ok;
			}
			else if (parser.startsWith("HI")) {
				parser.println(parser.toString());
				halt_immediate(motor);
				goto ok;
			}
			else if (parser.startsWith("HM?")) {
				parser.println(parser.toString());
				parser.advanceTail(2);
				s32 value = parser.toVariant().toInt();
				home(motor, value);
				goto ok;
			}
			else if (parser.startsWith("MI?")) {
				parser.println(parser.toString());
				parser.advanceTail(2);
				s32 value = parser.toVariant().toInt();
				parser.println(String(value));
				move_immediate(motor, value);
				goto ok;
			}
			else if (parser.startsWith("II?")) {
				parser.println(parser.toString());
				parser.advanceTail(2);
				s32 value = parser.toVariant().toInt();
				increment_immediate(motor, value);
				goto ok;
			}
			else if (parser.startsWith("RSD")) {
				parser.println(parser.toString());
				print_dec_s32(parser, motor_stp_delay[motor].value);
				goto done;
			}
			else if (parser.startsWith("SD?")) {
				parser.println(parser.toString());
				parser.advanceTail(2);
				us16 temp = parser.toVariant().toInt();
				noInterrupts(); //stepper_timer_int_enable = 0;
				motor_stp_delay[motor].value = temp;
				interrupts(); //stepper_timer_int_enable = 1;
				goto ok;
			}
			else if (parser.startsWith("RSM")) {
				parser.println(parser.toString());
				print_dec_s32(parser, motor_stp_delay_minimum[motor].value);
				goto done;
			}
			else if (parser.startsWith("SM")) {
				parser.println(parser.toString());
				parser.advanceTail(2);
				us16 temp = parser.toVariant().toInt();
				noInterrupts(); //stepper_timer_int_enable = 0;
				motor_stp_delay_minimum[motor].value = temp;
				interrupts(); //stepper_timer_int_enable = 1;
				goto ok;
			}
			else if (parser.startsWith("RSA")) {
				parser.println(parser.toString());
				print_dec_s32(parser, motor_accel_mod.value);
				goto done;
			}
			else if (parser.startsWith("SA?")) {
				parser.println(parser.toString());
				parser.advanceTail(2);
				us16 temp = parser.toVariant().toInt();
				noInterrupts(); //stepper_timer_int_enable = 0;
				motor_accel_mod.value = temp;
				interrupts(); //stepper_timer_int_enable = 1;
				goto ok;
			}
			else if (parser.startsWith("SO")) {
				parser.println(parser.toString());
				motor_fKeepOn[motor] = false;
				stepper_powered(motor, false);
				goto ok;
			}
			else if (parser.startsWith("SP")) {
				parser.println(parser.toString());
				motor_fKeepOn[motor] = true;
				stepper_powered(motor, true);
				goto ok;
			}
			else if (parser.startsWith("SF?")) {
				parser.println(parser.toString());
				parser.advanceTail(2);
				s32 value = parser.toVariant().toInt();
				print_us32(parser, value);
				print_cr(parser);
				noInterrupts(); //stepper_timer_int_enable = 0;
				motor_stp_delay_current[motor].value = timer_from_freq((us16)value);
				interrupts(); //stepper_timer_int_enable = 1;
				//        print_ok(0);
				print_us16(parser, motor_stp_delay_current[motor].value);
				goto done;
			}
			else if (parser.startsWith("RX")) {
				parser.println(parser.toString());
				print_byte(parser, readDirectionSign(motor));
				goto done;
			}
			else if (parser.startsWith("RC")) {
				parser.println(parser.toString());
				s32 value;
				noInterrupts(); //stepper_timer_int_enable = 0;
				value = motor_position[motor];
				interrupts(); //stepper_timer_int_enable = 1;
				print_dec_s32(parser, value);
				goto done;
			}
			else if (parser.startsWith("RD")) {
				parser.println(parser.toString());
				s32 value;
				noInterrupts(); //stepper_timer_int_enable = 0;
				value = motor_destination[motor];
				interrupts(); //stepper_timer_int_enable = 1;
				print_dec_s32(parser, value);
				goto done;
			}
			else if (parser.startsWith("RT")) {
				parser.println(parser.toString());
				s32 des;
				s32 pos;
				noInterrupts(); //stepper_timer_int_enable = 0;
				des = motor_destination[motor];
				pos = motor_position[motor];
				interrupts(); //stepper_timer_int_enable = 1;
				print_dec_s32(parser, des-pos);
				goto done;
			}
			else if (parser.startsWith("RLS")) {
				parser.println(parser.toString());
				if(  stepper_limit_minus_get(motor) ) {
					parser.print("-");
				}
				else {
					parser.print("!");
				}
				if(  stepper_limit_plus_get(motor) ) {
					parser.print("+");
				}
				else {
					parser.print("!");
				}

				goto done;
			}
			else if (parser.startsWith("STATUS")) {
				parser.println(parser.toString());
				us32 temp32;
				noInterrupts(); //stepper_timer_int_enable = 0;
				temp32 = (us32)motor_position[motor];
				interrupts(); //stepper_timer_int_enable = 1;
				parser.print("POS: ");
				print_dec_s32(parser, temp32);
				print_cr(parser);

				noInterrupts(); //stepper_timer_int_enable = 0;
				temp32 = (us32)motor_destination[motor];
				interrupts(); //stepper_timer_int_enable = 1;
				parser.print("DES: ");
				print_dec_s32(parser, temp32);
				print_cr(parser);

				parser.print("RLS: ");
				if(  stepper_limit_minus_get(motor) ) {
					parser.print("-");
				}
				else {
					parser.print("!");
				}
				if(  stepper_limit_plus_get(motor) ) {
					parser.print("+");
				}
				else {
					parser.print("!");
				}

				print_cr(parser);
				parser.print("motor_fAccelerate=");
				print_dec_s32(parser, motor_fAccelerate[motor]);
				print_cr(parser);
				parser.print("motor_fMoving=");
				print_dec_s32(parser, motor_fMoving[motor]);
				print_cr(parser);
				parser.print("motor_fAtSpeed=");
				print_dec_s32(parser, motor_fAtSpeed[motor]);
				print_cr(parser);
				parser.print("motor_fKeepOn=");
				print_dec_s32(parser, motor_fKeepOn[motor]);
				print_cr(parser);
				parser.print("motor_fContinuous=");
				print_dec_s32(parser, motor_fContinuous[motor]);
				print_cr(parser);
				parser.print("motor_fChangeDirection=");
				print_dec_s32(parser, motor_fChangeDirection[motor]);
				print_cr(parser);
				parser.print("motor_fForward=");
				print_dec_s32(parser, motor_fForward[motor]);
				print_cr(parser);
				parser.print("motor_stp_delay_accel=");
				print_dec_s32(parser, motor_stp_delay_accel[motor].value);
				print_cr(parser);
				parser.print("motor_stp_delay_minimum=");
				print_dec_s32(parser, motor_stp_delay_minimum[motor].value);
				print_cr(parser);
				parser.print("motor_stp_delay_target=");
				print_dec_s32(parser, motor_stp_delay_target[motor].value);
				print_cr(parser);
				parser.print("motor_stp_delay_current=");
				print_dec_s32(parser, motor_stp_delay_current[motor].value);
				print_cr(parser);
				parser.print("motor_stp_delay_counter=");
				print_dec_s32(parser, motor_stp_delay_counter[motor].value);
				print_cr(parser);
				parser.print("motor_stp_delay=");
				print_dec_s32(parser, motor_stp_delay[motor].value);
				
				goto done;
			}
			parser.print("N");
	ok:
			print_ok(parser,0);
	done:
			print_cr(parser);
		}
	}

private:
};

#endif // STEPANDDIRECTION_H
