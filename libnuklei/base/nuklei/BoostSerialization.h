// (C) Copyright Renaud Detry   2007-2015.
// Distributed under the GNU General Public License and under the
// BSD 3-Clause License (See accompanying file LICENSE.txt).

/** @file */


#ifndef NUKLEI_BOOST_SERIALIZATION_H
#define NUKLEI_BOOST_SERIALIZATION_H

#include <boost/config.hpp>

#ifdef NUKLEI_USE_CEREAL
#  define NUKLEI_SERIALIZATION_IMPL 1
#else
#  define NUKLEI_SERIALIZATION_IMPL 0
#endif

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
#if NUKLEI_SERIALIZATION_IMPL == 0 // Boost Serialization

#include <boost/archive/archive_exception.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/optional.hpp>

#include <boost/serialization/split_free.hpp>

#if BOOST_VERSION < 104100
#error Nuklei requires Boost >= 1.41
#endif

#define NUKLEI_SERIALIZATION_NVP(T) BOOST_SERIALIZATION_NVP(T)
#define NUKLEI_SERIALIZATION_FRIEND_CLASSNAME boost::serialization::access
#define NUKLEI_SERIALIZATION_MAKE_NVP boost::serialization::make_nvp
#define NUKLEI_SERIALIZATION_BASE(T) boost::serialization::base_object<T>( *this )
#define NUKLEI_SERIALIZATION_CLASS_VERSION BOOST_CLASS_VERSION
#define NUKLEI_SERIALIZATION_REGISTER_TYPE BOOST_CLASS_EXPORT_IMPLEMENT
#define NUKLEI_SERIALIZATION_DECLARE_TYPE_WITH_NAME BOOST_CLASS_EXPORT_KEY2
#define NUKLEI_SERIALIZATION_BINARY_IARCHIVE boost::archive::binary_iarchive
#define NUKLEI_SERIALIZATION_BINARY_OARCHIVE boost::archive::binary_oarchive
#define NUKLEI_SERIALIZATION_XML_IARCHIVE boost::archive::xml_iarchive
#define NUKLEI_SERIALIZATION_XML_OARCHIVE boost::archive::xml_oarchive
#define NUKLEI_SERIALIZATION_NAMESPACE_BEGIN namespace boost { namespace serialization {
#define NUKLEI_SERIALIZATION_NAMESPACE_END } }


#include <nuklei/Common.h>

#if (NUKLEI_UNIQUE_PTR_IMPL == 0 && defined(BOOST_NO_CXX11_SMART_PTR)) || NUKLEI_UNIQUE_PTR_IMPL == 1

// From boost_1_36_0/libs/serialization/example/demo_auto_ptr.cpp
// Modified to support XML archives.
// I don't know why this is not included in the installed headers.

namespace boost { 
  namespace serialization {

    /////////////////////////////////////////////////////////////
    // implement serialization for auto_ptr<T>
    // note: this must be added to the boost namespace in order to
    // be called by the library
    template<class Archive, class T>
    inline void save(
        Archive & ar,
        const std::auto_ptr<T> &t,
        const unsigned int file_version
    ){
        // only the raw pointer has to be saved
        // the ref count is rebuilt automatically on load
        const T * const raw_ptr = t.get();
        ar << NUKLEI_SERIALIZATION_NVP(raw_ptr);
    }

    template<class Archive, class T>
    inline void load(
        Archive & ar,
        std::auto_ptr<T> &t,
        const unsigned int file_version
    ){
        T *raw_ptr;
        ar >> NUKLEI_SERIALIZATION_NVP(raw_ptr);
        t.reset(raw_ptr);
    }

    // split non-intrusive serialization function member into separate
    // non intrusive save/load member functions
    template<class Archive, class T>
    inline void serialize(
        Archive & ar,
        std::auto_ptr<T> &t,
        const unsigned int file_version
    ){
        boost::serialization::split_free(ar, t, file_version);
    }

  } // namespace serialization
} // namespace boost

#else

#include <boost/serialization/unique_ptr.hpp>

#endif

#include <boost/ptr_container/serialize_ptr_vector.hpp>
#include <boost/ptr_container/serialize_ptr_array.hpp>

// Should come after archives.
#include <boost/serialization/export.hpp>
#include <boost/serialization/base_object.hpp>

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
#elif NUKLEI_SERIALIZATION_IMPL == 1 // Cereal

#include <nuklei/cereal/archives/binary.hpp>
#include <nuklei/cereal/archives/xml.hpp>
//#include <nuklei/cereal/archives/json.hpp>
#include <nuklei/cereal/types/polymorphic.hpp>

#include <nuklei/cereal/types/vector.hpp>
#include <nuklei/cereal/types/list.hpp>
#include <nuklei/cereal/types/string.hpp>
#include <nuklei/cereal/types/utility.hpp>
#include <nuklei/cereal/types/map.hpp>
#include <nuklei/cereal/types/memory.hpp>
#include <nuklei/cereal/types/common.hpp>

#if BOOST_VERSION < 104100
#error Nuklei requires Boost >= 1.41
#endif

#include <boost/optional.hpp>

namespace nuklei_cereal
{
  //! Saving for boost::optional
  template <class Archive, class Optioned> inline
  void save(Archive & ar, ::boost::optional<Optioned> const & optional)
  {
    bool initFlag = (bool)optional;
    if (initFlag)
    {
      ar(NUKLEI_CEREAL_NVP_("initialized", true));
      ar(NUKLEI_CEREAL_NVP_("value", optional.get()));
    }
    else
    {
      ar(NUKLEI_CEREAL_NVP_("initialized", false));
    }
  }
  
  //! Loading for boost::optional
  template <class Archive, class Optioned> inline
  void load(Archive & ar, ::boost::optional<Optioned> & optional)
  {
    
    bool initFlag;
    ar(NUKLEI_CEREAL_NVP_("initialized", initFlag));
    if (initFlag)
    {
      Optioned val;
      ar(NUKLEI_CEREAL_NVP_("value", val));
      optional = std::move(val);
    }
    else
      optional = ::boost::none; // this is all we need to do to reset the internal flag and value
    
  }
} // namespace nuklei_cereal

#define NUKLEI_SERIALIZATION_NVP(T) NUKLEI_CEREAL_NVP(T)
#define NUKLEI_SERIALIZATION_FRIEND_CLASSNAME nuklei_cereal::access
#define NUKLEI_SERIALIZATION_MAKE_NVP nuklei_cereal::make_nvp
#define NUKLEI_SERIALIZATION_BASE(T) nuklei_cereal::base_class<T>( this )
#define NUKLEI_SERIALIZATION_CLASS_VERSION NUKLEI_CEREAL_CLASS_VERSION
#define NUKLEI_SERIALIZATION_REGISTER_TYPE NUKLEI_CEREAL_REGISTER_TYPE
#define NUKLEI_SERIALIZATION_DECLARE_TYPE_WITH_NAME(T, U)
#define NUKLEI_SERIALIZATION_BINARY_IARCHIVE nuklei_cereal::BinaryInputArchive
#define NUKLEI_SERIALIZATION_BINARY_OARCHIVE nuklei_cereal::BinaryOutputArchive
#define NUKLEI_SERIALIZATION_XML_IARCHIVE nuklei_cereal::XMLInputArchive
#define NUKLEI_SERIALIZATION_XML_OARCHIVE nuklei_cereal::XMLOutputArchive
#define NUKLEI_SERIALIZATION_NAMESPACE_BEGIN namespace nuklei_cereal {
#define NUKLEI_SERIALIZATION_NAMESPACE_END }

#include <nuklei/Common.h>

#if (NUKLEI_UNIQUE_PTR_IMPL == 0 && defined(BOOST_NO_CXX11_SMART_PTR)) || NUKLEI_UNIQUE_PTR_IMPL == 1
#error Cereal requires C++ 11
#endif

#include <nuklei/cereal/types/base_class.hpp>

#endif

#endif

