#ifndef STEPANDDIRECTION_H
#define STEPANDDIRECTION_H

#include <WProgram.h>
#include "Core.h"
#include "TokenParser.h"

#define array_base_type 1

// Stepper Motors
// stepper count is used to let the stpper code know how many controllers to impliment
#define STEPPER_COUNT           3

#ifdef GONE // from plib project.h file
#define setup_steppers() \
    step1_stp_tris = 0; \
    step2_stp_tris = 0; \
    step3_stp_tris = 0; \
    step1_dir_tris = 0; \
    step2_dir_tris = 0; \
    step3_dir_tris = 0; \
    step1_enable_tris = 0; \
    step2_enable_tris = 0; \
    step3_enable_tris = 0; \
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
#define step1_enable_tris       pl_TRISE_1
#define step2_stp_tris          pl_TRISA_4
#define step2_dir_tris          pl_TRISG_3
#define step2_enable_tris       pl_TRISD_6
#define step3_stp_tris          pl_TRISF_7
#define step3_dir_tris          pl_TRISE_0
#define step3_enable_tris       pl_TRISD_5

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

class StepAndDirection {
public:
    StepAndDirection() {
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
	  parser.print("\r");
	}

    void command(TokenParser &parser) {

		if(parser.compare("stp?")) {
			parser.advanceTail(3);
			us8 motor = parser.toVariant().toInt() + array_base_type;
			//motor = parser.name[3] - ('0' + array_base_type);

            parser.nextToken();
			parser.print("Found STP");
			switch(motor){
				case 0:
					parser.print("0");
					break;
				case 1:
					parser.print("1");
					break;
				case 2:
					parser.print("2");
					break;
			}
			parser.println(".");

            if(parser.compare("status")) {
                parser.println("eh, okay");
            }
            if(parser.compare("stop")) {
                parser.println("OK");
            }
			
			goto done;

			if( !(motor < STEPPER_COUNT) ) {
				print_nok(parser, 0);
				goto done;
			}
	#ifdef GONE

			if (parser.compare("qc.ee")("V?")) {
				print("STP100 V2.3");
				goto done;
			}
			else if (eval_parser_opt("H+")) {
				move_immediate(motor, 0x7fffffff);
				motor_fContinuous[motor] = true;
				goto ok;
			}
			else if (eval_parser_opt("H-")) {
				move_immediate(motor, -0x80000000);
				motor_fContinuous[motor] = true;
				goto ok;
			}
			else if (eval_parser_opt("H0")) {
				decelStop(motor);
				motor_fContinuous[motor] = false;
				goto ok;
			}
			else if (eval_parser_opt("HA")) {
				us8 i;
				for(i = 0; i < STEPPER_COUNT; i++) {
					halt_immediate(i);
				}
				goto ok;
			}
			else if (eval_parser_opt("HI")) {
				halt_immediate(motor);
				goto ok;
			}
			else if (eval_parser_opt("HM")) {
				s32 value;
				value = parse_s32_decimal(2, OPT_SIZE, parser.opt);
				home(motor, value);
				goto ok;
			}
			else if (eval_parser_opt("MI")) {
				s32 value;
				value = parse_s32_decimal(2, OPT_SIZE, parser.opt);
				move_immediate(motor, value);
				goto ok;
			}
			else if (eval_parser_opt("II")) {
				s32 value;
				value = parse_s32_decimal(2, OPT_SIZE, parser.opt);
				increment_immediate(motor, value);
				goto ok;
			}
			else if (eval_parser_opt("RSD")) {
				print_dec_s32(motor_stp_delay[motor].value);
				goto done;
			}
			else if (eval_parser_opt("SD")) {
				us16 temp;
				temp = hexchar_to_hex(parser.value[0], parser.value[1]);
				temp <<= 8;
				temp |= hexchar_to_hex(parser.value[2], parser.value[3]);
				stepper_timer_int_enable = 0;
				motor_stp_delay[motor].value = temp;
				stepper_timer_int_enable = 1;
				goto ok;
			}
			else if (eval_parser_opt("RSM")) {
				print_dec_s32(motor_stp_delay_minimum[motor].value);
				goto done;
			}
			else if (eval_parser_opt("SM")) {
				us16 temp;
				temp = hexchar_to_hex(parser.value[0], parser.value[1]);
				temp <<= 8;
				temp |= hexchar_to_hex(parser.value[2], parser.value[3]);
				stepper_timer_int_enable = 0;
				motor_stp_delay_minimum[motor].value = temp;
				stepper_timer_int_enable = 1;
				goto ok;
			}
			else if (eval_parser_opt("RSA")) {
				print_dec_s32(motor_accel_mod.value);
				goto done;
			}
			else if (eval_parser_opt("SA")) {
				us16 temp;
				temp = hexchar_to_hex(parser.value[0], parser.value[1]);
				temp <<= 8;
				temp |= hexchar_to_hex(parser.value[2], parser.value[3]);
				stepper_timer_int_enable = 0;
				motor_accel_mod.value = temp;
				stepper_timer_int_enable = 1;
				goto ok;
			}
			else if (eval_parser_opt("SO")) {
				motor_fKeepOn[motor] = false;
				stepper_powered(motor, false);
				goto ok;
			}
			else if (eval_parser_opt("SP")) {
				motor_fKeepOn[motor] = true;
				stepper_powered(motor, true);
				goto ok;
			}
			else if (eval_parser_opt("SF")) {
				s32 value;
				value = parse_s32_decimal(2, OPT_SIZE, parser.opt);
				print_us32(value);
				print_cr();
				stepper_timer_int_enable = 0;
				motor_stp_delay_current[motor].value = timer_from_freq((us16)value);
				stepper_timer_int_enable = 1;
				//        print_ok(0);
				print_us16(motor_stp_delay_current[motor].value);
				goto done;
			}
			else if (eval_parser_opt("RX")) {
				print_byte(readDirectionSign(motor));
				goto done;
			}
			else if (eval_parser_opt("RC")) {
				s32 value;
				stepper_timer_int_enable = 0;
				value = motor_position[motor];
				stepper_timer_int_enable = 1;
				print_dec_s32(value);
				goto done;
			}
			else if (eval_parser_opt("RD")) {
				s32 value;
				stepper_timer_int_enable = 0;
				value = motor_destination[motor];
				stepper_timer_int_enable = 1;
				print_dec_s32(value);
				goto done;
			}
			else if (eval_parser_opt("RT")) {
				s32 des;
				s32 pos;
				stepper_timer_int_enable = 0;
				des = motor_destination[motor];
				pos = motor_position[motor];
				stepper_timer_int_enable = 1;
				print_dec_s32(des-pos);
				goto done;
			}
			else if (eval_parser_opt("RLS")) {
				if(  stepper_limit_minus_get(motor) ) {
					print("-");
				}
				else {
					print("!");
				}
				if(  stepper_limit_plus_get(motor) ) {
					print("+");
				}
				else {
					print("!");
				}

				goto done;
			}
			else if (eval_parser_opt("STATUS")) {
				us32 temp32;
				stepper_timer_int_enable = 0;
				temp32 = (us32)motor_position[motor];
				stepper_timer_int_enable = 1;
				print("POS: ");
				print_us32(temp32);
				print_cr();

				stepper_timer_int_enable = 0;
				temp32 = (us32)motor_destination[motor];
				stepper_timer_int_enable = 1;
				print("DES: ");
				print_us32(temp32);
				goto done;
			}
	#endif
	ok:
			print_ok(parser,0);
	done:
			print_cr(parser);
		}
	}

private:
};

#endif // STEPANDDIRECTION_H
