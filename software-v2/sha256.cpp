/*********************************************************************
* Filename:   sha256.c
* Author:     Brad Conte (brad AT bradconte.com)
* Copyright:
* Disclaimer: This code is presented "as is" without any guarantees.
* Details:    Implementation of the SHA-256 hashing algorithm.
              SHA-256 is one of the three algorithms in the SHA2
              specification. The others, SHA-384 and SHA-512, are not
              offered in this implementation.
              Algorithm specification can be found here:
               * http://csrc.nist.gov/publications/fips/fips180-2/fips180-2withchangenotice.pdf
              This implementation uses little endian byte order.
*********************************************************************/

#if !defined(NEON) 
/*************************** HEADER FILES ***************************/
#include <stdlib.h>
#include <memory.h>
#include "sha256.h"

/****************************** MACROS ******************************/
#define ROTLEFT(a,b) (((a) << (b)) | ((a) >> (32-(b))))
#define ROTRIGHT(a,b) (((a) >> (b)) | ((a) << (32-(b))))

#define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22))
#define EP1(x) (ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25))
#define SIG0(x) (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ ((x) >> 3))
#define SIG1(x) (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ ((x) >> 10))

/**************************** VARIABLES *****************************/
static const WORD k[64] = {
	0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
	0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
	0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
	0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
	0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
	0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
	0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
	0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

/*********************** FUNCTION DEFINITIONS ***********************/
void sha256_transform(SHA256_CTX *ctx, const BYTE data[])
{
	WORD a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];

	for (i = 0, j = 0; i < 16; ++i, j += 4)
		m[i] = (data[j] << 24) | (data[j + 1] << 16) | (data[j + 2] << 8) | (data[j + 3]);
	for ( ; i < 64; ++i)
		m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];

	a = ctx->state[0];
	b = ctx->state[1];
	c = ctx->state[2];
	d = ctx->state[3];
	e = ctx->state[4];
	f = ctx->state[5];
	g = ctx->state[6];
	h = ctx->state[7];

	for (i = 0; i < 64; ++i) {
		t1 = h + EP1(e) + CH(e,f,g) + k[i] + m[i];
		t2 = EP0(a) + MAJ(a,b,c);
		h = g;
		g = f;
		f = e;
		e = d + t1;
		d = c;
		c = b;
		b = a;
		a = t1 + t2;
	}

	ctx->state[0] += a;
	ctx->state[1] += b;
	ctx->state[2] += c;
	ctx->state[3] += d;
	ctx->state[4] += e;
	ctx->state[5] += f;
	ctx->state[6] += g;
	ctx->state[7] += h;
}

void sha256_init(SHA256_CTX *ctx)
{
	ctx->datalen = 0;
	ctx->bitlen = 0;
	ctx->state[0] = 0x6a09e667;
	ctx->state[1] = 0xbb67ae85;
	ctx->state[2] = 0x3c6ef372;
	ctx->state[3] = 0xa54ff53a;
	ctx->state[4] = 0x510e527f;
	ctx->state[5] = 0x9b05688c;
	ctx->state[6] = 0x1f83d9ab;
	ctx->state[7] = 0x5be0cd19;
}

void sha256_update(SHA256_CTX *ctx, const BYTE data[], size_t len)
{
	WORD i;

	for (i = 0; i < len; ++i) {
		ctx->data[ctx->datalen] = data[i];
		ctx->datalen++;
		if (ctx->datalen == 64) {
			sha256_transform(ctx, ctx->data);
			ctx->bitlen += 512;
			ctx->datalen = 0;
		}
	}
}

void sha256_final(SHA256_CTX *ctx, BYTE hash[])
{
	WORD i;

	i = ctx->datalen;

	// Pad whatever data is left in the buffer.
	if (ctx->datalen < 56) {
		ctx->data[i++] = 0x80;
		while (i < 56)
			ctx->data[i++] = 0x00;
	}
	else {
		ctx->data[i++] = 0x80;
		while (i < 64)
			ctx->data[i++] = 0x00;
		sha256_transform(ctx, ctx->data);
		memset(ctx->data, 0, 56);
	}

	// Append to the padding the total message's length in bits and transform.
	ctx->bitlen += ctx->datalen * 8;
	ctx->data[63] = ctx->bitlen;
	ctx->data[62] = ctx->bitlen >> 8;
	ctx->data[61] = ctx->bitlen >> 16;
	ctx->data[60] = ctx->bitlen >> 24;
	ctx->data[59] = ctx->bitlen >> 32;
	ctx->data[58] = ctx->bitlen >> 40;
	ctx->data[57] = ctx->bitlen >> 48;
	ctx->data[56] = ctx->bitlen >> 56;
	sha256_transform(ctx, ctx->data);

	// Since this implementation uses little endian byte ordering and SHA uses big endian,
	// reverse all the bytes when copying the final state to the output hash.
	for (i = 0; i < 4; ++i) {
		hash[i]      = (ctx->state[0] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 4]  = (ctx->state[1] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 8]  = (ctx->state[2] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 12] = (ctx->state[3] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 16] = (ctx->state[4] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 20] = (ctx->state[5] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 24] = (ctx->state[6] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 28] = (ctx->state[7] >> (24 - i * 8)) & 0x000000ff;
	}
}

 #else

#warning "Running Neon version"
 /*************************** HEADER FILES ***************************/
 #include <stdlib.h>
 #include <memory.h>
 #include <arm_neon.h>
 #include "sha256.h"

 /*************************** DEFINITIONS ****************************/
 // #define ROLLED_UP_METHOD

 /**************************** VARIABLES *****************************/
 static const WORD k[64] = {
 	0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
 	0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
 	0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
 	0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
 	0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
 	0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
 	0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
 	0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
 };

 /****************************** TYPES *******************************/
 struct hash_s {
 	uint32x4_t abcd, efgh;
 };
 typedef struct hash_s hash_state_t;

 /*********************** FUNCTION DEFINITIONS ***********************/
 void sha256_transform(SHA256_CTX *ctx, const BYTE data[])
 {
 	WORD i;
 	uint32x4_t round_input;
 	hash_state_t cur_state, old_state;
 #ifdef ROLLED_UP_METHOD
 	// we actually only need 1/4 of the schedule saved in memory
 	uint32x4_t sched[4];

 	// load state
 	cur_state.abcd = vld1q_u32(ctx->state);
 	cur_state.efgh = vld1q_u32(ctx->state+4);

 	// make the schedule
 	for (i = 0; i < 4; i++) {
 		// reverses the byte ordering
 		sched[i] = vreinterpretq_u32_u8(vrev32q_u8(vld1q_u8(data + 16*i)));
 		round_input = vaddq_u32(sched[i], vld1q_u32(k + i*4));
 		old_state.abcd = cur_state.abcd;
 		old_state.efgh = cur_state.efgh;
 		cur_state.abcd = vsha256hq_u32 (old_state.abcd, old_state.efgh, round_input);
 		cur_state.efgh = vsha256h2q_u32(old_state.efgh, old_state.abcd, round_input);
 	}
 	for (i = 4; i < 16; i++) {
 		// mod arithmetic, use sched in a round
 		sched[i%4] = vsha256su1q_u32(vsha256su0q_u32(
 					sched[ i   %4], sched[(i+1)%4]),
 					sched[(i+2)%4], sched[(i+3)%4]);
 		round_input = vaddq_u32(sched[i%4], vld1q_u32(k + i*4));
 		old_state.abcd = cur_state.abcd;
 		old_state.efgh = cur_state.efgh;
 		cur_state.abcd = vsha256hq_u32 (old_state.abcd, old_state.efgh, round_input);
 		cur_state.efgh = vsha256h2q_u32(old_state.efgh, old_state.abcd, round_input);
 	}
 #else
 	uint32x4_t sched[16];

 	// make the schedule
 	for (i = 0; i < 4; i++) {
 		// reverses the byte ordering
 		sched[i] = vreinterpretq_u32_u8(vrev32q_u8(vld1q_u8(data + 16*i)));
 	}
 	for (i = 4; i < 16; i++) {
 		sched[i] = vsha256su1q_u32(vsha256su0q_u32(
 				sched[i-4], sched[i-3]), sched[i-2], sched[i-1]);
 	}

 	// load state
 	cur_state.abcd = vld1q_u32(ctx->state);
 	cur_state.efgh = vld1q_u32(ctx->state+4);

 	// do the hashing
 	for (i = 0; i < 16; i++) {
 		round_input = vaddq_u32(sched[i], vld1q_u32(k + i*4));
 		old_state.abcd = cur_state.abcd;
 		old_state.efgh = cur_state.efgh;
 		cur_state.abcd = vsha256hq_u32 (old_state.abcd, old_state.efgh, round_input);
 		cur_state.efgh = vsha256h2q_u32(old_state.efgh, old_state.abcd, round_input);
 	}
 #endif

 	// add in the state
 	vst1q_u32(ctx->state, vaddq_u32(cur_state.abcd, vld1q_u32(ctx->state)));
 	vst1q_u32(ctx->state+4, vaddq_u32(cur_state.efgh, vld1q_u32(ctx->state+4)));
 }

 void sha256_init(SHA256_CTX *ctx)
 {
 	ctx->datalen = 0;
 	ctx->bitlen = 0;
 	ctx->state[0] = 0x6a09e667;
 	ctx->state[1] = 0xbb67ae85;
 	ctx->state[2] = 0x3c6ef372;
 	ctx->state[3] = 0xa54ff53a;
 	ctx->state[4] = 0x510e527f;
 	ctx->state[5] = 0x9b05688c;
 	ctx->state[6] = 0x1f83d9ab;
 	ctx->state[7] = 0x5be0cd19;
 }

 void sha256_update(SHA256_CTX *ctx, const BYTE data[], size_t len)
 {
 	WORD i;

 	for (i = 0; i < len; ++i) {
 		ctx->data[ctx->datalen] = data[i];
 		ctx->datalen++;
 		if (ctx->datalen == 64) {
 			sha256_transform(ctx, ctx->data);
 			ctx->bitlen += 512;
 			ctx->datalen = 0;
 		}
 	}
 }

 void sha256_final(SHA256_CTX *ctx, BYTE hash[])
 {
 	WORD i;

 	i = ctx->datalen;

 	// Pad whatever data is left in the buffer.
 	if (ctx->datalen < 56) {
 		ctx->data[i++] = 0x80;
 		while (i < 56)
 			ctx->data[i++] = 0x00;
 	}
 	else {
 		ctx->data[i++] = 0x80;
 		while (i < 64)
 			ctx->data[i++] = 0x00;
 		sha256_transform(ctx, ctx->data);
 		memset(ctx->data, 0, 56);
 	}

 	// Append to the padding the total message's length in bits and transform.
 	ctx->bitlen += ctx->datalen * 8;
 	ctx->data[63] = ctx->bitlen;
 	ctx->data[62] = ctx->bitlen >> 8;
 	ctx->data[61] = ctx->bitlen >> 16;
 	ctx->data[60] = ctx->bitlen >> 24;
 	ctx->data[59] = ctx->bitlen >> 32;
 	ctx->data[58] = ctx->bitlen >> 40;
 	ctx->data[57] = ctx->bitlen >> 48;
 	ctx->data[56] = ctx->bitlen >> 56;
 	sha256_transform(ctx, ctx->data);

 	// Since this implementation uses little endian byte ordering and SHA uses big endian,
 	// reverse all the bytes when copying the final state to the output hash.
 	// for (i = 0; i < 4; ++i) {
 	for (i = 0; i < 8; i+=4) {
 		vst1q_u8(hash + (i*4),
 				vrev32q_u8(vld1q_u8((BYTE*)&ctx->state[i])));
 	}
 }
#endif

void sha256_hash(SHA256_CTX* ctx, BYTE data[], BYTE hash[], size_t runLen, size_t len)
{
	size_t idx;
	sha256_init(ctx);
	for (idx = 0; idx < runLen; ++idx)
		sha256_update(ctx, data, len);
    sha256_final(ctx, hash);
}

void sha256_hash_file(SHA256_CTX* ctx, const BYTE data[], BYTE hash[], size_t dataLen)
{
	size_t idx;
	sha256_init(ctx);
	sha256_update(ctx, data, dataLen);
    sha256_final(ctx, hash);
}