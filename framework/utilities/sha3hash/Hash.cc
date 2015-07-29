/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/utilities/sha3hash/Hash.h>
#include <framework/logging/Logger.h>

extern "C" {
#include "KeccakHash.h"
}
/** Since we cannot forward declare the memory structure directly due to some
 * typedeffing we inherit from it */
struct KeccakHashInstance: public Keccak_HashInstance {};

namespace Belle2 {

  SHA3Hash::SHA3Hash(EHashMode mode): m_mode(mode)
  {
    m_instance = new KeccakHashInstance;
    clear();
  }

  SHA3Hash::~SHA3Hash()
  {
    delete m_instance;
  }

  void SHA3Hash::clear()
  {
    //Init the hash structure
    switch (m_mode) {
      case c_SHA3_224:
        Keccak_HashInitialize_SHA3_224((Keccak_HashInstance*)m_instance);
        break;
      case c_SHA3_256:
        Keccak_HashInitialize_SHA3_256((Keccak_HashInstance*)m_instance);
        break;
      case c_SHA3_384:
        Keccak_HashInitialize_SHA3_384((Keccak_HashInstance*)m_instance);
        break;
      case c_SHA3_512:
        Keccak_HashInitialize_SHA3_512((Keccak_HashInstance*)m_instance);
        break;
      default:
        B2FATAL("Unknown mode for SHA3Hash");
    }
  }

  void SHA3Hash::update(int n, unsigned char* buff)
  {
    Keccak_HashUpdate((Keccak_HashInstance*)m_instance, buff, n * 8);
  }

  void SHA3Hash::getHash(unsigned char* buff)
  {
    Keccak_HashFinal((Keccak_HashInstance*)m_instance, buff);
  }

  std::vector<unsigned char> SHA3Hash::getHash()
  {
    std::vector<unsigned char> buff(m_mode);
    getHash(buff.data());
    return buff;
  }

  ShakeHash::ShakeHash(EHashMode mode): m_mode(mode)
  {
    m_instance = new KeccakHashInstance;
    clear();
  }

  ShakeHash::~ShakeHash()
  {
    delete m_instance;
  }

  void ShakeHash::clear()
  {
    //Init the hash structure
    switch (m_mode) {
      case c_SHAKE128:
        Keccak_HashInitialize_SHAKE128((Keccak_HashInstance*)m_instance);
        break;
      case c_SHAKE256:
        Keccak_HashInitialize_SHAKE256((Keccak_HashInstance*)m_instance);
        break;
      default:
        B2FATAL("Unknown mode for ShakeHash");
    }
  }

  void ShakeHash::update(int n, unsigned char* buff)
  {
    Keccak_HashUpdate((Keccak_HashInstance*)m_instance, buff, n * 8);
  }

  void ShakeHash::getHash(int n, unsigned char* buff)
  {
    Keccak_HashSqueeze((Keccak_HashInstance*)m_instance, buff, n * 8);
  }

} //Belle2 namespace
