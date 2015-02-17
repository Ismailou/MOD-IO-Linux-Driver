#include "twi.h"
#include <linux/types.h>
#include <asm/io.h>
#include <linux/delay.h>
//--#include <device/i2c.h>

#define	A20_TWI_BASE										0x01C2AC00
#define TWI_BASE(n)                     (A20_TWI_BASE + 0x400 * (n))
#define TWI_TIMEOUT                     (50 * 1000)

static uint8_t is_busy(struct a20_twi *twi);
static void configure_clock(struct a20_twi *twi, uint32_t speed_hz);
static void clear_interrupt_flag(struct a20_twi *twi);
static void i2c_send_data(struct a20_twi *twi, uint8_t data);
static enum twi_status wait_for_status(struct a20_twi *twi);
static void i2c_send_start(struct a20_twi *twi);
static void i2c_send_stop(struct a20_twi *twi);
int i2c_read(unsigned bus, unsigned chip, unsigned addr,
             unsigned alen, uint8_t *buf, unsigned len);
int i2c_write(unsigned bus, unsigned chip, unsigned addr,
              unsigned alen, const uint8_t *buf, unsigned len);
              
              
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

void a20_twi_init(uint8_t bus, uint32_t speed_hz)
{
	uint32_t i = TWI_TIMEOUT;
	struct a20_twi *twi = (void *)TWI_BASE(bus);

	configure_clock(twi, speed_hz);

	/* Enable the I²C bus */
	iowrite32(TWI_CTL_BUS_EN, &twi->ctl);
	/* Issue soft reset */
	iowrite32(1, &twi->reset);

	while (i-- && ioread32(&twi->reset))
		       udelay(1);
}

static void clear_interrupt_flag(struct a20_twi *twi)
{
	iowrite32(ioread32(&twi->ctl) & ~TWI_CTL_INT_FLAG, &twi->ctl);
}

static void i2c_send_data(struct a20_twi *twi, uint8_t data)
{
	iowrite32(data, &twi->data);
	clear_interrupt_flag(twi);
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

static void i2c_send_start(struct a20_twi *twi)
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

static void i2c_send_stop(struct a20_twi *twi)
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
	struct a20_twi *twi = (void *)TWI_BASE(bus);

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
	struct a20_twi *twi = (void *)TWI_BASE(bus);

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
