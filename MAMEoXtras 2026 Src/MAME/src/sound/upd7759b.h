#ifndef UPD7759bS_H
#define UPD7759bS_H

#define MAX_UPD7759b 2

/* There are two modes for the uPD7759, selected through the !MD pin.
   This is the mode select input.  High is stand alone, low is slave.
   We're making the assumption that nobody switches modes through
   software. */

#define UPD7759b_STANDARD_CLOCK		640000

struct upd7759b_interface
{
	int num;					/* number of chips */
	int clock[MAX_UPD7759b];		/* clock (per chip) */
	int volume[MAX_UPD7759b];	/* volume (per chip) */
	int region[MAX_UPD7759b]; 	/* memory region (per chip, standalone mode only) */
	void (*drqcallback[MAX_UPD7759b])(int param);	/* drq callback (per chip, slave mode only) */
};

int upd7759b_sh_start(const struct MachineSound *msound);
void upd7759b_sh_stop(void);

void upd7759b_set_bank_base(int which, offs_t base);

void upd7759b_reset_w(int num, UINT8 data);

void upd7759b_w(int num, UINT8 data);
void upd7759b_port_w(int num, UINT8 data);
void upd7759b_start_w(int num, UINT8 data);
int upd7759b_busy_r(int num);

WRITE8_HANDLER( upd7759b_0_reset_w );
WRITE8_HANDLER( upd7759b_0_port_w );
WRITE8_HANDLER( upd7759b_0_start_w );
READ8_HANDLER( upd7759b_0_busy_r );

#endif

