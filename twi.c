/**
******************************************************************************
* @file 		twi.C
* @author 	Ismail ZEMNI (ismailzemni@gmail.com)
*						Mohamed Fadhel SASSI (mohamed.fadhel.sassi@gmail.com )
* @version 	1.0
* @date 		22/01/2015
* @brief		TWI hardware Driver.
******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "twi.h"
#include <linux/types.h>
#include <linux/kernel.h>
#include <asm/io.h>
#include <linux/delay.h>
//--#include <device/i2c.h>

/* Private define ------------------------------------------------------------*/
#define	A20_TWI_BASE										0x01C2AC00
#define TWI_BASE(n)                     (A20_TWI_BASE + 0x400 * (n))
#define TWI_TIMEOUT                     (50 * 1000)

/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
static uint8_t is_busy(struct a20_twi *twi);
static void configure_clock(struct a20_twi *twi, uint32_t speed_hz);
static void clear_interrupt_flag(struct a20_twi *twi);
static enum twi_status wait_for_status(struct a20_twi *twi);

/*************************** Functions ****************************************/              
static uint8_t is_busy(struct a20_twi *twi)
{
	return (ioread32(&twi->stat) != TWI_STAT_IDLE);
}

static enum twi_err wait_until_idle(struct a20_twi *twi)
{
  u32 i = TWI_TIMEOUT;
  while (i-- && is_busy((twi)))
          udelay(1);
  return i ? TWI_SUCCESS : TWI_ERR;
}

/* FIXME: This function is basic, and unintelligent */
static void configure_clock(struct a20_twi *twi, uint32_t speed_hz)
{
	/* FIXME: We assume clock is 24MHz, which may not be the case */
	uint32_t apb_clk = 24000000, m, n;

	/* Pre-divide the clock by 8 */
	n = 3;
	m = (apb_clk >> n) / speed_hz;
	iowrite32(TWI_CLK_M(m) | TWI_CLK_N(n), &twi->clk);
}

void a20_twi_init(struct a20_twi *twi, uint32_t speed_hz)
{
	uint32_t i = TWI_TIMEOUT;
	//--twi = (void *)bus;

	configure_clock(twi, speed_hz);

	/* Enable the I²C bus */
	iowrite32(TWI_CTL_BUS_EN, &twi->ctl);
	/* Issue soft reset */
	iowrite32(1, &twi->reset);
	/* set Allwinner-A20 adress */
	iowrite32(A20_ADDR, &twi->addr);

	while (i-- && ioread32(&twi->reset))
		       udelay(1);
}

static void clear_interrupt_flag(struct a20_twi *twi)
{
	iowrite32(ioread32(&twi->ctl) & ~TWI_CTL_INT_FLAG, &twi->ctl);
}

void i2c_send_data(struct a20_twi *twi, uint8_t data)
{
	iowrite32(data, &twi->data);
	clear_interrupt_flag(twi);
}

uint8_t i2c_read_data(struct a20_twi *twi)
{
	uint32_t tmpreg;
	
	tmpreg = ioread32(&twi->data);
	clear_interrupt_flag(twi);
	
	return tmpreg;
}

static enum twi_status wait_for_status(struct a20_twi *twi)
{
	uint32_t i = TWI_TIMEOUT;
	/* Wait until interrupt is asserted again */
	while (i-- && !(ioread32(&twi->ctl) & TWI_CTL_INT_FLAG))
		     udelay(1);
	/* A timeout here most likely indicates a bus error */
	return i ? ioread32(&twi->stat) : TWI_STAT_BUS_ERROR;
}

void i2c_send_start(struct a20_twi *twi)
{
	uint32_t reg32, i;

	/* Send START condition */
	reg32 = ioread32(&twi->ctl);
	reg32 &= ~TWI_CTL_INT_FLAG;
	reg32 |= TWI_CTL_M_START;
	iowrite32(reg32, &twi->ctl);

	/* M_START is automatically cleared after start condition is transmitted */
	i = TWI_TIMEOUT;
	while (i-- && (ioread32(&twi->ctl) & TWI_CTL_M_START))
		     udelay(1);
}

void i2c_send_stop(struct a20_twi *twi)
{
	uint32_t reg32;

	/* Send STOP condition */
	reg32 = ioread32(&twi->ctl);
	reg32 &= ~TWI_CTL_INT_FLAG;
	reg32 |= TWI_CTL_M_STOP;
	iowrite32(reg32, &twi->ctl);
}

int i2c_read(unsigned bus, unsigned chip, unsigned addr,
             unsigned alen, uint8_t *buf, unsigned len)
{
	unsigned count = len;
	enum twi_status expected_status;
	struct a20_twi *twi = (void *)bus;

	 if (wait_until_idle(twi) != TWI_SUCCESS)
		       return TWI_ERR;
	
	 i2c_send_start(twi);
	 if (wait_for_status(twi) != TWI_STAT_TX_START)
		       return TWI_ERR;

	 /* Send chip address */
	 i2c_send_data(twi, chip << 1);
	 if (wait_for_status(twi) != TWI_STAT_TX_AW_ACK)
		       return TWI_ERR;

	 /* Send data address */
	 i2c_send_data(twi, addr);
	 if (wait_for_status(twi) != TWI_STAT_M_TXD_ACK)
		       return TWI_ERR;

	 /* Send restart for read */
	 i2c_send_start(twi);
	 if (wait_for_status(twi) != TWI_STAT_TX_RSTART)
		       return TWI_ERR;

	 /* Send chip address */
	 i2c_send_data(twi, chip << 1 | 1);
	 if (wait_for_status(twi) != TWI_STAT_TX_AR_ACK)
		       return TWI_ERR;

	 /* Start ACK-ing received data */
	 // -- setbits_le32(&twi->ctl, TWI_CTL_A_ACK);
	 iowrite32((TWI_CTL_A_ACK | ioread32(&twi->ctl)), &twi->ctl); // -- TODO read and mask
	 expected_status = TWI_STAT_M_RXD_ACK;

	 /* Read data */
	 while (count > 0) {
		if (count == 1) {
				   /* Do not ACK the last byte */
				   // -- clrbits_le32(&twi->ctl, TWI_CTL_A_ACK);
				   iowrite32((~TWI_CTL_A_ACK & ioread32(&twi->ctl)), &twi->ctl); // -- TODO read and mask
				   expected_status = TWI_STAT_M_RXD_NAK;
		}

		clear_interrupt_flag(twi);

		if (wait_for_status(twi) != expected_status)
				   return TWI_ERR;

		*buf++ = ioread32(&twi->data);
		count--;
	 }

	 i2c_send_stop(twi);

	 return len;
}

int i2c_write(unsigned bus, unsigned chip, unsigned addr,
              unsigned alen, const uint8_t *buf, unsigned len)
{
	unsigned count = len;
	struct a20_twi *twi = (void *)bus;

	 if (wait_until_idle(twi) != TWI_SUCCESS)
		       return TWI_ERR;

	 i2c_send_start(twi);
	 if (wait_for_status(twi) != TWI_STAT_TX_START)
		       return TWI_ERR;

	 /* Send chip address */
	 i2c_send_data(twi, chip << 1);
	 if (wait_for_status(twi) != TWI_STAT_TX_AW_ACK)
		       return TWI_ERR;

	 /* Send data address */
	 i2c_send_data(twi, addr);
	 if (wait_for_status(twi) != TWI_STAT_M_TXD_ACK)
		       return TWI_ERR;

	 /* Send data */
	 while (count > 0) {
		i2c_send_data(twi, *buf++);
		if (wait_for_status(twi) != TWI_STAT_M_TXD_ACK)
				   return TWI_ERR;
		count--;
	 }

	 i2c_send_stop(twi);

	 return len;
}

int twi_write(struct a20_twi *twi, unsigned addr, unsigned cmd, unsigned value)
{
	if (wait_until_idle(twi) != TWI_SUCCESS)
	{
		PRINT_TWI_STATUS(twi)
		return TWI_ERR;
	}

	TWI_SNAPSHOT(twi)

	i2c_send_start(twi);
	if (wait_for_status(twi) != TWI_STAT_TX_START)
	{
		PRINT_TWI_STATUS(twi)
		return TWI_ERR;
	}

	TWI_SNAPSHOT(twi)

	/* Send chip address */
	i2c_send_data(twi, addr);
	if (wait_for_status(twi) != TWI_STAT_TX_AW_ACK)
	{
		PRINT_TWI_STATUS(twi)
    return TWI_ERR;
	}
	
	TWI_SNAPSHOT(twi)

	/* Send data address */
	i2c_send_data(twi, cmd);
	if (wait_for_status(twi) != TWI_STAT_M_TXD_ACK)
	{
		PRINT_TWI_STATUS(twi)
		return TWI_ERR;
	}

	TWI_SNAPSHOT(twi)

	/* Send value */
	i2c_send_data(twi, value);
	if (wait_for_status(twi) != TWI_STAT_M_TXD_ACK)
	{
		PRINT_TWI_STATUS(twi)
		return TWI_ERR;
	}

	TWI_SNAPSHOT(twi)

	i2c_send_stop(twi);

	return TWI_SUCCESS;
}

int twi_read(struct a20_twi *twi, unsigned addr, unsigned cmd, unsigned *value)
{
	if (wait_until_idle(twi) != TWI_SUCCESS)
	{
		PRINT_TWI_STATUS(twi)
		return TWI_ERR;
	}

	TWI_SNAPSHOT(twi)

	i2c_send_start(twi);
	if (wait_for_status(twi) != TWI_STAT_TX_START)
	{
		PRINT_TWI_STATUS(twi)
		return TWI_ERR;
	}

	TWI_SNAPSHOT(twi)

	/* Send chip address */
	i2c_send_data(twi, addr);
	if (wait_for_status(twi) != TWI_STAT_TX_AW_ACK)
	{
		PRINT_TWI_STATUS(twi)
    return TWI_ERR;
	}
	
	TWI_SNAPSHOT(twi)

	/* Send data address */
	i2c_send_data(twi, cmd);
	if (wait_for_status(twi) != TWI_STAT_M_TXD_ACK)
	{
		PRINT_TWI_STATUS(twi)
		return TWI_ERR;
	}

	TWI_SNAPSHOT(twi)

	/* Start ACK-ing received data */
	iowrite32((TWI_CTL_A_ACK | ioread32(&twi->ctl)), &twi->ctl);

	if (wait_for_status(twi) != TWI_STAT_M_RXD_ACK)
	{
		PRINT_TWI_STATUS(twi)
		return TWI_ERR;
	}

	/* Read value */
	*value = i2c_read_data(twi);

	TWI_SNAPSHOT(twi)

	i2c_send_stop(twi);

	return TWI_SUCCESS;
}

/* Debug functions -----------------------------------------------------------*/
void twi_registers_snapshot(struct a20_twi *twi)
{
	printk(KERN_ALERT "[ TWI Debug ] addr 0x%x | xaddr 0x%x | data 0x%x |\
 ctl 0x%x |  stat 0x%x | clk 0x%x | reset 0x%x | efr 0x%x | lcr 0x%x \n",
				ioread32(&twi->addr),
				ioread32(&twi->xaddr),
				ioread32(&twi->data),
				ioread32(&twi->ctl),
				ioread32(&twi->stat),
				ioread32(&twi->clk),
				ioread32(&twi->reset),
				ioread32(&twi->efr),
				ioread32(&twi->lcr));
}

void get_twi_status_msg(struct a20_twi *twi)
{
	switch(ioread32(&twi->stat))
	{
		case TWI_STAT_BUS_ERROR:      				printk(KERN_ALERT "Bus error \n"); break;
		case TWI_STAT_TX_START:       				printk(KERN_ALERT "START condition transmitted  \n"); break;
		case TWI_STAT_TX_RSTART:     				printk(KERN_ALERT "Repeated START condition transmitted  \n"); break;
		case TWI_STAT_TX_AW_ACK:      				printk(KERN_ALERT "Address + Write bit transmitted, ACK received  \n"); break;
		case TWI_STAT_TX_AW_NAK:      				printk(KERN_ALERT "Address + Write bit transmitted, ACK not received  \n"); break;
		case TWI_STAT_M_TXD_ACK:       				printk(KERN_ALERT "Data byte transmitted in master mode, ACK received \n"); break;
		case TWI_STAT_M_TXD_NAK:      				printk(KERN_ALERT "Data byte transmitted in master mode, ACK not received \n"); break;
		case TWI_STAT_LOST_ARB:      				printk(KERN_ALERT "Arbitration lost in address or data byte \n"); break;
		case TWI_STAT_TX_AR_ACK:      				printk(KERN_ALERT "Address + Read bit transmitted, ACK received \n"); break;
		case TWI_STAT_TX_AR_NAK:     				printk(KERN_ALERT "Address + Read bit transmitted, ACK not received \n"); break;
		case TWI_STAT_M_RXD_ACK:      				printk(KERN_ALERT "Data byte received in master mode, ACK transmitted \n"); break;
		case TWI_STAT_M_RXD_NAK:      				printk(KERN_ALERT "Data byte received in master mode, not ACK transmitted \n"); break;
		
		case TWI_STAT_M_RX_AW_ACK:						printk(KERN_ALERT "Slave address + Write bit received, ACK transmitted \n"); break;
		case TWI_STAT_M_RX_AW_ARB_LOST:				printk(KERN_ALERT "Arbitration lost in address as master, slave address + Write bit received, ACK transmitted \n"); break;
		case TWI_STAT_RX_GCA:									printk(KERN_ALERT "General Call address received, ACK transmitted \n"); break;
		case TWI_STAT_M_RX_GCA_ARB_LOST:			printk(KERN_ALERT "Arbitration lost in address as master, General Call address received, ACK transmitted \n"); break;
		case TWI_STAT_M_AFTER_SLAVE_RXD_ACK: 	printk(KERN_ALERT "Data byte received after slave address received, ACK transmitted \n"); break;
		case TWI_STAT_M_AFTER_SLAVE_RXD_NACK: printk(KERN_ALERT "Data byte received after slave address received, NACK transmitted \n"); break;
		case TWI_STAT_RX_AFTER_CGE_ACK:				printk(KERN_ALERT "Data byte received after General Call received, ACK transmitted \n"); break;
		case TWI_STAT_RX_AFTER_CGE_NACK:			printk(KERN_ALERT "Data byte received after General Call received, NACK transmitted \n"); break;
		case TWI_STAT_S_RX_STP_MULTI_STR: 		printk(KERN_ALERT "STOP or repeated START condition received in slave mode \n"); break;
		case TWI_STAT_M_RX_AR: 								printk(KERN_ALERT "Slave address + Read bit received, ACK transmitted \n"); break;
		case TWI_STAT_M_RX_AR_ARB_LOST: 			printk(KERN_ALERT "Arbitration lost in address as master, slave address + Read bit received, ACK transmitted \n"); break;
		case TWI_STAT_S_TXD_ACK: 							printk(KERN_ALERT "Data byte transmitted in slave mode, ACK received \n"); break;
		case TWI_STAT_S_TXD_NACK:  						printk(KERN_ALERT "Data byte transmitted in slave mode, ACK not received \n"); break;
		case TWI_STAT_S_TXLB_ACK: 						printk(KERN_ALERT "Last byte transmitted in slave mode, ACK received \n"); break;
		case TWI_STAT_TX_SCND_ADDR_ACK: 			printk(KERN_ALERT "Second Address byte + Write bit transmitted, ACK received \n"); break;
		case TWI_STAT_TX_SCND_ADDR_NACK: 			printk(KERN_ALERT "Second Address byte + Write bit transmitted, ACK not received \n"); break;
		case TWI_STAT_IDLE:           				printk(KERN_ALERT "Bus idle : No relevant status information, INT_FLAG=0 \n"); break;
	default:
 		printk(KERN_ALERT "No STAT code matches \n"); break;
	}
}
// *********************** END OF FILE *****************************************
