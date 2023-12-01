

/******************** EDITED CODE *************/

//#include "main.h"
//#include <fcntl.h>
//#include <errno.h>
//#include <fcntl.h>
//#include "stopwatch.h"
//#include <iterator>
//#include <fstream>
//
////#define TOTAL_PACKETS   397
//
//int offset = 0;
//unsigned char* file;
//volatile int done = 0;
//
//stopwatch cdc_timer;
//stopwatch dedup_timer;
//stopwatch lzw_timer;
//stopwatch sha_timer;
//stopwatch top_func_timer;
//
//void top_func_new(unsigned char* buff, uint64_t length)
//{
//    std::vector<hashtable_t> hashTable;
//    std::vector<int> chunk_indices;
//    std::vector<int> chunk_sizes;
//    //BYTE *chunk;
//    int max_chunk_size;
//    hashtable_t temp;
//    BYTE sha_output[SHA256_BLOCK_SIZE];
//    size_t chunk_size=0;
//
//    //FILE *outputfile = fopen(ENCODER_OUT_FILE, "wb");
//    uint32_t output_header;
//    uint32_t header;
//
//#ifdef LZWH
//    int unpacked_lzw_output[4096];
//    BYTE packed_lzw_output[4096];
//    int lzw_size;
//#else
//    std::vector<BYTE> lzw_encoded_output;
//    std::vector<int> unpacked_lzw_encoded_output;
//#endif
//
//    //Chunking
//    cdc_timer.start();
//    max_chunk_size = cdc(chunk_indices, buff, length);
//    cdc_timer.stop();
//
//    //chunk = (BYTE *) malloc(max_chunk_size * sizeof(BYTE));
//    BYTE chunk[4096]; //(BYTE *) malloc(* sizeof(BYTE));
////    if(chunk==NULL){
////        std::cout << "malloc returned NULL: Not enough space to allocate " << max_chunk_size * sizeof(BYTE) << " bytes" << std::endl;
////        //std::cout << "File size: " << file_size << std::endl;
////        std::cout << "Max Chunk size: " << max_chunk_size << std::endl;
////    }
//
//    sha_timer.start();
//    for(int i=0; i<chunk_indices.size(); i++) {
//        SHA256_CTX ctx;
//        if(i==0) {
//            chunk_size = chunk_indices[i];
//        } else {
//            chunk_size = chunk_indices[i] - chunk_indices[i-1];
//        }
//        chunk_sizes.push_back(chunk_size);
//        if(i==0) {
//            memcpy(chunk, &buff[0], chunk_size*sizeof(BYTE));
//        } else {
//            memcpy(chunk, &buff[chunk_indices[i]], chunk_size*sizeof(BYTE));
//        }
//
//        sha256_hash(&ctx, chunk, sha_output, 1, chunk_size);
//        temp.id = i;
//        temp.seen=0;
//        memcpy(temp.hashval, sha_output, (SHA256_BLOCK_SIZE * sizeof(BYTE)));
//        hashTable.push_back(temp);
//    }
//    sha_timer.stop();
//
//    dedup_timer.start();
//    dedup(hashTable);
//    dedup_timer.stop();
//
//    lzw_timer.start();
//    for(int i=0; i<hashTable.size(); i++) {
//        if(hashTable[i].seen == 0) {
//            header = 0;
//            std::string chunk_str = "";
//
//            if(i==0) {
//                for(int k=0; k<chunk_sizes[0]; k++)
//                    chunk_str += buff[k];
//            }
//            else {
//                for(int k=chunk_indices[i-1]; k < chunk_indices[i]; k++)
//                    chunk_str += buff[k];
//            }
//
//            int length = chunk_str.length();
//            char* chunk_arr = new char[length + 1];
//            strcpy(chunk_arr, chunk_str.c_str());
//
//#ifdef LZWH
//            hardware_encoding(&lzw_size, unpacked_lzw_output, chunk_arr, length);
//            int packed_size = lzw_size * 3 / 2;
//            if (lzw_size % 2 == 1) {
//                packed_size += 1;
//            }
//
//            delete[] chunk_arr;
//
//            bitpack(unpacked_lzw_output, lzw_size, packed_lzw_output, &packed_size);
//
//            header = packed_size;
//            header = header << 1;
//            header &= ~(1 << 0);
//
//            //memcpy(out_buff = header
//            //fwrite(&header, sizeof(uint32_t), 1, outputfile);
//            //fwrite(&packed_lzw_output[0], sizeof(unsigned char), packed_size, outputfile);
//#else
//            unpacked_lzw_encoded_output = encoding(chunk_str);
//            lzw_encoded_output = bitpack(unpacked_lzw_encoded_output);
//            header = lzw_encoded_output.size();
//            header = header << 1;
//            header &= ~(1<<0);
//            fwrite(&header, sizeof(uint32_t), 1, outputfile);
//            fwrite(&lzw_encoded_output[0], sizeof(unsigned char), lzw_encoded_output.size(), outputfile);
//#endif
//        } else {
//            header = (hashTable[i].id);
//            header = header << 1;
//            header |= (1<<0);
//            //fwrite(&header, sizeof(uint32_t), 1, outputfile);
//        }
//
//#ifndef LZWH
//        lzw_encoded_output.clear();
//#endif
//
//    }
//
//    lzw_timer.stop();
//
//    //fclose(outputfile);
//    //free(chunk);
//    //free(buff);
//    std::cout << std::endl;
//}
