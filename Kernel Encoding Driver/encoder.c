#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h>	/* printk() */
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/seq_file.h>
#include <linux/cdev.h>

#include <asm/switch_to.h>		/* cli(), *_flags */
#include <asm/uaccess.h>	/* copy_*_user */

#include "encoder.h"

#define ENCODER_MAJOR 0
#define ENCODER_NR_DEVS 4
#define ENCODER_QUANTUM 4000
#define ENCODER_QSET 1000

#define ENC_UTF8 0
#define ENC_UTF16 1
#define ENC_UTF32 2
#define ENC_88599 3
#define ENC_88591 4


int encoder_major = ENCODER_MAJOR;
int encoder_minor = 0;
int encoder_nr_devs = ENCODER_NR_DEVS;
int encoder_quantum = ENCODER_QUANTUM;
int encoder_qset = ENCODER_QSET;
int current_input_encoding  = ENC_UTF8;
int current_output_encoding = ENC_UTF8;

module_param(encoder_major, int, S_IRUGO);
module_param(encoder_minor, int, S_IRUGO);
module_param(encoder_nr_devs, int, S_IRUGO);
module_param(encoder_quantum, int, S_IRUGO);
module_param(encoder_qset, int, S_IRUGO);
module_param(current_input_encoding, int, S_IRUGO);
module_param(current_output_encoding, int, S_IRUGO);

MODULE_LICENSE("Dual BSD/GPL");



//#define SET_INPUT_ENCODING 0
//#define SET_OUTPUT_ENCODING 1


struct encoder_dev {
    char **data;
    int quantum;
    int qset;
    unsigned long size;
    struct semaphore sem;
    struct cdev cdev;
	atomic_t encoder_s_available;
};

struct encoder_dev *encoder_devices;


int encoder_trim(struct encoder_dev *dev)
{
    int i;

    if (dev->data) {
        for (i = 0; i < dev->qset; i++) {
            if (dev->data[i])
                kfree(dev->data[i]);
        }
        kfree(dev->data);
    }
    dev->data = NULL;
    dev->quantum = encoder_quantum;
    dev->qset = encoder_qset;
    dev->size = 0;
    return 0;
}



int encoder_open(struct inode *inode, struct file *filp)
{
    struct encoder_dev *dev;

    dev = container_of(inode->i_cdev, struct encoder_dev, cdev);
    filp->private_data = dev;
	
	
	if (! atomic_dec_and_test (&dev->encoder_s_available)) {
	atomic_inc(&dev->encoder_s_available);
	return -EBUSY; /* already open */
	}	
	
	
    /* trim the device if open was write-only */
    if ((filp->f_flags & O_ACCMODE) == O_WRONLY) {
        if (down_interruptible(&dev->sem))
            return -ERESTARTSYS;
        encoder_trim(dev);
        up(&dev->sem);
    }
    return 0;
}

int encoder_release(struct inode *inode, struct file *filp)
{
    return 0;
}


ssize_t encoder_read(struct file *filp, char __user *buf, size_t count,
                   loff_t *f_pos)
{
    struct encoder_dev *dev = filp->private_data;
    int quantum = dev->quantum;
    int s_pos, q_pos;
    ssize_t retval = 0;

    if (down_interruptible(&dev->sem))
        return -ERESTARTSYS;
    if (*f_pos >= dev->size)
        goto out;
    if (*f_pos + count > dev->size)
        count = dev->size - *f_pos;

    s_pos = (long) *f_pos / quantum;
    q_pos = (long) *f_pos % quantum;

    if (dev->data == NULL || ! dev->data[s_pos])
        goto out;
	if (count > quantum - q_pos)
        count = quantum - q_pos;
	
	char* decoded_new_data;
    /* read only up to the end of this quantum */
	switch(current_output_encoding){
		case ENC_UTF8:
			/*do nothing*/
			break;
		case ENC_UTF16:
			count = utf8_to_utf16(dev->data[s_pos] + q_pos, count, decoded_new_data); 
			break;
		case ENC_UTF32:
			count = utf8_to_utf32(dev->data[s_pos] + q_pos, count, decoded_new_data);
			break;
		case ENC_88591:
			count = utf8_to_iso88591(dev->data[s_pos] + q_pos, count, decoded_new_data);
			break;	
		case ENC_88599:
			count = utf8_to_iso88599(dev->data[s_pos] + q_pos, count, decoded_new_data);
			break;
	}	
    
    if (copy_to_user(buf, decoded_new_data, count)) {
        retval = -EFAULT;
        goto out;
    }
    *f_pos += count;
    retval = count;

  out:
    up(&dev->sem);
    return retval;
}
                   
                   
                   
                   
ssize_t encoder_write(struct file *filp, const char __user *buf, size_t count,
                    loff_t *f_pos)
{
    struct encoder_dev *dev = filp->private_data;
    int quantum = dev->quantum, qset = dev->qset;
    int s_pos, q_pos;
    ssize_t retval = -ENOMEM;

    if (down_interruptible(&dev->sem))
        return -ERESTARTSYS;

    if (*f_pos >= quantum * qset) {
        retval = 0;
        goto out;
    }

    s_pos = (long) *f_pos / quantum;
    q_pos = (long) *f_pos % quantum;

    if (!dev->data) {
        dev->data = kmalloc(qset * sizeof(char *), GFP_KERNEL);
        if (!dev->data)
            goto out;
        memset(dev->data, 0, qset * sizeof(char *));
    }
    if (!dev->data[s_pos]) {
        dev->data[s_pos] = kmalloc(quantum, GFP_KERNEL);
        if (!dev->data[s_pos])
            goto out;
    }
    /* write only up to the end of this quantum */
	char* encoded_new_data;
	
	switch(current_input_encoding){
		case ENC_UTF8:
			encoded_new_data = dev->data[s_pos] + q_pos;
			break;
		case ENC_UTF16:
		    count = utf16_to_utf8(dev->data[s_pos] + q_pos, count, encoded_new_data);
			break;
		case ENC_UTF32:
			count = utf32_to_utf8(dev->data[s_pos] + q_pos, count, encoded_new_data);
			break;
		case ENC_88591:
			/*do nothing*/
			break;
		case ENC_88599:
			/*do nothing*/
			break;
	}
	
	
    if (count > quantum - q_pos)
        count = quantum - q_pos;

    if (copy_from_user(encoded_new_data, buf, count)) {
        retval = -EFAULT;
        goto out;
    }
    *f_pos += count;
    retval = count;

    /* update the size */
    if (dev->size < *f_pos)
        dev->size = *f_pos;

  out:
    up(&dev->sem);
    return retval;
}
long encoder_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{

	int err = 0, tmp;
	int retval = 0;

	/*
	 * extract the type and number bitfields, and don't decode
	 * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
	 */
	if (_IOC_TYPE(cmd) != ENCODER_IOC_MAGIC) return -ENOTTY;
	if (_IOC_NR(cmd) > ENCODER_IOC_MAXNR) return -ENOTTY;

	/*
	 * the direction is a bitmask, and VERIFY_WRITE catches R/W
	 * transfers. `Type' is user-oriented, while
	 * access_ok is kernel-oriented, so the concept of "read" and
	 * "write" is reversed
	 */
	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err =  !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
	if (err) return -EFAULT;

	switch(cmd) {
	  case GET_ENCODER_CURRENT_INPUT:
		retval = __put_user(current_input_encoding, (int __user *)arg);
		break;
	  case GET_ENCODER_CURRENT_OUTPUT:
		retval = __put_user(current_output_encoding, (int __user *)arg);
		break;
	  case SET_ENCODER_CURRENT_INPUT:
		if (! capable (CAP_SYS_ADMIN))
			return -EPERM;
		current_input_encoding = arg;  
		break;
	  case SET_ENCODER_CURRENT_OUTPUT:		
		if (! capable (CAP_SYS_ADMIN))
			return -EPERM;
		current_output_encoding = arg;
		break;
	   case QUERY_ENCODER_CURRENT_INPUT:
		 return current_input_encoding;
	   case QUERY_ENCODER_CURRENT_OUTPUT:
		 return current_output_encoding;
	  default:  /* redundant, as cmd was checked against MAXNR */
		return -ENOTTY;
	}
	return retval;
}


loff_t encoder_llseek(struct file *filp, loff_t off, int whence)
{
    struct encoder_dev *dev = filp->private_data;
    loff_t newpos;

    switch(whence) {
        case 0: /* SEEK_SET */
            newpos = off;
            break;

        case 1: /* SEEK_CUR */
            newpos = filp->f_pos + off;
            break;

        case 2: /* SEEK_END */
            newpos = dev->size + off;
            break;

        default: /* can't happen */
            return -EINVAL;
    }
    if (newpos < 0)
        return -EINVAL;
    filp->f_pos = newpos;
    return newpos;
}


struct file_operations encoder_fops = {
    .owner =    THIS_MODULE,
    .llseek =   encoder_llseek,
    .read =     encoder_read,
    .write =    encoder_write,
    .unlocked_ioctl =  encoder_ioctl,
    .open =     encoder_open,
    .release =  encoder_release,
};

void encoder_cleanup_module(void)
{
    int i;
    dev_t devno = MKDEV(encoder_major, encoder_minor);

    if (encoder_devices) {
        for (i = 0; i < encoder_nr_devs; i++) {
            encoder_trim(encoder_devices + i);
            cdev_del(&encoder_devices[i].cdev);
        }
    kfree(encoder_devices);
    }

    unregister_chrdev_region(devno, encoder_nr_devs);
}

int encoder_init_module(void)
{
    int result, i;
    int err;
    dev_t devno = 0;
    struct encoder_dev *dev;

    if (encoder_major) {
        devno = MKDEV(encoder_major, encoder_minor);
        result = register_chrdev_region(devno, encoder_nr_devs, "encoder");
    } else {
        result = alloc_chrdev_region(&devno, encoder_minor, encoder_nr_devs,
                                     "encoder");
        encoder_major = MAJOR(devno);
    }
    if (result < 0) {
        printk(KERN_WARNING "encoder: can't get major %d\n", encoder_major);
        return result;
    }

    encoder_devices = kmalloc(encoder_nr_devs * sizeof(struct encoder_dev),
                            GFP_KERNEL);
    if (!encoder_devices) {
        result = -ENOMEM;
        goto fail;
    }
    memset(encoder_devices, 0, encoder_nr_devs * sizeof(struct encoder_dev));
	
    /* Initialize each device. */
    for (i = 0; i < encoder_nr_devs; i++) {
        dev = &encoder_devices[i];
        dev->quantum = encoder_quantum;
        dev->qset = encoder_qset;
		// GÝVES ERROR HERE dev->encoder_s_available = ATOMIC_INIT(1);
		dev->encoder_s_available = ((atomic_t) { (1) });
        sema_init(&dev->sem,1);
        devno = MKDEV(encoder_major, encoder_minor + i);
        cdev_init(&dev->cdev, &encoder_fops);
        dev->cdev.owner = THIS_MODULE;
        dev->cdev.ops = &encoder_fops;
        err = cdev_add(&dev->cdev, devno, 1);
        if (err)
            printk(KERN_NOTICE "Error %d adding encoder%d", err, i);
    }

    return 0; /* succeed */

  fail:
    encoder_cleanup_module();
    return result;
}

unsigned int utf32_to_utf8(char* in_data, int size,  char* out_data){
	int in_size = size;
	int out_size = (in_size + 1)/4;
	out_data = (char*) kmalloc(  out_size * sizeof(char), GFP_KERNEL); 
	int i = 0;
	for (i = 0; i < in_size ; i = i + 4 )
	{
		out_data[(i+1)/4] = in_data[i];
	}
	return out_size;
}

unsigned int utf16_to_utf8(char* in_data, int size, char* out_data){
	int in_size = size;
	int out_size = (in_size + 1)/2;
	out_data = (char*) kmalloc(  out_size * sizeof(char),GFP_KERNEL); 
	int i = 0;
	for (i = 0; i < in_size ; i = i + 2 )
	{
		out_data[(i+1)/2] = in_data[i];
	}
	return out_size;
}

unsigned int utf8_to_utf16(char* in_data, int size, char* out_data){
	int in_size = size;
	out_data = (char*) kmalloc( in_size* 2 * sizeof(char),GFP_KERNEL); 
	int i = 0;
	int j = 0;
	for (i = 0; i < in_size ; i++ )
	{
		out_data[j++]   = 0x0;
		out_data[j] = in_data[i];
	}
	return in_size*2;
}

unsigned int utf8_to_utf32(char* in_data, int size, char* out_data){
	int in_size = size;
	out_data = (char*) kmalloc( in_size* 4 * sizeof(char), GFP_KERNEL);
	int i = 0;
	int j = 0;
	for (i = 0; i < in_size ; i++ )
	{
		out_data[j++] = 0x0;
		out_data[j++] = 0x0;
		out_data[j++] = 0x0;
		out_data[j] = in_data[i];
	}
	return in_size*4;

}
unsigned int utf8_to_iso88599(char* in_data, int size, char* out_data){
	int in_size = size;
	out_data = (char*) kmalloc( in_size * sizeof(char),GFP_KERNEL); 
	int i = 0;
	for (i = 0; i < in_size ; i++ )
	{
		if ( in_data[i] > 256){
			out_data[i] = '?';
		}else
		    out_data[i] = in_data[i];
	}
	return in_size;
}
unsigned int utf8_to_iso88591(char* in_data, int size, char* out_data){
	int in_size = size;
	out_data = (char*) kmalloc( in_size * sizeof(char),GFP_KERNEL); 
	int i = 0;
	for (i = 0; i < in_size ; i++ )
	{
		if ( in_data[i] > 256){
			out_data[i] = '?';
		}else
		    out_data[i] = in_data[i];
	}
	return in_size;
}

module_init(encoder_init_module);
module_exit(encoder_cleanup_module);

