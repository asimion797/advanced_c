/*
 * CAP III : Advanced Data Structure Pointers
 * Modelling a dumb clocksource tree:
 *	Master CLK: mck : 200kHz
 *	|
 *	+--->i2c_clk : 100kHz
 *	|
 *	+--->usart_clk : 4kHz
 *	|
 *	+--->pwm_clk : 8kHz
 *	|
 *	+--->sdmmc_clk : 50kHz
 */

#include<stdio.h>
#include<stdlib.h>

#define u8 unsigned char

struct periph_layout {
	unsigned factor;//scaling factor
	u8 d_flag;
	char *name;
	char *parent;
	unsigned clk_in;
};

typedef struct div{
	char *conn_periph;//connected peripheral
	unsigned factor;//scaling factor
	u8 d_flag; // 1 up, 0 down
	unsigned clk_kHz;//from parent
	void (*init_DIV) (struct div *div, char *conn_periph,
			  unsigned factor, u8 d_flag,
			  unsigned parent_clk_kHz);
	unsigned (*generate_output) (struct div *div);
}DIV;

typedef struct dumb_pll {
	char *p_name;//parent_name
	unsigned in_clk_freq_kHz;//parent
	DIV *divs;//how much periph.
	void (*init_PLL) (struct dumb_pll *, struct periph_layout *,
			  u8, unsigned);
	void (*disp_and_set_PLL_output) (struct dumb_pll *, struct periph_layout *, u8);
	void (*free_mem) (struct dumb_pll *);
}Dumb_PLL_Driver;

void init_DIV_block(DIV *div,
		char *conn_periph,
		unsigned factor, u8 d_flag,
		unsigned parent_clk_kHz)
{
	div->conn_periph = conn_periph;
	div->factor = factor;
	div->d_flag = d_flag;
	div->clk_kHz = parent_clk_kHz;
}

unsigned get_DIV_output(DIV *div)
{
	if (div->d_flag)
		return div->clk_kHz * div->factor;
	return div->clk_kHz / div->factor;
}

void register_DIV(DIV *div)
{
	div->init_DIV = &init_DIV_block;
	div->generate_output = &get_DIV_output;
}

void init_PLL_block(Dumb_PLL_Driver *dPLL, struct periph_layout *periph,
		    u8 n_divs, unsigned clk_in)
{
	dPLL->p_name = periph[0].parent;
	dPLL->in_clk_freq_kHz = clk_in;
	dPLL->divs = (DIV *)malloc(sizeof(DIV) * n_divs);
	for(int i = 0; i < n_divs; i++)
	{
		register_DIV(&(dPLL->divs[i]));
		dPLL->divs[i].init_DIV(&(dPLL->divs[i]),
					periph[i].name,
					periph[i].factor,
					periph[i].d_flag,
					dPLL->in_clk_freq_kHz);
	}
}

void get_set_PLL_output (Dumb_PLL_Driver *dPLL,
			 struct periph_layout *periph,
			 u8 n_divs)
{
	printf("Master CLK: %s : %ukHz\n", dPLL->p_name, dPLL->in_clk_freq_kHz);
	printf("|\n");
	for(int i = 0; i < n_divs; i++)
	{
		DIV *d = &(dPLL->divs[i]);
		unsigned pll_out = d->generate_output(d);
		periph[i].clk_in = pll_out;
		printf("+--->%s : %ukHz\n", d->conn_periph, pll_out);
		if( i < n_divs - 1 )
			printf("|\n");
	}
}

void free_all(Dumb_PLL_Driver *dPLL)
{
	free(dPLL->divs);
	free(dPLL);
}

void register_PLL(Dumb_PLL_Driver *dPLL)
{
	dPLL->init_PLL = &init_PLL_block;
	dPLL->disp_and_set_PLL_output = &get_set_PLL_output;
	dPLL->free_mem = &free_all;
}

int main()
{
	struct periph_layout dummy_periph[4] = {
		{
			.factor = 2,
			.d_flag = 0,
			.name = "i2c_clk",
			.parent = "mck",
			.clk_in = 0,
		},
		{
			.factor = 50,
			.d_flag = 0,
			.name = "usart_clk",
			.parent = "mck",
			.clk_in = 0,
		},
		{
			.factor = 25,
			.d_flag = 0,
			.name = "pwm_clk",
			.parent = "mck",
			.clk_in = 0,
		},
		{
			.factor = 4,
			.d_flag = 0,
			.name = "sdmmc_clk",
			.parent = "mck",
			.clk_in = 0,
		}
	};

	Dumb_PLL_Driver *myPLL = malloc(sizeof(Dumb_PLL_Driver));
	register_PLL(myPLL);
	myPLL->init_PLL(myPLL, dummy_periph, 4, 200);
	myPLL->disp_and_set_PLL_output(myPLL, dummy_periph, 4);
	myPLL->free_mem(myPLL);

	return 0;
}
