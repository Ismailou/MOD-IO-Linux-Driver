/**
******************************************************************************
* @file 		twi.h
* @author 	Ismail ZEMNI (ismailzemni@gmail.com)
*						Mohamed Fadhel SASSI (mohamed.fadhel.sassi@gmail.com )
* @version 	1.0
* @date 		22/01/2015
* @brief		TWI hardware Driver.
******************************************************************************
*/


#ifndef TWI_H
#define TWI_H

/* Includes ------------------------------------------------------------------*/
#include <linux/types.h>

/* Private define ------------------------------------------------------------*/
/* TWI_CTL values */
#define TWI_CTL_INT_EN          (1 << 7)
#define TWI_CTL_BUS_EN          (1 << 6)
#define TWI_CTL_M_START         (1 << 5)
#define TWI_CTL_M_STOP          (1 << 4)
#define TWI_CTL_INT_FLAG        (1 << 3)
#define TWI_CTL_A_ACK						(1 << 2)	// -- Interrupt Flag

/* TWI_CLK values */
#define TWI_CLK_M_MASK          (0xF << 3)
#define TWI_CLK_M(m)            (((m - 1) << 3) & TWI_CLK_M_MASK)
#define TWI_CLK_N_MASK          (0x7 << 0)
#define TWI_CLK_N(n)            (((n) << 3) & TWI_CLK_N_MASK)

#define A20_ADDR	0x87

#define REG											uint32_t
#define DEBUG_TWI

#ifdef DEBUG_TWI
	#define TWI_SNAPSHOT(twi)								twi_registers_snapshot(twi);
	#define PRINT_TWI_STATUS(twi)						get_twi_status_msg(twi);
#else
	#define TWI_SNAPSHOT(twi)								" "
	#define PRINT_TWI_STATUS(twi)						" "
#endif

/* Private typedef -----------------------------------------------------------*/
enum twi_err { TWI_SUCCESS, TWI_ERR};

/* TWI Code Status values */
enum twi_status {
					TWI_STAT_BUS_ERROR      				= 0x00, // -- Bus error 
					TWI_STAT_TX_START       				= 0x08, // -- START condition transmitted 
					TWI_STAT_TX_RSTART      				= 0x10, // -- Repeated START condition transmitted 
					TWI_STAT_TX_AW_ACK      				= 0x18, // -- Address + Write bit transmitted, ACK received 
					TWI_STAT_TX_AW_NAK      				= 0x20, // -- Address + Write bit transmitted, ACK not received 
					TWI_STAT_M_TXD_ACK       				= 0x28, // -- Data byte transmitted in master mode, ACK received
					TWI_STAT_M_TXD_NAK       				= 0x30, // -- Data byte transmitted in master mode, ACK not received
					TWI_STAT_LOST_ARB       				= 0x38, // -- Arbitration lost in address or data byte
					TWI_STAT_TX_AR_ACK      				= 0x40, // -- Address + Read bit transmitted, ACK received
					TWI_STAT_TX_AR_NAK      				= 0x48, // -- Address + Read bit transmitted, ACK not received
					TWI_STAT_M_RXD_ACK       				= 0x50, // -- Data byte received in master mode, ACK transmitted
					TWI_STAT_M_RXD_NAK       				= 0x58, // -- Data byte received in master mode, not ACK transmitted
					
					TWI_STAT_M_RX_AW_ACK						= 0x60, // -- Slave address + Write bit received, ACK transmitted
					TWI_STAT_M_RX_AW_ARB_LOST				= 0x68, // -- Arbitration lost in address as master, slave address + Write bit received, ACK transmitted
					TWI_STAT_RX_GCA									= 0x70, // -- General Call address received, ACK transmitted
					TWI_STAT_M_RX_GCA_ARB_LOST			= 0x78, // -- Arbitration lost in address as master, General Call address received, ACK transmitted
					TWI_STAT_M_AFTER_SLAVE_RXD_ACK 	= 0x80, // -- Data byte received after slave address received, ACK transmitted
					TWI_STAT_M_AFTER_SLAVE_RXD_NACK = 0x88, // -- Data byte received after slave address received, NACK transmitted
					TWI_STAT_RX_AFTER_CGE_ACK				= 0x90, // -- Data byte received after General Call received, ACK transmitted
					TWI_STAT_RX_AFTER_CGE_NACK			= 0x98, // -- Data byte received after General Call received, NACK transmitted
					TWI_STAT_S_RX_STP_MULTI_STR 		= 0xA0, // -- STOP or repeated START condition received in slave mode
					TWI_STAT_M_RX_AR 								= 0xA8, // -- Slave address + Read bit received, ACK transmitted
					TWI_STAT_M_RX_AR_ARB_LOST 			= 0xB0, // -- Arbitration lost in address as master, slave address + Read bit received, ACK transmitted
					TWI_STAT_S_TXD_ACK 							= 0xB8, // -- Data byte transmitted in slave mode, ACK received
					TWI_STAT_S_TXD_NACK  						= 0xC0, // -- Data byte transmitted in slave mode, ACK not received
					TWI_STAT_S_TXLB_ACK 						= 0xC8, // -- Last byte transmitted in slave mode, ACK received
					TWI_STAT_TX_SCND_ADDR_ACK 			= 0xD0, // -- Second Address byte + Write bit transmitted, ACK received
					TWI_STAT_TX_SCND_ADDR_NACK 			= 0xD8, // -- Second Address byte + Write bit transmitted, ACK not received
					TWI_STAT_IDLE           				= 0xf8, // -- Bus idle : No relevant status information, INT_FLAG=0
};

struct a20_twi {
         REG addr;       // -- 0x00: Slave address
         REG xaddr;      // -- 0x04: Extended slave address
         REG data;       // -- 0x08: Data byte
         REG ctl;        // -- 0x0C: Control register
         REG stat;       // -- 0x10: Status register
         REG clk;        // -- 0x14: Clock control register
         REG reset;      // -- 0x18: Software reset
         REG efr;        // -- 0x1C: Enhanced Feature register
         REG lcr;        // -- 0x20: Line control register
};

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void a20_twi_init(struct a20_twi *twi, uint32_t speed_hz);
void i2c_send_data(struct a20_twi *twi, uint8_t data);
uint8_t i2c_read_data(struct a20_twi *twi);
void i2c_send_start(struct a20_twi *twi);
void i2c_send_stop(struct a20_twi *twi);
int i2c_read(unsigned bus, unsigned chip, unsigned addr,
             unsigned alen, uint8_t *buf, unsigned len);
int i2c_write(unsigned bus, unsigned chip, unsigned addr,
              unsigned alen, const uint8_t *buf, unsigned len);
void twi_registers_snapshot(struct a20_twi *twi);
int twi_write(struct a20_twi *twi, unsigned addr, unsigned cmd, unsigned value);
int twi_read(struct a20_twi *twi, unsigned addr, unsigned cmd, unsigned *value);
void get_twi_status_msg(struct a20_twi *twi);

#endif                          /* A13_TWI_H */
// *********************** END OF FILE *****************************************
