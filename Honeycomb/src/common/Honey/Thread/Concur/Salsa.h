// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Math/Random/Salsa.h"
#include "Honey/Thread/Thread.h"

namespace honey { namespace concur
{

/// Thread-safe random variate generator.  Maintains a separate generator for each thread.
class Salsa : public RandomGen
{
public:
    typedef honey::Salsa::Key Key;
    typedef honey::Salsa::Iv Iv;
    typedef honey::Salsa::Seed Seed;
    typedef honey::Salsa::State State;

    void setSeed()                                              { _gen->setSeed(); }
    void setSeed(const Seed& seed)                              { _gen->setSeed(seed); }

    virtual uint64 next()                                       { return _gen->next(); }

    void setKey(const Key& key)                                 { _gen->setKey(key); }
    void setIv(const Iv& iv)                                    { _gen->setIv(iv); }

    void encrypt(const uint8* msg, uint8* cipher, int len)      { _gen->encrypt(msg,cipher,len); }
    void decrypt(const uint8* cipher, uint8* msg, int len)      { _gen->decrypt(cipher,msg,len); }

    void setState(const State& state)                           { _gen->setState(state); }
    virtual State& getState()                                   { return _gen->getState(); }

private:
    thread::Local<honey::Salsa> _gen;
};

} }
