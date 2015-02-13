#include <framework/datastore/DependencyMap.h>

#include <algorithm>

using namespace std;
using namespace Belle2;

void DependencyMap::ModuleInfo::addEntry(const std::string& name, EEntryType type, bool isRelation)
{
  if (isRelation)
    relations[type].insert(name);
  else
    entries[type].insert(name);
}

bool DependencyMap::isUsedAs(std::string branchName, EEntryType type) const
{
  return any_of(m_moduleInfo.begin(), m_moduleInfo.end(),
  [branchName, type](const pair<string, ModuleInfo>& info) {
    if (info.second.entries[type].count(branchName))
      return true;
    return (bool)info.second.relations[type].count(branchName);
  });
}
