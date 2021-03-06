#include <stdio.h>
#include "my_des.h"
#include "public.h"

#define MBEDTLS_DES_ENCRYPT     1
#define MBEDTLS_DES_DECRYPT     0

#define MBEDTLS_ERR_DES_INVALID_INPUT_LENGTH              -0x0002 /**< The data input has an invalid length. */

#define MBEDTLS_DES_KEY_SIZE    8
//#define DES_KEY_SIZE         (8)
#define DES3_KEY2_SIZE       (16)
#define DES3_KEY3_SIZE       (24)

uint8_t des_key[8]= {0,1,3,2,4};
#define DES_C

#if defined(DES_C)

#include <string.h>
#include <stdlib.h>


#if !defined(DES_ALT)

/* Implementation that should never be optimized out by the compiler */
static void zeroize( void *v, size_t n ) {
    volatile unsigned char *p = (unsigned char*)v;
    while( n-- ) *p++ = 0;
}

/*
 * 32-bit integer manipulation macros (big endian)
 */
#ifndef GET_UINT32_BE
#define GET_UINT32_BE(n,b,i)                            \
{                                                       \
    (n) = ( (uint32_t) (b)[(i)    ] << 24 )             \
        | ( (uint32_t) (b)[(i) + 1] << 16 )             \
        | ( (uint32_t) (b)[(i) + 2] <<  8 )             \
        | ( (uint32_t) (b)[(i) + 3]       );            \
}
#endif

#ifndef PUT_UINT32_BE
#define PUT_UINT32_BE(n,b,i)                            \
{                                                       \
    (b)[(i)    ] = (unsigned char) ( (n) >> 24 );       \
    (b)[(i) + 1] = (unsigned char) ( (n) >> 16 );       \
    (b)[(i) + 2] = (unsigned char) ( (n) >>  8 );       \
    (b)[(i) + 3] = (unsigned char) ( (n)       );       \
}
#endif

/*
 * Expanded DES S-boxes
 */
static const uint32_t SB1[64] =
{
    0x01010400, 0x00000000, 0x00010000, 0x01010404,
    0x01010004, 0x00010404, 0x00000004, 0x00010000,
    0x00000400, 0x01010400, 0x01010404, 0x00000400,
    0x01000404, 0x01010004, 0x01000000, 0x00000004,
    0x00000404, 0x01000400, 0x01000400, 0x00010400,
    0x00010400, 0x01010000, 0x01010000, 0x01000404,
    0x00010004, 0x01000004, 0x01000004, 0x00010004,
    0x00000000, 0x00000404, 0x00010404, 0x01000000,
    0x00010000, 0x01010404, 0x00000004, 0x01010000,
    0x01010400, 0x01000000, 0x01000000, 0x00000400,
    0x01010004, 0x00010000, 0x00010400, 0x01000004,
    0x00000400, 0x00000004, 0x01000404, 0x00010404,
    0x01010404, 0x00010004, 0x01010000, 0x01000404,
    0x01000004, 0x00000404, 0x00010404, 0x01010400,
    0x00000404, 0x01000400, 0x01000400, 0x00000000,
    0x00010004, 0x00010400, 0x00000000, 0x01010004
};

static const uint32_t SB2[64] =
{
    0x80108020, 0x80008000, 0x00008000, 0x00108020,
    0x00100000, 0x00000020, 0x80100020, 0x80008020,
    0x80000020, 0x80108020, 0x80108000, 0x80000000,
    0x80008000, 0x00100000, 0x00000020, 0x80100020,
    0x00108000, 0x00100020, 0x80008020, 0x00000000,
    0x80000000, 0x00008000, 0x00108020, 0x80100000,
    0x00100020, 0x80000020, 0x00000000, 0x00108000,
    0x00008020, 0x80108000, 0x80100000, 0x00008020,
    0x00000000, 0x00108020, 0x80100020, 0x00100000,
    0x80008020, 0x80100000, 0x80108000, 0x00008000,
    0x80100000, 0x80008000, 0x00000020, 0x80108020,
    0x00108020, 0x00000020, 0x00008000, 0x80000000,
    0x00008020, 0x80108000, 0x00100000, 0x80000020,
    0x00100020, 0x80008020, 0x80000020, 0x00100020,
    0x00108000, 0x00000000, 0x80008000, 0x00008020,
    0x80000000, 0x80100020, 0x80108020, 0x00108000
};

static const uint32_t SB3[64] =
{
    0x00000208, 0x08020200, 0x00000000, 0x08020008,
    0x08000200, 0x00000000, 0x00020208, 0x08000200,
    0x00020008, 0x08000008, 0x08000008, 0x00020000,
    0x08020208, 0x00020008, 0x08020000, 0x00000208,
    0x08000000, 0x00000008, 0x08020200, 0x00000200,
    0x00020200, 0x08020000, 0x08020008, 0x00020208,
    0x08000208, 0x00020200, 0x00020000, 0x08000208,
    0x00000008, 0x08020208, 0x00000200, 0x08000000,
    0x08020200, 0x08000000, 0x00020008, 0x00000208,
    0x00020000, 0x08020200, 0x08000200, 0x00000000,
    0x00000200, 0x00020008, 0x08020208, 0x08000200,
    0x08000008, 0x00000200, 0x00000000, 0x08020008,
    0x08000208, 0x00020000, 0x08000000, 0x08020208,
    0x00000008, 0x00020208, 0x00020200, 0x08000008,
    0x08020000, 0x08000208, 0x00000208, 0x08020000,
    0x00020208, 0x00000008, 0x08020008, 0x00020200
};

static const uint32_t SB4[64] =
{
    0x00802001, 0x00002081, 0x00002081, 0x00000080,
    0x00802080, 0x00800081, 0x00800001, 0x00002001,
    0x00000000, 0x00802000, 0x00802000, 0x00802081,
    0x00000081, 0x00000000, 0x00800080, 0x00800001,
    0x00000001, 0x00002000, 0x00800000, 0x00802001,
    0x00000080, 0x00800000, 0x00002001, 0x00002080,
    0x00800081, 0x00000001, 0x00002080, 0x00800080,
    0x00002000, 0x00802080, 0x00802081, 0x00000081,
    0x00800080, 0x00800001, 0x00802000, 0x00802081,
    0x00000081, 0x00000000, 0x00000000, 0x00802000,
    0x00002080, 0x00800080, 0x00800081, 0x00000001,
    0x00802001, 0x00002081, 0x00002081, 0x00000080,
    0x00802081, 0x00000081, 0x00000001, 0x00002000,
    0x00800001, 0x00002001, 0x00802080, 0x00800081,
    0x00002001, 0x00002080, 0x00800000, 0x00802001,
    0x00000080, 0x00800000, 0x00002000, 0x00802080
};

static const uint32_t SB5[64] =
{
    0x00000100, 0x02080100, 0x02080000, 0x42000100,
    0x00080000, 0x00000100, 0x40000000, 0x02080000,
    0x40080100, 0x00080000, 0x02000100, 0x40080100,
    0x42000100, 0x42080000, 0x00080100, 0x40000000,
    0x02000000, 0x40080000, 0x40080000, 0x00000000,
    0x40000100, 0x42080100, 0x42080100, 0x02000100,
    0x42080000, 0x40000100, 0x00000000, 0x42000000,
    0x02080100, 0x02000000, 0x42000000, 0x00080100,
    0x00080000, 0x42000100, 0x00000100, 0x02000000,
    0x40000000, 0x02080000, 0x42000100, 0x40080100,
    0x02000100, 0x40000000, 0x42080000, 0x02080100,
    0x40080100, 0x00000100, 0x02000000, 0x42080000,
    0x42080100, 0x00080100, 0x42000000, 0x42080100,
    0x02080000, 0x00000000, 0x40080000, 0x42000000,
    0x00080100, 0x02000100, 0x40000100, 0x00080000,
    0x00000000, 0x40080000, 0x02080100, 0x40000100
};

static const uint32_t SB6[64] =
{
    0x20000010, 0x20400000, 0x00004000, 0x20404010,
    0x20400000, 0x00000010, 0x20404010, 0x00400000,
    0x20004000, 0x00404010, 0x00400000, 0x20000010,
    0x00400010, 0x20004000, 0x20000000, 0x00004010,
    0x00000000, 0x00400010, 0x20004010, 0x00004000,
    0x00404000, 0x20004010, 0x00000010, 0x20400010,
    0x20400010, 0x00000000, 0x00404010, 0x20404000,
    0x00004010, 0x00404000, 0x20404000, 0x20000000,
    0x20004000, 0x00000010, 0x20400010, 0x00404000,
    0x20404010, 0x00400000, 0x00004010, 0x20000010,
    0x00400000, 0x20004000, 0x20000000, 0x00004010,
    0x20000010, 0x20404010, 0x00404000, 0x20400000,
    0x00404010, 0x20404000, 0x00000000, 0x20400010,
    0x00000010, 0x00004000, 0x20400000, 0x00404010,
    0x00004000, 0x00400010, 0x20004010, 0x00000000,
    0x20404000, 0x20000000, 0x00400010, 0x20004010
};

static const uint32_t SB7[64] =
{
    0x00200000, 0x04200002, 0x04000802, 0x00000000,
    0x00000800, 0x04000802, 0x00200802, 0x04200800,
    0x04200802, 0x00200000, 0x00000000, 0x04000002,
    0x00000002, 0x04000000, 0x04200002, 0x00000802,
    0x04000800, 0x00200802, 0x00200002, 0x04000800,
    0x04000002, 0x04200000, 0x04200800, 0x00200002,
    0x04200000, 0x00000800, 0x00000802, 0x04200802,
    0x00200800, 0x00000002, 0x04000000, 0x00200800,
    0x04000000, 0x00200800, 0x00200000, 0x04000802,
    0x04000802, 0x04200002, 0x04200002, 0x00000002,
    0x00200002, 0x04000000, 0x04000800, 0x00200000,
    0x04200800, 0x00000802, 0x00200802, 0x04200800,
    0x00000802, 0x04000002, 0x04200802, 0x04200000,
    0x00200800, 0x00000000, 0x00000002, 0x04200802,
    0x00000000, 0x00200802, 0x04200000, 0x00000800,
    0x04000002, 0x04000800, 0x00000800, 0x00200002
};

static const uint32_t SB8[64] =
{
    0x10001040, 0x00001000, 0x00040000, 0x10041040,
    0x10000000, 0x10001040, 0x00000040, 0x10000000,
    0x00040040, 0x10040000, 0x10041040, 0x00041000,
    0x10041000, 0x00041040, 0x00001000, 0x00000040,
    0x10040000, 0x10000040, 0x10001000, 0x00001040,
    0x00041000, 0x00040040, 0x10040040, 0x10041000,
    0x00001040, 0x00000000, 0x00000000, 0x10040040,
    0x10000040, 0x10001000, 0x00041040, 0x00040000,
    0x00041040, 0x00040000, 0x10041000, 0x00001000,
    0x00000040, 0x10040040, 0x00001000, 0x00041040,
    0x10001000, 0x00000040, 0x10000040, 0x10040000,
    0x10040040, 0x10000000, 0x00040000, 0x10001040,
    0x00000000, 0x10041040, 0x00040040, 0x10000040,
    0x10040000, 0x10001000, 0x10001040, 0x00000000,
    0x10041040, 0x00041000, 0x00041000, 0x00001040,
    0x00001040, 0x00040040, 0x10000000, 0x10041000
};

/*
 * PC1: left and right halves bit-swap
 */
static const uint32_t LHs[16] =
{
    0x00000000, 0x00000001, 0x00000100, 0x00000101,
    0x00010000, 0x00010001, 0x00010100, 0x00010101,
    0x01000000, 0x01000001, 0x01000100, 0x01000101,
    0x01010000, 0x01010001, 0x01010100, 0x01010101
};

static const uint32_t RHs[16] =
{
    0x00000000, 0x01000000, 0x00010000, 0x01010000,
    0x00000100, 0x01000100, 0x00010100, 0x01010100,
    0x00000001, 0x01000001, 0x00010001, 0x01010001,
    0x00000101, 0x01000101, 0x00010101, 0x01010101,
};

/*
 * Initial Permutation macro
 */
#define DES_IP(X,Y)                                             \
{                                                               \
    T = ((X >>  4) ^ Y) & 0x0F0F0F0F; Y ^= T; X ^= (T <<  4);   \
    T = ((X >> 16) ^ Y) & 0x0000FFFF; Y ^= T; X ^= (T << 16);   \
    T = ((Y >>  2) ^ X) & 0x33333333; X ^= T; Y ^= (T <<  2);   \
    T = ((Y >>  8) ^ X) & 0x00FF00FF; X ^= T; Y ^= (T <<  8);   \
    Y = ((Y << 1) | (Y >> 31)) & 0xFFFFFFFF;                    \
    T = (X ^ Y) & 0xAAAAAAAA; Y ^= T; X ^= T;                   \
    X = ((X << 1) | (X >> 31)) & 0xFFFFFFFF;                    \
}

/*
 * Final Permutation macro
 */
#define DES_FP(X,Y)                                             \
{                                                               \
    X = ((X << 31) | (X >> 1)) & 0xFFFFFFFF;                    \
    T = (X ^ Y) & 0xAAAAAAAA; X ^= T; Y ^= T;                   \
    Y = ((Y << 31) | (Y >> 1)) & 0xFFFFFFFF;                    \
    T = ((Y >>  8) ^ X) & 0x00FF00FF; X ^= T; Y ^= (T <<  8);   \
    T = ((Y >>  2) ^ X) & 0x33333333; X ^= T; Y ^= (T <<  2);   \
    T = ((X >> 16) ^ Y) & 0x0000FFFF; Y ^= T; X ^= (T << 16);   \
    T = ((X >>  4) ^ Y) & 0x0F0F0F0F; Y ^= T; X ^= (T <<  4);   \
}

/*
 * DES round macro
 */
#define DES_ROUND(X,Y)                          \
{                                               \
    T = *SK++ ^ X;                              \
    Y ^= SB8[ (T      ) & 0x3F ] ^              \
         SB6[ (T >>  8) & 0x3F ] ^              \
         SB4[ (T >> 16) & 0x3F ] ^              \
         SB2[ (T >> 24) & 0x3F ];               \
                                                \
    T = *SK++ ^ ((X << 28) | (X >> 4));         \
    Y ^= SB7[ (T      ) & 0x3F ] ^              \
         SB5[ (T >>  8) & 0x3F ] ^              \
         SB3[ (T >> 16) & 0x3F ] ^              \
         SB1[ (T >> 24) & 0x3F ];               \
}

#define SWAP(a,b) { uint32_t t = a; a = b; b = t; t = 0; }

void des_init( des_context *ctx )
{
    memset( ctx, 0, sizeof( des_context ) );
}

void des_free( des_context *ctx )
{
    if( ctx == NULL )
        return;

    zeroize( ctx, sizeof( des_context ) );
}

void des3_init( des3_context *ctx )
{
    memset( ctx, 0, sizeof( des3_context ) );
}

void des3_free( des3_context *ctx )
{
    if( ctx == NULL )
        return;

    zeroize( ctx, sizeof( des3_context ) );
}

static const unsigned char odd_parity_table[128] = { 1,  2,  4,  7,  8,
        11, 13, 14, 16, 19, 21, 22, 25, 26, 28, 31, 32, 35, 37, 38, 41, 42, 44,
        47, 49, 50, 52, 55, 56, 59, 61, 62, 64, 67, 69, 70, 73, 74, 76, 79, 81,
        82, 84, 87, 88, 91, 93, 94, 97, 98, 100, 103, 104, 107, 109, 110, 112,
        115, 117, 118, 121, 122, 124, 127, 128, 131, 133, 134, 137, 138, 140,
        143, 145, 146, 148, 151, 152, 155, 157, 158, 161, 162, 164, 167, 168,
        171, 173, 174, 176, 179, 181, 182, 185, 186, 188, 191, 193, 194, 196,
        199, 200, 203, 205, 206, 208, 211, 213, 214, 217, 218, 220, 223, 224,
        227, 229, 230, 233, 234, 236, 239, 241, 242, 244, 247, 248, 251, 253,
        254
                                                   };

void des_key_set_parity( unsigned char key[DES_KEY_SIZE] )
{
    int i;

    for( i = 0; i < DES_KEY_SIZE; i++ )
        key[i] = odd_parity_table[key[i] / 2];
}

/*
 * Check the given key's parity, returns 1 on failure, 0 on SUCCESS
 */
int des_key_check_key_parity( const unsigned char key[DES_KEY_SIZE] )
{
    int i;

    for( i = 0; i < DES_KEY_SIZE; i++ )
        if( key[i] != odd_parity_table[key[i] / 2] )
            return( 1 );

    return( 0 );
}

/*
 * Table of weak and semi-weak keys
 *
 * Source: http://en.wikipedia.org/wiki/Weak_key
 *
 * Weak:
 * Alternating ones + zeros (0x0101010101010101)
 * Alternating 'F' + 'E' (0xFEFEFEFEFEFEFEFE)
 * '0xE0E0E0E0F1F1F1F1'
 * '0x1F1F1F1F0E0E0E0E'
 *
 * Semi-weak:
 * 0x011F011F010E010E and 0x1F011F010E010E01
 * 0x01E001E001F101F1 and 0xE001E001F101F101
 * 0x01FE01FE01FE01FE and 0xFE01FE01FE01FE01
 * 0x1FE01FE00EF10EF1 and 0xE01FE01FF10EF10E
 * 0x1FFE1FFE0EFE0EFE and 0xFE1FFE1FFE0EFE0E
 * 0xE0FEE0FEF1FEF1FE and 0xFEE0FEE0FEF1FEF1
 *
 */

#define WEAK_KEY_COUNT 16

static const unsigned char weak_key_table[WEAK_KEY_COUNT][DES_KEY_SIZE] =
{
    { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 },
    { 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE },
    { 0x1F, 0x1F, 0x1F, 0x1F, 0x0E, 0x0E, 0x0E, 0x0E },
    { 0xE0, 0xE0, 0xE0, 0xE0, 0xF1, 0xF1, 0xF1, 0xF1 },

    { 0x01, 0x1F, 0x01, 0x1F, 0x01, 0x0E, 0x01, 0x0E },
    { 0x1F, 0x01, 0x1F, 0x01, 0x0E, 0x01, 0x0E, 0x01 },
    { 0x01, 0xE0, 0x01, 0xE0, 0x01, 0xF1, 0x01, 0xF1 },
    { 0xE0, 0x01, 0xE0, 0x01, 0xF1, 0x01, 0xF1, 0x01 },
    { 0x01, 0xFE, 0x01, 0xFE, 0x01, 0xFE, 0x01, 0xFE },
    { 0xFE, 0x01, 0xFE, 0x01, 0xFE, 0x01, 0xFE, 0x01 },
    { 0x1F, 0xE0, 0x1F, 0xE0, 0x0E, 0xF1, 0x0E, 0xF1 },
    { 0xE0, 0x1F, 0xE0, 0x1F, 0xF1, 0x0E, 0xF1, 0x0E },
    { 0x1F, 0xFE, 0x1F, 0xFE, 0x0E, 0xFE, 0x0E, 0xFE },
    { 0xFE, 0x1F, 0xFE, 0x1F, 0xFE, 0x0E, 0xFE, 0x0E },
    { 0xE0, 0xFE, 0xE0, 0xFE, 0xF1, 0xFE, 0xF1, 0xFE },
    { 0xFE, 0xE0, 0xFE, 0xE0, 0xFE, 0xF1, 0xFE, 0xF1 }
};

int des_key_check_weak( const unsigned char key[DES_KEY_SIZE] )
{
    int i;

    for( i = 0; i < WEAK_KEY_COUNT; i++ )
        if( memcmp( weak_key_table[i], key, DES_KEY_SIZE) == 0 )
            return( 1 );

    return( 0 );
}


void des_setkey( uint32_t SK[32], const unsigned char key[DES_KEY_SIZE] )
{
    int i;
    uint32_t X, Y, T;

    GET_UINT32_BE( X, key, 0 );
    GET_UINT32_BE( Y, key, 4 );

    /*
     * Permuted Choice 1
     */
    T =  ((Y >>  4) ^ X) & 0x0F0F0F0F;
    X ^= T;
    Y ^= (T <<  4);
    T =  ((Y      ) ^ X) & 0x10101010;
    X ^= T;
    Y ^= (T      );

    X =   (LHs[ (X      ) & 0xF] << 3) | (LHs[ (X >>  8) & 0xF ] << 2)
          | (LHs[ (X >> 16) & 0xF] << 1) | (LHs[ (X >> 24) & 0xF ]     )
          | (LHs[ (X >>  5) & 0xF] << 7) | (LHs[ (X >> 13) & 0xF ] << 6)
          | (LHs[ (X >> 21) & 0xF] << 5) | (LHs[ (X >> 29) & 0xF ] << 4);

    Y =   (RHs[ (Y >>  1) & 0xF] << 3) | (RHs[ (Y >>  9) & 0xF ] << 2)
          | (RHs[ (Y >> 17) & 0xF] << 1) | (RHs[ (Y >> 25) & 0xF ]     )
          | (RHs[ (Y >>  4) & 0xF] << 7) | (RHs[ (Y >> 12) & 0xF ] << 6)
          | (RHs[ (Y >> 20) & 0xF] << 5) | (RHs[ (Y >> 28) & 0xF ] << 4);

    X &= 0x0FFFFFFF;
    Y &= 0x0FFFFFFF;

    /*
     * calculate subkeys
     */
    for( i = 0; i < 16; i++ )
    {
        if( i < 2 || i == 8 || i == 15 )
        {
            X = ((X <<  1) | (X >> 27)) & 0x0FFFFFFF;
            Y = ((Y <<  1) | (Y >> 27)) & 0x0FFFFFFF;
        }
        else
        {
            X = ((X <<  2) | (X >> 26)) & 0x0FFFFFFF;
            Y = ((Y <<  2) | (Y >> 26)) & 0x0FFFFFFF;
        }

        *SK++ =   ((X <<  4) & 0x24000000) | ((X << 28) & 0x10000000)
                  | ((X << 14) & 0x08000000) | ((X << 18) & 0x02080000)
                  | ((X <<  6) & 0x01000000) | ((X <<  9) & 0x00200000)
                  | ((X >>  1) & 0x00100000) | ((X << 10) & 0x00040000)
                  | ((X <<  2) & 0x00020000) | ((X >> 10) & 0x00010000)
                  | ((Y >> 13) & 0x00002000) | ((Y >>  4) & 0x00001000)
                  | ((Y <<  6) & 0x00000800) | ((Y >>  1) & 0x00000400)
                  | ((Y >> 14) & 0x00000200) | ((Y      ) & 0x00000100)
                  | ((Y >>  5) & 0x00000020) | ((Y >> 10) & 0x00000010)
                  | ((Y >>  3) & 0x00000008) | ((Y >> 18) & 0x00000004)
                  | ((Y >> 26) & 0x00000002) | ((Y >> 24) & 0x00000001);

        *SK++ =   ((X << 15) & 0x20000000) | ((X << 17) & 0x10000000)
                  | ((X << 10) & 0x08000000) | ((X << 22) & 0x04000000)
                  | ((X >>  2) & 0x02000000) | ((X <<  1) & 0x01000000)
                  | ((X << 16) & 0x00200000) | ((X << 11) & 0x00100000)
                  | ((X <<  3) & 0x00080000) | ((X >>  6) & 0x00040000)
                  | ((X << 15) & 0x00020000) | ((X >>  4) & 0x00010000)
                  | ((Y >>  2) & 0x00002000) | ((Y <<  8) & 0x00001000)
                  | ((Y >> 14) & 0x00000808) | ((Y >>  9) & 0x00000400)
                  | ((Y      ) & 0x00000200) | ((Y <<  7) & 0x00000100)
                  | ((Y >>  7) & 0x00000020) | ((Y >>  3) & 0x00000011)
                  | ((Y <<  2) & 0x00000004) | ((Y >> 21) & 0x00000002);
    }
}


/*
 * DES key schedule (56-bit, encryption)
 */
int des_setkey_enc( des_context *ctx, const unsigned char key[DES_KEY_SIZE] )
{
    des_setkey( ctx->sk, key );

    return( 0 );
}

/*
 * DES key schedule (56-bit, decryption)
 */
int des_setkey_dec( des_context *ctx, const unsigned char key[DES_KEY_SIZE] )
{
    int i;

    des_setkey( ctx->sk, key );

    for( i = 0; i < 16; i += 2 )
    {
        SWAP( ctx->sk[i    ], ctx->sk[30 - i] );
        SWAP( ctx->sk[i + 1], ctx->sk[31 - i] );
    }

    return( 0 );
}

static void des3_set2key( uint32_t esk[96],
                          uint32_t dsk[96],
                          const unsigned char key[DES_KEY_SIZE*2] )
{
    int i;

    des_setkey( esk, key );
    des_setkey( dsk + 32, key + 8 );

    for( i = 0; i < 32; i += 2 )
    {
        dsk[i     ] = esk[30 - i];
        dsk[i +  1] = esk[31 - i];

        esk[i + 32] = dsk[62 - i];
        esk[i + 33] = dsk[63 - i];

        esk[i + 64] = esk[i    ];
        esk[i + 65] = esk[i + 1];

        dsk[i + 64] = dsk[i    ];
        dsk[i + 65] = dsk[i + 1];
    }
}

/*
 * Triple-DES key schedule (112-bit, encryption)
 */
int des3_set2key_enc( des3_context *ctx,
                      const unsigned char key[DES_KEY_SIZE * 2] )
{
    uint32_t sk[96];

    des3_set2key( ctx->sk, sk, key );
    zeroize( sk,  sizeof( sk ) );

    return( 0 );
}

/*
 * Triple-DES key schedule (112-bit, decryption)
 */
int des3_set2key_dec( des3_context *ctx,
                      const unsigned char key[DES_KEY_SIZE * 2] )
{
    uint32_t sk[96];

    des3_set2key( sk, ctx->sk, key );
    zeroize( sk,  sizeof( sk ) );

    return( 0 );
}

static void des3_set3key( uint32_t esk[96],
                          uint32_t dsk[96],
                          const unsigned char key[24] )
{
    int i;

    des_setkey( esk, key );
    des_setkey( dsk + 32, key +  8 );
    des_setkey( esk + 64, key + 16 );

    for( i = 0; i < 32; i += 2 )
    {
        dsk[i     ] = esk[94 - i];
        dsk[i +  1] = esk[95 - i];

        esk[i + 32] = dsk[62 - i];
        esk[i + 33] = dsk[63 - i];

        dsk[i + 64] = esk[30 - i];
        dsk[i + 65] = esk[31 - i];
    }
}

/*
 * Triple-DES key schedule (168-bit, encryption)
 */
int des3_set3key_enc( des3_context *ctx,
                      const unsigned char key[DES_KEY_SIZE * 3] )
{
    uint32_t sk[96];

    des3_set3key( ctx->sk, sk, key );
    zeroize( sk,  sizeof( sk ) );

    return( 0 );
}

/*
 * Triple-DES key schedule (168-bit, decryption)
 */
int des3_set3key_dec( des3_context *ctx,
                      const unsigned char key[DES_KEY_SIZE * 3] )
{
    uint32_t sk[96];

    des3_set3key( sk, ctx->sk, key );
    zeroize( sk,  sizeof( sk ) );

    return( 0 );
}

/*
 * DES-ECB block encryption/decryption
 */

int des_crypt_ecb( des_context *ctx,
                   const unsigned char input[8],
                   unsigned char output[8] )
{
    int i;
    uint32_t X, Y, T, *SK;

    SK = ctx->sk;

    GET_UINT32_BE( X, input, 0 );
    GET_UINT32_BE( Y, input, 4 );

    DES_IP( X, Y );

    for( i = 0; i < 8; i++ )
    {
        DES_ROUND( Y, X );
        DES_ROUND( X, Y );
    }

    DES_FP( Y, X );

    PUT_UINT32_BE( Y, output, 0 );
    PUT_UINT32_BE( X, output, 4 );

    return( 0 );
}



/*
 * DES-CBC buffer encryption/decryption
 */
int des_crypt_cbc( des_context *ctx,
                   int mode,
                   size_t length,
                   unsigned char iv[8],
                   const unsigned char *input,
                   unsigned char *output )
{
    int i;
    unsigned char temp[8];

    if( length % 8 )
        return( ERR_DES_INVALID_INPUT_LENGTH );

    if( mode == DES_ENCRYPT )
    {
        while( length > 0 )
        {
            for( i = 0; i < 8; i++ )
                output[i] = (unsigned char)( input[i] ^ iv[i] );

            des_crypt_ecb( ctx, output, output );
            memcpy( iv, output, 8 );

            input  += 8;
            output += 8;
            length -= 8;
        }
    }
    else /* DES_DECRYPT */
    {
        while( length > 0 )
        {
            memcpy( temp, input, 8 );
            des_crypt_ecb( ctx, input, output );

            for( i = 0; i < 8; i++ )
                output[i] = (unsigned char)( output[i] ^ iv[i] );

            memcpy( iv, temp, 8 );

            input  += 8;
            output += 8;
            length -= 8;
        }
    }

    return( 0 );
}


/*
 * 3DES-ECB block encryption/decryption
 */

int des3_crypt_ecb( des3_context *ctx,
                    const unsigned char input[8],
                    unsigned char output[8] )
{
    int i;
    uint32_t X, Y, T, *SK;

    SK = ctx->sk;

    GET_UINT32_BE( X, input, 0 );
    GET_UINT32_BE( Y, input, 4 );

    DES_IP( X, Y );

    for( i = 0; i < 8; i++ )
    {
        DES_ROUND( Y, X );
        DES_ROUND( X, Y );
    }

    for( i = 0; i < 8; i++ )
    {
        DES_ROUND( X, Y );
        DES_ROUND( Y, X );
    }

    for( i = 0; i < 8; i++ )
    {
        DES_ROUND( Y, X );
        DES_ROUND( X, Y );
    }

    DES_FP( Y, X );

    PUT_UINT32_BE( Y, output, 0 );
    PUT_UINT32_BE( X, output, 4 );

    return( 0 );
}



/*
 * 3DES-CBC buffer encryption/decryption
 */
int des3_crypt_cbc( des3_context *ctx,
                    int mode,
                    size_t length,
                    unsigned char iv[8],
                    const unsigned char *input,
                    unsigned char *output )
{
    int i;
    unsigned char temp[8];

    if( length % 8 )
        return( ERR_DES_INVALID_INPUT_LENGTH );

    if( mode == DES_ENCRYPT )
    {
        while( length > 0 )
        {
            for( i = 0; i < 8; i++ )
                output[i] = (unsigned char)( input[i] ^ iv[i] );

            des3_crypt_ecb( ctx, output, output );
            memcpy( iv, output, 8 );

            input  += 8;
            output += 8;
            length -= 8;
        }
    }
    else /* DES_DECRYPT */
    {
        while( length > 0 )
        {
            memcpy( temp, input, 8 );
            des3_crypt_ecb( ctx, input, output );

            for( i = 0; i < 8; i++ )
                output[i] = (unsigned char)( output[i] ^ iv[i] );

            memcpy( iv, temp, 8 );

            input  += 8;
            output += 8;
            length -= 8;
        }
    }

    return( 0 );
}


#endif /* !DES_ALT */

#endif /* DES_C */

/*
 * DES-ECB buffer encryption API
 */
unsigned int des_ecb_encrypt(unsigned char *pout,
                             unsigned char *pdata,
                             unsigned int nlen,
                             unsigned char *pkey)
{
    unsigned char *tmp;
    unsigned int len,i;
    unsigned char ch = '\0';
    des_context ctx;

    des_setkey_enc( &ctx, pkey );

    len = (nlen / 8 + (nlen % 8 ? 1: 0)) * 8;

    //ch = 8 - nlen % 8;
    for(i = 0; i < nlen; i += 8)
    {
        des_crypt_ecb( &ctx, (pdata + i), (pout + i) );
    }
    if(len > nlen)
    {
        tmp = (unsigned char *)malloc(len);
        i -= 8;
        memcpy(tmp,pdata + i,nlen - i);
        memset(tmp + nlen % 8, ch, (8 - nlen % 8) % 8);
        des_crypt_ecb( &ctx, tmp, (pout + i));
        free(tmp);
    }

    des_free( &ctx );
    return len;


}
/*
 * DES-ECB buffer decryption API
 */
unsigned int des_ecb_decrypt(unsigned char *pout,
                             unsigned char *pdata,
                             unsigned int nlen,
                             unsigned char *pkey)
{

    unsigned int i;
    des_context ctx;

    if(nlen % 8)
        return 1;

    des_setkey_dec( &ctx, pkey );


    for(i = 0; i < nlen; i += 8)
    {
        des_crypt_ecb(&ctx, (pdata + i), (pout + i));
    }
    des_free( &ctx );
    return 0;

}

/*
 * DES-CBC buffer encryption API
 */
unsigned int des_cbc_encrypt(unsigned char *pout,
                             unsigned char *pdata,
                             unsigned int nlen,
                             unsigned char *pkey,
                             unsigned char *piv)
{
    des_context ctx;
    unsigned char iv[8] = {0};
    unsigned char *pivb;

    if(piv == NULL)
        pivb = iv;
    else
        pivb = piv;

    des_setkey_enc( &ctx, pkey );

    des_crypt_cbc( &ctx, 1, nlen, pivb, pdata, (pout));

    des_free( &ctx );

    return nlen;


}
/*
 * DES-CBC buffer decryption API
 */
unsigned int des_cbc_decrypt(unsigned char *pout,
                             unsigned char *pdata,
                             unsigned int nlen,
                             unsigned char *pkey,
                             unsigned char *piv)
{

    des_context ctx;
    unsigned char iv[8] = {0};
    unsigned char *pivb;

    if(piv == NULL)
        pivb = iv;
    else
        pivb = piv;

    des_setkey_dec( &ctx, pkey );

    des_crypt_cbc( &ctx, 0, nlen, pivb, pdata, (pout));

    des_free( &ctx );

    return 0;

}
/*
 * 3DES-ECB buffer encryption API
 */
unsigned int des3_ecb_encrypt(unsigned char *pout,
                              unsigned char *pdata,
                              unsigned int nlen,
                              unsigned char *pkey,
                              unsigned int klen)
{
    unsigned char *tmp;
    unsigned int len,i;
    unsigned char ch = '\0';
    des3_context ctx3;

    if(klen == DES3_KEY2_SIZE)//16????
        des3_set2key_enc( &ctx3, pkey );//????????????key
    else if(klen == DES3_KEY3_SIZE)//24????
        des3_set3key_enc( &ctx3, pkey );

    len = (nlen / 8 + (nlen % 8 ? 1: 0)) * 8;

    //ch = 8 - nlen % 8;//??????????????????????0??0xFF
    for(i = 0; i < nlen; i += 8)
    {
        des3_crypt_ecb( &ctx3, (pdata + i), (pout + i) );
    }
    if(len > nlen)//????8????????
    {
        tmp = (unsigned char *)malloc(len);
        i -= 8;
        memcpy(tmp,pdata + i,nlen - i);
        memset(tmp + nlen % 8, ch, (8 - nlen % 8) % 8);
        des3_crypt_ecb( &ctx3, tmp, (pout + i));
        free(tmp);
    }

    des3_free( &ctx3 );
    return len;


}
/*
 * 3DES-ECB buffer decryption API
 */
unsigned int des3_ecb_decrypt(unsigned char *pout,
                              unsigned char *pdata,
                              unsigned int nlen,
                              unsigned char *pkey,
                              unsigned int klen)
{

    unsigned int i;
    des3_context ctx3;

    if(nlen % 8)
        return 1;

    if(klen == DES3_KEY2_SIZE)
        des3_set2key_dec( &ctx3, pkey );
    else if(klen == DES3_KEY3_SIZE)
        des3_set3key_dec( &ctx3, pkey );


    for(i = 0; i < nlen; i += 8)
    {
        des3_crypt_ecb(&ctx3, (pdata + i), (pout + i));
    }
    des3_free( &ctx3 );
    return 0;

}
/*
 * 3DES-CBC buffer encryption API
 */
unsigned int des3_cbc_encrypt(unsigned char *pout,
                              unsigned char *pdata,
                              unsigned int nlen,
                              unsigned char *pkey,
                              unsigned int klen,
                              unsigned char *piv)
{
    des3_context ctx;
    unsigned char iv[8] = {0};
    unsigned char *pivb;
    unsigned int len;

    if(piv == NULL)
        pivb = iv;
    else
        pivb = piv;

    if(klen == DES3_KEY2_SIZE)
        des3_set2key_enc( &ctx, pkey );
    else if(klen == DES3_KEY3_SIZE)
        des3_set3key_enc( &ctx, pkey );

    if(nlen % 8)
    {

        len = nlen + 8 - nlen % 8;
        char *tmp = (unsigned char *)calloc(1, len);
        memcpy(tmp, pdata, nlen);
        des3_crypt_cbc( &ctx, 1, len, pivb, tmp, (pout));
        free(tmp);
    }
    else
    {
        des3_crypt_cbc( &ctx, 1, nlen, pivb, pdata, (pout));
    }

    des3_free( &ctx );

    return nlen;


}
/*
 * 3DES-CBC buffer decryption API
 */
unsigned int des3_cbc_decrypt(unsigned char *pout,
                              unsigned char *pdata,
                              unsigned int nlen,
                              unsigned char *pkey,
                              unsigned int klen,
                              unsigned char *piv)
{

    des3_context ctx;
    unsigned char iv[8] = {0};
    unsigned char *pivb;

    if(nlen % 8)
        return 1;

    if(piv == NULL)
        pivb = iv;
    else
        pivb = piv;


    if(klen == DES3_KEY2_SIZE)
        des3_set2key_dec( &ctx, pkey );
    else if(klen == DES3_KEY3_SIZE)
        des3_set3key_dec( &ctx, pkey );

    des3_crypt_cbc( &ctx, 0, nlen, pivb, pdata, (pout));

    des3_free( &ctx );

    return 0;

}


//main????????
int des_test_self(void)
{
//	static const unsigned char des3_test_keys[24] =
//	{
//		0xba, 0x6b, 0x11, 0x74, 0xe6, 0x57, 0x4c, 0x91,
//		0xeb, 0xbe, 0x71, 0x10, 0x0e, 0xa8, 0x05, 0x50,
//		0xba, 0x6b, 0x11, 0x74, 0xe6, 0x57, 0x4c, 0x91
//	};
    unsigned char buff[16] = {0};
//    unsigned char data[1024] = {0x3F,0x12,0xE7,0xC0,0x2D,0x66,0x5A,0xB0,0xC4,0x2E,0x58,0xF1};
    unsigned char data1[100] = { 0x01, 0x5B, 0x5F, 0x2C, 0xD0, 0x00, 0x00, 0x07,0x01,0x00};
    unsigned char data[32] = { 0x00};
    int ret,len,i;

//  len = MyStrToHex("3F12E7C02D665AB0C42E58F1", data);//????8????
    len = strlen((char*)data);
    my_printf("raw data(%d):\r\n",len);
    len =10;
    for(i = 0; i < len; i++)
    {
        my_printf("%02X",data1[i]);
    }
    my_printf("\r\n");

//    unsigned char key[16] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0x00,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF};
    unsigned char key[16] = {0xba, 0x6b, 0x11, 0x74, 0xe6, 0x57, 0x4c, 0x91,
                             0xeb, 0xbe, 0x71, 0x10, 0x0e, 0xa8, 0x05, 0x50
                            };
    //DES ECB ????

    ret = des_ecb_encrypt(buff,data1,len,key);
    my_printf("DES ECB ENC(%d):\r\n",len);
    for(i = 0; i < ret; i++)
    {
        my_printf("%02X",buff[i]);
    }
    my_printf("\r\n");
    //DES ECB ????

    memset(data,0,sizeof(data));
    des_ecb_decrypt(data,buff,ret,key);
    my_printf("DES ECB DEC(%d):\r\n",ret);
    for(i = 0; i < ret; i++)
    {
        my_printf("%02X",data[i]);
    }
    my_printf("\r\n");

    uint32_t pout_len=0;
    dessmann_des_ecb_encrypt(data, &pout_len,data1, len, key);

//		my_printf("dessmann DES ECB ENC(%d):\r\n",len);
//    for(i = 0;i < ret;i++)
//    {
//        my_printf("%02X",buff[i]);
//    }
//    my_printf("\r\n");

    dessmann_des_ecb_decrypt(data,&pout_len,data, pout_len, key);
//		my_printf("dessmann DES ECB DEC(%d):\r\n",ret);
//    for(i = 0;i < ret;i++)
//    {
//        my_printf("%02X",data[i]);
//    }
//    my_printf("\r\n");


    /*
    //    //DES CBC ????
    //    my_printf("DES CBC ENC(%d):\r\n",len);
    //    memset(buff,0,sizeof(buff));
    //    des_cbc_encrypt(buff,data1,ret,key,NULL);
    //    for(i = 0;i < ret;i++)
    //    {
    //        my_printf("%02X",buff[i]);
    //    }
    //    my_printf("\r\n");

    //    //DES CBC ????
    //
    //    memset(data,0,sizeof(data));
    //    des_cbc_decrypt(data,buff,ret,key,NULL);
    //		 my_printf("DES CBC DEC(%d):\r\n",ret);
    //    for(i = 0;i < ret;i++)
    //    {
    //        my_printf("%02X",data[i]);
    //    }
    //    my_printf("\r\n");
    //    my_printf("\r\n");


    //    //3DES ECB ????
    //    my_printf("3DES ECB ENC:\r\n");
    //    ret = des3_ecb_encrypt(buff,data,len,key,16);
    //    for(i = 0;i < ret;i++)
    //    {
    //        my_printf("%02X",buff[i]);
    //    }
    //    my_printf("\r\n");
    //    //3DES ECB ????
    //    my_printf("3DES ECB DEC:\r\n");
    //    memset(data,0,sizeof(data));
    //    des3_ecb_decrypt(data,buff,ret,key,16);
    //    for(i = 0;i < ret;i++)
    //    {
    //        my_printf("%02X",data[i]);
    //    }
    //    my_printf("\r\n");

    //    //3DES CBC ????
    //    my_printf("3DES CBC ENC:\r\n");
    //    memset(buff,0,sizeof(buff));
    //    des3_cbc_encrypt(buff,data,ret,key,16,NULL);
    //    for(i = 0;i < ret;i++)
    //    {
    //        my_printf("%02X",buff[i]);
    //    }
    //    my_printf("\r\n");

    //    //3DES CBC ????
    //    my_printf("3DES CBC DEC:\r\n");
    //    memset(data,0,sizeof(data));
    //    des3_cbc_decrypt(data,buff,ret,key,16,NULL);
    //    for(i = 0;i < ret;i++)
    //    {
    //        my_printf("%02X",data[i]);
    //    }
    //    my_printf("\r\n");
    */


    return 0;
}
/*
 * DES-ECB  encryption API
 */


uint8_t dessmann_des_ecb_encrypt(unsigned char *pout,
                                 unsigned int *pout_len,
                                 unsigned char *pdata,
                                 unsigned int pdata_len,
                                 unsigned char *pkey)
{

    uint8_t key[8];
//		memcpy(key,pkey,8);//copy the raw key

    dessmann_des_key_make(pkey,key);//??????????????

    //DES ECB ????

    uint8_t ret = des_ecb_encrypt(pout,pdata,pdata_len,key);
    *pout_len=ret;

    my_printf("dessmann_des_ecb_encrypt(%d):\r\n",pdata_len);
    for(uint8_t i = 0; i < ret; i++)
    {
        if(i&&i%8==0)
        {
            my_printf(" ");
        }
        my_printf("%02X",pout[i]);
    }
    my_printf("\r\n");
    return 0;

}




/*
 * DES-ECB  decryption API
 */
uint8_t dessmann_des_ecb_decrypt(unsigned char *pout,
                                 unsigned int *pout_len,
                                 unsigned char *pdata,
                                 unsigned int pdata_len,
                                 unsigned char *pkey)
{
    uint8_t key[8];

    dessmann_des_key_make(pkey,key);//??????????????

    //DES ECB ????
    uint8_t ret=des_ecb_decrypt(pout,pdata,pdata_len,key);
    *pout_len=pdata_len;

    my_printf("dessmann_des_ecb_decrypt(%d):\r\n",pdata_len);
    for(uint8_t i = 0; i < pdata_len; i++)
    {
        if(i&&i%8==0)
        {
            my_printf(" ");
        }
        my_printf("%02X",pout[i]);
    }
    my_printf("\r\n");
    return ret;
}
/*
 * dessmann_des_key_make API
 *
 *[in]raw_key ????????
 *[out]key ????????????
 */
void dessmann_des_key_make(const uint8_t  *raw_key,uint8_t *key)
{
    uint8_t buffer[8]= {0};
    uint8_t make_key[8]= {0};
    //step1 :??mac????????????
    #if 0
    buffer[7] = (uint8_t)NRF_FICR->DEVICEADDR[1];
    buffer[6] = (uint8_t)(NRF_FICR->DEVICEADDR[1]>>8) | 0xC0;
    buffer[5] = (uint8_t)NRF_FICR->DEVICEADDR[0];
    buffer[4] = (uint8_t)(NRF_FICR->DEVICEADDR[0]>>8);
    buffer[3] = (uint8_t)(NRF_FICR->DEVICEADDR[0]>>16);
    buffer[2] = (uint8_t)(NRF_FICR->DEVICEADDR[0]>>24);
    buffer[1] = (uint8_t)NRF_FICR->DEVICEADDR[1];
    buffer[0] = (uint8_t)(NRF_FICR->DEVICEADDR[1]>>8) | 0xC0;
    #else
    PUBLIC_GetMacAdd(&buffer[0]);
    #endif

    for(uint8_t i=0; i<8; i++)
    {
        make_key[i]=raw_key[i]^buffer[i];
    }

    //step2 :????????????byte
    if((make_key[7]%2)==0)//????????2468????
    {
        for(uint8_t i=1; i<8; i=i+2)
        {
            make_key[i]=~make_key[i];
        }
    }
    else//?????? 1357????
    {
        for(uint8_t i=0; i<8; i=i+2)
        {
            make_key[i]=~make_key[i];
        }
    }
    //copy the key
    memcpy(key,make_key,8);

//		my_printf("dessmann_des_key_make(%d):\r\n",8);
//		for(uint8_t i = 0;i < 8;i++)
//		{
//			my_printf("%02X",make_key[i]);
//		}
//		my_printf("\r\n");
}




