#include "main.h"
#include <iostream>
//#include <vector>
//#include <unordered_map>
#include <stdlib.h>
//****************************************************************************************************************
//#define CAPACITY 4096
// try  uncommenting the line above and commenting line 6 to make the hash table smaller
// and see what happens to the number of entries in the assoc mem
// (make sure to also comment line 27 and uncomment line 28)
#define SEED 524057
#define ASSOC_MEM_STORE 256
// #define CLOSEST_PRIME 65497
// #define CLOSEST_PRIME 32749
// #define CLOSEST_PRIME 65497
#define FILE_SIZE 4096

static inline uint32_t murmur_32_scramble(uint32_t k) {
	k *= 0xcc9e2d51;
	k = (k << 15) | (k >> 17);
	k *= 0x1b873593;
	return k;
}

unsigned int my_hash(unsigned long key) {
	uint32_t h = SEED;
	uint32_t k = key;
	h ^= murmur_32_scramble(k);
	h = (h << 13) | (h >> 19);
	h = h * 5 + 0xe6546b64;

	h ^= murmur_32_scramble(k);
	/* Finalize. */
	h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;
	//return h & 0xFFFF;
	return h & (CAPACITY - 1);
	// return key % CLOSEST_PRIME;
}


void hash_lookup(unsigned long (*hash_table)[2], unsigned int key, bool *hit,
		unsigned int *result) {
	//std::cout << "hash_lookup():" << std::endl;
	key &= 0xFFFFF; // make sure key is only 20 bits

	unsigned int hash_val = my_hash(key);

	unsigned long lookup = hash_table[hash_val][0];

	// [valid][value][key]
	unsigned int stored_key = lookup & 0xFFFFF;       // stored key is 20 bits
	unsigned int value = (lookup >> 20) & 0xFFF;      // value is 12 bits
	unsigned int valid = (lookup >> (20 + 12)) & 0x1; // valid is 1 bit

	if (valid && (key == stored_key)) {
		*hit = 1;
		*result = value;
		//std::cout << "\thit the hash" << std::endl;
		//std::cout << "\t(k,v,h) = " << key << " " << value << " " << my_hash(key) << std::endl;
	} else {
		lookup = hash_table[hash_val][1];

		// [valid][value][key]
		stored_key = lookup & 0xFFFFF;       // stored key is 20 bits
		value = (lookup >> 20) & 0xFFF;      // value is 12 bits
		valid = (lookup >> (20 + 12)) & 0x1; // valid is 1 bit
		if (valid && (key == stored_key)) {
			*hit = 1;
			*result = value;
		} else {
			*hit = 0;
			*result = 0;
		}
		// std::cout << "\tmissed the hash" << std::endl;
	}
}


void hash_insert(unsigned long (*hash_table)[2], unsigned int key,
		unsigned int value, bool *collision) {
	//std::cout << "hash_insert():" << std::endl;
	key &= 0xFFFFF;   // make sure key is only 20 bits
	value &= 0xFFF;   // value is only 12 bits

	unsigned int hash_val = my_hash(key);

	unsigned long lookup = hash_table[hash_val][0];
	unsigned int valid = (lookup >> (20 + 12)) & 0x1;

	if (valid) {
		lookup = hash_table[hash_val][1];
		valid = (lookup >> (20 + 12)) & 0x1;
		if (valid) {
			*collision = 1;
		} else {
			hash_table[hash_val][1] = (1UL << (20 + 12)) | (value << 20) | key;
			*collision = 0;
		}
		// std::cout << "\tKey is:" << key << std::endl;
		// std::cout << "\tcollision in the hash" << std::endl;
	} else {
		hash_table[hash_val][0] = (1UL << (20 + 12)) | (value << 20) | key;
		*collision = 0;
		//std::cout << "\tinserted into the hash table" << std::endl;
		//std::cout << "\t(k,v,h) = " << key << " " << value << " " << my_hash(key) << std::endl;
	}
}
//****************************************************************************************************************
typedef struct {
	// Each key_mem has a 9 bit address (so capacity = 2^9 = 512)
	// and the key is 20 bits, so we need to use 3 key_mems to cover all the key bits.
	// The output width of each of these memories is 64 bits, so we can only store 64 key
	// value pairs in our associative memory map.

	unsigned long upper_key_mem[512]; // the output of these  will be 64 bits wide (size of unsigned long).
	unsigned long middle_key_mem[512];
	unsigned long lower_key_mem[512];
	unsigned int value[ASSOC_MEM_STORE]; // value store is 64 deep, because the lookup mems are 64 bits wide
	unsigned int fill;       // tells us how many entries we've currently stored
} assoc_mem;

// cast to struct and use ap types to pull out various feilds.

void assoc_insert(assoc_mem *mem, unsigned int key, unsigned int value,
		bool *collision) {
	//std::cout << "assoc_insert():" << std::endl;
	key &= 0xFFFFF; // make sure key is only 20 bits
	value &= 0xFFF;   // value is only 12 bits

	if (mem->fill < ASSOC_MEM_STORE) {
		mem->upper_key_mem[(key >> 18) % 512] |= (1 << mem->fill); // set the fill'th bit to 1, while preserving everything else
		mem->middle_key_mem[(key >> 9) % 512] |= (1 << mem->fill); // set the fill'th bit to 1, while preserving everything else
		mem->lower_key_mem[(key >> 0) % 512] |= (1 << mem->fill); // set the fill'th bit to 1, while preserving everything else
		mem->value[mem->fill] = value;
		mem->fill++;
		*collision = 0;
		//std::cout << "\tinserted into the assoc mem" << std::endl;
		//std::cout << "\t(k,v) = " << key << " " << value << std::endl;
	} else {
		*collision = 1;
	}
}

void assoc_lookup(assoc_mem *mem, unsigned int key, bool *hit,
		unsigned int *result) {
	key &= 0xFFFFF; // make sure key is only 20 bits

	unsigned int match_high = mem->upper_key_mem[(key >> 18) % 512];
	unsigned int match_middle = mem->middle_key_mem[(key >> 9) % 512];
	unsigned int match_low = mem->lower_key_mem[(key >> 0) % 512];

	unsigned int match = match_high & match_middle & match_low;

	unsigned int address = 0;
	for (; address < ASSOC_MEM_STORE; address++) {
		if ((match >> address) & 0x1) {
			break;
		}
	}

	if (address != ASSOC_MEM_STORE) {
		*result = mem->value[address];
		*hit = 1;
	} else {
		*hit = 0;
	}
}
//****************************************************************************************************************
void insert(unsigned long hash_table[][2], assoc_mem *mem, unsigned int key,
		unsigned int value, bool *collision) {
	hash_insert(hash_table, key, value, collision);
	if (*collision) {
		assoc_insert(mem, key, value, collision);
	}
}

void lookup(unsigned long hash_table[][2], assoc_mem *mem, unsigned int key,
		bool *hit, unsigned int *result) {
	hash_lookup(hash_table, key, hit, result);
	if (!*hit) {
		assoc_lookup(mem, key, hit, result);
	}
}

void hardware_encoding(int *out_hw_size, unsigned char *output_hw, char *chunk_arr, unsigned int s1_len)
{

	int next_code = 256;
	unsigned int prefix_code = chunk_arr[0];
	unsigned int code = 0;
	unsigned char next_char = 0;
	unsigned char output_byte = 0;
	uint32_t bp_push_index = 0;    

	// create hash table and assoc mem
	unsigned long hash_table[CAPACITY][2];
	assoc_mem my_assoc_mem;

	// make sure the memories are clear
	for (int i = 0; i < CAPACITY; i++) {
		for (int j = 0; j < 2; j++) {
			hash_table[i][j] = 0;
		}
	}
	my_assoc_mem.fill = 0;
	for (int i = 0; i < 512; i++) {
		my_assoc_mem.upper_key_mem[i] = 0;
		my_assoc_mem.middle_key_mem[i] = 0;
		my_assoc_mem.lower_key_mem[i] = 0;
	}

    // main loop to iterate through the chunk
	unsigned int i = 0;
	while (i < s1_len) {
        // Complete bitpacking in the last iteration 
		if (i + 1 == s1_len) 
        {
            if(bp_push_index % 3 == 0)
            {
                output_byte |= (prefix_code & 0xFF0) >> 4;
                output_hw[bp_push_index] = output_byte;
                bp_push_index++;
                output_byte = 0;
                output_byte |= (prefix_code & 0x00F) << 4;
                output_byte &= ~(0xF);
                output_hw[bp_push_index] = output_byte;
                bp_push_index++;
            }
            else
            {
                output_byte |= (prefix_code & 0xF00) >> 8;
                output_hw[bp_push_index] = output_byte;
                bp_push_index++;
                output_byte = 0;
                output_byte |= prefix_code & 0x0FF;
                output_hw[bp_push_index] = output_byte;
                bp_push_index++;
            }
            *out_hw_size = bp_push_index;
			break;
		}

		next_char = chunk_arr[i + 1];

		bool hit = 0;
		lookup(hash_table, &my_assoc_mem, (prefix_code << 8) + next_char, &hit, &code);

		if (!hit) 
        {
            // insert prefix code and next character into hash table (assoc mem if collision)
			bool collision = 0;
			insert(hash_table, &my_assoc_mem, (prefix_code << 8) + next_char, next_code, &collision);
			if (collision) 
            {
				std::cout << "ERROR: FAILED TO INSERT! NO MORE ROOM IN ASSOC MEM!" << std::endl;
				break;
			}
			next_code += 1;

            // Perform bitpacking
            if(bp_push_index % 3 == 0)
            {
                output_byte |= (prefix_code & 0xFF0) >> 4;
                output_hw[bp_push_index] = output_byte;
                bp_push_index++;
                output_byte = 0;
                output_byte |= (prefix_code & 0x00F) << 4;
                output_byte &= ~(0xF);
            }
            else
            {
                output_byte |= (prefix_code & 0xF00) >> 8;
                output_hw[bp_push_index] = output_byte;
                bp_push_index++;
                output_byte = 0;
                output_byte |= prefix_code & 0x0FF;
                output_hw[bp_push_index] = output_byte;
                bp_push_index++;
                output_byte = 0;
            }

			prefix_code = next_char;
		} else {
            // Take value from hash table if found there
			prefix_code = code;
		}
		i++;
	}
}
