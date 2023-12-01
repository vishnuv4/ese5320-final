#ifndef __MAIN_H_
#define __MAIN_H_

#include<iostream>
#include<cstdlib>
#include<string>
#include<vector>
#include<stdio.h>
#include "sha256.h"
#include<cstring>
#include "stopwatch.h"

//#define HOST            1    //uncomment to run on FPGA

#include "server.h"


#define DONE_BIT_L      (1 << 7)
#define DONE_BIT_H      (1 << 15)
#define TOTAL_PACKETS   2
#define WIN_SIZE        16
#define MODULUS         2048
#define TARGET          0
#define PACKET_SIZE 	8192
#define CHUNK_SIZE      8192
#define PRIME           3
#define COMPRESSED_DATA_SIZE    20000

#define CAPACITY   8192 // hash output is 15 bits, and we have 1 entry per bucket, so capacity is 2^15
//#define CAPACITY 4096
// try  uncommenting the line above and commenting line 6 to make the hash table smaller
// and see what happens to the number of entries in the assoc mem
// (make sure to also comment line 27 and uncomment line 28)

#define LZWH            1

//using namespace std;

typedef struct hashtable
{
    uint32_t id;
    unsigned char hashval[SHA256_BLOCK_SIZE];
    bool seen;
}hashtable_t;

void cdc(unsigned char* buff, unsigned int buff_size, std::vector<hashtable_t> &hashTable, FILE *encode_fp);
bool dedup(std::vector<hashtable_t> &hashTable, unsigned long chunk_ctr, unsigned char *sha_output);
void hardware_encoding(int *out_hw_size, unsigned char *output_hw, char *chunk_arr, unsigned int s1_len);
std::vector<int> encoding(std::string s1);
void bitpack(int *input, int input_size, BYTE *output);
std::vector<unsigned char> bitpack_sw(std::vector<int> input);
void top_func_new(unsigned char* buff, uint64_t length);
int decoder(const char* in_file, const char* out_file);

#endif
