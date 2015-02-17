// *************************** Includes ******************************
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
#include "mod_io_driver.h"
#include "mod_io.h"
#include "twi.h"

// module argument's

// -- functions declarationp
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

// **************** Internal declaration ******************************
// -- inode informations
dev_t dev;

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

// -- data management structure
typedef struct mod_io_handler {
	struct cdev *cdev;		/* Char device structure */
} mod_io_handler;

mod_io_handler *mod_io_dev;
// **************** module functions *********************************

/*
 * Init function of MOD-IO module
 */
static int mod_io_init(void){
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
	
	return 0;
}

/*
 * exit function of MOD-IO module
 */
static void mod_io_clenup(void){
#ifdef DEBUG
	printk(KERN_ALERT "[ MOD-IO Driver Debug ] cleanup MOD-IO module !\n");
#endif

	unregister_chrdev_region(dev, 1);
	cdev_del(mod_io_dev->cdev);
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

	// -- list of MOD-IO commands (to extract from user manual PDF)
	switch(cmd)
	{
		case MOD_IO_IOC_NO_COMMAND:	// -- No commands
			break;
			
		case MOD_IO_IOC_SET_OUTPUTS: // -- Command to set relays
			/*
			// -- Send start condition
			i2cStart();
			// -- This is 0x58 shifted to left one time and added 0 as W: i2cSend(0xb0)
			i2cSend(MOD_IO_ARRD_W);
			// -- Command to set relays: i2cSend(0x10)
			i2cSend(MOD_IO_SET_OUTPUTS);
			// 0x05 → 0b00000101 → This way we will set REL1 and REL3: i2cSend(0x05)
			i2cSend(*argp);
			// -- Send stop condition
			i2cClose();
			*/
			break;
			
		case MOD_IO_IOC_GET_DINPUTS: // -- Read inputs commands
			break;
			
		case MOD_IO_IOC_GET_AIN_0:	// -- Read Analog input 0 commands
			break;
			
		case MOD_IO_IOC_GET_AIN_1:	// -- Read Analog input 1 commands
			break;
			
		case MOD_IO_IOC_GET_AIN_2:	// -- Read Analog input 2 commands
			break;
			
		case MOD_IO_IOC_GET_AIN_3:	// -- Read Analog input 3 commands
			break;
			
		case MOD_IO_IOC_SET_SLAVE_ADDR: // -- New Slave ADDR commands
			break;
			
		default:
			break;
	}

	return 0;
}

// -- Init exit association
module_init(mod_io_init);
module_exit(mod_io_clenup);

// -- Property assigment
MODULE_LICENSE(LICENSE);
MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION(DESCRIPTION);

// *********************** END OF FILE ********************
