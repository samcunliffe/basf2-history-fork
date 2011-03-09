/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/touschek/TouschekReaderTURTLE.h>

#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>
#include <generators/dataobjects/MCParticle.h>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>

using namespace std;
using namespace Belle2;


TouschekReaderTURTLE::TouschekReaderTURTLE(TGeoHMatrix* transMatrix, int pdg): m_transMatrix(transMatrix), m_pdg(pdg),
    m_lineNum(0)
{
}


TouschekReaderTURTLE::~TouschekReaderTURTLE()
{
  if (m_input) m_input.close();
}


void TouschekReaderTURTLE::open(const string& filename) throw(TouschekCouldNotOpenFileError)
{
  m_lineNum = 0;
  m_input.open(filename.c_str());
  if (!m_input) throw(TouschekCouldNotOpenFileError() << filename);
}


int TouschekReaderTURTLE::getParticles(int number, MCParticleGraph &graph) throw(TouschekConvertFieldError)
{
  if (m_transMatrix == NULL) {
    B2ERROR("The transformation matrix is NULL !")
  }

  int numParticles = 0;
  string currLine;
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",; \t");
  double fields[7];
  double particlePosTouschek[3] = {0.0, 0.0, 0.0};
  double particlePosGeant4[3] = {0.0, 0.0, 0.0};
  double particleMomTouschek[3] = {0.0, 0.0, 0.0};
  double particleMomGeant4[3] = {0.0, 0.0, 0.0};

  do {
    getline(m_input, currLine);
    m_lineNum++;

    boost::trim(currLine);
    if (currLine == "") continue;

    tokenizer tokens(currLine, sep);

    int index = 0;
    BOOST_FOREACH(const string &tok, tokens) {
      try {
        if (index >= 7) {
          B2WARNING("This Touschek file has more than 7 fields ! Only the first 7 fields were read.")
          break;
        }
        fields[index] = boost::lexical_cast<double>(tok);
        index++;
      } catch (boost::bad_lexical_cast) {
        throw(TouschekConvertFieldError() << m_lineNum << index << tok);
      }
    }

    //Convert the position of the particle from local Touschek plane space to global geant4 space.
    //Flip the sign for the y and z component to go from the accelerator to the detector coordinate system
    particlePosTouschek[0] = fields[2] * Unit::m;
    particlePosTouschek[1] = -fields[3] * Unit::m;
    particlePosTouschek[2] = 0.0;
    m_transMatrix->LocalToMaster(particlePosTouschek, particlePosGeant4);

    //Convert the momentum of the particle from local Touschek plane space to global geant4 space.
    //Flip the sign for the y and z component to go from the accelerator to the detector coordinate system
    particleMomTouschek[0] = fields[4];
    particleMomTouschek[1] = -fields[5];
    particleMomTouschek[2] = -fields[6];
    m_transMatrix->LocalToMasterVect(particleMomTouschek, particleMomGeant4);

    double totalMom2 = particlePosGeant4[0] * particlePosGeant4[0];
    totalMom2 += particlePosGeant4[1] * particlePosGeant4[1];
    totalMom2 += particlePosGeant4[2] * particlePosGeant4[2];

    //Add particles to MCParticle collection
    MCParticleGraph::GraphParticle &particle = graph.addParticle();
    particle.setStatus(MCParticle::c_PrimaryParticle);
    particle.setPDG(m_pdg);
    particle.setMassFromPDG();
    particle.setMomentum(TVector3(particleMomGeant4));
    particle.setProductionVertex(TVector3(particlePosGeant4));
    particle.setEnergy(totalMom2 + m_pdg*m_pdg);
    particle.setProductionTime(0.0);
    particle.setValidVertex(true);

    numParticles++;
  } while ((!m_input.eof()) && ((number > -1) && (numParticles < number)));

  return numParticles;
}
