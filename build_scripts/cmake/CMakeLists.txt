PROJECT (nuklei)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(PROJECT_NAMESPACE nuklei)
SET(CMAKE_BUILD_TYPE RelWithDebInfo)

ADD_DEFINITIONS(-Wall)
ADD_DEFINITIONS(-Wno-c++11-extensions -Wno-unused-variable -Wno-unneeded-internal-declaration)
# '-Wno-mismatched-tags', '-Wno-gnu-designator', '-Wno-parentheses'
# '-Wno-sign-compare', '-Wno-deprecated'

ADD_DEFINITIONS(-DNUKLEI_BUILDTYPE="deploy")

IF(NOT APPLE)
  ADD_DEFINITIONS(-D__LINUX__ -pthread)
  SET(PROJECT_LINK_FLAGS ${PROJECT_LINK_FLAGS} -pthread)
ENDIF(NOT APPLE)

# CPPFLAGS #

STRING(REGEX REPLACE "-I([^ ]+)" "\\1" CPPFLAGS "$ENV{CPPFLAGS}")
SEPARATE_ARGUMENTS(CPPFLAGS)
INCLUDE_DIRECTORIES(${CPPFLAGS})

IF(APPLE)
   FIND_LIBRARY(ACCELERATE_LIBRARY Accelerate)
   SET(PROJECT_EXTRA_LIBS ${ACCELERATE_LIBRARY})
ENDIF (APPLE)
# skip: lapack/blas on linux

LINK_DIRECTORIES(/usr/X11R6/lib)

# It is important that this line comes before any other INCLUDE_DIRECTORIES:

INCLUDE_DIRECTORIES(BEFORE libnuklei/contrib/WildMagic5p4 libnuklei/contrib/WildMagic5p4/nuklei/Wm5 libnuklei/contrib/trsl-0.2.2 libnuklei/base libnuklei/kernel libnuklei/io)

INCLUDE_DIRECTORIES(BEFORE contrib/CImg/include contrib/tclap-1.1.0/include contrib/ticpp-r97/src contrib/libkdtree++/include contrib/libklr-2010_05_07/src)

# CImg

ADD_DEFINITIONS(-Dcimg_debug=1 -Dcimg_display_type=0 -Dcimg_use_jpeg -Dcimg_use_png)
SET(PROJECT_LIBS ${PROJECT_LIBS} jpeg png)

# GSL:

SET(PROJECT_LIBS ${PROJECT_LIBS} gsl)

# Boost:

set(Boost_USE_STATIC_RUNTIME    OFF)
#set(Boost_DIR /usr/local/static/boost_1_51_0)
find_package(Boost 1.36.0 COMPONENTS system serialization filesystem thread iostreams)

if(Boost_FOUND)
   include_directories(${Boost_INCLUDE_DIRS})
   SET(PROJECT_LIBS ${PROJECT_LIBS} ${Boost_LIBRARIES})
else()
   MESSAGE("Boost not found!!!")
endif()

ADD_DEFINITIONS(-DNUKLEI_TRSL_USE_BSD_BETTER_RANDOM_GENERATOR)

FIND_PACKAGE(PkgConfig)

# SET(EIGEN_LIB_NAME eigen3)
# PKG_CHECK_MODULES(EIGEN ${EIGEN_LIB_NAME})
# INCLUDE_DIRECTORIES(${EIGEN_INCLUDE_DIRS})
# LINK_DIRECTORIES(${EIGEN_LIBRARY_DIRS})
# ADD_DEFINITIONS(${EIGEN_CFLAGS})
# 
# SET(OPENCV_LIB_NAME opencv)
# PKG_CHECK_MODULES(OPENCV ${OPENCV_LIB_NAME})
# INCLUDE_DIRECTORIES(${OPENCV_INCLUDE_DIRS})
# LINK_DIRECTORIES(${OPENCV_LIBRARY_DIRS})
# ADD_DEFINITIONS(${OPENCV_CFLAGS})
# 
# SET(PCL_LIB_NAME pcl_io pcl_sample_consensus pcl_registration pcl_kdtree)
# PKG_CHECK_MODULES(PCL ${PCL_LIB_NAME})
# INCLUDE_DIRECTORIES(${PCL_INCLUDE_DIRS})
# LINK_DIRECTORIES(${PCL_LIBRARY_DIRS})
# ADD_DEFINITIONS(${PCL_CFLAGS})
# 
# SET(CGAL_LIBS CGAL CGAL_Core gmpxx mpfr gmp)

SET(PROJECT_LIBS ${PROJECT_LIBS} X11 z pthread m)

# skipped openmp

FILE(GLOB PROJECT_CONTRIB_CODE contrib/libklr-2010_05_07/src/*.cpp contrib/ticpp-r97/src/ticpp.cpp contrib/ticpp-r97/src/tinystr.cpp contrib/ticpp-r97/src/tinyxml.cpp contrib/ticpp-r97/src/tinyxmlerror.cpp contrib/ticpp-r97/src/tinyxmlparser.cpp)

ADD_DEFINITIONS(-DTIXML_USE_TICPP)

FILE(GLOB_RECURSE PROJECT_LIBNUKLEI_CODE libnuklei/*.cpp)
FILE(GLOB_RECURSE PROJECT_LIBNUKLEI_HEADERS libnuklei/*.h)

SOURCE_GROUP(io REGULAR_EXPRESSION libnuklei/io/*)
SOURCE_GROUP(base REGULAR_EXPRESSION libnuklei/base/*)
SOURCE_GROUP(kernel REGULAR_EXPRESSION libnuklei/kernel/*)
SOURCE_GROUP(contrib-wm5 REGULAR_EXPRESSION libnuklei/contrib/WildMagic5p4/*)
SOURCE_GROUP(contrib-trsl REGULAR_EXPRESSION libnuklei/contrib/trsl-0.2.2/*)

# Product Definition #

ADD_LIBRARY(${PROJECT_NAMESPACE} SHARED ${PROJECT_LIBNUKLEI_CODE} ${PROJECT_LIBNUKLEI_HEADERS} ${PROJECT_CONTRIB_CODE})
#set_target_properties(${PROJECT_NAMESPACE} PROPERTIES ${PROJECT_LINK_FLAGS})
TARGET_LINK_LIBRARIES(${PROJECT_NAMESPACE} ${PROJECT_LIBS} ${PROJECT_EXTRA_LIBS})


#ADD_EXECUTABLE(${PROJECT_NAMESPACE} ${PROJECT_NAMESPACE} ${PROJECT_LIBS} ${PROJECT_EXTRA_LIBS})
#set_target_properties(${PROJECT_NAMESPACE} PROPERTIES ${PROJECT_LINK_FLAGS})
#SET_TARGET_PROPERTIES(tsp_ascii_writer-bin
#  PROPERTIES OUTPUT_NAME tsp_ascii_writer)

# Install #


#INSTALL(TARGETS ${PROJECT_NAMESPACE}
#          RUNTIME DESTINATION bin
#          BUNDLE DESTINATION bin
#          LIBRARY DESTINATION lib)
