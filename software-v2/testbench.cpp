#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "server.h"
#include <unistd.h>
#include <fcntl.h>
#include "main.h"

#include "main.h"
#include <fcntl.h>
#include <errno.h>
#include <fcntl.h>
#include "stopwatch.h"
#include <iterator>
#include <fstream>

int offset = 0;
unsigned char* file;
volatile int done = 0;

stopwatch cdc_timer;
stopwatch dedup_timer;
stopwatch lzw_timer;
stopwatch sha_timer;
stopwatch top_func_timer;

bool Compare_matrices(unsigned char *ch_hw, std::vector<unsigned char> &ch_sw, int size)
{
	bool fail = 0;

    for (int X = 0; X < size; X++) {
      if (ch_hw[X] != ch_sw[X])
      {
        std::cout << "Data not match at " << X << std::endl;
        fail = 1;
        break;
      }
    }

    return fail;
}

int main()
{
  	int length = 0;
    volatile int done = 0;


    std::vector<hashtable_t> hashTable;
    std::vector<uint32_t> output_packet;
    std::vector<int> output_sw_buff;
    std::vector<unsigned char> output_sw;
    
    int fail;
    unsigned char input[PACKET_SIZE + HEADER];
//    char chunk_arr[8192];
    FILE *encode_fp = fopen(ENCODER_OUT_FILE, "w");

    static unsigned long hashTableCounter = 0;

    ESE532_Server server;
    server.setup_server();

    do {

        if(hashTable.size() > 10000){
            hashTable.clear();
        }
		server.get_packet(&input[0]);

		done = input[1] & DONE_BIT_L;
		length = input[0] | (input[1] << 8);
		length &= ~DONE_BIT_H;

        if(length != 0){
            unsigned char *in_buf = input + HEADER;
            cdc(in_buf, length, hashTable, encode_fp);
            memset(input,0,PACKET_SIZE);
        }
        else{
            done = 1;
        }

   
    } while (!done);
    //lzw_timer.stop();


	fclose(encode_fp);

	decoder(ENCODER_OUT_FILE, DECODER_OUT_FILE);

	return fail;
}

