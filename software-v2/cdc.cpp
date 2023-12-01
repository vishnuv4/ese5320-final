#include "main.h"
#include <math.h>
#include <climits>
static const uint32_t pow_replace[17] = {3, 9, 27, 81, 243, 729, 2187, 6561, 19683, 59049, 177147, 531441, 1594323, 4782969, 14348907, 43046721, 129140163};
static uint32_t chunk_ctr=0;

uint64_t hash_func(unsigned char* input, unsigned int pos)
{
   
    // put your hash function implementation here
    uint64_t hash = 0;
    for (int i = 0; i < WIN_SIZE; i++)
        hash += input[pos + WIN_SIZE - 1 - i] * pow_replace[i];
    return hash;
}

void cdc(unsigned char* buff, unsigned int buff_size, std::vector<hashtable_t> &hashTable, FILE *encode_fp)
{

    std::vector<unsigned long> chunk_indices;   // store the chunk indices, this is cleared for every new packet
    int out_hw_size;                            // store the output of the lzw 
    uint32_t header;                    
    unsigned char output_hw[PACKET_SIZE];
    char chunk_arr[8192];
    
    unsigned int counter = 0;
    SHA256_CTX ctx;
    unsigned int chunk_size=0;
    unsigned char sha_output[SHA256_BLOCK_SIZE];
    unsigned int i;
    bool firstfound=1;
    uint64_t hash = hash_func(buff, WIN_SIZE);
    bool seen;
    unsigned int j=0;


    for (i = WIN_SIZE; i < buff_size-WIN_SIZE; i++) {

        counter++;

        memset(sha_output,0,SHA256_BLOCK_SIZE);
        
        if ((hash % MODULUS == TARGET || counter == 4096) && counter > 100) {
            if(firstfound == 1){
                chunk_size = i+1;
                //sha_timer.start();
                sha256_hash(&ctx, &buff[0], sha_output, 1, chunk_size);
                //sha_timer.stop();
            }
            else {
                chunk_size = i - chunk_indices.back();
                //sha_timer.start();
                sha256_hash(&ctx, &buff[chunk_indices.back()+1], sha_output, 1, chunk_size);
                //sha_timer.stop();
            }
            //dedup_timer.start();
            seen = dedup(hashTable, chunk_ctr,sha_output);
            //dedup_timer.stop();
            chunk_ctr++;
            chunk_indices.push_back(i);
            if(seen){
				header = (hashTable.back().id);
				header = header << 1;
				header |= (1<<0);
				fwrite(&header, sizeof(uint32_t), 1, encode_fp);
            }
            else{
                memset(chunk_arr, 0, 8192);
                if(firstfound==1){
                    memcpy(chunk_arr, &buff[0], chunk_size);
                    firstfound = 0;
                }
                else{
                    memcpy(chunk_arr, &buff[chunk_indices[j-1]+1], chunk_size);
                }
				hardware_encoding(&out_hw_size, output_hw, chunk_arr, chunk_size);

				header = out_hw_size;
				header = header << 1;
				header &= ~(1<<0);
				fwrite(&header, sizeof(uint32_t), 1, encode_fp);
				fwrite(output_hw, sizeof(unsigned char), out_hw_size, encode_fp);
            }
            j++;
            counter = 0;
        }
        hash = (hash * PRIME) - (buff[i] * pow_replace[16]) + (buff[i + WIN_SIZE] * PRIME);
    }

    chunk_size = buff_size - 1 - chunk_indices.back();
    //hard chunking at the end of packet

    //sha_timer.start();
    sha256_hash(&ctx, &buff[chunk_indices.back()+1], sha_output, 1, chunk_size);
    //sha_timer.stop();
    chunk_indices.push_back(buff_size-1);


    //dedup_timer.start();
    seen = dedup(hashTable, chunk_ctr,sha_output);
    //dedup_timer.stop();
    chunk_ctr++;
    if(seen){
        header = (hashTable.back().id);
        header = header << 1;
        header |= (1<<0);
        fwrite(&header, sizeof(uint32_t), 1, encode_fp);
    }
    else{
        memset(chunk_arr, 0, 8192);
        if(firstfound==1){
            chunk_size = chunk_indices[0]+1;
            memcpy(chunk_arr, &buff[0], chunk_size);
            firstfound = 0;
        }
        else{
            chunk_size = chunk_indices[j] - chunk_indices[j-1];
            memcpy(chunk_arr, &buff[chunk_indices[j-1]+1], chunk_size);
        }

        hardware_encoding(&out_hw_size, output_hw, chunk_arr, chunk_size);

        header = out_hw_size;
        header = header << 1;
        header &= ~(1<<0);
        fwrite(&header, sizeof(uint32_t), 1, encode_fp);
        fwrite(output_hw, sizeof(unsigned char), out_hw_size, encode_fp);
        
    }
    chunk_indices.clear();
    chunk_size = 0;
}


