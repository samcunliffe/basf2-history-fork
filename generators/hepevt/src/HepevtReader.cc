/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/datastore/Units.h>
#include <framework/logging/Logger.h>
#include <generators/hepevt/HepevtReader.h>

#include <string>
#include <stdexcept>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace Belle2;

typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
boost::char_separator<char> sep(",; \t");


void HepevtReader::open(const string& filename) throw(HepEvtCouldNotOpenFileError)
{
  m_lineNr = 0;
  m_input.open(filename.c_str());
  if (!m_input) throw(HepEvtCouldNotOpenFileError() << filename);
}


int HepevtReader::getEvent(MCParticleGraph &graph, double & eventWeight) throw(HepEvtInvalidDaughterIndicesError, HepEvtEmptyEventError)
{
  int eventID = -1;
  int nparticles = readEventHeader(eventID, eventWeight);
  if (nparticles <= 0) {
    throw(HepEvtEmptyEventError() << m_lineNr << nparticles);
  }

  int first = graph.size();
  //Make list of particles
  for (int i = 0; i < nparticles; i++) {
    graph.addParticle();
  }
  //Read particles from file
  for (int i = 0; i < nparticles; ++i) {
    MCParticleGraph::GraphParticle &p = graph[first+i];
    readParticle(p);

    //Check for sensible daughter indices
    int d1 = p.getFirstDaughter();
    int d2 = p.getLastDaughter();
    if (d1 < 0 || d1 > nparticles || d2 < d1 || d2 > nparticles) {
      throw(HepEvtInvalidDaughterIndicesError() << m_lineNr << d1 << d2 << nparticles);
    }
    if (d1 == 0) p.addStatus(MCParticle::StableInGenerator);
    //Add decays
    for (int index = d1; index <= d2; ++index) {
      if (index > 0) p.decaysInto(graph[first+index-1]);
    }
  }
  return eventID;
}


bool HepevtReader::skipEvents(int n)
{
  int eventID;
  double weight;
  for (int i = 0; i < n; i++) {
    int nparticles = readEventHeader(eventID, weight);
    if (nparticles < 0) return false;
    for (int j = 0; j < nparticles; j++) getLine();
  }
  return true;
}


//===================================================================
//                  Protected methods
//===================================================================

std::string HepevtReader::getLine()
{
  std::string line;
  do {
    getline(m_input, line);
    m_lineNr++;
    size_t commentPos = line.find_first_of('#');
    if (commentPos != string::npos) {
      line = line.substr(0, commentPos);
    }
    boost::trim(line);
  } while (line == "" && !m_input.eof());
  return line;
}


int HepevtReader::readEventHeader(int &eventID, double &eventWeight) throw(HepEvtHeaderNotValidError)
{
  //Get number of particles from file
  int nparticles = -1;
  string line = getLine();
  if (line == "" || m_input.eof()) return -1;

  vector<double> fields;
  fields.reserve(15);

  tokenizer tokens(line, sep);
  int index(0);

  BOOST_FOREACH(const string &tok, tokens) {
    ++index;
    try {
      fields.push_back(boost::lexical_cast<double>(tok));
    } catch (boost::bad_lexical_cast &e) {
      throw(HepEvtConvertFieldError() << m_lineNr << index << tok);
    }
  }

  switch (fields.size()) {
    case 1:
      nparticles = static_cast<int>(fields[0]);
      break;
    case 2:
      nparticles = static_cast<int>(fields[1]);
      eventID = static_cast<int>(fields[0]);
      break;
    case 3:
      nparticles = static_cast<int>(fields[1]);
      eventID = static_cast<int>(fields[0]);
      eventWeight = fields[2];
      break;
    default:
      nparticles = static_cast<int>(fields[0]);
      break;
  }
  return nparticles;
}


void HepevtReader::readParticle(MCParticleGraph::GraphParticle &particle) throw(HepEvtConvertFieldError, HepEvtParticleFormatError)
{
  string line = getLine();
  vector<double> fields;
  fields.reserve(15);

//   typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
//   boost::char_separator<char> sep(",; \t");

  tokenizer tokens(line, sep);
  int index(0);

  BOOST_FOREACH(const string &tok, tokens) {
    ++index;
    try {
      fields.push_back(boost::lexical_cast<double>(tok));
    } catch (boost::bad_lexical_cast &e) {
      throw(HepEvtConvertFieldError() << m_lineNr << index << tok);
    }
  }

  switch (fields.size()) {
    case 8:
      particle.setStatus(MCParticle::PrimaryParticle);
      particle.setPDG(static_cast<int>(fields[1]));
      particle.setFirstDaughter(static_cast<int>(fields[2]));
      particle.setLastDaughter(static_cast<int>(fields[3]));
      particle.setMomentum(TVector3(&fields[4]));
      particle.setMass(fields[7]);
      break;
    case 6:
      particle.setStatus(MCParticle::PrimaryParticle);
      particle.setPDG(static_cast<int>(fields[5]));
      particle.setFirstDaughter(0);
      particle.setLastDaughter(0);
      particle.setMomentum(TVector3(&fields[0]));
      particle.setMass(fields[4]);
      particle.setEnergy(fields[3]);
      break;
    case 15:
      particle.setStatus(MCParticle::PrimaryParticle);
      particle.setPDG(static_cast<int>(fields[1]));
      particle.setFirstDaughter(static_cast<int>(fields[4]));
      particle.setLastDaughter(static_cast<int>(fields[5]));
      particle.setMomentum(TVector3(&fields[6]));
      //particle.setEnergy(fields[9]);
      particle.setMass(fields[10]);
      particle.setProductionVertex(TVector3(&fields[11])*mm);
      particle.setProductionTime(fields[14]*mm / speed_of_light);
      particle.setValidVertex(true);
      {
        //Warn if energy in Hepevt file differs from calculated energy by more than 0.1%
        const double &E  = particle.getEnergy();
        double dE = fabs(fields[9] - E) / E;
        if (dE > 1e-3) {
          B2WARNING(boost::format("line %d: Energy of particle does not match with expected energy: %.6e != %.6e")
                    % m_lineNr % fields[9] % E);
          B2WARNING(boost::format("delta E = %.2f%% -> ignoring given value") % (dE*100));
        }
      }
      break;
    default:
      throw(HepEvtParticleFormatError() << m_lineNr << fields.size());
  }
}
