/*
	BUSE NUR SABAH, 150170002
	05.12.2021
*/


#include<stdio.h>
#include<fcntl.h>  //O_RDWR
#include<string.h>
#include<stdlib.h> //malloc
#include <unistd.h> //sleep
#include <sys/ioctl.h> //_IOW


#define MAGIC 'b'
#define VIGENERE_MODE_SIMPLE 	_IOW(MAGIC, 0, char*) //default, second paramter is used in switch-case
#define VIGENERE_MODE_DECRYPT 	_IOW(MAGIC, 1, char*) 


int main(int argc, char *argv[]){  //first argv is key, second argv is message

	if(argv[1] == NULL || argv[2] == NULL){
		printf("first argv is key, second argv is message\n");
		return 0;
	}

	char *key = argv[1];
	char *buffer1 = argv[2];
	int mes_length = strlen(buffer1);
	char *buffer2 = (char*)malloc(mes_length);

	//fork(); //make two process

	int fd = open("/dev/vigenere", O_RDWR);
	if(fd < 0 ){
		printf("Problem in open file!\n");
		return 0;
	}

	printf("writing...\n");
	write(fd, buffer1, mes_length); /* data will be encrypted */

	printf("\nreading in default mode...\n");
	read(fd, buffer2, mes_length); /* read encrypted data */
	printf("mes: %s\n", buffer2);

	printf("\nmode: decrypt is setting...\n");
	ioctl(fd, VIGENERE_MODE_DECRYPT, key); /* set decrypting mode */

	printf("reading in decrypt mode...\n");
	read(fd, buffer2, mes_length); /* read decrypted data */
	printf("mes: %s\n", buffer2);

	printf("\nmode: encrypt is setting...\n");
	ioctl(fd, VIGENERE_MODE_SIMPLE, key); /* set non-decrypting mode */

	printf("reading in encrypt mode...\n");
	read(fd, buffer2, mes_length); /* read encrypted data */
	printf("mes: %s\n", buffer2);
	
	printf("\nmode: decrypt is setting with wrong key...\n");
	ioctl(fd, VIGENERE_MODE_DECRYPT, "LOL"); /* set non-decrypting mode */
	
	printf("reading in decrypt mode...\n");
	read(fd, buffer2, mes_length); /* read decrypted data */
	printf("mes: %s\n", buffer2);

	printf("sleeping 10 seconds...\n");
	sleep(10);

	close(fd);
}


