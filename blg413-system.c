/*
	BUSE NUR SABAH, 150170002
	05.12.2021

make
rm blg413-system.ko
insmod blg413-system.ko currentKey="LINUX" dev_mem_size=512

gcc userspace.c -o o
./o LINUX SYSTEMSPROGRAMMING

cat /proc/devices - dmesg

*/


#include <linux/module.h> //inside of kernel source tree, module_init(), module_exit()
#include <linux/cdev.h> //cdev structure, cdev_init(), cdev_add(), cdev_del()
#include <linux/uaccess.h> //copy_to_user(), copy_from_user() --> to communicate between user space and kernel space
#include <linux/fs.h> //alloc_chrdev_region(), unregister_chrdev_region(), VFS structure definitions
#include <linux/slab.h> //kfree()
#include <linux/ioctl.h> //_IOW
#include <uapi/linux/string.h> //strlen(), strcmp() BUNU SOR, KULLANABİLİR MİYİM DİYE.
#include <linux/spinlock.h> //single-open device


//ioctl command
#ifndef __IOCTL
#define __IOCTL
#define MAGIC 'b'
#define VIGENERE_MODE_DECRYPT 	_IOW(MAGIC, 1, char*) 
#define VIGENERE_MODE_SIMPLE 	_IOW(MAGIC, 0, char*) //default, second parameter is used in switch-case
#endif


/*************** MODULE PARAMETERS ***************/
static char *currentKey = "A";
static int dev_mem_size = 4096;  //4KB
module_param(currentKey, charp, S_IRUGO);
module_param(dev_mem_size, int, S_IRUGO);


/*************** DECLARATION ***************/
static int __init vigenere_init(void);
static void __exit vigenere_cleanup(void);

static int vigenere_open(struct inode *inode, struct file *fd);
static int vigenere_release(struct inode *inode, struct file *fd);
static ssize_t vigenere_read(struct file *fd, char __user *buffer2, size_t length_buffer2, loff_t *off);
static ssize_t vigenere_write(struct file *fd, const char *buffer1, size_t length_buffer1, loff_t *off);
static long vigenere_ioctl(struct file *file,unsigned int cmd, unsigned long arg); //third argument is an untyped pointer to memory


/*************** VARIABLES ***************/
dev_t device_number = 0;
char *KERNELbuffer; 
unsigned int readFlag = 0; //deafult non-decrypted
spinlock_t access_lock; // = SPIN_LOCK_UNLOCK; //compile time initialization, processes spin until the unlock 
 
static struct class *vigenere_class;
static struct cdev vigenere_cdev; //as much as number of device
static struct file_operations vigenere_fops = { //for read, write, open, close
	.owner   		= THIS_MODULE,
	.read  	 		= vigenere_read,
	.write 	 		= vigenere_write,
	.open  	 		= vigenere_open,
	.release 		= vigenere_release,
	.unlocked_ioctl = vigenere_ioctl,
}; 


/*************** DRIVER STARTING ***************/
//MACROS, indicate role of functions
module_init(vigenere_init);
module_exit(vigenere_cleanup);
/*************** DRIVER TERMINATING ***************/


/*************** MODULE DESCRIPTION ***************/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("buse");
MODULE_DESCRIPTION("blg");
MODULE_INFO(blg,"blg");


/*************** DEFINITION ***************/
static int __init vigenere_init(void) {

	//dynamicly create-allocate a device number, major number 
	if(alloc_chrdev_region(&device_number,0,1,"BLG413-Major") < 0) {
		printk(KERN_ALERT"Problem in major number allocation!\n");
		return -1;
	}
	printk(KERN_ERR"OKAY, Major:%d ,Minor:%d\n", MAJOR(device_number), MINOR(device_number)); //extracting numbers

	//user level system call will connected to the file operations' methods of the driver via registration
	//initialize file ops structure with driver's system call implementation methods
	cdev_init(&vigenere_cdev, &vigenere_fops); //create cdev structure
	if(cdev_add(&vigenere_cdev,device_number,1) < 0){ //add char device to the system, make a char device registration with the VFS  
		printk(KERN_ALERT"Problem in adding device!\n");

		unregister_chrdev_region(device_number, 1);
		return -1;
	}
	printk(KERN_INFO"OKAY, (cdev_add)\n");

	//create class structure
	vigenere_class = class_create(THIS_MODULE, "BLG413-Class");
	if(vigenere_class == NULL){	
		printk(KERN_ERR"Problem in creating class struct!\n");

		unregister_chrdev_region(device_number, 1);
		return -1;
	}
	printk(KERN_INFO"OKAY, (class_create)\n");

	//create device files
	if(device_create(vigenere_class, NULL, device_number, NULL, "vigenere") == NULL){ 
		printk(KERN_ALERT"Problem in creating device!\n");

		class_destroy(vigenere_class);
		return -1;

	}
	printk(KERN_INFO"OKAY, (device_create)\n");
	
	//create kernel memory
	KERNELbuffer = kzalloc(dev_mem_size, GFP_KERNEL); //device buffer, pseudo device's memory, only one device
	if( KERNELbuffer == 0) {
		printk(KERN_ALERT"Problem in allocation kernel memory!\n");
		return -1; 
	}
	KERNELbuffer[dev_mem_size-1] = '\0';
	printk(KERN_DEFAULT"OKAY KZALLOC\n");
	
	spin_lock_init(&access_lock); //run-time initialization

	printk(KERN_CRIT"INIT OKAY\n");
	return 0; //SUCCESS
}

static void __exit vigenere_cleanup(void) {
	
	kfree(KERNELbuffer); //free kernel memory
	printk(KERN_DEFAULT"OKAY KFREE\n");

	device_destroy(vigenere_class, device_number); 
	class_destroy(vigenere_class);
	cdev_del(&vigenere_cdev);
	unregister_chrdev_region(device_number, 1);
	printk(KERN_CRIT"EXIT OKAY\n");
}

static int vigenere_open(struct inode *inode, struct file *fd) { //device file will open

	spin_lock(&access_lock);
	printk(KERN_CRIT"LOCKED!\n");
	
	printk(KERN_CRIT"OPEN OKAY\n"); 

	return 0;
}

static int vigenere_release(struct inode *inode, struct file *fd) { //device file will close
	
	//clear kernel buffer
	memset(KERNELbuffer, '\0', strlen(KERNELbuffer));
	
	printk(KERN_CRIT"CLOSE OKAY\n"); 
	
	printk(KERN_CRIT"UNLOCKED!\n");
	spin_unlock(&access_lock);

	return 0;
}


static ssize_t vigenere_read(struct file *fd, char __user *buffer2, size_t length_buffer2, loff_t *off) {

	printk(KERN_DEFAULT"READ START\n"); 

	if(readFlag == 0) { //NON-DECRYPT
		copy_to_user(buffer2, KERNELbuffer, strlen(KERNELbuffer)); //__user is for here
	}

	else if(readFlag == 1) { //DECRYPT
		KERNELbuffer[length_buffer2] = '\0';

		int length_of_key = strlen(currentKey);
		int mes_size = strlen(KERNELbuffer);
		int size = mes_size < length_buffer2 ? mes_size:length_buffer2;
		int i;
		int temp;
		for(i=0; i < size; i++){ 
			temp = (KERNELbuffer[i] - currentKey[i % length_of_key]);
			if (temp <0) {
				temp+=26; 
			}
			temp+=65;
			put_user(temp, &buffer2[i]);
		}
		//put_user('\0', &buffer2[i]);
	}
	
	else{
		printk(KERN_ERR"READ, INVALID FLAG!\n");
	}

	printk(KERN_DEFAULT"READ END\n"); 

	return length_buffer2;
}


static ssize_t vigenere_write(struct file *fd, const char *buffer1, size_t length_buffer1, loff_t *off) {

	printk(KERN_DEFAULT"WRITE START\n"); 

	copy_from_user(KERNELbuffer, buffer1, length_buffer1); //copy message from user space into the kernel space
	KERNELbuffer[length_buffer1] = '\0';

	printk(KERN_INFO"ORIGINAL: %s\n",KERNELbuffer);

	int length_of_key = strlen(currentKey);
	int i;
	for(i=0; i < length_buffer1; i++){ 
		KERNELbuffer[i] = ((KERNELbuffer[i] + currentKey[i % length_of_key]) % 26) + 65;
	}
	
	printk(KERN_INFO"ENCRYPT: %s\n",KERNELbuffer);
	
	printk(KERN_DEFAULT"WRITE END\n"); 

	return length_buffer1;
}


static long vigenere_ioctl(struct file *file, unsigned int cmd, unsigned long arg) { //inode is for old kernel ver.
	
	printk(KERN_DEFAULT"IOCTL START\n"); 
	
	if(strcmp(currentKey, (char*)arg) == 0) {
		switch(cmd){
			case VIGENERE_MODE_SIMPLE: 
				readFlag = 0;
				printk(KERN_INFO"OKAY, FLAG SET!\n");
				break;
			case VIGENERE_MODE_DECRYPT:
				readFlag = 1;
				printk(KERN_INFO"OKAY, FLAG SET!\n");
				break;
			default:
				readFlag = 0;
				printk(KERN_ERR"NON VALID CMD!\n"); 
		}
	}
	else{
		printk(KERN_ERR"IOCTL, INVALID KEY. FLAG is NOT updated!\n"); 
	}
	
	printk(KERN_DEFAULT"IOCTL END\n"); 
	
	return 0;
}
