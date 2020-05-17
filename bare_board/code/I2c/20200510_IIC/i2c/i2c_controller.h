 

#ifndef _I2C_CONTROLLER_H
#define _I2C_CONTROLLER_H
typedef struct i2c_msg {
	unsigned int addr;
	int flags;/*0  write 1 read*/
	int len;
	int cnt_transferred;
	int err;
	unsigned char *buf;
}i2c_msg, *p_i2c_msg;

typedef struct i2c_controller {
	int (*init) (void);
	int (*master_xfer)(p_i2c_msg msg, int num);
	char *name;
}i2c_controller,*p_i2c_controller;

#endif
