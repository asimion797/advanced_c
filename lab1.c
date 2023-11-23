/*
* LAB pointers and structure and unions
* Modelling a PWM
*/

#include <stdio.h>

#define BIT(nr)    (1U << (nr))

typedef struct pwmchip
{
    unsigned long mck;
    unsigned pwm_setup[5];
    char *output[3];
    char samples;
    unsigned period;
} SW_PWM;

struct dummy
{
    float x;
    char y;
} test;

struct setup_reg_pwm
{
    unsigned char b0_enable : 1;
    unsigned char b1_pol : 1;
    unsigned char b2to3_select_channel : 2;
    unsigned char b3to6_duty_cycle : 3;
    unsigned char b7_period : 1;
} x_pwm_setup;

void setup_pwm(struct setup_reg_pwm *reg, unsigned char val)
{
    unsigned char tmp_bits = 0;
    tmp_bits |= (BIT(2) | BIT(3));
    reg->b0_enable = val & BIT(0);
    reg->b1_pol = (val & BIT(1)) >> 1;
    reg->b2to3_select_channel = (val & tmp_bits) >> 2;
    reg->b3to6_duty_cycle = (val & (BIT(4) | BIT(5) | BIT(6))) >> 4;
    reg->b7_period = (val & BIT(7)) >> 7;
}

typedef union
{
    volatile struct setup_reg_pwm pwm_setup;
    unsigned pwm_period_value;
    float pwm_duty_cycle_value;
} PWM;

void register_pwm(SW_PWM *sw_pwm, PWM *pwm_intfs, unsigned long master_clock)
{
        sw_pwm->mck = master_clock;
        sw_pwm->pwm_setup[0] = pwm_intfs->pwm_setup.b0_enable;
        sw_pwm->pwm_setup[1] = pwm_intfs->pwm_setup.b1_pol;
        sw_pwm->pwm_setup[2] = pwm_intfs->pwm_setup.b2to3_select_channel;
        sw_pwm->pwm_setup[3] = pwm_intfs->pwm_setup.b3to6_duty_cycle;
        sw_pwm->pwm_setup[4] = pwm_intfs->pwm_setup.b7_period;

        sw_pwm->period = !sw_pwm->pwm_setup[4] ? 1 / sw_pwm->mck : 1 / sw_pwm->mck;

        pwm_intfs->pwm_period_value = sw_pwm->period;

        switch(sw_pwm->pwm_setup[3]){
            case 0x0:
                pwm_intfs->pwm_duty_cycle_value = 10.00f;
                break;
            case 0x1:
                pwm_intfs->pwm_duty_cycle_value = 25.00f;
                break;
            case 0x2:
                pwm_intfs->pwm_duty_cycle_value = 50.00f;
                break;
            case 0x3:
                pwm_intfs->pwm_duty_cycle_value = 75.00f;
                break;
        }
}

/*TODO: generate output function*/


int main () {
    SW_PWM sw_pwm;
    PWM pwm0;
    PWM *p_pwm0 = &pwm0;
    setup_pwm(&x_pwm_setup, 0x9D);
    p_pwm0->pwm_setup = x_pwm_setup;

    printf("=========Mem Allign=======\n");
    printf( "struct setup_reg_pwm size : %d\n", sizeof(x_pwm_setup));
    printf( "union size : %d\n", sizeof(pwm0));
    printf( "struct dummy size : %d\n", sizeof(test));

    printf("\n=========SW PWM SIM=======\n");
    printf("enable bit: %d\n", p_pwm0->pwm_setup.b0_enable);
    printf("polarity bit: %d\n", p_pwm0->pwm_setup.b1_pol);
    printf("channel : %d\n", p_pwm0->pwm_setup.b2to3_select_channel);
    printf("duty_cycle : %d\n", p_pwm0->pwm_setup.b3to6_duty_cycle);
    printf("period bit: %d\n", p_pwm0->pwm_setup.b7_period);

    register_pwm(&sw_pwm, p_pwm0, 200000000000 );


    return 0;
}
