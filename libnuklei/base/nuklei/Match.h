// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See
// accompanying file LICENSE.txt or copy at
// http://www.gnu.org/copyleft/gpl.html)

/** @file */

#ifndef NUKLEI_MATCH_H
#define NUKLEI_MATCH_H

#include <string>
#include <iostream>
#include <stdexcept>
#include <nuklei/Common.h>

namespace nuklei {
  
  class MatchError : public Error
  {
  public:
    MatchError(const std::string &s) : Error(s) {}
  };
  
  class Match
  {
  public:
    Match(const std::string &s) : s(s) {};
    ~Match() {};
    
    friend std::istream& operator>>(std::istream &in, const Match &match);
    
  private:
    const std::string s;
  };
  
  inline std::istream& operator>>(std::istream &in, const Match &match)
  {
    std::string t;
    in >> t;
    if (match.s != t) throw MatchError
      (std::string("expected `") + match.s + "', found `" + t + "'");
    return in;
  }
  
}

#endif

