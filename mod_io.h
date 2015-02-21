/**
******************************************************************************
* @file 		mod_io.h
* @author 	Ismail ZEMNI (ismailzemni@gmail.com)
*						Mohamed Fadhel SASSI (mohamed.fadhel.sassi@gmail.com )
* @version 	1.0
* @date 		22/01/2015
* @brief		User application of MOD-IO borad.
******************************************************************************
*/

#ifndef MOD_IO_H
#define MOD_IO_H

/* Includes ------------------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define MOD_ID_DEVICE_PATH  "/dev/vtwi"

#define MOD_IO_IOC_MAGIC		'k'
// -- #define MOD_IO_IOCRESET			_IO(MOD_IO_IOC_MAGIC, 0)
#define MOD_IO_IOC_MAXNBR		7

#define MOD_IO_IOC_NO_COMMAND				_IO(MOD_IO_IOC_MAGIC, 0)				// -- No commands
#define MOD_IO_IOC_SET_OUTPUTS			_IOW(MOD_IO_IOC_MAGIC, 1, int) 	// -- Command to set relays
#define MOD_IO_IOC_GET_DINPUTS			_IOR(MOD_IO_IOC_MAGIC, 2, int) 	// -- Read inputs commands
#define MOD_IO_IOC_GET_AIN_0				_IOR(MOD_IO_IOC_MAGIC, 3, int)	// -- Read Analog input 0 commands
#define MOD_IO_IOC_GET_AIN_1				_IOR(MOD_IO_IOC_MAGIC, 4, int)	// -- Read Analog input 1 commands
#define MOD_IO_IOC_GET_AIN_2				_IOR(MOD_IO_IOC_MAGIC, 5, int)	// -- Read Analog input 2 commands
#define MOD_IO_IOC_GET_AIN_3				_IOR(MOD_IO_IOC_MAGIC, 6, int)	// -- Read Analog input 3 commands
#define MOD_IO_IOC_SET_SLAVE_ADDR		_IOW(MOD_IO_IOC_MAGIC, 7, int)	// -- Set MOD-IO slave adress

#define MSG_GET_DINPUTS		">> The current value of digital inputs is : %d\n"
#define MSG_GET_AIN_0			">> The current value of analog input 0 is : %d\n"
#define MSG_GET_AIN_1			">> The current value of analog input 1 is : %d\n"
#define MSG_GET_AIN_2			">> The current value of analog input 2 is : %d\n"
#define MSG_GET_AIN_3			">> The current value of analog input 3 is : %d\n"

#define	MOD_IO_ARRD_W		0x5A
#define	MOD_IO_ARRD_R		0x58

/* Private typedef -----------------------------------------------------------*/
/* enumerate MOD-IO commands */
typedef enum MOD_IO_COMMAND_CODES {
	MOD_IO_NO_COMMAND				= 0x00,	// -- No commands
	MOD_IO_SET_OUTPUTS			= 0x10, // -- Command to set relays
	MOD_IO_GET_DINPUTS			= 0x20, // -- Read inputs commands
	MOD_IO_GET_AIN_0				= 0x30,	// -- Read Analog input 0 commands
	MOD_IO_GET_AIN_1				= 0x31,	// -- Read Analog input 1 commands
	MOD_IO_GET_AIN_2				= 0x32,	// -- Read Analog input 2 commands
	MOD_IO_GET_AIN_3				= 0x33,	// -- Read Analog input 3 commands
	MOD_IO_SET_SLAVE_ADDR		= 0xF0	// -- New Slave ADDR commands
} MOD_IO_COMMAND_CODES;

typedef enum mod_io_cmd {
	NO_COMMAND = 0,	// -- No commands
	SET_OUTPUTS, 		// -- Command to set relays
	GET_DINPUTS, 		// -- Read inputs commands
	GET_AIN_0,			// -- Read Analog input 0 commands
	GET_AIN_1,			// -- Read Analog input 1 commands
	GET_AIN_2,			// -- Read Analog input 2 commands
	GET_AIN_3,			// -- Read Analog input 3 commands
	SET_SLAVE_ADDR 	// -- New Slave ADDR commands
} mod_io_cmd;

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

#endif                          /* MOD_IO_H */
// *********************** END OF FILE *****************************************
