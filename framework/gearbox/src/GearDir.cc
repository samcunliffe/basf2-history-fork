/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <gearbox/Gearbox.h>
#include <gearbox/GearDir.h>

using namespace std;
using namespace Belle2;


GearDir::GearDir(const std::string& path) : GearboxIOAbs(), m_dirPath(path)
{

}


GearDir::GearDir(GearDir& gearDir, const std::string& path = "") : GearboxIOAbs()
{
  m_dirPath = gearDir.getDirPath() + path;
}


GearDir::~GearDir()
{

}


bool GearDir::close()
{
  return false;
}


bool GearDir::isOpen() const
{
  return Gearbox::Instance().getGearboxIO().isOpen();
}


int GearDir::getNumberNodes(const string& path) const
throw(GbxExcIONotConnected, GbxExcPathNotValid, GbxExcPathEmptyResult, GbxExcPathResultNotValid)
{
  return Gearbox::Instance().getGearboxIO().getNumberNodes(m_dirPath + path);
}


double GearDir::getParamLength(const string& path) const
throw(GbxExcIONotConnected, GbxExcPathNotValid, GbxExcPathEmptyResult,
      GbxExcPathResultNotValid, GbxExcStringNumConvFailed)
{
  return Gearbox::Instance().getGearboxIO().getParamLength(m_dirPath + path);
}


double GearDir::getParamAngle(const string& path) const
throw(GbxExcIONotConnected, GbxExcPathNotValid, GbxExcPathEmptyResult,
      GbxExcPathResultNotValid, GbxExcStringNumConvFailed)
{
  return Gearbox::Instance().getGearboxIO().getParamAngle(m_dirPath + path);
}


double GearDir::getParamNumValue(const string& path) const
throw(GbxExcIONotConnected, GbxExcPathNotValid, GbxExcPathEmptyResult,
      GbxExcPathResultNotValid, GbxExcStringNumConvFailed)
{
  return Gearbox::Instance().getGearboxIO().getParamNumValue(m_dirPath + path);
}


string GearDir::getParamString(const string& path) const
throw(GbxExcIONotConnected, GbxExcPathNotValid, GbxExcPathEmptyResult,
      GbxExcPathResultNotValid)
{
  return Gearbox::Instance().getGearboxIO().getParamString(m_dirPath + path);
}


void GearDir::setDirPath(const string& path)
{
  m_dirPath = path;
}


void GearDir::append(const string& path)
{
  m_dirPath += path;
}


void GearDir::append(GearDir& gearDir)
{
  m_dirPath += gearDir.getDirPath();
}
