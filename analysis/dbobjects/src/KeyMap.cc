/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Ilya Komarov                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/dbobjects/KeyMap.h>
#include <framework/logging/Logger.h>
//#include <analysis/VariableManager/Manager.h>
//#include <analysis/VariableManager/Utility.h>

using namespace Belle2;

BinLimits KeyMap::unusedBin()
{
  return std::pair<double, double>(-999, 999);
}

bool KeyMap::isUnusedBin(BinLimits bin)
{
  BinLimits unused_bin = this->unusedBin();
  if ((bin.first == unused_bin.first) and (bin.second == unused_bin.second)) {
    return true;
  }
  return false;
}

bool KeyMap::isNewKeymap()
{
  B2INFO("Checking if keymap is new");
  if (m_3DkeyTable.second.empty()) {
    return true;
  }
  return false;
}

bool KeyMap::isUnnamed()
{
  B2INFO("Checking if keymap is unnamed");
  if (m_3DkeyTable.first.empty()) {
    return true;
  }
  return false;
}

int KeyMap::numberOfDimensions()
{
  B2INFO("Checking number of dimensions of the keymap.");
  bool dim1_is_unused = this->isUnusedBin(m_3DkeyTable.second.begin()->first);
  bool dim2_is_unused = this->isUnusedBin(m_3DkeyTable.second.begin()->second.begin()->first);
  bool dim3_is_unused = this->isUnusedBin(m_3DkeyTable.second.begin()->second.begin()->second.begin()->first);
  if ((not dim1_is_unused) and (dim2_is_unused) and (dim3_is_unused)) {
    B2INFO("The keymap is 1-D");
    return 1;
  } else if ((not dim1_is_unused) and (not dim2_is_unused) and (dim3_is_unused)) {
    B2INFO("The keymap is 2-D");
    return 2;
  } else if ((not dim1_is_unused) and (not dim2_is_unused) and (not dim3_is_unused)) {
    B2INFO("The keymap is 3-D");
    return 3;
  } else {
    B2ERROR("The keymap is incorrectly intialized: dimension can't be defined.");
  }
  return -1;
}

bool KeyMap::isKey(BinLimits var1_minimax)
{
  B2INFO("Check if 1D bin exists in the keymap");
  return this->isKey(var1_minimax, this->unusedBin());
}

bool KeyMap::isKey(BinLimits var1_minimax, BinLimits var2_minimax)
{
  B2INFO("Check if 2D bin exists in the keymap");
  return this->isKey(var1_minimax, var2_minimax, this->unusedBin());
}

bool KeyMap::isKey(BinLimits var1_minimax, BinLimits var2_minimax, BinLimits var3_minimax)
{
  if (this->isNewKeymap()) {
    B2INFO("Keymap is new, key is not defined");
    return false;
  }
  B2INFO("Check if 3D bin exists in the keymap");
  if (m_3DkeyTable.second.find(var1_minimax) != m_3DkeyTable.second.end()) {
    if (m_3DkeyTable.second.find(var1_minimax)->second.find(var2_minimax) != m_3DkeyTable.second.find(var1_minimax)->second.end()) {
      if (m_3DkeyTable.second.find(var1_minimax)->second.find(var2_minimax)->second.find(var3_minimax) != m_3DkeyTable.second.find(
            var1_minimax)->second.find(var2_minimax)->second.end()) {
        return true;
      }
    }
  }
  return false;
}

bool KeyMap::isKey(double key)
{
  B2INFO("Check if key " << key << " is defined for the map");
  if (this->isNewKeymap()) {
    B2INFO("Keymap is new, key is not defined");
    return false;
  }
  for (auto var1_minimax : m_3DkeyTable.second) {
    for (auto var2_minimax : var1_minimax.second) {
      for (auto var3_minimax : var2_minimax.second) {
        if (key ==  var3_minimax.second) {
          B2INFO("Keymap is not new, the key " << key << " is found");
          return true;
        }
      }
    }
  }
  B2INFO("Keymap is not new, the key " << key << " is not found");
  return false;
}

double KeyMap::getKey(BinLimits var1_minimax)
{
  B2INFO("Getting key for 1D bin");
  return this->getKey(var1_minimax, this->unusedBin());
}

double KeyMap::getKey(BinLimits var1_minimax, BinLimits var2_minimax)
{
  B2INFO("Getting key for 2D bin");
  return this->getKey(var1_minimax, var2_minimax, this->unusedBin());
}

double KeyMap::getKey(BinLimits var1_minimax, BinLimits var2_minimax, BinLimits var3_minimax)
{
  B2INFO("Getting key for 3D bin");
  if (not this->isKey(var1_minimax, var2_minimax, var3_minimax)) {
    B2ERROR("Trying to get unexisting kinematic key: \n var1 in [" << var1_minimax.first << "; " << var1_minimax.second <<
            "], \n var2 in [" << var2_minimax.first << "; " << var2_minimax.second <<
            "], \n var3 in [" << var3_minimax.first << "; " << var3_minimax.second << "].");
  }
  return m_3DkeyTable.second.find(var1_minimax)->second.find(var2_minimax)->second.find(var3_minimax)->second;
}

std::vector<std::string> KeyMap::getNames()
{
  B2INFO("Retreiving axis names");
  std::vector<std::string> names = m_3DkeyTable.first;
  for (auto name : names) {
    B2INFO("Axis name found: " << name);
  }
  return names;
}

NDBin KeyMap::getNDBin(double key_ID)
{
  B2INFO("Getting N-dim bin for the given key");
  if (not this->isKey(key_ID)) {
    B2ERROR("Trying to get unexisting key" << key_ID);
  }

  BinLimits limitx_x;
  BinLimits limitx_y;
  BinLimits limitx_z;
  for (auto var1_minimax : m_3DkeyTable.second) {
    for (auto var2_minimax : var1_minimax.second) {
      for (auto var3_minimax : var2_minimax.second) {
        if (key_ID ==  var3_minimax.second) {
          limitx_x = var1_minimax.first;
          limitx_y = var2_minimax.first;
          limitx_z = var3_minimax.first;
        }
      }
    }
  }

  int nDim = numberOfDimensions();
  std::vector<std::string> names = this->getNames();
  NDBin bin;

  B2INFO("Getting name of the first axis variable");
  std::string name_x = names[0];
  bin.insert(std::pair<std::string, BinLimits>(name_x, limitx_x));

  if (nDim > 1) {
    B2INFO("Getting name of the second axis variable");
    std::string name_y = names[1];
    bin.insert(std::pair<std::string, BinLimits>(name_y, limitx_y));
  }

  if (nDim > 2) {
    B2INFO("Getting name of the third axis variable");
    std::string name_z = names[2];
    bin.insert(std::pair<std::string, BinLimits>(name_z, limitx_z));
  }
  return bin;
}

void KeyMap::checkLimits(BinLimits var_minimax)
{
  if (var_minimax.first > var_minimax.second) {
    B2ERROR("Bin limits are incorrectly defined: " << var_minimax.first << " > " << var_minimax.second <<
            ". Please redefine them as (min,max)");
  }
}

double KeyMap::addKey(BinLimits var1_minimax, BinLimits var2_minimax, BinLimits var3_minimax,
                      double key_ID)
{
  checkLimits(var1_minimax);
  checkLimits(var2_minimax);
  checkLimits(var3_minimax);
  B2INFO("Adding 3D bin to the keymap with specific key " << key_ID);
  if (this->isKey(var1_minimax, var2_minimax, var3_minimax)) {
    double found_key = this->getKey(var1_minimax, var2_minimax, var3_minimax);
    if (found_key == key_ID) {
      return key_ID;
    } else {
      B2WARNING("Trying to assign existing limits to the new key # " << key_ID << ": \n var1 in [" << var1_minimax.first << "; "
                << var1_minimax.second <<
                "], \n var2 in [" << var2_minimax.first << "; " << var2_minimax.second << "], \n var3 in ["
                << var3_minimax.first << "; " << var3_minimax.second << "].\n. Ignoring the attempt.");
      return found_key;
    }
  }

  if (m_3DkeyTable.second.find(var1_minimax) != m_3DkeyTable.second.end()) {
    if (m_3DkeyTable.second.find(var1_minimax)->second.find(var2_minimax) != m_3DkeyTable.second.find(var1_minimax)->second.end()) {
      m_3DkeyTable.second.find(var1_minimax)->second.find(var2_minimax)->second.insert(std::make_pair(var3_minimax, key_ID));
    } else {
      Nameless1DMap var3_key_map;
      var3_key_map[var3_minimax] = key_ID;
      m_3DkeyTable.second[var1_minimax][var2_minimax] = var3_key_map;
    }
  } else {
    Nameless1DMap var3_key_map;
    var3_key_map[var3_minimax] = key_ID;
    Nameless2DMap var2_key_map;
    var2_key_map[var2_minimax] = var3_key_map;
    m_3DkeyTable.second[var1_minimax] = var2_key_map;
  }
  return key_ID;
}

std::vector<std::string> KeyMap::getBinNameVector(NDBin bin)
{
  B2INFO("Getting vector of axis names from N-dim bin");
  std::vector<std::string> names;
  for (auto const& name : bin) {
    names.push_back(name.first);
    B2INFO("Name found: " << name.first);
  }

  return names;
}

std::vector<std::string> KeyMap::addNames(NDBin bin)
{
  B2INFO("Setting axis names of the keymap from N-dim bin");
  std::vector<std::string> new_names = this->getBinNameVector(bin);

  if (this->isUnnamed()) {
    m_3DkeyTable.first = new_names;
    return this->getNames();
  }

  std::vector<std::string> existing_names = this->getNames();

  bool is_equal = false;
  if (new_names.size() == existing_names.size()) {
    is_equal = std::equal(new_names.begin(), new_names.end(), existing_names.begin());
  }
  if (not is_equal) {
    B2ERROR("Error with axis naming: added and existing name vectors don't match");
  }

  return existing_names;
}

double KeyMap::addKey(NDBin bin)
{
  B2INFO("Adding entry to keymap for N-dim bin object");

  double bin_number = m_3DkeyTable.second.size();
  while (this->isKey(bin_number)) {
    bin_number += 1;
  }
  return this->addKey(bin, bin_number);
}


double KeyMap::addKey(NDBin bin, double key_ID)
{
  B2INFO("Adding entry to keymap for N-dim bin object with specific key ID " << key_ID);
  std::vector<std::string> names = this->getBinNameVector(bin);
  int nDim = names.size();
  double added_key_ID;
  switch (nDim) {
    case 1:
      added_key_ID = this->addKey(bin[names[0]], this->unusedBin(), this->unusedBin(), key_ID);
      break;
    case 2:
      added_key_ID = this->addKey(bin[names[0]], bin[names[1]],     this->unusedBin(), key_ID);
      break;
    case 3:
      added_key_ID = this->addKey(bin[names[0]], bin[names[1]],     bin[names[2]], key_ID);
      break;
    default:
      B2ERROR("Error in adding key due to dimensionality of the table");
      return -1;
  }
  this->addNames(bin);
  return added_key_ID;
}


/*
// Get kinematic key for the particle's momentum
double KeyMap::getKey(const Particle* p)
{
  int nDim = numberOfDimensions();
  switch (nDim) {
    case 1: {
      double var1_val;
      std::vector<std::string> variables = Variable::Manager::Instance().resolveCollections(m_1DkeyTable.first);
      const Variable::Manager::Var* var1 = Variable::Manager::Instance().getVariable(variables[0]);
      if (!var1) {
        B2ERROR("Variable '" << variables[0] << "' is not available in Variable::Manager!");
        return -1;
      } else {
        var1_val = var1->function(p);
      }
      for (auto var1_minimax : m_1DkeyTable.second) {
        if (var1_minimax.first.first <= var1_val && var1_minimax.first.second > var1_val) {
          return var1_minimax.second;
        }
      }
      B2WARNING("Particle is out of defined binning scheme. Returning default for this cases -1 key.");
      return -1;
      break;
    }
    case 2: {
      double var1_val;
      double var2_val;
      std::vector<std::string> variables = Variable::Manager::Instance().resolveCollections(m_2DkeyTable.first);
      const Variable::Manager::Var* var1 = Variable::Manager::Instance().getVariable(variables[0]);
      const Variable::Manager::Var* var2 = Variable::Manager::Instance().getVariable(variables[1]);
      if (!var1) {
        B2ERROR("Variable '" << variables[0] << "' is not available in Variable::Manager!");
        return -1;
      } else {
        var1_val = var1->function(p);
      }
      if (!var2) {
        B2ERROR("Variable '" << variables[2] << "' is not available in Variable::Manager!");
        return -1;
      } else {
        var2_val = var2->function(p);
      }
      for (auto var1_minimax : m_2DkeyTable.second) {
        if (var1_minimax.first.first <= var2_val && var1_minimax.first.second > var2_val) {
          for (auto var2_minimax : var1_minimax.second) {
            if (var2_minimax.first.first <= var1_val && var2_minimax.first.second > var1_val) {
              return var2_minimax.second;
            }
          }
        }
      }
      B2WARNING("Particle is out of defined binning scheme. Returning default for this cases -1 key.");
      return -1;
      break;
    }
    case 3: {
      double var1_val;
      double var2_val;
      double var3_val;
      std::vector<std::string> variables = Variable::Manager::Instance().resolveCollections(m_3DkeyTable.first);
      const Variable::Manager::Var* var1 = Variable::Manager::Instance().getVariable(variables[0]);
      const Variable::Manager::Var* var2 = Variable::Manager::Instance().getVariable(variables[1]);
      const Variable::Manager::Var* var3 = Variable::Manager::Instance().getVariable(variables[2]);
      if (!var1) {
        B2ERROR("Variable '" << variables[0] << "' is not available in Variable::Manager!");
        return -1;
      } else {
        var1_val = var1->function(p);
      }
      if (!var2) {
        B2ERROR("Variable '" << variables[2] << "' is not available in Variable::Manager!");
        return -1;
      } else {
        var2_val = var2->function(p);
      }
      if (!var3) {
        B2ERROR("Variable '" << variables[2] << "' is not available in Variable::Manager!");
        return -1;
      } else {
        var3_val = var3->function(p);
      }
      for (auto var1_minimax : m_3DkeyTable.second) {
        if (var1_minimax.first.first <= var1_val && var1_minimax.first.second > var1_val) {
          for (auto var2_minimax : var1_minimax.second) {
            if (var2_minimax.first.first <= var2_val && var2_minimax.first.second > var2_val) {
              for (auto var3_minimax : var2_minimax.second) {
                if (var3_minimax.first.first <= var3_val && var3_minimax.first.second > var3_val) {
                  return var3_minimax.second;
                }
              }
            }
          }
        }
      }
      B2WARNING("Particle is out of defined binning scheme. Returning default for this cases -1 key.");
      return -1;
      break;
    }
    default:
      B2ERROR("Error in finding of the key due to dimensionality of the table");
  }
  return -1;
}
*/


