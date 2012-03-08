import os.path
import fnmatch

Import('conf')

# Global contributions

# Adding to CCFLAGS instead of CPPPATH prevents scons from hashing everything inside
conf.env.Append(CCFLAGS = [ '-I/usr/X11R6/include' ])
conf.env.Append(LIBPATH = [ '/usr/X11R6/lib' ])

# Local contributions


if conf.env['UseCIMG']:
  conf.env['CImg_include'] = '#/contrib/CImg/include'
conf.env['tclap_include'] = '#/contrib/tclap-1.1.0/include'
conf.env['ticpp_include'] = '#/contrib/ticpp-r97/src'
conf.env['libkdtree_include'] = '#/contrib/libkdtree++/include'
conf.env['libklr_include'] = '#/contrib/libklr-2010_05_07/src'


if not conf.CheckPKGConfig('0.15.0'):
  print 'pkg-config not found.'
  Exit(1)

# Local Headers

conf.env.Prepend(CPPPATH = [ '#libnuklei/contrib/WildMagic5p4',
                             '#libnuklei/contrib/trsl-0.2.2', '#libnuklei/base',
                             '#libnuklei/kernel', '#libnuklei/io' ])

# contrib: libkdtree++

conf.env.Prepend(CPPPATH = [ '$libkdtree_include' ])

# contrib: libklr

conf.env.Prepend(CPPPATH = [ '$libklr_include' ])

# contrib: CImg

if conf.env['UseCIMG']:
  conf.env.Append(CPPDEFINES = ['NUKLEI_USE_CIMG'])
  conf.env.Prepend(CPPPATH = [ '$CImg_include' ])
  if conf.env['BuildType'] == 'develop':
    conf.env.Append(CPPDEFINES = [ ('cimg_debug', 2) ])
  else:
    conf.env.Append(CPPDEFINES = [ ('cimg_debug', 1) ])
    conf.env.Append(CPPDEFINES = [ ('cimg_display_type', 0) ])

  have_imconvert = conf.CheckIMConvert()
  have_libjpeg = conf.CheckCXXHeader( [ 'stdio.h', 'sys/types.h', 'jpeglib.h' ] ) and \
                 conf.CheckLib('jpeg', language = 'C++')
  have_libpng = conf.CheckCXXHeader( 'png.h' ) and \
                conf.CheckLib('png', language = 'C++')
  if have_libjpeg:
    conf.env.Append(CPPDEFINES = [ 'cimg_use_jpeg' ])
    conf.env.Append(LIBS = [ 'jpeg' ])
  if have_libpng:
    conf.env.Append(CPPDEFINES = [ 'cimg_use_png' ])
    conf.env.Append(LIBS = [ 'png' ])

  if not have_imconvert:
    if not have_libjpeg:
      print 'Warning: you have neither libjpeg nor ImageMagick convert.'
      print 'No JPEG I/O.'
    if not have_libpng:
      print 'Warning: you have neither libpng nor ImageMagick convert.'
      print 'No PNG I/O.'
    print 'Warning: supported image I/O:'
    print '  BMP'
    print '  PPM/PGM'
    if have_libpng: print '  PNG'
    if have_libjpeg: print '  JPG'
    print 'Install ImageMagick command line tools and/or libjpeg/libpng to enable a larger format support.'

# BLAS, LAPACK

if conf.env['PLATFORM'] == 'darwin':
  conf.env.Append(FRAMEWORKS = [ 'vecLib' ])
elif conf.env['PLATFORM'] == 'posix':
  if not conf.CheckLib('lapack', language = 'C++'):
    print 'A LAPACK library is required.'
    print '** For more information, refer to the INSTALL document **'
    Exit(1)
  conf.env.Append(LIBS = [ 'lapack' ])
  if not conf.CheckLib('cblas', language = 'C++'):
    if not conf.CheckLib('blas', language = 'C++'):
      print 'A BLAS library is required.'
      print '** For more information, refer to the INSTALL document **'
      Exit(1)
    else:
      conf.env.Append(LIBS = [ 'blas' ])
  else:
    conf.env.Append(LIBS = [ 'cblas' ])
else:
  print 'Unknown platform.'
  Exit(1)


# CGAL

if conf.env['EnableQPL']:
  if not conf.CheckCXXHeader('CGAL/version.h') or \
     not conf.CheckLib('CGAL', language = 'C++'):
    print 'Please check your CGAL installation.'
    print '** For more information, refer to the INSTALL document **'
    Exit(1)
  else:
    # Note: CGAL may need the following:
    # 'CGALcore++', 'CGALPDB', 'mpfr', 'gmpxx', 'gmp'
    conf.env.Append(CPPDEFINES = ['NUKLEI_ENABLE_QPL'])
    conf.env.Append(LIBS = [ 'CGAL' ])

# GSL

if not conf.CheckCXXHeader('gsl/gsl_version.h') or \
   not conf.CheckLib('gsl', language = 'C++'):
  print 'Please check your GSL installation.'
  print '** For more information, refer to the INSTALL document **'
  Exit(1)
else:
  conf.env.Append(LIBS = [ 'gsl' ])
  # GSL requires a BLAS library for vector and matrix operations.  The
  # default CBLAS library supplied with GSL (gslcblas) can be replaced
  # by the tuned ATLAS library for better performance. This is what
  # happens here, since CGAL already links to a fast BLAS
  # implementation.

  # GSL is not used in headers: all GSL symbols are linked from libnuklei.so,
  # we don't need to add gsl to PkgCLibs.
  # conf.env['PkgCLibs'] += ' -lgsl -lgslcblas'

# Boost

if not conf.CheckCXXHeader('boost/version.hpp'):
  print 'Please check your Boost installation.'
  print 'Note that if you installed boost from source, headers are '
  print 'installed in $prefix/include/boost_1_xx_x/, which is not '
  print 'automatically detected.'
  print '** For more information, refer to the INSTALL document **'
  Exit(1)

def bln(env, name):
  return conf.env['boost_format'] % name

if not (conf.CheckBoost('1.35', '0.0')):
  print 'Boost version 1.35 or higher needed.'
  print 'Please check your Boost installation.'
  Exit(1)
else:
  if not conf.CheckLib(bln(conf.env, 'boost_system'), language = 'C++'):
    print 'Please check your Boost installation.'
    print 'Note that if you installed boost from source, libs are '
    print 'named as libboost_LIBNAME-COMPILER-THREAD.so, which is not '
    print 'automatically detected.'
    print '** For more information, refer to the INSTALL document **'
    Exit(1)
  else:
    conf.env.Append(LIBS = [ bln(conf.env, 'boost_system') ])
    conf.env['PkgCLibs'] += ' -l' + bln(conf.env, 'boost_system')



if not conf.CheckLib(bln(conf.env, 'boost_serialization'), language = 'C++') or \
   not conf.CheckLib(bln(conf.env, 'boost_filesystem'), language = 'C++') or \
   not conf.CheckLib(bln(conf.env, 'boost_thread'), language = 'C++'):
  print 'Please check your Boost installation.'
  print 'Note that if you installed boost from source, libs are '
  print 'named as libboost_LIBNAME-COMPILER-THREAD.so, which is not '
  print 'automatically detected.'
  print '** For more information, refer to the INSTALL document **'
  Exit(1)


conf.env.Append(LIBS = [ bln(conf.env, 'boost_serialization') ])
conf.env['PkgCLibs'] += ' -l' + bln(conf.env, 'boost_serialization')
conf.env.Append(LIBS = [ bln(conf.env, 'boost_iostreams') ])
conf.env['PkgCLibs'] += ' -l' + bln(conf.env, 'boost_iostreams')
conf.env.Append(LIBS = [ bln(conf.env, 'boost_thread') ])
conf.env['PkgCLibs'] += ' -l' + bln(conf.env, 'boost_thread')
conf.env.Append(LIBS = [ bln(conf.env, 'boost_filesystem') ])
conf.env['PkgCLibs'] += ' -l' + bln(conf.env, 'boost_filesystem')

conf.env.Append(CPPDEFINES = ['NUKLEI_TRSL_USE_BSD_BETTER_RANDOM_GENERATOR'])
conf.env['PkgCCflags'] += ' -DNUKLEI_TRSL_USE_BSD_BETTER_RANDOM_GENERATOR'

# OpenCV

if conf.env['UseOpenCV']:
  if not conf.CheckPKG('opencv >= 1.0.0'):
    print 'OpenCV >= 1.0.0 not found.'
    Exit(1)

  opencvdict = conf.env.ParseFlags("!pkg-config --cflags --libs opencv")
  for i in opencvdict['CPPPATH']:
    opencvdict['CCFLAGS'].append('-I' + i)
  opencvdict['CPPPATH'] = []
  conf.env.MergeFlags(opencvdict)
  conf.env.Append(CPPDEFINES = [ 'NUKLEI_USE_OPENCV' ])

  if not conf.CheckCXXHeader('cv.h'):
    print 'Please check your OpenCV installation.'
    print '** For more information, refer to the INSTALL document **'
    Exit(1)

# PCL

if conf.env['UsePCL']:
  if not conf.CheckPKG('pcl_io >= 1.0.0'):
    print 'PCL >= 1.0.0 not found.'
    Exit(1)
  
  pcldict = conf.env.ParseFlags("!pkg-config --cflags --libs pcl_io")
  for i in pcldict['CPPPATH']:
    pcldict['CCFLAGS'].append('-I' + i)
  pcldict['CPPPATH'] = []
  conf.env.MergeFlags(pcldict)
  conf.env.Append(CPPDEFINES = [ 'NUKLEI_USE_PCL' ])
  conf.env['PkgCCflags'] += ' -DNUKLEI_USE_PCL ' + \
                            os.popen("pkg-config --cflags pcl_io").read().rstrip("\n")
  conf.env['PkgCLibs'] += ' ' + os.popen("pkg-config --libs pcl_io").read().rstrip("\n")

  if not conf.CheckCXXHeader('pcl/point_cloud.h'):
    print 'Please check your PCL installation.'
    print '** For more information, refer to the INSTALL document **'
    Exit(1)

# OpenMP

if conf.env['UseOpenMP']:
  conf.env.Append(CPPDEFINES = [ 'NUKLEI_USE_OPENMP' ])
  conf.env.Append(CCFLAGS = [ '-fopenmp' ])
  conf.env.Append(LINKFLAGS = [ '-fopenmp' ])
  conf.env['PkgCCflags'] += ' -DNUKLEI_USE_OPENMP'
  conf.env['PkgCCflags'] += ' -fopenmp'
  conf.env['PkgCLibs'] += ' -fopenmp'


# These are simply expected...
conf.env.Append(LIBS = [ 'X11', 'z', 'pthread', 'm' ])

