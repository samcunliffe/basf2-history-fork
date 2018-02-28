/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tzu-An Sheng                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCTRIGGERUNPACKERMODULE_H
#define CDCTRIGGERUNPACKERMODULE_H

#include <framework/core/Module.h>
#include <rawdata/dataobjects/RawTRG.h>
#include <framework/datastore/StoreArray.h>

#include <trg/cdc/dataobjects/Bitstream.h>
#include <trg/cdc/Unpacker.h>
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <trg/cdc/dataobjects/CDCTriggerFinderClone.h>

#include <array>
#include <bitset>
#include <vector>
#include <string>
#include <utility>
#include <typeinfo>

namespace Belle2 {

  using NodeList = std::vector<std::vector<int> >;

  static constexpr int mergerWidth = 256;
  static constexpr int nAllMergers = 146;
  /** width of a single word in the raw int buffer */
  static constexpr int wordWidth = 32;
  static constexpr int nFinesse = 4;
  using MergerBus = std::array<std::bitset<mergerWidth>, nAllMergers>;
  using MergerBits = Bitstream<MergerBus>;

  /* enum class SubTriggerType : unsigned char {Merger, TSF, T2D, T3D, Neuro, ETF}; */

  struct SubTrigger {
    SubTrigger(std::string inName,
               unsigned inEventWidth, unsigned inOffset,
               int inHeaderSize, std::vector<int> inNodeID,
               int& inDelay, int inDebugLevel = 0) :
      name(inName), eventWidth(inEventWidth), offset(inOffset),
      headerSize(inHeaderSize), iNode(inNodeID.front()),
      iFinesse(inNodeID.back()), delay(inDelay),
      debugLevel(inDebugLevel) {};

    std::string name;
    unsigned eventWidth;
    unsigned offset;
    int headerSize;
    int iNode;
    int iFinesse;

    /** information from Belle2Link header */
    std::string firmwareType;
    std::string firmwareVersion;
    /** reference to the Belle2Link delay member attribute */
    int& delay;

    int debugLevel;

    virtual void reserve(int, std::array<int, nFinesse>) {};
    virtual void unpack(int, std::array<int*, nFinesse>, std::array<int, nFinesse>) {};

    /** Get the B2L header information */
    virtual void getHeaders(int subDetectorId,
                            std::array<int*, 4> data32tab,
                            std::array<int, 4> nWords)
    {
      if (subDetectorId != iNode) {
        return;
      }
      if (nWords[iFinesse] < headerSize) {
        return;
      }
      /* get event header information
       * Ideally, these parameters should not change in the same run,
       * so it is more efficiency to do it in beginRun().
       * However, since they are present in all events,
       * let's check if they really remain unchanged.
       */
      if (headerSize >= 2) {
        firmwareType = CDCTriggerUnpacker::rawIntToAscii(data32tab.at(iFinesse)[0]);
        firmwareVersion = CDCTriggerUnpacker::rawIntToString(data32tab.at(iFinesse)[1]);
        // get the Belle2Link delay
        // TODO: what is the exact date that this word is introduced?
        if (headerSize >= 3 || firmwareVersion > "17121900") {
          std::bitset<wordWidth> thirdWord(data32tab.at(iFinesse)[2]);
          int newDelay = CDCTriggerUnpacker::subset<32, 12, 20>(thirdWord).to_ulong();
          if (delay > 0 && delay != newDelay) {
            B2WARNING(" the Belle2Link delay for " << name <<
                      "has changed from " << delay << " to " << newDelay << "!");
          }
          delay = newDelay;
        }
        B2DEBUG(50, name << ": " << firmwareType << ", version " <<
                firmwareVersion << ", node " << std::hex << iNode <<
                ", finesse " << iFinesse << ", delay: " << delay);
      }
    };

    virtual ~SubTrigger() {};
  };

  /**
   * Unpack the trigger data recorded in B2L
   *
   * There are 2 output formats:
   * 1. the Bitstream containing the bit content of each module
   * 2. decoded Basf2 object
   *
   */
  class CDCTriggerUnpackerModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    CDCTriggerUnpackerModule();

    /** Register input and output data */
    virtual void initialize();

    /** Delete dynamically allocated variables */
    virtual void terminate();

    /** convert raw data (in B2L buffer to bitstream) */
    virtual void event();

    /** data width of a single merger unit */
    /** number of merger unit in each super layers */
    static constexpr std::array<int, 9> nMergers = {10, 10, 12, 14, 16, 18, 20, 22, 24};

  private:
    /** number of words (number of bits / 32) of the B2L header */
    int m_headerSize;

    StoreArray<RawTRG> m_rawTriggers; /**< array containing the raw trigger data object */

    NodeList m_mergerNodeID; /**< list of (COPPER ID, FTSW ID) of Merger reader (TSF) */
    bool m_unpackMerger;  /**< flag to unpack merger data (recorded by Merger Reader / TSF) */
    MergerBits m_mergerBitsPerClock;
    StoreArray<MergerBits> m_mergerBits; /**< merger output bitstream */

    bool m_decodeTSHit;  /**< flag to decode track segment  */
    NodeList m_tracker2DNodeID; /**< list of (COPPER ID, FTSW ID) of 2D tracker */
    bool m_unpackTracker2D;  /**< flag to unpack 2D tracker data */
    bool m_decode2DFinderTrack;  /**< flag to decode 2D finder track  */
    bool m_decode2DFinderInputTS;  /**< flag to decode 2D finder input TS */

    /** bitstream of TSF output to 2D tracker */
    StoreArray<CDCTriggerUnpacker::TSFOutputBitStream> m_bitsTo2D;

    /** decoded track segment hit */
    StoreArray<CDCTriggerSegmentHit> m_TSHits;

    /** bitstream of 2D output to 3D/Neuro */
    StoreArray<CDCTriggerUnpacker::T2DOutputBitStream> m_bits2DTo3D;

    /** decoded 2D finder track */
    StoreArray<CDCTriggerTrack> m_2DFinderTracks;

    /** additional information of the 2D finder track */
    StoreArray<CDCTriggerFinderClone> m_2DFinderClones;

    /** debug level specified in the steering file */
    int m_debugLevel;

    /** Belle2Link delay of the merger reader */
    int m_mergerDelay = 0;

    /** Belle2Link delay of the 2D finder */
    int m_2DFinderDelay = 0;

    std::vector<SubTrigger*> m_subTrigger;

    void unpack(SubTrigger& subTrigger, int subDetectorId,
                std::array<int*, nFinesse> data32tab,
                std::array<int, nFinesse> nWords);

  };

}

#endif /* CDCTRIGGERUNPACKERMODULE_H */
