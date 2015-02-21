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
    
	if ((vtwi = ioremap(TWI1_BASE_ADDR, TWI_SIZE)) == NULL)
	{
		  printk(KERN_ERR "[ MOD-IO Driver Debug ] Mapping TWI1 failed\n");
		  return -1;
	}
	
#ifdef DEBUG
		printk(KERN_ALERT "[ MOD-IO Driver Debug ] virtual address of TWI1 is : %p\n", vtwi);
#endif

	// -- init A20 TWI1
	twi = (void *)vtwi
	a20_twi_init(vtwi,100000);
	
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
		return -EINTR;
}

/*
 * release function of MOD-IO module
 */
ssize_t mod_io_write(struct file *filp, const char __user *buff,
		size_t count, loff_t *offp){
	return 0;
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

		uint32_t tmpreg;
		
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
			i2c_send_data(twi, I2C_GET_OUTPUTS);
			tmpreg = i2c_read_data(twi);
#ifdef DEBUG
		printk(KERN_ALERT "[ MOD-IO Driver Debug ] Inputs value is : %p!\n", tmpreg);
#endif
			*argp = tmp_reg;
			break;
			
		case MOD_IO_IOC_GET_AIN_0:	// -- Read Analog input 0 commands
			i2c_send_data(twi, MOD_IO_ARRD_R);
			i2c_send_data(twi, I2C_GET_AIN_0);
			tmpreg = i2c_read_data(twi);
#ifdef DEBUG
		printk(KERN_ALERT "[ MOD-IO Driver Debug ] Analog input 0 value is : %p!\n", tmpreg);
#endif
			*argp = tmp_reg;
			break;
			
		case MOD_IO_IOC_GET_AIN_1:	// -- Read Analog input 1 commands
			i2c_send_data(twi, MOD_IO_ARRD_R);
			i2c_send_data(twi, I2C_GET_AIN_1);
			tmpreg = i2c_read_data(twi);
#ifdef DEBUG
		printk(KERN_ALERT "[ MOD-IO Driver Debug ] Analog input 1 value is : %p!\n", tmpreg);
#endif
			*argp = tmp_reg;
			break;
			
		case MOD_IO_IOC_GET_AIN_2:	// -- Read Analog input 2 commands
			i2c_send_data(twi, MOD_IO_ARRD_R);
			i2c_send_data(twi, I2C_GET_AIN_2);
			tmpreg = i2c_read_data(twi);
#ifdef DEBUG
		printk(KERN_ALERT "[ MOD-IO Driver Debug ] Analog input 2 value is : %p!\n", tmpreg);
#endif
			*argp = tmp_reg;
			break;
			
		case MOD_IO_IOC_GET_AIN_3:	// -- Read Analog input 3 commands
			i2c_send_data(twi, MOD_IO_ARRD_R);
			i2c_send_data(twi, I2C_GET_AIN_3);
			tmpreg = i2c_read_data(twi);
#ifdef DEBUG
		printk(KERN_ALERT "[ MOD-IO Driver Debug ] Analog input 3 value is : %p!\n", tmpreg);
#endif
			*argp = tmp_reg;
			break;
			
		case MOD_IO_IOC_SET_SLAVE_ADDR: // -- New Slave ADDR commands
			i2c_send_data(twi, MOD_IO_ARRD_W);
			// -- i2c_send_data(twi, I2C_SET_SLAVE_ADDR);
			// -- i2c_send_data(twi, (uint8_t)*argp);
#ifdef DEBUG
		printk(KERN_ALERT "[ MOD-IO Driver Debug ] New Slave ADDR value is : %p!\n", tmpreg);
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
