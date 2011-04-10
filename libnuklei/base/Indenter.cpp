// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See
// accompanying file LICENSE.txt or copy at
// http://www.gnu.org/copyleft/gpl.html)

/** @file */

#include <iostream>
#include <nuklei/Indenter.h>

namespace nuklei {

  int Indenter::level = -1;

  Indenter::Indenter(std::ostream &out, std::string step) :

    out(out), step(step)

  { level++; };

  Indenter::~Indenter() { level--; };


  int Indenter::main(int argc, char ** argv)
  {
    Indenter idt0(std::cout);
  
    idt0 << "This is a level 0 display." << std::endl;
  
    {
      Indenter idt1 = Indenter();
      idt1 << "This is a level " << 1 << " display." << std::endl;
    } // End of block, idt1 is destructed.
  
    idt0 << "Back to level 0." << std::endl;
  
    return 0;
  }

}
