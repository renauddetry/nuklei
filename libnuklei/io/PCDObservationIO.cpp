// (C) Copyright Renaud Detry   2007-2011.
// Distributed under the GNU General Public License. (See accompanying file
// LICENSE.txt or copy at http://www.gnu.org/copyleft/gpl.html)

/** @file */

#include <fstream>
#include <cassert>
#include <algorithm>
#include <cmath>
#include <boost/tuple/tuple.hpp>

#include <nuklei/PCDObservationIO.h>
#include <nuklei/PCDObservation.h>
#include <nuklei/Common.h>
#include <nuklei/Match.h>
#include <nuklei/Indenter.h>

#ifdef NUKLEI_USE_PCL
#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>
#include <nuklei/PCLBridge.h>
#endif

namespace nuklei {



  PCDReader::PCDReader(const std::string &observationFileName) :
    KernelReader(observationFileName)
  {
  }

  PCDReader::~PCDReader()
  {
  }

  void PCDReader::init_()
  {
    NUKLEI_TRACE_BEGIN();
#ifdef NUKLEI_USE_PCL
    kc_.clear();
    
    {
      std::ifstream ifs(observationFileName_.c_str());
      if (!ifs.is_open())
        throw ObservationIOError(std::string("Could not open file ") +
                                 observationFileName_ + " for reading.");    
      
      std::string line;
      if (!std::getline(ifs, line) ||
          line.size() < 1 ||
          line.at(0) != '#' ||
          line.find("PCD") == std::string::npos)
        throw ObservationIOError("File " +
                                 observationFileName_ +
                                 " does not appear to be PCD.");
    }
    
    try {
      pcl::PointCloud<pcl::PointXYZRGBNormal>::Ptr cloud
      (new pcl::PointCloud<pcl::PointXYZRGBNormal>);
      
      if (pcl::io::loadPCDFile<pcl::PointXYZRGBNormal>
          (observationFileName_, *cloud) == -1)
        throw ObservationIOError("PCL cannot read file `" + observationFileName_ + "'.");
      else
        kc_ = nukleiFromPcl(*cloud);
    } catch (std::exception& e) {
      try {
        pcl::PointCloud<pcl::PointNormal>::Ptr cloud
        (new pcl::PointCloud<pcl::PointNormal>);
        
        if (pcl::io::loadPCDFile<pcl::PointNormal>
            (observationFileName_, *cloud) == -1)
          throw ObservationIOError("PCL cannot read file `" + observationFileName_ + "'.");
        else
          kc_ = nukleiFromPcl(*cloud);
      } catch (std::exception& e) {
        try {
          pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud
          (new pcl::PointCloud<pcl::PointXYZRGB>);
          
          if (pcl::io::loadPCDFile<pcl::PointXYZRGB>
              (observationFileName_, *cloud) == -1)
            throw ObservationIOError("PCL cannot read file `" + observationFileName_ + "'.");
          else
            kc_ = nukleiFromPcl(*cloud);
        } catch (std::exception& e) {
          try {
            pcl::PointCloud<pcl::PointXYZ>::Ptr cloud
            (new pcl::PointCloud<pcl::PointXYZ>);
            
            if (pcl::io::loadPCDFile<pcl::PointXYZ>
                (observationFileName_, *cloud) == -1)
              throw ObservationIOError("PCL cannot read file `" + observationFileName_ + "'.");
            else
              kc_ = nukleiFromPcl(*cloud);
          } catch (std::exception& e) {
            throw ObservationIOError(e.what());
          }
        }
      }
    }
    idx_ = 0;
#else
    throw ObservationIOError("This method requires PCL.");
#endif
    NUKLEI_TRACE_END();
  }

  std::auto_ptr<Observation> PCDReader::readObservation_()
  {
    NUKLEI_TRACE_BEGIN();
    if (idx_ < 0) NUKLEI_THROW("Reader does not seem inited.");
    if (idx_ >= int(kc_.size())) return std::auto_ptr<Observation>();
    else return std::auto_ptr<Observation>(new PCDObservation(kc_.at(idx_++)));
    NUKLEI_TRACE_END();
  }

  PCDWriter::PCDWriter(const std::string &observationFileName) :
    KernelWriter(observationFileName)
  {
    NUKLEI_TRACE_BEGIN();
    NUKLEI_TRACE_END();
  }
  
  PCDWriter::~PCDWriter()
  {
  }

  
  void PCDWriter::writeBuffer()
  {
    NUKLEI_TRACE_BEGIN();
    
#ifdef NUKLEI_USE_PCL
    NUKLEI_ASSERT(kc_.size() > 0);
    
    if (kc_.kernelType() == kernel::base::R3)
    {
      if (kc_.front().hasDescriptor())
      {
        pcl::PointCloud<pcl::PointXYZRGB> cloud = pclFromNuklei<pcl::PointXYZRGB>(kc_);
        pcl::io::savePCDFileASCII(observationFileName_, cloud);
      }
      else
      {
        pcl::PointCloud<pcl::PointXYZ> cloud = pclFromNuklei<pcl::PointXYZ>(kc_);
        pcl::io::savePCDFileASCII(observationFileName_, cloud);
      }
    }
    else if (kc_.kernelType() == kernel::base::R3XS2P)
    {
      if (kc_.front().hasDescriptor())
      {
        pcl::PointCloud<pcl::PointXYZRGBNormal> cloud = pclFromNuklei<pcl::PointXYZRGBNormal>(kc_);
        pcl::io::savePCDFileASCII(observationFileName_, cloud);
      }
      else
      {
        pcl::PointCloud<pcl::PointNormal> cloud = pclFromNuklei<pcl::PointNormal>(kc_);
        pcl::io::savePCDFileASCII(observationFileName_, cloud);
      }
    }
    else NUKLEI_THROW("Kernel type unsupported by PCD.");
#else
    NUKLEI_THROW("This method requires PCL.");
#endif

    NUKLEI_TRACE_END();
  }

}
