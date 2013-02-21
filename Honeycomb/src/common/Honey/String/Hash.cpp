// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma hdrstop

#include "Honey/String/Hash.h"
#include "Honey/String/Stream.h"
#include "Honey/Misc/BitOp.h"

namespace honey
{

/// MurmurHash3_x86_32
template<int EndianId>
struct Murmur
{
    static uint32 fMix(uint32 h)
    {
        h ^= h >> 16;
        h *= 0x85ebca6b;
        h ^= h >> 13;
        h *= 0xc2b2ae35;
        h ^= h >> 16;
        return h;
    }

    // Block read - if your platform needs to do endian-swapping or can only
    // handle aligned reads, do the conversion here
    static uint32 block(const uint32* p, size_t i)      { return p[i]; }


    static uint32 hash(const void* key, size_t len, uint32 seed)
    {
        const uint8 * data = (const uint8*)key;
        const int nblocks = len / 4;

        uint32 h1 = seed;

        uint32 c1 = 0xcc9e2d51;
        uint32 c2 = 0x1b873593;

        //----------
        // body

        const uint32 * blocks = (const uint32 *)(data + nblocks*4);

        for(int i = -nblocks; i; i++)
        {
            uint32 k1 = block(blocks,i);

            k1 *= c1;
            k1 = BitOp::rotLeft(k1,15);
            k1 *= c2;
    
            h1 ^= k1;
            h1 = BitOp::rotLeft(h1,13); 
            h1 = h1*5+0xe6546b64;
        }

        //----------
        // tail

        const uint8 * tail = (const uint8*)(data + nblocks*4);

        uint32 k1 = 0;

        switch(len & 3)
        {
        case 3: k1 ^= tail[2] << 16;
        case 2: k1 ^= tail[1] << 8;
        case 1: k1 ^= tail[0];
                k1 *= c1; k1 = BitOp::rotLeft(k1,15); k1 *= c2; h1 ^= k1;
        };

        //----------
        // finalization

        h1 ^= len;

        h1 = fMix(h1);

        return h1;
    }
};

template<>
inline uint32 Murmur<ENDIAN_BIG>::block(const uint32* p, size_t i)
{
    return BitOp::swap(p[i]);
}


int Hash::fast(const uint8* data, int len, int seed)
{
    return Murmur<ENDIAN>::hash(data, len, seed);
}

int Hash::fast(const String& str, int seed)
{
    return fast(reinterpret_cast<const uint8*>(str.data()), str.length()*sizeof(Char), seed);
}

String Hash::toString(int hash)
{
    uint8 a[4];
    BitOp::toPartsBig(static_cast<uint32>(hash), a);
    StringStream os;
    for (int i = 0; i < 4; ++i)
        os << std::setw(2) << std::setfill(L'0') << std::hex << static_cast<uint32>(a[i]);
    return os;
}


/// Data is read and operated on a per-byte basis, so it works for any endian
struct CubeHash
{
    typedef uint64 DataLength;
    typedef enum { SUCCESS = 0, FAIL = 1, BAD_HASHBITLEN = 2 } HashReturn;

    typedef struct {
        int hashbitlen;
        int pos; /* number of bits read into x from current block */
        uint32 x[32];
    } hashState;

    static const int CUBEHASH_ROUNDS = 16;
    static const int CUBEHASH_BLOCKBYTES = 32;

    static void transform(hashState *state)
    {
        int i;
        int r;
        uint32 y[16];

        for (r = 0;r < CUBEHASH_ROUNDS;++r) {
            for (i = 0;i < 16;++i) state->x[i + 16] += state->x[i];
            for (i = 0;i < 16;++i) y[i ^ 8] = state->x[i];
            for (i = 0;i < 16;++i) state->x[i] = BitOp::rotLeft(y[i],7);
            for (i = 0;i < 16;++i) state->x[i] ^= state->x[i + 16];
            for (i = 0;i < 16;++i) y[i ^ 2] = state->x[i + 16];
            for (i = 0;i < 16;++i) state->x[i + 16] = y[i];
            for (i = 0;i < 16;++i) state->x[i + 16] += state->x[i];
            for (i = 0;i < 16;++i) y[i ^ 4] = state->x[i];
            for (i = 0;i < 16;++i) state->x[i] = BitOp::rotLeft(y[i],11);
            for (i = 0;i < 16;++i) state->x[i] ^= state->x[i + 16];
            for (i = 0;i < 16;++i) y[i ^ 1] = state->x[i + 16];
            for (i = 0;i < 16;++i) state->x[i + 16] = y[i];
        }
    }

    static HashReturn init(hashState *state, int hashbitlen, int seed)
    {
        if (hashbitlen < 8) return BAD_HASHBITLEN;
        if (hashbitlen > 512) return BAD_HASHBITLEN;
        if (hashbitlen != 8 * (hashbitlen / 8)) return BAD_HASHBITLEN;

        state->hashbitlen = hashbitlen;
        memset(state->x, 0, sizeof(state->x));
        state->x[0] = hashbitlen / 8;
        state->x[1] = CUBEHASH_BLOCKBYTES;
        state->x[2] = CUBEHASH_ROUNDS;
        state->x[3] = seed;
        transform(state);
        state->pos = 0;
        return SUCCESS;
    }

    static HashReturn update(hashState *state, const uint8 *data, DataLength databitlen)
    {
        /* caller promises us that previous data had integral number of bytes */
        /* so state->pos is a multiple of 8 */

        while (databitlen >= 8) {
            uint32 u = *data;
            u <<= 8 * ((state->pos / 8) % 4);
            state->x[state->pos / 32] ^= u;
            data += 1;
            databitlen -= 8;
            state->pos += 8;
            if (state->pos == 8 * CUBEHASH_BLOCKBYTES) {
                transform(state);
                state->pos = 0;
            }
        }
        if (databitlen > 0) {
            uint32 u = *data;
            u <<= 8 * ((state->pos / 8) % 4);
            state->x[state->pos / 32] ^= u;
            state->pos += databitlen;
        }
        return SUCCESS;
    }

    static HashReturn final(hashState *state, uint8 *hashval)
    {
        int i;
        uint32 u;

        u = (128 >> (state->pos % 8));
        u <<= 8 * ((state->pos / 8) % 4);
        state->x[state->pos / 32] ^= u;
        transform(state);
        state->x[31] ^= 1;
        transform(state);
        transform(state);
        for (i = 0;i < state->hashbitlen / 8;++i) hashval[i] = state->x[i / 4] >> (8 * (i % 4));

        return SUCCESS;
    }

    static HashReturn hash( int hashbitlen, const uint8 *data,
                            DataLength databitlen, uint8 *hashval, int seed)
    {
        hashState state;
        if (init(&state,hashbitlen,seed) != SUCCESS) return BAD_HASHBITLEN;
        update(&state,data,databitlen);
        return final(&state,hashval);
    }
};

/** \cond */
StringStream& operator<<(StringStream& os, const Hash::SecureData& val)
{
    for (int i = 0; i < val.size; ++i)
        os << std::setw(2) << std::setfill(L'0') << std::hex << static_cast<uint32>(val.a[i]);
    return os;
}
/** \endcond */

Hash::SecureData Hash::secure(const uint8* data, int len, int seed)
{
    SecureData res;
    CubeHash::hash(res.size*8, data, len*8, res.a, seed);
    return res;
}

Hash::SecureData Hash::secure(const String& str, int seed)
{
    return secure(reinterpret_cast<const uint8*>(str.data()), str.length()*sizeof(Char), seed);
}

}
