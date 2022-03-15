/**
 * @file aesdchar.c
 * @brief Functions and data related to the AESD char driver implementation
 *
 * Based on the implementation of the "scull" device driver, found in
 * Linux Device Drivers example code.
 *
 * @author Dan Walkes
 * @date 2019-10-22
 * @copyright Copyright (c) 2019
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/fs.h> // file_operations
#include <linux/slab.h> 
#include <linux/uaccess.h> 
#include <linux/string.h>
#include "aesdchar.h"
int aesd_major =   0; // use dynamic major
int aesd_minor =   0;

MODULE_AUTHOR("Karthik Baggaon Rajendra"); /** TODO: fill in your name **/
MODULE_LICENSE("Dual BSD/GPL");

struct aesd_dev aesd_device;

int aesd_open(struct inode *inode, struct file *filp)
{
	struct aesd_dev *dev = NULL; 
	PDEBUG("open");
	/**
	 * TODO: handle open
	 */
	
	dev = container_of(inode->i_cdev, struct aesd_dev, cdev);
	filp->private_data = dev;  
	 
	return 0;
}

int aesd_release(struct inode *inode, struct file *filp)
{
	PDEBUG("release");
	/**
	 * TODO: handle release
	 */
	 
	 
	return 0;
}

ssize_t aesd_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{
		
	struct aesd_dev* device = filp->private_data;
	struct aesd_buffer_entry * read_entry = NULL;
	size_t bytes_to_read = 0;
	size_t entry_offset_byte = 0;
	size_t available_bytes = 0;
	ssize_t retval = 0;	
	
	PDEBUG("read %zu bytes with offset %lld",count,*f_pos);
	/**
	 * TODO: handle read
	 */
	 
	if(mutex_lock_interruptible(&device->lock))
		return -ERESTARTSYS; 
	
	read_entry = aesd_circular_buffer_find_entry_offset_for_fpos(&device->device_buffer, *f_pos, &entry_offset_byte);	
		
	if(read_entry == NULL) 
	{
	   retval = 0;
	   goto out;
	} 
	
	available_bytes = read_entry->size - entry_offset_byte;
	
	if(available_bytes > count)
	{
	   bytes_to_read = count;
	}
	else
	{ 
	   bytes_to_read = available_bytes;
	}
	
	int ret = copy_to_user(buf, read_entry->buffptr + entry_offset_byte, bytes_to_read);

	if(ret)
	{
	   retval = -EFAULT;
	   goto out;
	}
	
	retval= bytes_to_read;
	
	*f_pos += bytes_to_read;
	
out: 
	mutex_unlock(&device->lock);	
	return retval;	
	 
}

ssize_t aesd_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
	ssize_t retval = -ENOMEM;
	PDEBUG("write %zu bytes with offset %lld",count,*f_pos);
	/**
	 * TODO: handle write
	 */
	 
	const char* ret_entry = NULL;
	struct aesd_dev* device = filp->private_data;
	
	if(mutex_lock_interruptible(&device->lock))
		return -ERESTARTSYS;
		
	if(device->write_entry.size == 0)
	{
	   device->write_entry.buffptr = kzalloc(count, GFP_KERNEL);
	}	
	else 
	{
	   device->write_entry.buffptr = krealloc(device->write_entry.buffptr, device->write_entry.size + count, GFP_KERNEL);
	}
	 
	if(device->write_entry.buffptr == NULL)
	{
	   retval = -ENOMEM;
	   goto out;
	}  
	 
	retval = count;
	
	size_t error_bytes_num = copy_from_user((void*)(&device->write_entry.buffptr[device->write_entry.size]), buf, count);
	
	if(error_bytes_num)
	{
	   retval = retval - error_bytes_num;
	}
	
	device->write_entry.size += retval;
	
	
	if(strchr((char*)(device->write_entry.buffptr), '\n'))
	{
	   ret_entry = aesd_circular_buffer_add_entry(&device->device_buffer, &device->write_entry);
	  
	   if(ret_entry)
	   {
	      kfree(ret_entry);
	   }
	
	   device->write_entry.buffptr = NULL;
	   device->write_entry.size = 0; 
	}
out: 
	mutex_unlock(&device->lock);	
	return retval;
}


struct file_operations aesd_fops = {
	.owner =    THIS_MODULE,
	.read =     aesd_read,
	.write =    aesd_write,
	.open =     aesd_open,
	.release =  aesd_release,
};

static int aesd_setup_cdev(struct aesd_dev *dev)
{
	int err, devno = MKDEV(aesd_major, aesd_minor);

	cdev_init(&dev->cdev, &aesd_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &aesd_fops;
	err = cdev_add (&dev->cdev, devno, 1);
	if (err) {
		printk(KERN_ERR "Error %d adding aesd cdev", err);
	}
	return err;
}



int aesd_init_module(void)
{
	dev_t dev = 0;
	int result;
	result = alloc_chrdev_region(&dev, aesd_minor, 1,
			"aesdchar");
	aesd_major = MAJOR(dev);
	if (result < 0) {
		printk(KERN_WARNING "Can't get major %d\n", aesd_major);
		return result;
	}
	memset(&aesd_device,0,sizeof(struct aesd_dev));

	/**
	 * TODO: initialize the AESD specific portion of the device
	 */
	 
	mutex_init(&aesd_device.lock);

	result = aesd_setup_cdev(&aesd_device);

	if( result ) {
		unregister_chrdev_region(dev, 1);
	}
	return result;

}

void aesd_cleanup_module(void)
{

	dev_t devno = MKDEV(aesd_major, aesd_minor);

	cdev_del(&aesd_device.cdev);
	
	uint8_t index;
	
	struct aesd_buffer_entry *entry;

	/**
	 * TODO: cleanup AESD specific poritions here as necessary
	 */
	 
	kfree(aesd_device.write_entry.buffptr);
	
	AESD_CIRCULAR_BUFFER_FOREACH(entry, &aesd_device.device_buffer, index){
		if(entry->buffptr != NULL)
			kfree(entry->buffptr);
			}


	unregister_chrdev_region(devno, 1);
}



module_init(aesd_init_module);
module_exit(aesd_cleanup_module);
