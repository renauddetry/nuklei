import os.path
import fnmatch
import shlex
import subprocess
import os

def system_ret(cmd):
  a = shlex.split(cmd)
  po = subprocess.Popen(a, stdout = subprocess.PIPE, stderr = subprocess.PIPE, env=os.environ)
  rv = po.communicate()
  rc = po.returncode
  if rc == 0:
    return rv[0]
  else:
    print rv[0] + "\n" + rv[1]
    raise subprocess.CalledProcessError(rc, a, rv[0] + "\n" + rv[1])

Import('conf')

# Local contributions

if conf.env['UseCIMG']:
  conf.env['CImg_include'] = '#/contrib/CImg/include'
conf.env['tclap_include'] = '#/contrib/tclap-1e1cc4fb/include'
if conf.env['UseTICPP']:
  conf.env['ticpp_include'] = '#/contrib/ticpp-r97/src'
conf.env['trimesh_include'] = '#/contrib/trimesh2-2.12/include'
conf.env['libkdtree_include'] = '#/contrib/libkdtree++/include'
conf.env['nanoflann_include'] = '#/contrib/nanoflann/25b54675aa555146afae2724ba2bcb0e3f5f1224/include'
conf.env['libklr_include'] = '#/contrib/libklr-2010_05_07/src'


if not conf.CheckPKGConfig('0.15.0'):
  print 'pkg-config not found.'
  Exit(1)

# Local Headers

conf.env.Prepend(CPPPATH = [ '#libnuklei/contrib/WildMagic5p4',
                             '#libnuklei/contrib/trsl-0.2.2', '#libnuklei/base',
                             '#libnuklei/kernel', '#libnuklei/io' ])
if conf.env['UseCereal']:
  conf.env.Prepend(CPPPATH = [ '#libnuklei/contrib/cereal/v1.2.2' ])
  conf.env.Append(CPPDEFINES = [ 'NUKLEI_USE_CEREAL' ])
  conf.env['PkgCCflags'] += ' -DNUKLEI_USE_CEREAL'
  conf.env['PkgCompilerOptions'].append('-DNUKLEI_USE_CEREAL')

# contrib: libkdtree++

conf.env.Prepend(CPPPATH = [ '$libkdtree_include' ])

# contrib: libklr

conf.env.Prepend(CPPPATH = [ '$libklr_include' ])

# contrib: nanoflann

conf.env.Prepend(CPPPATH = [ '$nanoflann_include' ])

# contrib: ticpp

if conf.env['UseTICPP']:
  conf.env.Append(CPPDEFINES = ['NUKLEI_USE_TICPP'])

# contrib: CImg

if conf.env['UseCIMG']:
  conf.env.Append(CPPDEFINES = ['NUKLEI_USE_CIMG'])
  conf.env.Prepend(CPPPATH = [ '$CImg_include' ])
  conf.env.Append(CPPDEFINES = [ ('cimg_verbosity', 1) ])
  conf.env.Append(CPPDEFINES = [ ('cimg_display', 0) ])

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

# TinyXML-2

if not conf.env['UseTICPP']:
  if not conf.CheckCXXHeader('tinyxml2.h') or \
    not conf.CheckLib('tinyxml2', language = 'C++'):
      print 'If TICPP is disabled, TinyXML-2 is required.'
      print 'Please check your TinyXML-2 installation.'
      print '** For more information, refer to the INSTALL document **'
      Exit(1)
  else:
    conf.env.Append(LIBS = [ 'tinyxml2' ])


# BLAS, LAPACK

if conf.env['PLATFORM'] == 'darwin':
  conf.env.Append(FRAMEWORKS = [ 'Accelerate' ])
elif conf.env['PLATFORM'] == 'posix':
  if not conf.CheckLib('lapack', language = 'C++'):
    print 'A LAPACK library is required.'
    print '** For more information, refer to the INSTALL document **'
    Exit(1)
  conf.env.Append(LIBS = [ 'lapack' ])
  hasABlas = False
  if conf.CheckLib('blas', language = 'C++'):
    conf.env.Append(LIBS = [ 'blas' ])
    hasABlas = True
  if not hasABlas:
    print 'A BLAS library is required.'
    print '** For more information, refer to the INSTALL document **'
    Exit(1)
else:
  print 'Unknown platform.'
  Exit(1)


# CGAL

if conf.env['UseCGAL']:
  if not conf.CheckCXXHeader('CGAL/version.h') or \
     not conf.CheckLib('CGAL', language = 'C++') or \
     not conf.CheckLib('CGAL_Core', language = 'C++'):
    print 'Please check your CGAL installation.'
    print '** For more information, refer to the INSTALL document **'
    Exit(1)
  else:
    # Note: CGAL may need the following:
    # 'CGALcore++', 'CGALPDB', 'mpfr', 'gmpxx', 'gmp'
    conf.env.Append(CPPDEFINES = ['NUKLEI_USE_CGAL'])

    conf.env.Append(LIBS = [ 'CGAL', 'CGAL_Core' ])

    if not conf.CheckLib('gmpxx', language = 'C++') or \
       not conf.CheckLib('mpfr', language = 'C++') or \
       not conf.CheckLib('gmp', language = 'C++'):
      print 'Please check your GMP installation.'
      print '** For more information, refer to the INSTALL document **'
      Exit(1)
    conf.env.Append(LIBS = [ 'gmpxx', 'mpfr', 'gmp' ])
    conf.env.Append(CPPDEFINES = [ 'NUKLEI_USE_PRECISION' ])

    if not conf.CheckPKG('eigen3 >= 3.1.0'):
      print 'Please check your Eigen installation.'
      print '** For more information, refer to the INSTALL document **'
      Exit(1)
    eigen3dict = conf.env.ParseFlags("!pkg-config --cflags --libs eigen3")
    for i in eigen3dict['CPPPATH']:
      eigen3dict['CCFLAGS'].append('-I' + i)
    eigen3dict['CPPPATH'] = []
    conf.env.MergeFlags(eigen3dict)
    conf.env.Append(CPPDEFINES = [ 'NUKLEI_USE_EIGEN3' ])
    if conf.env['PartialView']:
      conf.env.Append(CPPDEFINES = [ 'NUKLEI_HAS_PARTIAL_VIEW' ])

# GSL

#conf.env.Append(LIBS = [ 'gslcblas' ])
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
    conf.env['PkgLibraries'].append(bln(conf.env, 'boost_system'))



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
conf.env['PkgLibraries'].append(bln(conf.env, 'boost_serialization'))
conf.env.Append(LIBS = [ bln(conf.env, 'boost_iostreams') ])
conf.env['PkgCLibs'] += ' -l' + bln(conf.env, 'boost_iostreams')
conf.env['PkgLibraries'].append(bln(conf.env, 'boost_iostreams'))
conf.env.Append(LIBS = [ bln(conf.env, 'boost_thread') ])
conf.env['PkgCLibs'] += ' -l' + bln(conf.env, 'boost_thread')
conf.env['PkgLibraries'].append(bln(conf.env, 'boost_thread'))
conf.env.Append(LIBS = [ bln(conf.env, 'boost_filesystem') ])
conf.env['PkgCLibs'] += ' -l' + bln(conf.env, 'boost_filesystem')
conf.env['PkgLibraries'].append(bln(conf.env, 'boost_filesystem'))

conf.env.Append(CPPDEFINES = ['NUKLEI_TRSL_USE_BSD_BETTER_RANDOM_GENERATOR'])
conf.env['PkgCCflags'] += ' -DNUKLEI_TRSL_USE_BSD_BETTER_RANDOM_GENERATOR'
conf.env['PkgCompilerOptions'].append('-DNUKLEI_TRSL_USE_BSD_BETTER_RANDOM_GENERATOR')

# OpenCV

if conf.env['UseOpenCV']:
  opencvname = 'opencv'
  if not conf.CheckPKG('opencv >= 1.0.0'):
    try:
      allPkgs = system_ret("pkg-config --list-all")
      opencvs = []
      for pkgi in allPkgs.splitlines():
        pkgName = pkgi.split(' ', 1)
        if pkgName:
          pkgName = pkgName[0]
          if 'opencv' in pkgName.lower():
            opencvs.append(pkgName)
      if not opencvs:
        print 'OpenCV >= 1.0.0 not found.'
        Exit(1)
      opencvs.sort()
      if not conf.CheckPKG(opencvs[-1]):
        print 'OpenCV >= 1.0.0 not found.'
        Exit(1)
      opencvname = opencvs[-1]
    except:
      print 'OpenCV >= 1.0.0 not found.'
      Exit(1)

  opencvdict = conf.env.ParseFlags("!pkg-config --cflags --libs " + opencvname)
  for i in opencvdict['CPPPATH']:
    opencvdict['CCFLAGS'].append('-I' + i)
  opencvdict['CPPPATH'] = []
  conf.env.MergeFlags(opencvdict)
  conf.env.Append(CPPDEFINES = [ 'NUKLEI_USE_OPENCV' ])

  if not conf.CheckCXXHeader('opencv2/core/core.hpp'):
    print 'Please check your OpenCV installation.'
    print '** For more information, refer to the INSTALL document **'
    Exit(1)

# yaml-cpp

if conf.env['UseYamlCPP']:
  if not conf.CheckPKG('yaml-cpp >= 0.5'):
    print 'yaml-cpp >= 0.5 not found.'
    Exit(1)
  
  yamlcppdict = conf.env.ParseFlags("!pkg-config --cflags --libs yaml-cpp")
  for i in yamlcppdict['CPPPATH']:
    yamlcppdict['CCFLAGS'].append('-I' + i)
  yamlcppdict['CPPPATH'] = []
  conf.env.MergeFlags(yamlcppdict)
  conf.env.Append(CPPDEFINES = [ 'NUKLEI_USE_YAMLCPP' ])

  if not conf.CheckCXXHeader('yaml-cpp/yaml.h'):
    print 'Please check your yaml-cpp installation.'
    print '** For more information, refer to the INSTALL document **'
    Exit(1)

# PCL

if conf.env['UsePCL']:
  pcl_io = ''
  
  for version in [ '1.9', '1.8', '1.7', '1.6', '1.5', '1.4' ]:
    if conf.CheckPKG('pcl_io-' + version):
      if conf.CheckPKG('pcl_io >= ' + version):
        print "Found two versions of PCL: " + 'pcl_io-' + version + " and " + \
          "pcl_io. Using pcl_io."
        pcl_io = 'pcl_io'
        conf.env['PCLVersion'] = ''
      else:
        pcl_io = 'pcl_io-' + version
        conf.env['PCLVersion'] = version
      break
  else:
    if conf.CheckPKG('pcl_io >= 1.4.0'):
      pcl_io = 'pcl_io'
      conf.env['PCLVersion'] = ''
    else:
      print 'PCL >= 1.4.0 not found.'
      Exit(1)
  pcl_features = 'pcl_features'
  if len(conf.env['PCLVersion']) > 0:
    pcl_features += '-' + conf.env['PCLVersion']
  conf.CheckPKG(pcl_features)

  pcl_kdtree = 'pcl_kdtree'
  if len(conf.env['PCLVersion']) > 0:
    pcl_kdtree += '-' + conf.env['PCLVersion']
  conf.CheckPKG(pcl_kdtree)

  pcl_packages = pcl_io + ' ' + pcl_features + ' ' + pcl_kdtree

  pcldict = conf.env.ParseFlags("!pkg-config --cflags --libs " + pcl_packages)
  for i in pcldict['CPPPATH']:
    pcldict['CCFLAGS'].append('-I' + i)
  pcldict['CPPPATH'] = []
  conf.env.MergeFlags(pcldict)
  conf.env.Append(CPPDEFINES = [ 'NUKLEI_USE_PCL' ])
  conf.env['PkgCCflags'] += ' -DNUKLEI_USE_PCL ' + \
                            os.popen("pkg-config --cflags " + pcl_packages).read().rstrip("\n")
  conf.env['PkgCLibs'] += ' ' + os.popen("pkg-config --libs " + pcl_packages).read().rstrip("\n")

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
  conf.env['PkgCompilerOptions'].append('-DNUKLEI_USE_OPENMP')

  conf.env['PkgCCflags'] += ' -fopenmp'
  conf.env['PkgCLibs'] += ' -fopenmp'


# These are simply expected...
conf.env.Append(LIBS = [ 'z', 'pthread', 'm' ])

