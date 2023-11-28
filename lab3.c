/*
 *
 * LAB3 : State Machine
 * LVDS Controller
 *
 */

#include<stdio.h>
#include<stdlib.h>

const unsigned char CRC7_POLY = 0x91;

typedef enum {
	STOP = -1,
	INIT = 0,
	TX,
	WAIT,
	RX,
	CHECK,
	ERROR
} LVDS_STATES;

struct data_pkg_lvds;

struct in_pkg_lvds_ctl {
	LVDS_STATES state;
	struct_pkg_lvds data;
};

struct data_pkg_lvds {
	unsigned data;
	unsigned short crc;
};

struct lvds_ctl {
	LVDS_STATES lvdsctl_state;
	struct data_pkg_lvds tx_buffer[128];
	struct data_pkg_lvds rx_buffer[128];
	unsigned tx_buff_size;
	unsigned rx_buff_size;
	struct data_pkg_lvds tx_peek;
	struct data_pkg_lvds rx_peek;
	unsigned clk_in;//in kHz
	struct in_pkg_lvds_ctl input;

	// simplified API LVDS Controller
	// init buffers size, clk (kHz)
	void (*init) (struct lvds_ctl *, unsigned, unsigned);
	// data, wire
	void (*transmit) (struct lvds_ctl *,
			  struct data_pkg_lvds);
	struct data_pkg_lvds (*recv) (struct lvds_ctl *);
	void (*wait) (struct lvds_ctl *);
	unsigned (*check) (struct data_pkg_lvds);
};

// Simple CRC
unsigned compute_crc(unsigned data)
{
	unsigned crc = 0;
	for (int i = 0 ; i < 4; i++) {
		unsigned byte = data & 0x000000FF;
		crc ^= byte;
		for (int j = 0; j < 8; j++) {
			if (crc & 1)
				crc ^= CRC7_POLY;
			crc >>= 1;
		}
		data >>= 8;
	}
	return crc;
}

void init_lvds_ctl(struct lvds_ctl *lvdsc,
		  unsigned init_size,
		  unsigned init_clk_in)
{
	lvdsc->clk_in = init_clk_in;
	lvdsc->tx_buff_size = init_size;
	lvdsc->rx_buff_size = init_size;
	lvdsc->state = WAIT;
}

void wait_lvds_ctl(struct lvds_ctl *lvds_ctl, struct in_pkg_lvds_ctl instr)
{
	while(1)
	{
		if (lvds_ctl->lvdsctl_state == STOP)
			break;

		lvds_ctl->state = instr.state;

		switch(lvds_ctl->lvdsctl_state) {
		case STOP:
			return;
		case RX:
			lvds_ctl->rx_peek = lvds_ctl->recv();
			__attribute__((fallthrough));
		case WAIT:
			break;//donothing
		case TX:
			lvds_ctl->transmit(lvds_ctl,
					   instr.data);
			__attribute__((fallthrough));
		case WAIT:
			break;//donothing
		}
	}
}

void transmission(struct lvds_ctl *lvds_ctl, struct data_pkg_lvds data)
{
	int err = 0;
	lvds_ctl->tx_buff_size %= 128;
	lvds_ctl->tx_buffer[lvds_ctl->tx_buff_size] = data;
	lvds_ctl->tx_buff_size += 1;
	lvds_ctl->tx_peek = data;
	do {
		err = printf("data sent : %d, %d(crc)\n", lvds_ctl->tx_peek.data,
					    	    	  lvds_ctl->tx_peek.crc);
		if (err)
			lvds_ctl-state = ERROR;
	}while(lvds_ctl->state == ERROR);
	lvds_ctl->state = WAIT;
}

struct data_pkg_lvds receive(struct lvds_ctl *lvds_ctl)
{
	struct data_pkg_lvds recv_dpl;
	do {
		scanf("%d %d", recv_dpl.data, recv_dpl.crc);
		lvds_ctl->state = lvds_ctl->check(recv_dpl);
	} while(lvds_ctl->state == ERROR);

	lvds_ctl->rx_buff_size %= 128;
	lvds_ctl->rx_buffer[lvds_ctl->rx_buff_size] = recv_dpl;
	lvds_ctl->rx_buff_size += 1;
	lvds_ctl->rx_peek = recv_dpl;
	lvds_ctl->state = WAIT;

	return dpl;
}

unsigned check_crc(struct data_pkg_lvds dpl)
{
	if (compute_crc(dpl.data) == dpl.crc)
		return ERROR;
	return WAIT;
}


////////////////////////////////////////////////////////////////////////////////

struct lvds_ctl dummy_lvds_ctl = {
	.init	  = init_lvds_ctl;
	.wait	  = wait_lvds_ctl;
	.transmit = transmission;
	.recv	  = receive;
	.check	  = check_crc;
};

int main()
{
	dummy_lvds_ctl->init();
	switch(dummy_lvds_ctl->state){
	case WAIT:
		//dumm_lvds_ctl->wait(&dummy_lvds_ctl, instr);
	case STOP:
		break;
	}
	return 0;
}
