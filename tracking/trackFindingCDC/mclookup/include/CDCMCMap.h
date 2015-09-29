/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <boost/bimap/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>
#include <boost/range/iterator_range.hpp>

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <mdst/dataobjects/MCParticle.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    ///Class to organize and present the monte carlo hit information
    class CDCMCMap {

    public:

      /// Type for a one to one relation form CDCHits to CDCSimHits
      typedef
      boost::bimaps::bimap <
      boost::bimaps::set_of< boost::bimaps::tagged<const CDCHit*, CDCHit> >,
            boost::bimaps::set_of< boost::bimaps::tagged<const CDCSimHit*, CDCSimHit> >
            > CDCSimHitByCDCHitMap;

    public:
      /// Type which is returned by MCParticleByCDCHitMap the in a range based for loop
      typedef CDCSimHitByCDCHitMap::value_type CDCSimHitByCDCHitRelation;


    public:
      /// Type for a one to n relation form CDCHits to MCParticles
      typedef
      boost::bimaps::bimap <
      boost::bimaps::set_of< boost::bimaps::tagged<const CDCHit*, CDCHit> >,
            boost::bimaps::multiset_of< boost::bimaps::tagged<const MCParticle*, MCParticle> >
            > MCParticleByCDCHitMap;

    private:
      /// Iterator type for an iteration over several CDCHit <-> MCParticle relations as view from the MCParticle side.
      typedef MCParticleByCDCHitMap::map_by<MCParticle>::const_iterator CDCHitByMCParticleRelationIterator;

    public:
      /// Range type for an iteration over several CDCHit <-> MCParticle relations as view from the MCParticle side.
      typedef boost::iterator_range<CDCHitByMCParticleRelationIterator>  CDCHitByMCParticleRelationRange;

      /// Type which is returned by CDCHitByMCParticleRelationRange the in a range based for loop
      typedef CDCHitByMCParticleRelationIterator::value_type CDCHitByMCParticleRelation;



      /// Type which is returned by MCParticleByCDCHitMap the in a range based for loop
      typedef MCParticleByCDCHitMap::value_type MCParticleByCDCHitRelation;




      /// Type for a one to n relation form CDCSimHits to MCParticles
      typedef
      boost::bimaps::bimap <
      boost::bimaps::set_of< boost::bimaps::tagged<const CDCSimHit*, CDCSimHit> >,
            boost::bimaps::multiset_of< boost::bimaps::tagged<const MCParticle*, MCParticle> >
            > MCParticleByCDCSimHitMap;

    public:
      /// Iterator type for an iteration over several CDCSimHit <-> MCParticle relations as view from the MCParticle side.
      typedef MCParticleByCDCSimHitMap::map_by<MCParticle>::const_iterator CDCSimHitByMCParticleRelationIterator;

    public:
      /// Range type for an iteration over several CDCSimHit <-> MCParticle relations as view from the MCParticle side.
      typedef boost::iterator_range<CDCSimHitByMCParticleRelationIterator>  CDCSimHitByMCParticleRelationRange;

      /// Type which is returned by CDCSimHitByMCParticleRelationRange the in a range based for loop
      typedef CDCSimHitByMCParticleRelationIterator::value_type CDCSimHitByMCParticleRelation;



      /// Type which is returned by MCParticleByCDCSimHitMap the in a range based for loop
      typedef MCParticleByCDCSimHitMap::value_type MCParticleByCDCSimHitRelation;



    public:
      /// Empty constructor
      CDCMCMap();


      ~CDCMCMap();

      /// Delete the assignement operator in order to avoid accidental copies.
      CDCMCMap& operator=(const CDCMCMap&) = delete;

    public:

      /// Clear all information from the former event.
      void clear();

      /// Fill the Monte Carlo information retrieved from the DataStore into the local bimaps.
      void fill();

    private:
      /// Indicate if the given weight suggests that the corresponding hit to MCParticle relation has been redirected to point to a primary particle instead of the discarded secondary particle.
      static bool indicatesReassignedSecondary(double weight)
      { return weight <= 0; }

      /// Retrieve the relations array from CDCSimHits to CDCHits and fill it in to the local map which does the inverse mapping
      void fillSimHitByHitMap();

      /// Retrieve the relations array from MCParticle to CDCHits and fill it in to the local map which does the inverse mapping
      void fillMCParticleByHitMap();

      /// Retrieve the relations array from MCParticle to CDCSimHits and fill it in to the local map which does the inverse mapping
      void fillMCParticleBySimHitMap();

      /// Checks if the relations CDCHit -> MCParticle and CDCHit -> CDCSimHit -> MCParticle commute.
      void validateRelations() const;

      /// Checks if each CDCHit is marked as reassigned secondary is related to a reassigned secondary CDCSimHit.
      void validateReassignedSecondaries() const;

    public:



      /// Seeks the CDCSimHit related to the CDCHit.
      const Belle2::CDCSimHit* getSimHit(const CDCHit* hit) const
      { return hit ? m_simHitByHit.by<CDCHit>().at(hit) : nullptr; }

      /// Seeks the CDCHit related to the CDCSimHit - nullptr if no CDCHit is related.
      const Belle2::CDCHit* getHit(const CDCSimHit* simHit) const
      {
        auto itFound = m_simHitByHit.by<CDCSimHit>().find(simHit);
        return itFound != m_simHitByHit.by<CDCSimHit>().end() ? itFound->get<CDCHit>() : nullptr;
      }
      //{ return hit.getRelated<CDCSimHit>(); }




      /// Indicates if the CDCSimHit is considered background.
      bool isBackground(const CDCSimHit* simHit) const
      { return simHit ? simHit->getBackgroundTag() != CDCSimHit::bg_none : false; }

      /// Indicates if the CDCSimHit is considered background.
      bool isBackground(const CDCHit* hit) const
      { return isBackground(getSimHit(hit)); }



      /// Seeks the MCParticle related to the CDCHit - nullptr if no MCParticle is related, this is also the case for background hits.
      const Belle2::MCParticle* getMCParticle(const CDCHit* hit) const
      {
        auto itFound = m_mcParticlesByHit.by<CDCHit>().find(hit);
        return itFound != m_mcParticlesByHit.by<CDCHit>().end() ? itFound->get<MCParticle>() : nullptr;
      }
      //{ return hit ? hit->getRelated<MCParticle>() : nullptr; }

      /// Seeks the MCParticle related to the CDCSimHit - nullptr if no MCParticle is related, this is also the case for background hits.
      const Belle2::MCParticle* getMCParticle(const CDCSimHit* simHit) const
      {
        auto itFound = m_mcParticlesBySimHit.by<CDCSimHit>().find(simHit);
        return itFound != m_mcParticlesBySimHit.by<CDCSimHit>().end() ? itFound->get<MCParticle>() : nullptr;
      }
      //{ return simHit ? simHit->getRelated<MCParticle>() : nullptr; }


      /// Getter for the range MCParticle to CDCSimHits relations which come from the given MCParticle
      CDCSimHitByMCParticleRelationRange getSimHits(const MCParticle* mcParticle) const
      { return CDCSimHitByMCParticleRelationRange(m_mcParticlesBySimHit.by<MCParticle>().equal_range(mcParticle)); }
      //{ return mcParticle ? mcParticle->getRelated<CDCSimHit>() : nullptr; }

      /// Getter for the range MCParticle to CDCHits relations which come from the given MCParticle.
      CDCHitByMCParticleRelationRange getHits(const MCParticle* mcParticle) const
      { return CDCHitByMCParticleRelationRange(m_mcParticlesByHit.by<MCParticle>().equal_range(mcParticle)); }
      //{ return mcParticle ? mcParticle->getRelated<CDCHit>() : nullptr; }



      /// Indicates if the CDCSimHit has been reassigned to a primary MCParticle
      bool isReassignedSecondary(const CDCSimHit* ptrSimHit) const
      { return m_reassignedSecondarySimHits.count(ptrSimHit) > 0; }

      /// Indicates if the CDCHit has been reassigned to a primary MCParticle
      bool isReassignedSecondary(const CDCHit* ptrHit) const
      { return m_reassignedSecondaryHits.count(ptrHit) > 0; }



      /// Getter for all reassigned secondary CDCHits.
      const std::set<const CDCHit*>& getReassignedSecondaryHits() const
      { return m_reassignedSecondaryHits; }

      /// Getter for all reassigned secondary CDCSimHits.
      const std::set<const CDCSimHit*>& getReassignedSecondarySimHits() const
      { return m_reassignedSecondarySimHits; }



      /// Getter for the CDCHit <-> MCParticle relations.
      const CDCSimHitByCDCHitMap& getSimHitByHitRelations() const
      { return m_simHitByHit; }

      /// Getter for the CDCHit <-> MCParticle relations.
      const MCParticleByCDCHitMap& getMCParticleByHitRelations() const
      { return m_mcParticlesByHit; }

      /// Getter for the CDCSimHit <-> MCParticle relations.
      const MCParticleByCDCSimHitMap& getMCParticleBySimHitRelations() const
      { return m_mcParticlesBySimHit; }

    private:
      /// Memory for a one to one relation form CDCHits to CDCSimHits
      CDCSimHitByCDCHitMap m_simHitByHit;

      /// Memory for a one to n relation form CDCHits to MCParticles
      MCParticleByCDCHitMap m_mcParticlesByHit;

      /// Memory for a one to n relation form CDCSimHits to MCParticles
      MCParticleByCDCSimHitMap m_mcParticlesBySimHit;

      /// The set of reassigned secondary CDCHits
      std::set<const CDCHit*> m_reassignedSecondaryHits;

      /// The set of reassigned secondary CDCSimHits
      std::set<const CDCSimHit*> m_reassignedSecondarySimHits;

    }; //class
  } // end namespace TrackFindingCDC
} // namespace Belle2
