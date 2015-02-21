/**
******************************************************************************
* @file 		mod_io_driver.c
* @author 	Ismail ZEMNI (ismailzemni@gmail.com)
*						Mohamed Fadhel SASSI (mohamed.fadhel.sassi@gmail.com )
* @version 	1.0
* @date 		22/01/2015
* @brief		MOD-IO Device Driver.
******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/types.h>
#include <linux/cdev.h> // for cdev structure 
#include <linux/slab.h> // for kmalloc() and kfree()
#include <linux/uaccess.h> // for copy_to_user and copy_from_user
#include <linux/errno.h> // error code and defines: -EINTR(interrupted system call) or -EFAULT(bad address)
#include <linux/thread_info.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/ioctl.h>
#include <linux/device.h>
#include <asm/io.h>
#include "mod_io_driver.h"
#include "mod_io.h"
#include "twi.h"

/* Private define ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
typedef struct mod_io_handler {
	struct cdev *cdev;		/* Char device structure */
} mod_io_handler;

/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
int mod_io_open(struct inode *inode, struct file *filp);
int mod_io_release(struct inode *inode, struct file *filp);
ssize_t mod_io_read(struct file *filp, char __user *buff,
		size_t count, loff_t *offp);
ssize_t mod_io_write(struct file *filp, const char __user *buff,
		size_t count, loff_t *offp);
#if defined(HAVE_COMPAT_IOCTL) && defined(HAVE_UNLOCKED_IOCTL)
long mod_io_compat_ioctl (struct file *filp, unsigned int cmd,
															 unsigned long argp);
long mod_io_unlocked_ioctl (struct file *filp, unsigned int cmd,
															 unsigned long argp);
#else 
long mod_io_ioctl (struct inode *inode, struct file *filp, 
																unsigned int cmd, unsigned long argp);
#endif

/* Private variables ---------------------------------------------------------*/
// -- inode informations
dev_t dev;
static void __iomem *vtwi;
static struct class *cl;
static struct a20_twi *twi;

// -- file operation structure
struct file_operations fops = {
	.owner 	= THIS_MODULE,
	.llseek = NULL,
	.read 	= mod_io_read,
	.write 	= mod_io_write,
#if defined(HAVE_COMPAT_IOCTL) && defined(HAVE_UNLOCKED_IOCTL)
	.compat_ioctl 	= NULL,
	.unlocked_ioctl = mod_io_unlocked_ioctl,
#else 
	.ioctl 		= mod_io_ioctl,
#endif
	.open 		= mod_io_open,
	.release 	= mod_io_release,
};

mod_io_handler *mod_io_dev;

// **************** module functions *********************************

/*
 * Init function of MOD-IO module
 */
static int mod_io_init(void)
{
	printk(KERN_ALERT "[ MOD-IO Driver Debug ] Init MOD-IO module !");
	
	if (alloc_chrdev_region(&dev, 0, 1, "MOD-IO") == -1)
	{
#ifdef DEBUG
		printk(KERN_ALERT "[ MOD-IO Driver Debug ] Error  alloc_chrdev_region !\n");
#endif
		return -EINVAL;
	}

	printk(KERN_ALERT "[ MOD-IO Driver Debug ] Init allocated (major, minor)=(%d, %d) !\n", MAJOR(dev), MINOR(dev));

	mod_io_dev = kmalloc( sizeof(mod_io_dev), GFP_KERNEL);
	mod_io_dev->cdev = cdev_alloc();
	mod_io_dev->cdev->ops = &fops;
	mod_io_dev->cdev->owner = THIS_MODULE;

	cdev_add(mod_io_dev->cdev, dev, 1);
	
  if ((cl = class_create(THIS_MODULE, "twi")) == NULL)
  {
      unregister_chrdev_region(dev, 1);
      return -1;
  }
  
  if (device_create(cl, NULL, dev, NULL, "vtwi") == NULL)
  {
      class_destroy(cl);
      unregister_chrdev_region(dev, 1);
      return -1;
  }
    
	if ((vtwi = ioremap(TWI0_BASE_ADDR, TWI_SIZE)) == NULL)
	{
		  printk(KERN_ERR "[ MOD-IO Driver Debug ] Mapping TWI1 failed\n");
		  return -1;
	}
	
#ifdef DEBUG
		printk(KERN_ALERT "[ MOD-IO Driver Debug ] virtual address of TWI1 is : %p\n", vtwi);
#endif

	// -- init A20 TWI1
	twi = (void *)vtwi;

	twi_registers_snapshot(twi);

	a20_twi_init(vtwi,100000);
#ifdef DEBUG
		printk(KERN_ALERT "[ TWI Debug ] Initialize TWI \n");
#endif
	twi_registers_snapshot(twi);
	return 0;
}

/*
 * Clenup function of MOD-IO module
 */
static void mod_io_clenup(void){
#ifdef DEBUG
	printk(KERN_ALERT "[ MOD-IO Driver Debug ] cleanup MOD-IO module !\n");
#endif
	unregister_chrdev_region(dev, 1);
	cdev_del(mod_io_dev->cdev);
	device_destroy(cl, dev);
	class_destroy(cl);
	iounmap(vtwi);
}

/*
 * open function of MOD-IO module
 */
int mod_io_open(struct inode *inode, struct file *filp){
	unsigned int major = 0, minor = 0;
	pid_t process_pid = 0;
	
#ifdef DEBUG
	printk(KERN_ALERT "[ MOD-IO Driver Debug ] open MOD-IO module !\n");
#endif
	minor = iminor(inode);
	major = imajor(inode);
	
#ifdef DEBUG
	printk(KERN_ALERT "[ MOD-IO Driver Debug ] (major, minor) of requsted device are (%d,%d) !\n", major, minor);
#endif

	process_pid = current->pid;
#ifdef DEBUG
	printk(KERN_ALERT "[ MOD-IO Driver Debug ] Current process PID is : %d !\n", process_pid);
#endif

	return 0;
}

/*
 * release function of MOD-IO module
 */
int mod_io_release(struct inode *inode, struct file *filp){
#ifdef DEBUG
	printk(KERN_ALERT "[ MOD-IO Driver Debug ] release MOD-IO module !\n");
#endif
	return 0;
}

/*
 * open function of MOD-IO module
 */
ssize_t mod_io_read(struct file *filp, char __user *buff,
		size_t count, loff_t *offp){
#ifdef DEBUG
		printk(KERN_ALERT "[ MOD-IO Driver Debug ] MOD-IO read !!\n");
#endif
		return -EINTR;
}

/*
 * release function of MOD-IO module
 */
ssize_t mod_io_write(struct file *filp, const char __user *buff,
		size_t count, loff_t *offp){

	uint32_t tmp_reg = 0;

#ifdef DEBUG
		printk(KERN_ALERT "[ MOD-IO Driver Debug ] MOD-IO write !!\n");
#endif
/*
tmp_reg = ioread32(&twi->stat);
printk(KERN_ALERT "[ MOD-IO Driver Debug ] stat value 0x%x \n",tmp_reg);
	
	tmp_reg = ioread32(&twi->ctl);
printk(KERN_ALERT "[ MOD-IO Driver Debug ] crtl value 0x%x \n",tmp_reg);
	iowrite32(TWI_CTL_BUS_EN, &twi->ctl);
	tmp_reg = ioread32(&twi->ctl);
printk(KERN_ALERT "[ MOD-IO Driver Debug ] crtl value 0x%x \n",tmp_reg);

tmp_reg = ioread32(&twi->stat);
printk(KERN_ALERT "[ MOD-IO Driver Debug ] stat value 0x%x \n",tmp_reg);

	tmp_reg = ioread32(&twi->ctl);
printk(KERN_ALERT "[ MOD-IO Driver Debug ] crtl value before START 0x%x \n",tmp_reg);
	tmp_reg &= ~TWI_CTL_INT_FLAG;
	tmp_reg |= TWI_CTL_M_START;
	iowrite32(tmp_reg, &twi->ctl);

printk(KERN_ALERT "[ MOD-IO Driver Debug ] crtl value after START 0x%x \n", ioread32(&twi->ctl));
printk(KERN_ALERT "[ MOD-IO Driver Debug ] stat value 0x%x \n",ioread32(&twi->stat));

printk(KERN_ALERT "[ MOD-IO Driver Debug ] data value 0x%x \n",ioread32(&twi->data));
iowrite8(MOD_IO_ARRD_R, &twi->data);
printk(KERN_ALERT "[ MOD-IO Driver Debug ] data value 0x%x \n",ioread32(&twi->data));
printk(KERN_ALERT "[ MOD-IO Driver Debug ] stat value 0x%x \n",ioread32(&twi->stat));

twi_registers_snapshot(twi);
*/
/*
#ifdef DEBUG
		printk(KERN_ALERT "[ MOD-IO Driver Debug ] send i2c START \n");
#endif
	i2c_send_start(twi);
	twi_registers_snapshot(twi);

#ifdef DEBUG
		printk(KERN_ALERT "[ MOD-IO Driver Debug ] send MOD-IO adress \n");
#endif
	i2c_send_data(twi, MOD_IO_ARRD_R);
	twi_registers_snapshot(twi);

#ifdef DEBUG
		printk(KERN_ALERT "[ MOD-IO Driver Debug ] send MOD-IO command \n");
#endif
	i2c_send_data(twi, I2C_GET_DINPUTS);
	twi_registers_snapshot(twi);

#ifdef DEBUG
		printk(KERN_ALERT "[ MOD-IO Driver Debug ] read MOD-IO data \n");
#endif
	tmp_reg = i2c_read_data(twi);
	twi_registers_snapshot(twi);
#ifdef DEBUG
		printk(KERN_ALERT "[ MOD-IO Driver Debug ] value %d \n",tmp_reg);
#endif
	

#ifdef DEBUG
		printk(KERN_ALERT "[ MOD-IO Driver Debug ] send i2c STOP \n");
#endif
	i2c_send_stop(twi);
	twi_registers_snapshot(twi);
*/
//	if ( twi_write(twi, MOD_IO_ARRD_W, I2C_GET_DINPUTS,0x14) == TWI_ERR)
//		printk(KERN_ALERT "[ MOD-IO Driver Debug ] TWI send data error ! \n");
	if ( twi_read(twi, MOD_IO_ARRD_R, I2C_GET_DINPUTS, &tmp_reg) == TWI_ERR)
		printk(KERN_ALERT "[ MOD-IO Driver Debug ] TWI read data error ! \n");
	return count;
}

#if defined(HAVE_COMPAT_IOCTL) && defined(HAVE_UNLOCKED_IOCTL)
long mod_io_unlocked_ioctl (struct file *filp, unsigned int cmd,
											 unsigned long argp)
{
#else
long mod_io_ioctl (struct inode *inode, struct file *filp, 
												unsigned int cmd, unsigned long argp)
{
#endif

		uint32_t tmp_reg = 0;
		
#ifdef DEBUG
		printk(KERN_ALERT "[ MOD-IO Driver Debug ] IOCTL command !!\n");
#endif

	// -- Verify if command number is valid
	if ( _IOC_TYPE(cmd) != MOD_IO_IOC_MAGIC )
		return -ENOTTY;
	if ( _IOC_NR(cmd) > MOD_IO_IOC_MAXNBR )
		return -ENOTTY;

#ifdef DEBUG
		printk(KERN_ALERT "[ MOD-IO Driver Debug ] IOCTL command verification done : OK!\n");
#endif

	// -- Send start condition
	i2c_send_start(twi);
			
	// -- list of MOD-IO commands (extracted from user manual PDF)
	switch(cmd)
	{			
		case MOD_IO_IOC_SET_OUTPUTS: // -- Command to set relays
			// -- This is 0x58 shifted to left one time and added 0 as W: i2cSend(0xb0)
			i2c_send_data(twi, MOD_IO_ARRD_W);
			// -- Command to set relays: i2cSend(0x10)
			i2c_send_data(twi, I2C_SET_OUTPUTS);
			// 0x05 → 0b00000101 → This way we will set REL1 and REL3: i2cSend(0x05)
			i2c_send_data(twi, 0x05); // TODO later replace 0x05 by *argp
			break;
			
		case MOD_IO_IOC_GET_DINPUTS: // -- Read inputs commands
			i2c_send_data(twi, MOD_IO_ARRD_R);
			i2c_send_data(twi, I2C_GET_DINPUTS);
			tmp_reg = i2c_read_data(twi);
#ifdef DEBUG
		printk(KERN_ALERT "[ MOD-IO Driver Debug ] Inputs value is : 0x%x!\n", tmp_reg);
#endif
			argp = (long unsigned int)tmp_reg;
			break;
			
		case MOD_IO_IOC_GET_AIN_0:	// -- Read Analog input 0 commands
			i2c_send_data(twi, MOD_IO_ARRD_R);
			i2c_send_data(twi, I2C_GET_AIN_0);
			tmp_reg = i2c_read_data(twi);
#ifdef DEBUG
		printk(KERN_ALERT "[ MOD-IO Driver Debug ] Analog input 0 value is : 0x%x!\n", tmp_reg);
#endif
			argp = (long unsigned int)tmp_reg;
			break;
			
		case MOD_IO_IOC_GET_AIN_1:	// -- Read Analog input 1 commands
			i2c_send_data(twi, MOD_IO_ARRD_R);
			i2c_send_data(twi, I2C_GET_AIN_1);
			tmp_reg = i2c_read_data(twi);
#ifdef DEBUG
		printk(KERN_ALERT "[ MOD-IO Driver Debug ] Analog input 1 value is : 0x%x!\n", tmp_reg);
#endif
			argp = (long unsigned int)tmp_reg;
			break;
			
		case MOD_IO_IOC_GET_AIN_2:	// -- Read Analog input 2 commands
			i2c_send_data(twi, MOD_IO_ARRD_R);
			i2c_send_data(twi, I2C_GET_AIN_2);
			tmp_reg = i2c_read_data(twi);
#ifdef DEBUG
		printk(KERN_ALERT "[ MOD-IO Driver Debug ] Analog input 2 value is : 0x%x!\n", tmp_reg);
#endif
			argp = (long unsigned int)tmp_reg;
			break;
			
		case MOD_IO_IOC_GET_AIN_3:	// -- Read Analog input 3 commands
			i2c_send_data(twi, MOD_IO_ARRD_R);
			i2c_send_data(twi, I2C_GET_AIN_3);
			tmp_reg = i2c_read_data(twi);
#ifdef DEBUG
		printk(KERN_ALERT "[ MOD-IO Driver Debug ] Analog input 3 value is : 0x%x!\n", tmp_reg);
#endif
			argp = (long unsigned int)tmp_reg;
			break;
			
		case MOD_IO_IOC_SET_SLAVE_ADDR: // -- New Slave ADDR commands
			i2c_send_data(twi, MOD_IO_ARRD_W);
			// -- i2c_send_data(twi, I2C_SET_SLAVE_ADDR);
			// -- i2c_send_data(twi, (uint8_t)*argp);
#ifdef DEBUG
		printk(KERN_ALERT "[ MOD-IO Driver Debug ] New Slave ADDR value is : 0x%x!\n", tmp_reg);
#endif
			break;
			
		default:
			break;
	}
	
	// -- Send stop condition
	i2c_send_stop(twi);
	
	return 0;
}

// -- Init exit association
module_init(mod_io_init);
module_exit(mod_io_clenup);

// -- Property assigment
MODULE_LICENSE(LICENSE);
MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION(DESCRIPTION);

// *********************** END OF FILE *****************************************
