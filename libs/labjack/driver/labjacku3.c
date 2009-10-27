/*
 * LabJack U3 USB Linux driver - 0.32
 *
 *
 * Copyright (C) 2005 LabJack Corporation <support@labjack.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, version 2.
 *
 * This driver is derived from the USB Skeleton driver 1.1.
 *   Copyright (C) 2001-2003 Greg Kroah-Hartman (greg@kroah.com)
 *
 * History:
 *
 * 2005-12-01 - 0.1
 * -initial release
 *
 * 2006-02-07 - 0.2
 * -fixed read so that smaller buffers than expected can be received
 *
 * 2006-03-28 - 0.21
 * -removed "mode" from labjacku3_class struct and URB_ASYNC_UNLINK in kernels
 *  2.6.14 and above
 * -removed "owner" from labjacku3_driver struct for kernels 2.6.16 and above
 *
 * 2006-10-09 - 0.30
 * -added the IOCTL command IOCTL_LJ_SET_READ_TIMEOUT to set read timeouts
 * -replaced MODULE_PARM with module_param
 *
 * 2007-02-19 - 0.31
 * -in labjacku3_read, now using a loop for usb_bulk_msg reads
 *
 * 2007-09-04 - 0.32
 * -removed #include <linux/config.h>
 *
 */

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/smp_lock.h>
#include <linux/completion.h>
#include <asm/uaccess.h>
#include <linux/usb.h>
#include <linux/version.h>

#ifdef CONFIG_USB_DEBUG
static int debug = 1;
#else
static int debug = 0;
#endif

/* Use our own dbg macro */
#undef dbg
#define dbg(format, arg...) do { if (debug) printk(KERN_DEBUG __FILE__ ": " format "\n" , ## arg); } while (0)

/* Version Information */
#define DRIVER_VERSION "v0.32"
#define DRIVER_AUTHOR "LabJack, support@labjack.com"
#define DRIVER_DESC "USB LabJack U3 Driver"

/* Timeout for reads */
#define READ_TIMEOUT 5*HZ

/* Module parameters */
//MODULE_PARM(debug, "i");
module_param( debug, int, S_IRUGO | S_IWUSR );
MODULE_PARM_DESC( debug, "Debug enabled or not" );

#define USB_LABJACK_VENDOR_ID		0x0cd5
#define USB_LABJACKU3_PRODUCT_ID	0x0003

/* IO controls */
#define IOCTL_LJ_SET_READ_TIMEOUT	0x1

/* table of devices that work with this driver */

static struct usb_device_id labjacku3_table [] = {
	{ USB_DEVICE( USB_LABJACK_VENDOR_ID, USB_LABJACKU3_PRODUCT_ID ) },
	{ }					/* Terminating entry */
};

MODULE_DEVICE_TABLE( usb, labjacku3_table );

/* Get a minor range for your devices from the usb maintainer */
#define USB_LABJACKU3_MINOR_BASE	208

/* Structure to hold all of our device specific stuff */

struct usb_labjacku3 {

	struct usb_device*	udev;				/* save off the usb device pointer */

	struct usb_interface*	interface;			/* the interface for this device */
	unsigned char		minor;				/* the starting minor number for this device */
	unsigned char		num_ports;			/* the number of ports this device has */
	char			num_interrupt_in;		/* number of interrupt in endpoints we have */
	char			num_bulk_in;			/* number of bulk in endpoints we have */
	char			num_bulk_out;			/* number of bulk out endpoints we have */
	unsigned char*		bulk_in_buffer[2];		/* the buffer to receive data */
	size_t			bulk_in_size[2];		/* the size of the receive buffer */
	__u8			bulk_in_endpointAddr[2];	/* the address of the bulk in endpoint */
	unsigned char*		bulk_out_buffer[2];		/* the buffer to send data */
	size_t			bulk_out_size[2];		/* the size of the send buffer */

	struct urb*		write_urb[2];			/* the urb used to send data */
	__u8			bulk_out_endpointAddr[2];	/* the address of the bulk out endpoint */
	atomic_t		write_busy;			/* true iff write urb is busy */

	struct completion	write_finished;			/* wait for the write to finish */
	int			open;				/* if the port is open or not */
	int			present;			/* if the device is not disconnected */

	struct semaphore	sem;				/* locks this structure */
	int			read_timeout;			/* timeout for reads, in ms */
};


/* prevent races between open() and disconnect() */
static DECLARE_MUTEX( disconnect_sem );

/* local function prototypes */
static ssize_t labjacku3_read( struct file *file, char *buffer, size_t count, loff_t *ppos );
static ssize_t labjacku3_write( struct file *file, const char *buffer, size_t count, loff_t *ppos );
static int labjacku3_ioctl( struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg );
static int labjacku3_open( struct inode *inode, struct file *file );
static int labjacku3_release( struct inode *inode, struct file *file );

static int labjacku3_probe( struct usb_interface *interface, const struct usb_device_id *id );
static void labjacku3_disconnect( struct usb_interface *interface );

static void labjacku3_write_bulk_callback( struct urb *urb, struct pt_regs *regs );

static struct file_operations labjacku3_fops = {
	.owner =	THIS_MODULE,
	.read =		labjacku3_read,
	.write =	labjacku3_write,
	.ioctl =	labjacku3_ioctl,
	.open =		labjacku3_open,
	.release =	labjacku3_release,
};

/*
 * usb class driver info in order to get a minor number from the usb core,
 * and to have the device registered with devfs and the driver core
 */

static struct usb_class_driver labjacku3_class = {
	.name =		"usb/labjacku3_%d",
	.fops =		&labjacku3_fops,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,14)
	.mode =		S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH,
#endif
	.minor_base =	USB_LABJACKU3_MINOR_BASE,
};

/* usb specific object needed to register this driver with the usb subsystem */

static struct usb_driver labjacku3_driver = {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,16)
	.owner =	THIS_MODULE,
#endif
	.name =		"labjacku3",
	.probe =	labjacku3_probe,
	.disconnect =	labjacku3_disconnect,
	.id_table =	labjacku3_table,
};


/**
 *	usb_labjacku3_debug_data
 */
static inline void usb_labjacku3_debug_data( const char *function, int size, const unsigned char *data )
{
	int i;

	if ( !debug )
		return;

	printk( KERN_DEBUG __FILE__": %s - length = %d, data = ",
	        function, size );

	for ( i = 0; i < size; ++i )
		printk( "%.2x ", data[i] );

	printk( "\n" );
}


/**
 *	labjacku3_delete
 */
static inline void labjacku3_delete( struct usb_labjacku3 *dev )
{
	int i;

	for ( i = 0; i < 2; i++ ) {
		kfree( dev->bulk_in_buffer[i] );
		usb_buffer_free( dev->udev, dev->bulk_out_size[i],
		                 dev->bulk_out_buffer[i],
		                 dev->write_urb[i]->transfer_dma );

		usb_free_urb( dev->write_urb[i] );
	}

	kfree( dev );
}


/**
 *	labjacku3_open
 */
static int labjacku3_open( struct inode *inode, struct file *file )
{

	struct usb_labjacku3 *dev = NULL;

	struct usb_interface *interface;
	int subminor;
	int retval = 0;

	dbg( "%s", __FUNCTION__ );

	subminor = iminor( inode );

	/* prevent disconnects */
	down( &disconnect_sem );

	interface = usb_find_interface( &labjacku3_driver, subminor );

	if ( !interface ) {
		err( "%s - error, can't find device for minor %d",
		     __FUNCTION__, subminor );
		retval = -ENODEV;
		goto exit_no_device;
	}

	dev = usb_get_intfdata( interface );

	if ( !dev ) {
		retval = -ENODEV;
		goto exit_no_device;
	}

	/* lock this device */
	down( &dev->sem );

	/* increment our usage count for the driver */
	++dev->open;

	/* save our object in the file's private structure */
	file->private_data = dev;

	/* unlock this device */
	up( &dev->sem );

exit_no_device:
	up( &disconnect_sem );

	return retval;
}


/**
 *	labjacku3_release
 */
static int labjacku3_release( struct inode *inode, struct file *file )
{

	struct usb_labjacku3 *dev;
	int retval = 0;

	dev = ( struct usb_labjacku3 * )file->private_data;

	if ( dev == NULL ) {
		dbg( "%s - object is NULL", __FUNCTION__ );
		return -ENODEV;
	}

	dbg( "%s - minor %d", __FUNCTION__, dev->minor );

	/* lock our device */
	down( &dev->sem );

	if ( dev->open <= 0 ) {
		dbg( "%s - device not opened", __FUNCTION__ );
		retval = -ENODEV;
		goto exit_not_opened;
	}

	/* wait for any bulk writes that might be going on to finish up */
	if ( atomic_read( &dev->write_busy ) )
		wait_for_completion( &dev->write_finished );

	--dev->open;

	if ( !dev->present && !dev->open ) {
		/* the device was unplugged before the file was released */
		up( &dev->sem );
		labjacku3_delete( dev );
		return 0;
	}

exit_not_opened:

	up( &dev->sem );
	return retval;
}


/**
 *	labjacku3_read
 *
 *	By default, endpoint 1 is used to read from the device.	To use the endpoint 2,
 *	set bit 16 of count, or add 32768 to count.
 *
 */
static ssize_t labjacku3_read( struct file *file, char *buffer, size_t count, loff_t *ppos )
{

	struct usb_labjacku3 *dev;
	int retval = 0;
	unsigned int bytes_read;
	int bytes_read_count = 0;
	int ep = 0;

	dev = ( struct usb_labjacku3 * )file->private_data;

	dbg( "%s - minor %d, count = %ld", __FUNCTION__, dev->minor, count );

	/* lock this object */
	down( &dev->sem );

	/* verify that the device wasn't unplugged */

	if ( !dev->present ) {
		up( &dev->sem );
		return -ENODEV;
	}

	if ( count <= 0 ) {
		dbg( "%s - read request of 0 bytes", __FUNCTION__ );
		goto exit;
	}

	if ( count < 32768 ) {
		ep = 0;
	}
	else {
		ep = 1;
		count -= 32768;
	}

	while ( count > 0 ) {
		/* do a blocking bulk read to get data from the device */
		retval = usb_bulk_msg( dev->udev,
		                       usb_rcvbulkpipe( dev->udev,
		                                        dev->bulk_in_endpointAddr[ep] ),
		                       dev->bulk_in_buffer[ep],
		                       min( dev->bulk_in_size[ep], count ),
		                       &bytes_read, dev->read_timeout );

		dbg( "%s - ep = %d, count = %ld, bytes_read = %d", __FUNCTION__, ep + 1, count, bytes_read );
		/* if the read was successful, copy the data to userspace */

		if ( !retval  || retval == -ETIMEDOUT ) {
			if ( copy_to_user( buffer + bytes_read_count, dev->bulk_in_buffer[ep], min( ( size_t )bytes_read, count ) ) ) {
				retval = -EFAULT;
				goto exit;
			}
			else
				bytes_read_count += bytes_read;
		}
		else
			goto exit;

		if ( count > bytes_read ) {
			if ( bytes_read <= 0 )
				goto set_retval;

			count -= bytes_read;
		}
		else
			count = 0;

		if ( dev->bulk_in_size[ep] > bytes_read )
			goto set_retval;
	}

set_retval:

	retval = bytes_read_count;

exit:
	/* unlock the device */
	up( &dev->sem );
	dbg( "     %s - retval %d", __FUNCTION__, retval );
	return retval;
}


/**
 *	labjacku3_write
 *
 *	By default, endpoint 1 is used to write to the device.
 *
 */
static ssize_t labjacku3_write( struct file *file, const char *buffer, size_t count, loff_t *ppos )
{

	struct usb_labjacku3 *dev;
	int retval = 0;
	int ep = 0;

	dev = ( struct usb_labjacku3 * )file->private_data;

	dbg( "%s - minor %d, count = %ld", __FUNCTION__, dev->minor, count );

	/* lock this object */
	down( &dev->sem );

	/* verify that the device wasn't unplugged */

	if ( !dev->present ) {
		retval = -ENODEV;
		goto exit;
	}

	/* verify that we actually have some data to write */
	if ( count <= 0 ) {
		dbg( "%s - write request of 0 bytes", __FUNCTION__ );
		goto exit;
	}

	ep = 0;

	if ( copy_from_user( dev->write_urb[ep]->transfer_buffer, buffer,
	                     count ) ) {
		retval = -EFAULT;
		err( "%s - failed copying buffer from user, error %d",
		     __FUNCTION__, retval );
		goto exit;
	}

	/* wait for a previous write to finish up; we don't use a timeout
	* and so a nonresponsive device can delay us indefinitely.
	*/
	if ( atomic_read( &dev->write_busy ) )
		wait_for_completion( &dev->write_finished );

	usb_labjacku3_debug_data( __FUNCTION__, count,
	                          dev->write_urb[ep]->transfer_buffer );

	dbg( "%s - ep = %d, count = %ld", __FUNCTION__, ep + 1, count );

	/* this urb was already set up, except for this write size */
	dev->write_urb[ep]->transfer_buffer_length = count;

	/* send the data out the bulk port */
	/* a character device write uses GFP_KERNEL,
	 unless a spinlock is held */
	init_completion( &dev->write_finished );

	atomic_set( &dev->write_busy, 1 );

	retval = usb_submit_urb( dev->write_urb[ep], GFP_KERNEL );

	if ( retval ) {
		atomic_set( &dev->write_busy, 0 );
		err( "%s - failed submitting write urb, error %d",
		     __FUNCTION__, retval );
	}
	else {
		retval = count;
	}

exit:

	/* unlock the device */
	up( &dev->sem );

	return retval;
}


/**
 *	labjacku3_ioctl
 */
static int labjacku3_ioctl( struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg )
{

	struct usb_labjacku3 *dev;
	void __user *data;
	int retval = 0;
	int timeout = 0;

	dev = ( struct usb_labjacku3 * )file->private_data;

	/* lock this object */
	down( &dev->sem );

	/* verify that the device wasn't unplugged */

	if ( !dev->present ) {
		up( &dev->sem );
		return -ENODEV;
	}

	dbg( "%s - minor %d, cmd 0x%.4x, arg %ld", __FUNCTION__,

	     dev->minor, cmd, arg );

	/* driver specific commands */

	switch ( cmd ) {
			/* Sets the timeout for bulk read transfers in ms from an integer argument */
			/* If the timeout is set to zero, reads will wait forever */

		case IOCTL_LJ_SET_READ_TIMEOUT:
			data = ( void __user * ) arg;

			if ( data == NULL )
				break;

			if ( copy_from_user( &timeout, data, sizeof( int ) ) ) {
				retval = -EFAULT;
				break;
			}

			dbg( "%s - current timeout of %d changed to %d", __FUNCTION__, dev->read_timeout, timeout );

			dev->read_timeout = timeout;
			break;

		default:
			retval = -ENOTTY;
			break;
	}

	/* unlock the device */
	up( &dev->sem );

	return retval;
}

/**
 *	labjacku3_write_bulk_callback
 */
static void labjacku3_write_bulk_callback( struct urb *urb, struct pt_regs *regs )
{

	struct usb_labjacku3 *dev = ( struct usb_labjacku3 * )urb->context;

	dbg( "%s - minor %d", __FUNCTION__, dev->minor );

	/* sync/async unlink faults aren't errors */

	if ( urb->status && !( urb->status == -ENOENT ||
	                       urb->status == -ECONNRESET ) ) {
		dbg( "%s - nonzero write bulk status received: %d",
		     __FUNCTION__, urb->status );
	}

	/* notify anyone waiting that the write has finished */
	atomic_set( &dev->write_busy, 0 );

	complete( &dev->write_finished );
}


/**
 *	labjacku3_probe
 *
 *	Called by the usb core when a new device is connected that it thinks
 *	this driver might be interested in.
 */
static int labjacku3_probe( struct usb_interface *interface, const struct usb_device_id *id )
{

	struct usb_device *udev = interface_to_usbdev( interface );

	struct usb_labjacku3 *dev = NULL;

	struct usb_host_interface *iface_desc;

	struct usb_endpoint_descriptor *endpoint;
	size_t buffer_size;
	int i;
	int retval = -ENOMEM;
	int ep = 0;

	/* See if the device offered us matches what we can accept */

	if ( ( udev->descriptor.idVendor != USB_LABJACK_VENDOR_ID ) ||
	        ( udev->descriptor.idProduct != USB_LABJACKU3_PRODUCT_ID ) ) {
		return -ENODEV;
	}

	/* allocate memory for our device state and initialize it */
	dev = kmalloc( sizeof( struct usb_labjacku3 ), GFP_KERNEL );

	if ( dev == NULL ) {
		err( "Out of memory" );
		return -ENOMEM;
	}

	memset( dev, 0x00, sizeof( *dev ) );

	init_MUTEX( &dev->sem );
	dev->udev = udev;

	dev->interface = interface;

	/* set up the endpoint information */
	/* check out the endpoints */

	iface_desc = &interface->altsetting[0];

	for ( i = 0; i < iface_desc->desc.bNumEndpoints; ++i ) {
		endpoint = &iface_desc->endpoint[i].desc;

		if ( i < 2 )
			ep = 0;
		else
			ep = 1;

		if ( !dev->bulk_in_endpointAddr[ep] &&
		        ( endpoint->bEndpointAddress & USB_DIR_IN ) &&
		        ( ( endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK )
		          == USB_ENDPOINT_XFER_BULK ) ) {
			/* we found a bulk in endpoint */
			buffer_size = endpoint->wMaxPacketSize;
			dev->bulk_in_size[ep] = buffer_size;
			dev->bulk_in_endpointAddr[ep] = endpoint->bEndpointAddress;
			dev->bulk_in_buffer[ep] = kmalloc( buffer_size, GFP_KERNEL );

			if ( !dev->bulk_in_buffer[ep] ) {
				err( "Couldn't allocate bulk_in_buffer %d", ep + 1 );
				goto error;
			}
		}


		if ( !dev->bulk_out_endpointAddr[ep] &&
		        !( endpoint->bEndpointAddress & USB_DIR_IN ) &&
		        ( ( endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK )
		          == USB_ENDPOINT_XFER_BULK ) ) {
			/* we found a bulk out endpoint */
			/* a probe() may sleep and has no restrictions on memory allocations */
			dev->write_urb[ep] = usb_alloc_urb( 0, GFP_KERNEL );

			if ( !dev->write_urb[ep] ) {
				err( "No free urbs available" );
				goto error;
			}

			dev->bulk_out_endpointAddr[ep] = endpoint->bEndpointAddress;

			/* on some platforms using this kind of buffer alloc
			 * call eliminates a dma "bounce buffer".
			 *
			 * NOTE: you'd normally want i/o buffers that hold
			 * more than one packet, so that i/o delays between
			 * packets don't hurt throughput.
			 */
			buffer_size = endpoint->wMaxPacketSize;
			dev->bulk_out_size[ep] = buffer_size;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,14)
			dev->write_urb[ep]->transfer_flags = ( URB_NO_TRANSFER_DMA_MAP |
			                                       URB_ASYNC_UNLINK );
#else
			dev->write_urb[ep]->transfer_flags = ( URB_NO_TRANSFER_DMA_MAP );
#endif

			dev->bulk_out_buffer[ep] = usb_buffer_alloc( udev,
			                           buffer_size, GFP_KERNEL,
			                           &dev->write_urb[ep]->transfer_dma );

			if ( !dev->bulk_out_buffer[ep] ) {
				err( "Couldn't allocate bulk_out_buffer %d", ep );
				goto error;
			}

			usb_fill_bulk_urb( dev->write_urb[ep], udev,

			                   usb_sndbulkpipe( udev,
			                                    endpoint->bEndpointAddress ),
			                   dev->bulk_out_buffer[ep], buffer_size,
			                   labjacku3_write_bulk_callback, dev );
		}
	}

	for ( i = 0; i < 2; i++ ) {
		if ( !( dev->bulk_in_endpointAddr[i] && dev->bulk_out_endpointAddr[i] ) ) {
			err( "Could not find both bulk-in and bulk-out endpoint %d", i + 1 );
			goto error;
		}
	}

	/* allow device read, write and ioctl */
	dev->present = 1;

	/* we can register the device now, as it is ready */
	usb_set_intfdata( interface, dev );

	retval = usb_register_dev( interface, &labjacku3_class );

	if ( retval ) {
		/* something prevented us from registering this driver */
		err( "Not able to get a minor for this device." );
		usb_set_intfdata( interface, NULL );
		goto error;
	}

	dev->minor = interface->minor;

	dev->read_timeout = READ_TIMEOUT;

	/* let the user know what node this device is now attached to */
	info( "USB LabJack device now attached to labjacku3-%d", dev->minor );
	return 0;

error:
	labjacku3_delete( dev );
	return retval;
}


/**
 *	labjacku3_disconnect
 *
 *	Called by the usb core when the device is removed from the system.
 *
 *	This routine guarantees that the driver will not submit any more urbs
 *	by clearing dev->udev.  It is also supposed to terminate any currently
 *	active urbs.  Unfortunately, usb_bulk_msg(), used in labjacku3_read(), does
 *	not provide any way to do this.  But at least we can cancel an active
 *	write.
 */
static void labjacku3_disconnect( struct usb_interface *interface )
{

	struct usb_labjacku3 *dev;
	int minor;

	/* prevent races with open() */
	down( &disconnect_sem );

	dev = usb_get_intfdata( interface );
	usb_set_intfdata( interface, NULL );

	down( &dev->sem );

	minor = dev->minor;

	/* give back our minor */
	usb_deregister_dev( interface, &labjacku3_class );

	/* terminate an ongoing write */

	if ( atomic_read( &dev->write_busy ) ) {
		usb_unlink_urb( dev->write_urb[0] );
		usb_unlink_urb( dev->write_urb[1] );
		wait_for_completion( &dev->write_finished );
	}

	/* prevent device read, write and ioctl */
	dev->present = 0;

	up( &dev->sem );

	/* if the device is opened, labjacku3_release will clean this up */
	if ( !dev->open )
		labjacku3_delete( dev );

	up( &disconnect_sem );

	info( "USB LabJack #%d now disconnected", minor );
}


/**
 *	usb_labjacku3_init
 */
static int __init usb_labjacku3_init( void )
{
	int result;

	/* register this driver with the USB subsystem */
	result = usb_register( &labjacku3_driver );

	if ( result ) {
		err( "usb_register failed. Error number %d", result );
		return result;
	}

	info( DRIVER_DESC " " DRIVER_VERSION );

	return 0;
}


/**
 *	usb_labjacku3_exit
 */
static void __exit usb_labjacku3_exit( void )
{
	/* deregister this driver with the USB subsystem */
	usb_deregister( &labjacku3_driver );
}


module_init( usb_labjacku3_init );
module_exit( usb_labjacku3_exit );

MODULE_AUTHOR( DRIVER_AUTHOR );
MODULE_DESCRIPTION( DRIVER_DESC );
MODULE_LICENSE( "GPL" );
