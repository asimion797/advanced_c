/*
 * LAB pointers and structure and unions
 * Modelling a PWM
 *
 */

#include <stdio.h>
#include <stdlib.h>

#define BIT(nr)    (1U << (nr))

int _duty_cycle;
int _period;

typedef struct pwmchip
{
	unsigned long mck;
	unsigned pwm_setup[5];
	char *output[3];
	unsigned samples;
	double period;
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
	double pwm_period_value;
	int pwm_duty_cycle_value;
} PWM;

/*
 * @master_clock : in kHz
 *
 */
void register_pwm(SW_PWM *sw_pwm, PWM *pwm_intfs, unsigned long master_clock)
{
	sw_pwm->mck = master_clock;
	sw_pwm->pwm_setup[0] = pwm_intfs->pwm_setup.b0_enable;
	sw_pwm->pwm_setup[1] = pwm_intfs->pwm_setup.b1_pol;
	sw_pwm->pwm_setup[2] = pwm_intfs->pwm_setup.b2to3_select_channel;
	sw_pwm->pwm_setup[3] = pwm_intfs->pwm_setup.b3to6_duty_cycle;
	sw_pwm->pwm_setup[4] = pwm_intfs->pwm_setup.b7_period;

	if (sw_pwm->pwm_setup[4])  sw_pwm->period = (double) 100 / sw_pwm->mck;
	else sw_pwm->period = (double) 200 / sw_pwm->mck;

	switch(sw_pwm->pwm_setup[3]){
	case 0x0:
		pwm_intfs->pwm_duty_cycle_value = 10;
		break;
	case 0x1:
		pwm_intfs->pwm_duty_cycle_value = 25;
		break;
	case 0x2:
		pwm_intfs->pwm_duty_cycle_value = 50;
		break;
	case 0x3:
		pwm_intfs->pwm_duty_cycle_value = 75;
		break;
	}
	_duty_cycle = pwm_intfs->pwm_duty_cycle_value;//save the value
	pwm_intfs->pwm_period_value = sw_pwm->period;
	_period = pwm_intfs->pwm_period_value;//save the value
}

/*
 * sim the output of a PWM based on the user setup functions
 * setup_pwm() and register_pwm()
 *
 */
void generate_output(SW_PWM *sw_pwm, PWM *pwm_intfs, int num_per)
{
	int samples_per_period;

	if (pwm_intfs->pwm_period_value == 0.5f)
		samples_per_period = pwm_intfs->pwm_period_value * 20;
	else
		samples_per_period = pwm_intfs->pwm_period_value * 10;

	sw_pwm->samples = (unsigned)samples_per_period * num_per;

	(sw_pwm->output[pwm_intfs->pwm_setup.b2to3_select_channel]) = (char *)malloc(samples_per_period * num_per);
	int duty_sample = (int)(_duty_cycle * samples_per_period ) / 100;

	for (int i = 0; i < samples_per_period * num_per; i++)
	{
		if ( (i % samples_per_period) - duty_sample < 0 )
			(sw_pwm->output[pwm_intfs->pwm_setup.b2to3_select_channel ])[i] = 1;
		else
			(sw_pwm->output[pwm_intfs->pwm_setup.b2to3_select_channel ])[i] = 0;
	}
}

int main () {
	int num_per = 3;
	SW_PWM sw_pwm;
	PWM pwm0;
	PWM *p_pwm0 = &pwm0;
	setup_pwm(&x_pwm_setup, 0x9D);
	p_pwm0->pwm_setup = x_pwm_setup;

	printf("=========Mem Allign=======\n");
	printf( "struct setup_reg_pwm size : %d\n", sizeof(x_pwm_setup));
	printf( "union size : %d\n", sizeof(pwm0));
	printf( "struct dummy size : %d\n", sizeof(test));

	printf("\n=========CONF PWM SIM=======\n");
	printf("enable bit: %d\n", p_pwm0->pwm_setup.b0_enable);
	printf("polarity bit: %d\n", p_pwm0->pwm_setup.b1_pol);
	printf("channel : %d\n", p_pwm0->pwm_setup.b2to3_select_channel);
	printf("duty_cycle : %d\n", p_pwm0->pwm_setup.b3to6_duty_cycle);
	printf("period bit: %d\n", p_pwm0->pwm_setup.b7_period);

	register_pwm(&sw_pwm, p_pwm0, 200 );//mck in kHz

	printf("\n=========SW PWM SIM=======\n");
	printf("enable bit: %d\n", sw_pwm.pwm_setup[0]);
	printf("polarity bit: %d\n", sw_pwm.pwm_setup[1]);
	printf("channel : %d\n", sw_pwm.pwm_setup[2]);
	printf("duty_cycle : %d\n", sw_pwm.pwm_setup[3]);
	printf("period bit: %d\n",sw_pwm.pwm_setup[4]);
	printf("mck : %dkHz\n", sw_pwm.mck);
	printf("period : %fs\n",sw_pwm.period);
	printf("duty_cycle: %d%\n", _duty_cycle);

	generate_output(&sw_pwm, p_pwm0, num_per);

	printf("PWM Bits OUTPUT:");
	for(int i = 0; i < sw_pwm.samples; i++)
		printf("%d", sw_pwm.output[0][i]);
	printf("\nsamples / period : %d", sw_pwm.samples / num_per);

	return 0;
}
