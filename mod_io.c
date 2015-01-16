#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <linux/ioctl.h>
#include "mod_io.h"

unsigned short int get_mod_io_cmd();

int main(int argc, char** argv)
{
	unsigned int command;
	int argp;
	char *msg, *mod_io_device_path;
	FILE *mod_io_periph;

	if ( argc > 1 )
		mod_io_device_path = argv[1];
	else
		mod_io_device_path = MOD_ID_DEVICE_PATH;
		
	// -- mod_io_periph = fopen(mod_io_device_path, "w+");
	while(1)
	{	
		switch(get_mod_io_cmd()){
			case NO_COMMAND:	// -- No commands
				command = MOD_IO_IOC_NO_COMMAND;
				break;
				
			case SET_OUTPUTS: // -- Command to set relays
				command = MOD_IO_IOC_SET_OUTPUTS;
				argp = 0xFF;
				break;
				
			case GET_DINPUTS: // -- Read inputs commands
				command = MOD_IO_IOC_GET_DINPUTS;
				msg = MSG_GET_DINPUTS;
				break;
				
			case GET_AIN_0:	// -- Read Analog input 0 commands
				command = MOD_IO_IOC_GET_AIN_0;
				msg = MSG_GET_AIN_0;
				break;
				
			case GET_AIN_1:	// -- Read Analog input 1 commands
				command = MOD_IO_IOC_GET_AIN_1;
				msg = MSG_GET_AIN_1;
				break;
				
			case GET_AIN_2:	// -- Read Analog input 2 commands
				command = MOD_IO_IOC_GET_AIN_2;
				msg = MSG_GET_AIN_2;
				break;
				
			case GET_AIN_3:	// -- Read Analog input 3 commands
				command = MOD_IO_IOC_GET_AIN_3;
				msg = MSG_GET_AIN_3;
				break;
				
			case SET_SLAVE_ADDR: // -- New Slave ADDR commands
				command = MOD_IO_IOC_SET_SLAVE_ADDR;
				argp = 0xFF;
				break;
				
			default:
				//-- fclose(mod_io_periph);
				return(EXIT_SUCCESS);
		}
	
	// --	ioctl(mod_io_periph, command, &argp);
			// -- print the return value in case of GET command
			if (	command == MOD_IO_IOC_GET_DINPUTS ||
						command == MOD_IO_IOC_GET_AIN_0 ||
						command == MOD_IO_IOC_GET_AIN_1 ||
						command == MOD_IO_IOC_GET_AIN_2 ||
						command == MOD_IO_IOC_GET_AIN_3 )
			{
				printf(msg, argp);
				argp = 0;
			}
	}
}

/* @breif		This function print console messages' and return the vaue of chosen
 *					MOD-IO command
 * @return	return the command value
 */
unsigned short int get_mod_io_cmd()
{
	unsigned int cmd;
	
	printf("\n >> Select the MOD-IO command:\n");
selection:
	printf("\t0 - No command\n");
	printf("\t1 - Set outputs\n");
	printf("\t2 - Read digital inputs\n");
	printf("\t3 - Read analog input 0\n");
	printf("\t4 - Read analog input 1\n");
	printf("\t5 - Read analog input 2\n");
	printf("\t6 - Read analog input 3\n");
	printf("\t7 - Change MOD-IO adress\n");
	printf("\t8 - Exit\n");
	
	printf("<< ");
	scanf("%d",&cmd);
	
	// -- input control
	if (cmd < 0 || cmd > 9) 
	{
		printf("Please select one command !\n");
		goto selection;
	}

	return cmd;
}
