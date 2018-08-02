/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sergei Gribanov, Mikhail Remnev                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// ECL
#include <ecl/dbobjects/ECLDspData.h>

// Framework
#include <framework/logging/Logger.h>

using namespace Belle2;

ECLDspData::ECLDspData(int boardNumber, const char* filename):
  m_boardNumber(boardNumber),
  m_fg31(49152),
  m_fg32(49152),
  m_fg33(49152),
  m_fg41(6144),
  m_fg43(6144),
  m_f(49152),
  m_f1(49152)
{
  FILE* fl;
  fl = fopen(filename, "rb");
  if (fl == NULL) {
    B2ERROR("Can't open file " << filename);
  }
  // Word size
  int nsiz = 2;
  // Size of fragment to read (in words)
  int nsiz1 = 256;
  short int id[256];
  int size = fread(id, nsiz, nsiz1, fl);
  if (size != nsiz1) {
    B2ERROR("Error reading data");
  }
  m_kb = id[13] >> 8;
  m_ka = id[13] - 256 * m_kb;
  m_y0Startr = id[14] >> 8;
  m_kc = id[14] - 256 * m_y0Startr;
  m_chiThresh = id[15];
  m_k2Chi = id[16] >> 8;
  m_k1Chi = id[16] - 256 * m_k2Chi;
  for (int i = 0; i < 16; ++i) {
    nsiz1 = 384;
    size = fread(&(*(m_fg41.begin() + i * nsiz1)), nsiz, nsiz1, fl);
    nsiz1 = 3072;
    size = fread(&(*(m_fg31.begin() + i * nsiz1)), nsiz, nsiz1, fl);
    size = fread(&(*(m_fg32.begin() + i * nsiz1)), nsiz, nsiz1, fl);
    size = fread(&(*(m_fg33.begin() + i * nsiz1)), nsiz, nsiz1, fl);
    nsiz1 = 384;
    size = fread(&(*(m_fg43.begin() + i * nsiz1)), nsiz, nsiz1, fl);
    nsiz1 = 3072;
    size = fread(&(*(m_f.begin() + i * nsiz1)), nsiz, nsiz1, fl);
    size = fread(&(*(m_f1.begin() + i * nsiz1)), nsiz, nsiz1, fl);
  }
  fclose(fl);
}

ECLDspData::~ECLDspData()
{
}

std::vector<short int> ECLDspData::getF31() const
{
  return m_fg31;
}

std::vector<short int> ECLDspData::getF32() const
{
  return m_fg32;
}

std::vector<short int>  ECLDspData::getF33() const
{
  return m_fg33;
}

std::vector<short int> ECLDspData::getF41() const
{
  return m_fg41;
}

std::vector<short int> ECLDspData::getF43() const
{
  return m_fg43;
}

std::vector<short int> ECLDspData::getF() const
{
  return m_f;
}

std::vector<short int> ECLDspData::getF1() const
{
  return m_f1;
}

short int ECLDspData::getlAT() const
{
  return m_lowAmpThresh;
}

short int ECLDspData::getsT() const
{
  return m_skipThresh;
}

short int ECLDspData::getthT() const
{
  return m_hitThresh;
}

short int ECLDspData::getk1() const
{
  return m_k1Chi;
}

short int ECLDspData::getk2() const
{
  return m_k2Chi;
}

short int ECLDspData::getka() const
{
  return m_ka;
}

short int ECLDspData::getkb() const
{
  return m_kb;
}

short int ECLDspData::getkc() const
{
  return m_kc;
}

short int ECLDspData::getky0s() const
{
  return m_y0Startr;
}

int ECLDspData::getBoardNumber() const
{
  return m_boardNumber;
}

void ECLDspData::write(const char* filename)
{
  // Default header for DSP file.
  // Words '0xABCD' are overwitten with current parameters.
  const unsigned short DEFAULT_HEADER[256] {
    // ECLDSP FILE.....
    0x4543, 0x4c44, 0x5350, 0x2046, 0x494c, 0x4500, 0x0000, 0x0000,
    0x0102, 0xffff, 0x0000, 0x0000, 0x0000, 0xABCD, 0xABCD, 0xABCD,
    0xABCD, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x9cff, 0x9cff, 0x9cff, 0x9cff, 0x9cff, 0x9cff, 0x9cff, 0x9cff,
    0x9cff, 0x9cff, 0x9cff, 0x9cff, 0x9cff, 0x9cff, 0x9cff, 0x9cff,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x9e00, 0x9e00, 0x9e00, 0x9e00, 0x9e00, 0x9e00, 0x9e00, 0x9e00,
    0x9e00, 0x9e00, 0x9e00, 0x9e00, 0x9e00, 0x9e00, 0x9e00, 0x9e00,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x6200, 0x6200, 0x6200, 0x6200, 0x6200, 0x6200, 0x6200, 0x6200,
    0x6200, 0x6200, 0x6200, 0x6200, 0x6200, 0x6200, 0x6200, 0x6200,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  };

  FILE* fl;
  fl = fopen(filename, "wb");
  // Word size
  int nsiz = 2;
  // Size of fragment to write (in words)
  int nsiz1 = 256;
  short header[256];

  for (int i = 0; i < 256; i++) {
    switch (i) {
      case 13:
        header[i] = (m_kb << 8) | m_ka;
        break;
      case 14:
        header[i] = (m_y0Startr << 8) | m_kc;
        break;
      case 15:
        header[i] = m_chiThresh;
        break;
      case 16:
        header[i] = (m_k2Chi << 8) | m_k1Chi;
        break;
      default:
        // Reverse bytes for header.
        int high = (DEFAULT_HEADER[i] & 0xFF00) >> 8;
        int low  = (DEFAULT_HEADER[i] & 0x00FF);
        header[i] = (low << 8) + high;
    }
  }

  int size = fwrite(header, nsiz, nsiz1, fl);
  if (size != nsiz1) {
    B2ERROR("Error reading data");
  }

  for (int i = 0; i < 16; ++i) {
    nsiz1 = 384;
    size = fwrite(&(*(m_fg41.begin() + i * nsiz1)), nsiz, nsiz1, fl);
    nsiz1 = 3072;
    size = fwrite(&(*(m_fg31.begin() + i * nsiz1)), nsiz, nsiz1, fl);
    size = fwrite(&(*(m_fg32.begin() + i * nsiz1)), nsiz, nsiz1, fl);
    size = fwrite(&(*(m_fg33.begin() + i * nsiz1)), nsiz, nsiz1, fl);
    nsiz1 = 384;
    size = fwrite(&(*(m_fg43.begin() + i * nsiz1)), nsiz, nsiz1, fl);
    nsiz1 = 3072;
    size = fwrite(&(*(m_f.begin() + i * nsiz1)), nsiz, nsiz1, fl);
    size = fwrite(&(*(m_f1.begin() + i * nsiz1)), nsiz, nsiz1, fl);
  }
  fclose(fl);
}

