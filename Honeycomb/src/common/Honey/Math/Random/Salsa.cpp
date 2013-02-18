// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma hdrstop

#include "Honey/Math/Random/Salsa.h"
#include "Honey/String/Hash.h"
#include "Honey/Misc/BitOp.h"
#include "Honey/Thread/Atomic.h"

namespace honey
{

#define ROTATE(v,c) (BitOp::rotLeft(v,c))
#define XOR(v,w)    ((v) ^ (w))
#define PLUS(v,w)   ((v) + (w))
#define PLUSONE(v)  (PLUS((v),1))

const uint8 Salsa::_sigma[] = "expand 32-byte k";

void Salsa::setSeed()
{
    //Fill seed with device entropy
    Seed seed;

    //Clock for fast changing but low period
    seed.a[0] = static_cast<uint32>(clock());
    //Time for high period but slow changing
    seed.a[1] = static_cast<uint32>(time(nullptr));
    //Process id
    seed.a[2] = static_cast<uint32>(_getpid());
    //Value of static counter
    static atomic::Var<int> counter = 0;
    seed.a[3] = counter++;
    //Address of static counter
    seed.a[4] = reinterpret_cast<uint32>(&counter);
    //Stack address
    seed.a[5] = reinterpret_cast<uint32>(&seed);
    //Value of unitialized stack data
    seed.a[6] = seed.a[6];

    //Dynamically allocated address
    uint32* ptr = new uint32;
    seed.a[7] = reinterpret_cast<uint32>(ptr);
    //Value of uninitialized heap data
    seed.a[8] = *ptr;
    delete_(ptr);

    setSeed(scrambleSeed(seed));
}

#define SCRAMBLE(a)                             \
    {                                           \
       a[0]^=a[1]<<11; a[3]+=a[0]; a[1]+=a[2];  \
       a[1]^=a[2]>>2;  a[4]+=a[1]; a[2]+=a[3];  \
       a[2]^=a[3]<<8;  a[5]+=a[2]; a[3]+=a[4];  \
       a[3]^=a[4]>>16; a[6]+=a[3]; a[4]+=a[5];  \
       a[4]^=a[5]<<10; a[7]+=a[4]; a[5]+=a[6];  \
       a[5]^=a[6]>>4;  a[8]+=a[5]; a[6]+=a[7];  \
       a[6]^=a[7]<<8;  a[9]+=a[6]; a[7]+=a[8];  \
       a[7]^=a[8]>>9;  a[0]+=a[7]; a[8]+=a[9];  \
       a[8]^=a[9]<<7;  a[1]+=a[8]; a[9]+=a[0];  \
       a[9]^=a[0]>>5;  a[2]+=a[9]; a[0]+=a[1];  \
    }

Salsa::Seed Salsa::scrambleSeed(const Seed& seed)
{
    Seed res;

    //Golden ratio: (sqrt(5/4) - 0.5) * 2^32
    for (int i = 0; i < 10; ++i)
        res.a[i] = 0x9e3779b9;

    //Scramble the ratio
    for (int i = 0; i < 4; ++i)
        SCRAMBLE(res.a);             

    //Scramble the seed
    for (int i = 0; i < 10; ++i)
        res.a[i] += seed.a[i];
    SCRAMBLE(res.a);

    //Further scramble the seed with itself
    for (int i = 0; i < 10; ++i)
        res.a[i] += res.a[i];
    SCRAMBLE(res.a);

    return res;
}

void Salsa::setSeed(const Seed& seed)
{
    //Key
    _state.a[1] = seed.a[0];
    _state.a[2] = seed.a[1];
    _state.a[3] = seed.a[2];
    _state.a[4] = seed.a[3];
    _state.a[11] = seed.a[4];
    _state.a[12] = seed.a[5];
    _state.a[13] = seed.a[6];
    _state.a[14] = seed.a[7];
    //Constants
    _state.a[0] = BitOp::fromPartsLittle<uint32>(_sigma + 0);
    _state.a[5] = BitOp::fromPartsLittle<uint32>(_sigma + 4);
    _state.a[10] = BitOp::fromPartsLittle<uint32>(_sigma + 8);
    _state.a[15] = BitOp::fromPartsLittle<uint32>(_sigma + 12);
    //IV
    _state.a[6] = seed.a[8];
    _state.a[7] = seed.a[9];
    //Block counter
    _state.a[8] = 0;
    _state.a[9] = 0;

    //Advance a number of iterations to remove any random bias
    for (int i = 0; i < 10; ++i)
        step();
}

void Salsa::step()
{
    uint32 x0  = _state.a[0];
    uint32 x1  = _state.a[1];
    uint32 x2  = _state.a[2];
    uint32 x3  = _state.a[3];
    uint32 x4  = _state.a[4];
    uint32 x5  = _state.a[5];
    uint32 x6  = _state.a[6];
    uint32 x7  = _state.a[7];
    uint32 x8  = _state.a[8];
    uint32 x9  = _state.a[9];
    uint32 x10 = _state.a[10];
    uint32 x11 = _state.a[11];
    uint32 x12 = _state.a[12];
    uint32 x13 = _state.a[13];
    uint32 x14 = _state.a[14];
    uint32 x15 = _state.a[15];

    for (int i = 8; i > 0; i -= 2)
    {
        x4  = XOR( x4,ROTATE(PLUS( x0,x12), 7));
        x8  = XOR( x8,ROTATE(PLUS( x4, x0), 9));
        x12 = XOR(x12,ROTATE(PLUS( x8, x4),13));
        x0  = XOR( x0,ROTATE(PLUS(x12, x8),18));
        x9  = XOR( x9,ROTATE(PLUS( x5, x1), 7));
        x13 = XOR(x13,ROTATE(PLUS( x9, x5), 9));
        x1  = XOR( x1,ROTATE(PLUS(x13, x9),13));
        x5  = XOR( x5,ROTATE(PLUS( x1,x13),18));
        x14 = XOR(x14,ROTATE(PLUS(x10, x6), 7));
        x2  = XOR( x2,ROTATE(PLUS(x14,x10), 9));
        x6  = XOR( x6,ROTATE(PLUS( x2,x14),13));
        x10 = XOR(x10,ROTATE(PLUS( x6, x2),18));
        x3  = XOR( x3,ROTATE(PLUS(x15,x11), 7));
        x7  = XOR( x7,ROTATE(PLUS( x3,x15), 9));
        x11 = XOR(x11,ROTATE(PLUS( x7, x3),13));
        x15 = XOR(x15,ROTATE(PLUS(x11, x7),18));
        x1  = XOR( x1,ROTATE(PLUS( x0, x3), 7));
        x2  = XOR( x2,ROTATE(PLUS( x1, x0), 9));
        x3  = XOR( x3,ROTATE(PLUS( x2, x1),13));
        x0  = XOR( x0,ROTATE(PLUS( x3, x2),18));
        x6  = XOR( x6,ROTATE(PLUS( x5, x4), 7));
        x7  = XOR( x7,ROTATE(PLUS( x6, x5), 9));
        x4  = XOR( x4,ROTATE(PLUS( x7, x6),13));
        x5  = XOR( x5,ROTATE(PLUS( x4, x7),18));
        x11 = XOR(x11,ROTATE(PLUS(x10, x9), 7));
        x8  = XOR( x8,ROTATE(PLUS(x11,x10), 9));
        x9  = XOR( x9,ROTATE(PLUS( x8,x11),13));
        x10 = XOR(x10,ROTATE(PLUS( x9, x8),18));
        x12 = XOR(x12,ROTATE(PLUS(x15,x14), 7));
        x13 = XOR(x13,ROTATE(PLUS(x12,x15), 9));
        x14 = XOR(x14,ROTATE(PLUS(x13,x12),13));
        x15 = XOR(x15,ROTATE(PLUS(x14,x13),18));
    }

    _state.res[ 0] = PLUS(x0 ,_state.a[ 0]);
    _state.res[ 1] = PLUS(x1 ,_state.a[ 1]);
    _state.res[ 2] = PLUS(x2 ,_state.a[ 2]);
    _state.res[ 3] = PLUS(x3 ,_state.a[ 3]);
    _state.res[ 4] = PLUS(x4 ,_state.a[ 4]);
    _state.res[ 5] = PLUS(x5 ,_state.a[ 5]);
    _state.res[ 6] = PLUS(x6 ,_state.a[ 6]);
    _state.res[ 7] = PLUS(x7 ,_state.a[ 7]);
    _state.res[ 8] = PLUS(x8 ,_state.a[ 8]);
    _state.res[ 9] = PLUS(x9 ,_state.a[ 9]);
    _state.res[10] = PLUS(x10,_state.a[10]);
    _state.res[11] = PLUS(x11,_state.a[11]);
    _state.res[12] = PLUS(x12,_state.a[12]);
    _state.res[13] = PLUS(x13,_state.a[13]);
    _state.res[14] = PLUS(x14,_state.a[14]);
    _state.res[15] = PLUS(x15,_state.a[15]);

    //Increase block counter. Stopping at 2^70 bytes per IV is user's responsibility
    _state.a[8] = PLUSONE(_state.a[8]);
    if (!_state.a[8])
        _state.a[9] = PLUSONE(_state.a[9]);

    //Reset index of next integer
    _state.resIdx = 0;
}


uint64 Salsa::next()
{
    //Every generator step produces coreSize count of random 32-bit integers
    if (_state.resIdx >= _state.coreSize)
        step();

    uint64 res = BitOp::fromPartsLittle(_state.res + _state.resIdx);
    _state.resIdx += 2;
    return res;
}

void Salsa::setKey(const Key& key)
{
    //Key
    _state.a[1] = key.a[0];
    _state.a[2] = key.a[1];
    _state.a[3] = key.a[2];
    _state.a[4] = key.a[3];
    _state.a[11] = key.a[4];
    _state.a[12] = key.a[5];
    _state.a[13] = key.a[6];
    _state.a[14] = key.a[7];
    //Constants
    _state.a[0] = BitOp::fromPartsLittle<uint32>(_sigma + 0);
    _state.a[5] = BitOp::fromPartsLittle<uint32>(_sigma + 4);
    _state.a[10] = BitOp::fromPartsLittle<uint32>(_sigma + 8);
    _state.a[15] = BitOp::fromPartsLittle<uint32>(_sigma + 12);
}

void Salsa::setIv(const Iv& iv)
{
    //IV
    _state.a[6] = iv.a[0];
    _state.a[7] = iv.a[1];
    //Block counter
    _state.a[8] = 0;
    _state.a[9] = 0;
}

void Salsa::encrypt(const uint8* m, uint8* c, int bytes)
{
    uint32 x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15;
    uint32 j0, j1, j2, j3, j4, j5, j6, j7, j8, j9, j10, j11, j12, j13, j14, j15;
    uint8* ctarget = 0;
    uint8 tmp[64];

    if (bytes <= 0) return;

    j0 = _state.a[0];
    j1 = _state.a[1];
    j2 = _state.a[2];
    j3 = _state.a[3];
    j4 = _state.a[4];
    j5 = _state.a[5];
    j6 = _state.a[6];
    j7 = _state.a[7];
    j8 = _state.a[8];
    j9 = _state.a[9];
    j10 = _state.a[10];
    j11 = _state.a[11];
    j12 = _state.a[12];
    j13 = _state.a[13];
    j14 = _state.a[14];
    j15 = _state.a[15];

    for (;;)
    {
        if (bytes < 64)
        {
            for (int i = 0; i < bytes; ++i)
                tmp[i] = m[i];
            m = tmp;
            ctarget = c;
            c = tmp;
        }

        x0 = j0;
        x1 = j1;
        x2 = j2;
        x3 = j3;
        x4 = j4;
        x5 = j5;
        x6 = j6;
        x7 = j7;
        x8 = j8;
        x9 = j9;
        x10 = j10;
        x11 = j11;
        x12 = j12;
        x13 = j13;
        x14 = j14;
        x15 = j15;

        for (int i = 8; i > 0; i -= 2)
        {
             x4 = XOR( x4,ROTATE(PLUS( x0,x12), 7));
             x8 = XOR( x8,ROTATE(PLUS( x4, x0), 9));
            x12 = XOR(x12,ROTATE(PLUS( x8, x4),13));
             x0 = XOR( x0,ROTATE(PLUS(x12, x8),18));
             x9 = XOR( x9,ROTATE(PLUS( x5, x1), 7));
            x13 = XOR(x13,ROTATE(PLUS( x9, x5), 9));
             x1 = XOR( x1,ROTATE(PLUS(x13, x9),13));
             x5 = XOR( x5,ROTATE(PLUS( x1,x13),18));
            x14 = XOR(x14,ROTATE(PLUS(x10, x6), 7));
             x2 = XOR( x2,ROTATE(PLUS(x14,x10), 9));
             x6 = XOR( x6,ROTATE(PLUS( x2,x14),13));
            x10 = XOR(x10,ROTATE(PLUS( x6, x2),18));
             x3 = XOR( x3,ROTATE(PLUS(x15,x11), 7));
             x7 = XOR( x7,ROTATE(PLUS( x3,x15), 9));
            x11 = XOR(x11,ROTATE(PLUS( x7, x3),13));
            x15 = XOR(x15,ROTATE(PLUS(x11, x7),18));
             x1 = XOR( x1,ROTATE(PLUS( x0, x3), 7));
             x2 = XOR( x2,ROTATE(PLUS( x1, x0), 9));
             x3 = XOR( x3,ROTATE(PLUS( x2, x1),13));
             x0 = XOR( x0,ROTATE(PLUS( x3, x2),18));
             x6 = XOR( x6,ROTATE(PLUS( x5, x4), 7));
             x7 = XOR( x7,ROTATE(PLUS( x6, x5), 9));
             x4 = XOR( x4,ROTATE(PLUS( x7, x6),13));
             x5 = XOR( x5,ROTATE(PLUS( x4, x7),18));
            x11 = XOR(x11,ROTATE(PLUS(x10, x9), 7));
             x8 = XOR( x8,ROTATE(PLUS(x11,x10), 9));
             x9 = XOR( x9,ROTATE(PLUS( x8,x11),13));
            x10 = XOR(x10,ROTATE(PLUS( x9, x8),18));
            x12 = XOR(x12,ROTATE(PLUS(x15,x14), 7));
            x13 = XOR(x13,ROTATE(PLUS(x12,x15), 9));
            x14 = XOR(x14,ROTATE(PLUS(x13,x12),13));
            x15 = XOR(x15,ROTATE(PLUS(x14,x13),18));
        }

         x0 = PLUS( x0,j0);
         x1 = PLUS( x1,j1);
         x2 = PLUS( x2,j2);
         x3 = PLUS( x3,j3);
         x4 = PLUS( x4,j4);
         x5 = PLUS( x5,j5);
         x6 = PLUS( x6,j6);
         x7 = PLUS( x7,j7);
         x8 = PLUS( x8,j8);
         x9 = PLUS( x9,j9);
        x10 = PLUS(x10,j10);
        x11 = PLUS(x11,j11);
        x12 = PLUS(x12,j12);
        x13 = PLUS(x13,j13);
        x14 = PLUS(x14,j14);
        x15 = PLUS(x15,j15);

         x0 = XOR( x0,BitOp::fromPartsLittle<uint32>(m + 0));
         x1 = XOR( x1,BitOp::fromPartsLittle<uint32>(m + 4));
         x2 = XOR( x2,BitOp::fromPartsLittle<uint32>(m + 8));
         x3 = XOR( x3,BitOp::fromPartsLittle<uint32>(m + 12));
         x4 = XOR( x4,BitOp::fromPartsLittle<uint32>(m + 16));
         x5 = XOR( x5,BitOp::fromPartsLittle<uint32>(m + 20));
         x6 = XOR( x6,BitOp::fromPartsLittle<uint32>(m + 24));
         x7 = XOR( x7,BitOp::fromPartsLittle<uint32>(m + 28));
         x8 = XOR( x8,BitOp::fromPartsLittle<uint32>(m + 32));
         x9 = XOR( x9,BitOp::fromPartsLittle<uint32>(m + 36));
        x10 = XOR(x10,BitOp::fromPartsLittle<uint32>(m + 40));
        x11 = XOR(x11,BitOp::fromPartsLittle<uint32>(m + 44));
        x12 = XOR(x12,BitOp::fromPartsLittle<uint32>(m + 48));
        x13 = XOR(x13,BitOp::fromPartsLittle<uint32>(m + 52));
        x14 = XOR(x14,BitOp::fromPartsLittle<uint32>(m + 56));
        x15 = XOR(x15,BitOp::fromPartsLittle<uint32>(m + 60));

        //Increase block counter. Stopping at 2^70 bytes per IV is user's responsibility
        j8 = PLUSONE(j8);
        if (!j8)
          j9 = PLUSONE(j9);

        BitOp::toPartsLittle( x0,c + 0);
        BitOp::toPartsLittle( x1,c + 4);
        BitOp::toPartsLittle( x2,c + 8);
        BitOp::toPartsLittle( x3,c + 12);
        BitOp::toPartsLittle( x4,c + 16);
        BitOp::toPartsLittle( x5,c + 20);
        BitOp::toPartsLittle( x6,c + 24);
        BitOp::toPartsLittle( x7,c + 28);
        BitOp::toPartsLittle( x8,c + 32);
        BitOp::toPartsLittle( x9,c + 36);
        BitOp::toPartsLittle(x10,c + 40);
        BitOp::toPartsLittle(x11,c + 44);
        BitOp::toPartsLittle(x12,c + 48);
        BitOp::toPartsLittle(x13,c + 52);
        BitOp::toPartsLittle(x14,c + 56);
        BitOp::toPartsLittle(x15,c + 60);

        if (bytes <= 64)
        {
            if (bytes < 64)
                for (int i = 0; i < bytes; ++i)
                    ctarget[i] = c[i];
            _state.a[8] = j8;
            _state.a[9] = j9;
            break;
        }
        bytes -= 64;
        c += 64;
        m += 64;
    }

    //Reset index, state is dirty. A call to get a random number from next() will require a new step
    _state.resIdx = _state.coreSize;
}

void Salsa::decrypt(const uint8* cipher, uint8* msg, int len)
{
    encrypt(cipher, msg, len);
}

}
