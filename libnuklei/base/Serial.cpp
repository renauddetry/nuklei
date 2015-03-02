// (C) Copyright Renaud Detry   2007-2015.
// Distributed under the GNU General Public License and under the
// BSD 3-Clause License (See accompanying file LICENSE.txt).

/** @file */

#include <nuklei/Serial.h>
#include <nuklei/Types.h>
#include <nuklei/KernelCollection.h>
#include <nuklei/SerialDefinitions.h>


namespace nuklei
{
  
  const std::string Serial::TypeNames[] =
  { "bxml", "bxmlc",
    "bbin", "bbinc" };
  
  // Template instantiations
  
  template
  void Serial::readObject
  <KernelCollection>(KernelCollection &object,
                     const std::string& filename,
                     const std::string& typeName);
  
  template
  void Serial::writeObject
  <KernelCollection>(const KernelCollection &object, const std::string& filename,
                     const std::string &typeName,
                     const int precision);
  
  template
  void Serial::readObject
  <kernel::se3>(kernel::se3 &object,
                const std::string& filename,
                const std::string& typeName);
  
  template
  void Serial::writeObject
  <kernel::se3>(const kernel::se3 &object, const std::string& filename,
                const std::string &typeName,
                const int precision);
  
  template
  void Serial::readObject
  <RGBColor>(RGBColor &object,
             const std::string& filename,
             const std::string& typeName);
  
  template
  void Serial::writeObject
  <RGBColor>(const RGBColor &object, const std::string& filename,
             const std::string &typeName,
             const int precision);
  
  
}
