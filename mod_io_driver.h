// *********************** Defines ***********************************
#define LICENSE			"GPL"
#define AUTHOR			"ismailzemni@gmail.com"
#define DESCRIPTION	"MOD-IO board Driver Module"

#define DEBUG		1
#define DEBUG_LEVEL_1	1

#define	TWI1_BASE_ADDR	0x01C2AC00
#define	TWI2_BASE_ADDR	0x01C2B000
#define	TWI3_BASE_ADDR	0x01C2B400

#define TWI2_REG_ADDR(i) (*(volatile uint32_t *)(TWI2_BASE_ADDR + i))

#define TWI_ADDR_OFFSET 	0x0000 	//-- TWI Slave address
#define TWI_XADDR_OFFSET 	0x0004 	//-- TWI Extended slave address
#define TWI_DATA_OFFSET 	0x0008 	//-- TWI Data byte
#define TWI_CNTR_OFFSET 	0x000C 	//-- TWI Control register
#define TWI_STAT_OFFSET 	0x0010 	//-- TWI Status register
#define TWI_CCR_OFFSET 		0x0014	//-- TWI Clock control register
#define TWI_SRST_OFFSET 	0x0018 	//-- TWI Software reset
#define TWI_EFR_OFFSET 		0x001C 	//-- TWI Enhance Feature register
#define TWI_LCR_OFFSET 		0x0020 	//-- TWI Line Control register


#define TWI_ADDR 		TWI2_REG_ADDR(TWI_ADDR_OFFSET) 	//-- TWI2 Slave address
#define TWI_XADDR 	TWI2_REG_ADDR(TWI_XADDR_OFFSET) //-- TWI2 Extended slave address
#define TWI_DATA 		TWI2_REG_ADDR(TWI_DATA_OFFSET) 	//-- TWI2 Data byte
#define TWI_CNTR 		TWI2_REG_ADDR(TWI_CNTR_OFFSET) 	//-- TWI2 Control register
#define TWI_STAT 		TWI2_REG_ADDR(TWI_STAT_OFFSET) 	//-- TWI2 Status register
#define TWI_CCR 		TWI2_REG_ADDR(TWI_CCR_OFFSET)		//-- TWI2 Clock control register
#define TWI_SRST 		TWI2_REG_ADDR(TWI_SRST_OFFSET) 	//-- TWI2 Software reset
#define TWI_EFR 		TWI2_REG_ADDR(TWI_EFR_OFFSET) 	//-- TWI2 Enhance Feature register
#define TWI_LCR 		TWI2_REG_ADDR(TWI_LCR_OFFSET) 	//-- TWI2 Line Control register


/* enumerate MOD-IO commands */
typedef enum I2C_COMMAND_CODES {
	I2C_NO_COMMAND			= 0x00,	// -- No commands
	I2C_SET_OUTPUTS			= 0x10, // -- Command to set relays
	I2C_GET_DINPUTS			= 0x20, // -- Read inputs commands
	I2C_GET_AIN_0				= 0x30,	// -- Read Analog input 0 commands
	I2C_GET_AIN_1				= 0x31,	// -- Read Analog input 1 commands
	I2C_GET_AIN_2				= 0x32,	// -- Read Analog input 2 commands
	I2C_GET_AIN_3				= 0x33,	// -- Read Analog input 3 commands
	I2C_SET_SLAVE_ADDR	= 0xF0	// -- New Slave ADDR commands
} I2C_COMMAND_CODES;
