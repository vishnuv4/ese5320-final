// Server side implementation of for Simulating on test bench
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "main.h"
#include "server.h"


#define PORT	  8080
#define CHUNKSIZE BLOCKSIZE // tuneable but must match client
#define HEADER    2
#define DONE_BIT (1 << 7)

//unsigned char table[TOTAL_PACKETS][CHUNKSIZE+HEADER];
FILE* fp;

/*
* checks for a system call error if so exits the application
* code supplied by Andre Dehon from previous homeworks
*/
void Check_error(int Error, const char * Message)
{
  if (Error)
  {
    //fputs(Message, stderr);
    perror(Message);
    exit(EXIT_FAILURE);
  }
}

/*
* checks for a sys call error if so then exits
* code supplied by Andre Dehon from previous homeworks
*/
void Exit_with_error(void)
{
  perror(NULL);
  exit(EXIT_FAILURE);
}

/*
* Frees alloc'd or malloc'd data
* code supplied by Andre Dehon from previous homeworks
*/
void Free(unsigned char * Frame)
{
  free(Frame);
}

/*
* loads data from sd card or file
* code supplied by Andre Dehon from previous homeworks
*/
uint32_t Load_data(unsigned char* Input_data, uint32_t in_size)
{

	fp = fopen(INPUT_FILE, "r");
	if (fp == NULL) {
		perror("invalid file");
		exit(EXIT_FAILURE);
	}

	fseek(fp, 0, SEEK_END); // seek to end of file
	int file_size = ftell(fp); // get current file pointer
	std::cout << file_size << std::endl;
	fseek(fp, 0, SEEK_SET); // seek back to beginning of fil

	return file_size;
}

/*
* Stores an output buffer to sd card or file name
* code supplied by Andre Dehon from previous homeworks
*/
void Store_data(const char * Filename,unsigned char * Data, uint32_t Size)
{
  FILE * File = fopen(Filename, "wb");
  if (File == NULL)
    Exit_with_error();

  if (fwrite(Data, 1, Size, File) != Size)
    Exit_with_error();

  if (fclose(File) != 0)
    Exit_with_error();
}

// basic
int ESE532_Server::setup_server()
{
	// Does nothing but opens the file.
	// Can ignore the parameter.
	int bytes_read = Load_data(input_data, 72000);

	printf("server setup complete!\n");

	return 0;
}

int ESE532_Server::get_packet(unsigned char *buffer)
{
	// Directly reading to avoid any memcpy
	uint16_t bytes_read = fread((buffer+HEADER), sizeof(unsigned char), CHUNKSIZE, fp);

	if (!bytes_read) {
		// if zero bytes read probably file end or something
		return 0;
	}

	buffer[0] = (unsigned char)(bytes_read & 0xFF);  //be care full here

	if (bytes_read < CHUNKSIZE) {
		buffer[1] =  (unsigned char)((bytes_read >> 8) | DONE_BIT);
		fclose(fp);
	} else {
		buffer[1] =  (unsigned char)(bytes_read >> 8);
	}

	return (bytes_read + HEADER);
}
