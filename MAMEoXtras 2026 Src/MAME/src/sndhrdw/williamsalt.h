/***************************************************************************

	Midway/williamsalt Audio Board

****************************************************************************/

MACHINE_DRIVER_EXTERN( williamsalt_cvsd_sound );
MACHINE_DRIVER_EXTERN( williamsalt_adpcm_sound );
MACHINE_DRIVER_EXTERN( williamsalt_narc_sound );

void williamsalt_cvsd_init(int cpunum, int pianum);
void williamsalt_cvsd_data_w(int data);
void williamsalt_cvsd_reset_w(int state);

void williamsalt_adpcm_init(int cpunum);
void williamsalt_adpcm_data_w(int data);
void williamsalt_adpcm_reset_w(int state);

void williamsalt_narc_init(int cpunum);
void williamsalt_narc_data_w(int data);
void williamsalt_narc_reset_w(int state);



