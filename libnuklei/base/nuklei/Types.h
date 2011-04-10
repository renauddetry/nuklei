// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See
// accompanying file LICENSE.txt or copy at
// http://www.gnu.org/copyleft/gpl.html)

/** @file */

#ifndef NUKLEI_TYPES_H
#define NUKLEI_TYPES_H

#include <vector>

#include <nuklei/Common.h>

namespace nuklei {
  
  template<class T>
  std::string catTypeNames()
  {
    std::string s;
    for (int i = 0; i < T::UNKNOWN; i++)
    {
      s += T::TypeNames[i];
      if (i == T::defaultType) s += "*";
      if (i != T::UNKNOWN-1) s += std::string("|");
    }
    return s;
  }
  
  template<class T>
  std::vector<std::string> listTypeNames()
  {
    std::vector<std::string> l;
    for (int i = 0; i < T::UNKNOWN; i++)
      l.push_back(T::TypeNames[i]);
    return l;
  }
  
  template<class T>
  std::string defaultTypeName()
  {
    return T::TypeNames[T::defaultType];
  }
  
  template<class T>
  typename T::Type defaultType()
  {
    return T::defaultType;
  }
  
  template<class T>
  std::string nameFromType(int t)
  {
    NUKLEI_ASSERT(0 <= t && t <= T::UNKNOWN);
    if (t == T::UNKNOWN) return "unknown";
    else return T::TypeNames[t];
  }
  
  template<class T>
  typename T::Type typeFromName(std::string s)
  {
    for (int i = 0; i < T::UNKNOWN; i++)
    {
      if (s == T::TypeNames[i]) return typename T::Type(i);
    }
    if (s == "unknown") return T::UNKNOWN;
    NUKLEI_THROW("Invalid type `" << s << "'.");
  }
  
}
#endif
