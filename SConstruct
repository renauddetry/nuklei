import os
from os.path import join
import fnmatch
import time

env = Environment(ENV = os.environ)

##############################
## customizables & defaults ##
##############################

# will be used for libprojectName.a/so/dylib, include/projectName, ...
env['projectName'] = 'nuklei'

# default install path is /usr/local
prefix = '/usr/local';

# default c++ compiler command
cxx = 'g++'

# default name format for boost libs
boost_libraries_name_format = '%s'

# file to save config options in
conf_file = env['projectName'] + '-' + env['PLATFORM'] + '.conf'

# .py for python-mode in a lot of editors
SConscriptName = 'SConscript.py'

# directory in which build files will be written
build_prefix = 'scons.build'

# files to be removed when cleaning at top dir
# those files will also not be included in tar backups
dirtyFiles = ['.DS_Store', '.gdb_history']

# Search for all subdirectories containing a SConscript.
parts = []
for root, dirs, files in os.walk('.'):
  if SConscriptName in files:
      parts.append(os.path.normpath(root))
  if '.git' in dirs:
      dirs.remove('.git')
  if '.svn' in dirs:
      dirs.remove('.svn')
parts = list(sorted(parts))
try:
  # speeds up -jN builds by requesting build jobs for complex files first
  reorder_attempt = [ 'libnuklei/kernel', 'libnuklei/base', 'libnuklei/io' ]
  reorder_confirm = [ p for p in reorder_attempt if p in parts]
  parts_hardcoded_order = parts
  for p in reorder_confirm:
    parts_hardcoded_order.remove(p)
  parts_hardcoded_order = reorder_confirm + parts_hardcoded_order
  parts = parts_hardcoded_order
except:
  print 'Could not re-order parts.'

import shlex
import subprocess
import os
import textwrap

red = ''
green = ''
blue = ''
defColor = ''

if 'TERM' in os.environ and os.environ['TERM'] != 'dumb':
  red = '\033[0;31m'
  green = '\033[0;32m'
  blue = '\033[0;34m'
  defColor = '\033[0m'

print 'Welcome to Nuklei.\n' + red + \
  "If this install script doesn't work, please carefully read again the page\n" + \
  '  http://nuklei.sourceforge.net/doxygen/group__install.html' + \
  defColor + '\n' \
  "If installation still doesn't work, feel free to contact me. See http://renaud-detry.net for my email address."


if os.getuid() == 0:
  print red + textwrap.fill("* WARNING * You appear to be running scons as root, probably via sudo. Please keep in mind that by default sudo will not forward your environment variables to SCons. This can be a problem, because Nuklei's SCons script does make use of environment variables to find libraries in nonstandard places. For instance, the ROS PCL package installs PCL in /opt. The ROS init script adds something like PKG_CONFIG_PATH=/opt/lib/pkgconfig to your shell's environment, to allow libraries like Nuklei to find PCL in /opt. To prevent sudo from discarding variables like PKG_CONFIG_PATH, you should use sudo with the '-E' option.") + "\n\nFor instance, use\n  sudo -E ./scons.py install\n***" + defColor

def system_ret(cmd):
  return subprocess.Popen(shlex.split(cmd), stdout = subprocess.PIPE, stderr = subprocess.PIPE).communicate()[0]
try:
  branch = system_ret("git status")
except OSError:
  print "Running Git didn't work. Is git installed?"
  branch = ''

if branch.find("On branch") == 0:
  branch = branch.replace("On branch ", "")
  branch = branch[:branch.find('\n')]
else:
  branch = ""

################
## procedures ##
################

# addfiles: scan procedure, for use in SConscript files.
#           adds to 'sources' all names in 'names'
#           that match 'pattern'
def addfiles(sources, names, pattern):
  for name in names:
    if fnmatch.fnmatch(name, pattern):
      sources.append(name)

# buildPkgConfigFile: see scons Actions for signature definition
def buildPkgConfigFile(target = None, source = None, env = None):
  pkgcName = target[0].abspath
  pkgc = file(pkgcName, 'w')
  pkgc.write('prefix=' + env['prefix'] + '\n' +
             'exec_prefix=${prefix}' + '\n' +
             'libdir=' + env['LibInstallDir'] + '\n' +
             'includedir=' + env['HdrInstallDir'] + '\n\n')

  pkgc.write('Name: ' + env['projectName'] + '\nVersion: 1\n' +
             'Description: Nuklei\n')
  pkgc.write('Cflags: -I${includedir} ')
  pkgc.write(env['PkgCCflags'])
  pkgc.write('\n')
  pkgc.write('Libs: -L${libdir} ')
  pkgc.write(env['PkgCLibs'])
  pkgc.write('\n')
  pkgc.close()
  return 0

# verbosePkgConfigFile: see scons Actions for signature definition
def verbosePkgConfigFile(target, source, env):
    return "Building pkg-config file '%s'" % (target[0])

# buildPkgConfigFile: see scons Actions for signature definition
def buildCMakeConfigFile(target = None, source = None, env = None):

  cmake_header = """
# Config file for the nuklei package
# It defines the following variables
#  NUKLEI_INCLUDE_DIRS  - include directories (Nuklei & header dependencies)
#  NUKLEI_LIBRARIES     - libraries to link against (libnuklei & header dependencies)
#  NUKLEI_DEFINITIONS   - compiler flags
#  NUKLEI_LIBRARY_DIRS  - library directories (Nuklei & header dependencies)
# It also find_library for PCL, if Nuklei uses PCL, and find_package for OpenMP
# if Nuklei uses OpenMP.

# Use as:
#FIND_PACKAGE(nuklei)
#ADD_DEFINITIONS(${NUKLEI_DEFINITIONS})
#INCLUDE_DIRECTORIES(${NUKLEI_INCLUDE_DIRS})
#LINK_DIRECTORIES(${NUKLEI_LIBRARY_DIRS})
#TARGET_LINK_LIBRARIES(target ${NUKLEI_LIBRARIES})

"""

  cmake_find_openmp_commands = """
find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()
"""

  cmake_find_pcl_commads = """
find_package(PCL %s REQUIRED COMPONENTS io)
SET(NUKLEI_DEFINITIONS ${PCL_DEFINITIONS} ${NUKLEI_DEFINITIONS})
SET(NUKLEI_INCLUDE_DIRS ${PCL_INCLUDE_DIRS} ${NUKLEI_INCLUDE_DIRS})
SET(NUKLEI_LIBRARY_DIRS ${PCL_LIBRARY_DIRS} ${NUKLEI_LIBRARY_DIRS})
SET(NUKLEI_LIBRARIES ${PCL_LIBRARIES} ${NUKLEI_LIBRARIES})
""" % (env['PCLVersion'])

  pkgcName = target[0].abspath
  pkgc = file(pkgcName, 'w')
  pkgc.write(cmake_header)
  if env['UseOpenMP']:
    pkgc.write(cmake_find_openmp_commands)
  if env['UsePCL']:
    pkgc.write(cmake_find_pcl_commads)

  compiler_options_string = ' '.join(env['PkgCompilerOptions'])
  include_dir_string = ' '.join(env['PkgIncludeDirectories']) + ' ' + env['HdrInstallDir']
  link_dir_string = ' '.join(env['PkgLinkDirectories']) + ' ' + env['LibInstallDir']
  libs_string = ' '.join(env['PkgLibraries'])

  cmake_nuklei_commands = """
SET(NUKLEI_DEFINITIONS %s ${NUKLEI_DEFINITIONS})
SET(NUKLEI_INCLUDE_DIRS %s ${NUKLEI_INCLUDE_DIRS})
SET(NUKLEI_LIBRARY_DIRS %s ${NUKLEI_LIBRARY_DIRS})
SET(NUKLEI_LIBRARIES %s ${NUKLEI_LIBRARIES})
""" % (compiler_options_string, include_dir_string, link_dir_string, libs_string)

  pkgc.write(cmake_nuklei_commands)

  pkgc.write('\n')
  pkgc.close()
  return 0

# verbosePkgConfigFile: see scons Actions for signature definition
def verboseCMakeConfigFile(target, source, env):
    return "Building cmake-config file '%s'" % (target[0])

########################
# command line options #
########################

opts = Variables([ conf_file ])

opts.AddVariables(
  ('prefix', 'Installation path prefix.', prefix),
  ('boost_format',
   'Format string for Boost library names.',
   boost_libraries_name_format),
  ('build_dir',
   'Path to the build directory.',
   build_prefix),
  EnumVariable('bt', 'Sets the build type', 'deploy',
               allowed_values = ('deploy', 'develop', 'profile'),
               map={'opt':'deploy', 'debug':'develop'}),
  ('CXX', 'Sets the c++ compiler command', cxx),
  EnumVariable('qpl', 'Obsolete option. Use use_cgal=yes instead.', 'no',
               allowed_values = ('yes', 'no')),
  EnumVariable('use_openmp', 'Use OpenMP sync instead of pthread mutexes.', 'yes',
               allowed_values = ('yes', 'no')),
  EnumVariable('use_opencv', 'Enables functions that depend on OpenCV', 'no',
               allowed_values = ('yes', 'no')),
  EnumVariable('use_yamlcpp', 'Enables functions that depend on yaml-cpp', 'no',
               allowed_values = ('yes', 'no')),
  EnumVariable('use_cereal', 'Enables the use of bundled Cereal instead of Boost Serialization', 'yes',
               allowed_values = ('yes', 'no')),
  EnumVariable('use_cgal', 'Enables functions that depend on CGAL', 'no',
               allowed_values = ('yes', 'no')),
  EnumVariable('use_pcl', 'Enables functions that depend on PCL', 'no',
               allowed_values = ('yes', 'no')),
  EnumVariable('use_cimg', 'Enables functions that depend on CIMG', 'no',
               allowed_values = ('yes', 'no')),
  EnumVariable('use_ticpp', 'Enables functions that depend on TICPP', 'yes',
               allowed_values = ('yes', 'no')),
  EnumVariable('partial_view', 'Enables code for partial-view--aware pose estimation.', 'no',
               allowed_values = ('yes', 'no')),
  EnumVariable('branch_in_build_dir', 'This option is for Nuklei developers only. ' + \
               'It tells SCons to use the name of the current git branch in build ' + \
               'directory', 'no',
               allowed_values = ('yes', 'no')),
  EnumVariable('static', 'This option is for Nuklei developers only. ' + \
               'It tells SCons to build a static nuklei executable', 'no',
               allowed_values = ('yes', 'no')),
  EnumVariable('volatile_parameters',
               'Refrain from writing configuration parameters to disk.', 'no',
               allowed_values = ('yes', 'no'))

)

opts.Update(env)

env['BuildType'] = env['bt']
env['VolatileParameters'] = env['volatile_parameters'] == 'yes'

if env['qpl'] == 'yes':
  print red + "Warning: qpl=yes is an obsolete build option. The qpl option used " + \
    "to enable the then-QPL-licenced library CGAL. CGAL is now GPL, which makes " + \
    "everything easier. The qpl option is is being removed " + \
    "from your build config file, and replaced by use_cgal=yes. The option qpl=yes " + \
    "is deprecated. Please run scons again to build." + defColor
  env['qpl'] = 'no'
  env['use_cgal'] = 'yes'
  opts.Save(conf_file, env)
  Exit(1)

env['UseCGAL'] = env['use_cgal'] == 'yes'
env['UseOpenMP'] = env['use_openmp'] == 'yes'
env['UseOpenCV'] = env['use_opencv'] == 'yes'
env['UseYamlCPP'] = env['use_yamlcpp'] == 'yes'
env['UseCereal'] = env['use_cereal'] == 'yes'
env['UsePCL'] = env['use_pcl'] == 'yes'
env['UseCIMG'] = env['use_cimg'] == 'yes'
env['UseTICPP'] = env['use_ticpp'] == 'yes'
env['PartialView'] = env['partial_view'] == 'yes'
env['BuildStaticExecutable'] = env['static'] == 'yes'
env['PCLVersion'] = ''

if env['CXX'].find('clang++') >= 0:
  if env['UseOpenMP']:
    print 'Clang does not support OpenMP yet. Disabling OpenMP support.'
    env['UseOpenMP'] = False

# this is obsolete, should not be used.
env['InstallPrefix'] = env['prefix']
env['BranchInBuildDir'] = env['branch_in_build_dir'] == 'yes'
if env['branch_in_build_dir'] == 'yes':
  env['BuildDirectory'] = join(env['build_dir'], branch)
else:
  env['BuildDirectory'] = env['build_dir']

if env['BuildType'] == 'deploy':
  buildColor = green
else:
  buildColor = red

print 'Build of type --<   ' + buildColor + env['BuildType'] + \
      defColor + '   >-- started.'
print 'Install prefix currently set to `' + blue + \
      env['prefix'] + defColor + '\'.'
print 'C++ compiler currently set to `' + blue + \
      env['CXX'] + defColor + '\'.'

if not env['VolatileParameters']:
  opts.Save(conf_file, env)

Help(opts.GenerateHelpText(env))


###############################################################
## basic environment setup (to be completed in SConscript's) ##
###############################################################

sconsign_file = join(env['BuildDirectory'],
                     'sconsign-' + env['PLATFORM'] + '_' + env['BuildType'])
configure_dir = join(env['BuildDirectory'],
                     'sconf_temp-' + env['PLATFORM'] + '_' + env['BuildType'])
configure_log_file = join(env['BuildDirectory'],
                          'config-' + env['PLATFORM'] + '_' + env['BuildType'] + '.log')

env.SConsignFile(sconsign_file)
if ARGUMENTS.get('fullclean', 0):
  dirtyFiles.append(sconsign_file)
  dirtyFiles.append(configure_dir)
  dirtyFiles.append(configure_log_file)

#env.SetOption('implicit_cache', 1)
#env.SetOption('max_drift', 4)

# local products recipients
env['BinDir'] = join('#' + env['BuildDirectory'], 'bin')
env['ObjDir'] = join('#' + env['BuildDirectory'], 'obj')
env['HdrDir'] = join('#' + env['BuildDirectory'], 'include')
env['LibDir'] = join('#' + env['BuildDirectory'], 'lib')
env['DocDir'] = join('#' + env['BuildDirectory'], 'doc')
env['TarDir'] = join('#' + env['BuildDirectory'], 'tar')

# platform/build-dependant local recipients
env['BinDir'] = join(env['BinDir'], env['PLATFORM'], env['BuildType'])
env['ObjDir'] = join(env['ObjDir'], env['PLATFORM'], env['BuildType'])
env['LibDir'] = join(env['LibDir'], env['PLATFORM'], env['BuildType'])

# install products recipients
env['BinInstallDir'] = join(env['prefix'], 'bin')
env['HdrInstallDir'] = join(env['prefix'], 'include')
env['LibInstallDir'] = join(env['prefix'], 'lib')

# `scons install' will install everything in ~/usr/{bin,lib,include}
env.Alias('install', [ '$BinInstallDir', '$HdrInstallDir', '$LibInstallDir' ])
env.Clean('install', join('$HdrInstallDir', '$projectName'))

env['PkgCCflags'] = ' '
env['PkgCLibs'] = ' -lnuklei'
env['PkgCompilerOptions'] = [ ]
env['PkgIncludeDirectories'] = []
env['PkgLibraries'] = [ 'nuklei' ]
env['PkgLinkDirectories'] = []


##############################################
## configuration: checks & pkg-config setup ##
##############################################

proj_root_path = Dir('.').abspath

# CXXCOM defaults to "$CXX -o $TARGET -c $CXXFLAGS $_CCCOMCOM $SOURCES"
# SHCXXCOM defaults to $SHCXX -o $TARGET -c $SHCXXFLAGS $_CCCOMCOM $SOURCES
# This is fine in default setup.
# However, This script follow a time from
#   http://www.scons.org/wiki/GoFastButton
# saying that
#  CPPPATH: normally you tell Scons about include directories by
#  setting the CPPPATH construction variable, which causes SCons to
#  search those directories when doing implicit dependency scans and
#  also includes those directories in the compile command line. If you
#  have header files that never or rarely change (e.g. system headers,
#  or C run-time headers), then you can exclude them from CPPPATH and
#  include them in the CCFLAGS construction variable instead, which
#  causes SCons to ignore those include directories when scanning for
#  implicit dependencies. Carefully tuning the include directories in
#  this way can usually result in a dramatic speed increase with very
#  little loss of accuracy.
# However, for this to work, CCFLAGS (CXXFLAGS) have to come *after*
# CPPFLAGS (which are in $_CCCOMCOM)
if env['CXXCOM'] == "$CXX -o $TARGET -c $CXXFLAGS $_CCCOMCOM $SOURCES":
  env['CXXCOM'] = "$CXX -o $TARGET -c $_CCCOMCOM $CXXFLAGS $SOURCES"
elif env['CXXCOM'] == "$CXX -o $TARGET -c $CXXFLAGS $CCFLAGS $_CCCOMCOM $SOURCES":
  if env['PLATFORM'] == 'darwin':
    env['INCPREFIX'] = '-I' + proj_root_path + os.sep
    env['CXXCOM'] = "$CXX -o $TARGET -c $_CCCOMCOM $CXXFLAGS $CCFLAGS ${SOURCES.abspath}"
  else:
    env['CXXCOM'] = "$CXX -o $TARGET -c $_CCCOMCOM $CXXFLAGS $CCFLAGS $SOURCES"
else:
  print "Unexpected default CXXCOM"
  Exit(1)

if env['SHCXXCOM'] == "$SHCXX -o $TARGET -c $SHCXXFLAGS $_CCCOMCOM $SOURCES":
  env['SHCXXCOM'] = "$SHCXX -o $TARGET -c $_CCCOMCOM $SHCXXFLAGS $SOURCES"
elif env['SHCXXCOM'] == "$SHCXX -o $TARGET -c $SHCXXFLAGS $SHCCFLAGS $_CCCOMCOM $SOURCES":
  if env['PLATFORM'] == 'darwin':
    env['INCPREFIX'] = '-I' + proj_root_path + os.sep
    env['SHCXXCOM'] = "$SHCXX -o $TARGET -c $_CCCOMCOM $SHCXXFLAGS $SHCCFLAGS ${SOURCES.abspath}"
  else:
    env['SHCXXCOM'] = "$SHCXX -o $TARGET -c $_CCCOMCOM $SHCXXFLAGS $SHCCFLAGS $SOURCES"
else:
  print "Unexpected default SHCXXCOM"
  Exit(1)

# process crucial variables from external environ
for K in ['CPPFLAGS', 'CFLAGS', 'CXXFLAGS', 'LDFLAGS', 'CXX']:
  if K in os.environ.keys():
    dict = env.ParseFlags(os.environ[K])
    # These headers are supposed static. Don't check at each build.
    for i in dict['CPPPATH']:
      dict['CCFLAGS'].append('-I' + i)
    dict['CPPPATH'] = []
    env.MergeFlags(dict)

def NumifyBoostVersion(version):
  # Boost versions are in format major.minor.subminor
  v_arr = version.split(".")
  version_n = 0
  if len(v_arr) > 0:
      version_n += int(v_arr[0])*100000
  if len(v_arr) > 1:
      version_n += int(v_arr[1])*100
  if len(v_arr) > 2:
      version_n += int(v_arr[2])
  return version_n

def CheckBoost(context, min_version, max_version):
  min_version_n = NumifyBoostVersion(min_version)
  max_version_n = NumifyBoostVersion(max_version)

  if max_version_n < min_version_n:
    context.Message('Checking for Boost version >= %s... ' % (min_version))
    ret = context.TryCompile("""
#include <boost/version.hpp>

#if BOOST_VERSION < %d
#error Installed boost is too old!
#endif
  int main()
  {
      return 0;
  }
    """ % min_version_n, '.cpp')
    context.Result(ret)
    return ret
  else:
    context.Message('Checking for Boost [%s, %s[... ' % (min_version, max_version))
    ret = context.TryCompile("""
#include <boost/version.hpp>

#if BOOST_VERSION < %d
#error Installed boost is too old!
#endif
#if BOOST_VERSION >= %d
#error Installed boost is too new!
#endif
  int main()
  {
      return 0;
  }
    """ % (min_version_n, max_version_n), '.cpp')
    context.Result(ret)
    return ret


def CheckCGAL_LAPACK(context):
  context.Message( 'Checking for BLAS/LAPACK-enabled CGAL... ' )
  ret = conf.TryLink("""
    #define CGAL_LAPACK_ENABLED
    #include <CGAL/Cartesian.h>
    #include <CGAL/Monge_via_jet_fitting.h>
    #include <vector>

    namespace cgal_jet_fitting_types
    {
    typedef double                   DFT;
    typedef CGAL::Cartesian<DFT>     Data_Kernel;
    typedef Data_Kernel::Point_3     DPoint;
    typedef CGAL::Monge_via_jet_fitting<Data_Kernel> Monge_via_jet_fitting;
    typedef Monge_via_jet_fitting::Monge_form     Monge_form;
    }

    int main()
    {
    using namespace cgal_jet_fitting_types;
    std::vector<DPoint> in_points;
    size_t d_fitting = 4;
    size_t d_monge = 4;

    Monge_form monge_form;
    Monge_via_jet_fitting monge_fit;
    monge_form = monge_fit(in_points.begin(), in_points.end(), d_fitting, d_monge);

    return monge_fit.condition_number();
    }
    """, '.cpp')
  context.Result( ret )
  return ret

def CheckPKGConfig(context, version):
     context.Message( 'Checking for pkg-config... ' )
     ret = context.TryAction('pkg-config --atleast-pkgconfig-version=%s' % version)[0]
     context.Result( ret )
     return ret

def CheckIMConvert(context):
     context.Message( 'Checking for ImageMagick convert... ' )
     ret = context.TryAction('convert --version')[0]
     context.Result( ret )
     return ret

def CheckOctaveAPI(context):
     context.Message( 'Checking for Octave API... ' )
     ret = context.TryAction('octave-config -v')[0]
     context.Result( ret )
     return ret

def CheckPKG(context, name):
     context.Message( 'Checking for %s... ' % name )
     ret = context.TryAction('pkg-config --exists \'%s\'' % name)[0]
     context.Result( ret )
     return ret

env.Append(CCFLAGS = [ '-std=c++11' ])

conf = env.Configure(conf_dir = configure_dir, log_file = configure_log_file,
                     custom_tests = { 'CheckBoost' : CheckBoost,
                                      'CheckPKGConfig' : CheckPKGConfig,
                                      'CheckPKG' : CheckPKG,
                                      'CheckIMConvert': CheckIMConvert,
                                      'CheckOctaveAPI': CheckOctaveAPI,
                                      'CheckCGAL_LAPACK': CheckCGAL_LAPACK })
SConscript('build_scripts/SConscheck.py',
           exports = 'conf')
env = conf.Finish()

env.Append(CPPDEFINES = [ ('NUKLEI_BUILDTYPE', "\\\"" + env['BuildType'] + "\\\"") ])

# darwin specific stuff
if env['PLATFORM'] == 'darwin':
  pass
elif env['PLATFORM'] == 'posix':
  env.Append(CCFLAGS = [ '-pthread' ])
  env.Append(LINKFLAGS = [ '-pthread' ])
  # Required by Wm5:
  conf.env.Append(CPPDEFINES = "__LINUX__")
  conf.env['PkgCCflags'] += ' -D__LINUX__'
  conf.env['PkgCompilerOptions'].append('-D__LINUX__')

# build type

extra_cxx_args =  [ '-Wall', '-Wno-sign-compare', '-Wno-unused-local-typedefs' ]
if env['CXX'].find('clang++') >= 0:
  extra_cxx_args += [ '-ftemplate-depth=256', '-Wno-gnu-designator', '-Wno-undefined-var-template', '-Wno-unused-private-field', '-Wno-deprecated-declarations', '-Wno-redeclared-class-member' ] # '-Wno-sign-compare', '-Wno-deprecated', '-Wno-mismatched-tags', '-Wno-parentheses', '-Wno-unused-local-typedef'
else:
  extra_cxx_args += [ '-Wno-unused-result', '-Wno-maybe-uninitialized' ] # trimesh

if env['BuildType'] == 'deploy':
  env.Append(CCFLAGS = [ '-pipe', '-O3' ])
  env.Append(CCFLAGS = extra_cxx_args)
# May bring a 1% speedup:
#  env.Append(CPPDEFINES = [ 'NDEBUG' ])
elif env['BuildType'] == 'profile':
  env.Append(CCFLAGS = [ '-pipe', '-O3' ])
  env.Append(CCFLAGS = [ '-pg' ])
  env.Append(LINKFLAGS = [ '-pg' ])
elif env['BuildType'] == 'develop':
  env.Append(CCFLAGS = [ '-pipe', '-g' ])
  env.Append(CCFLAGS = extra_cxx_args)
else:
  print 'Unknown build type "' + env['BuildType'] + '".'

###################
## General rules ##
###################

# search tree for dirtyFiles, add those to the default
# clean target (.)
# unused if -c is not in the cmd line opts
if GetOption('clean'):
  filesToClean = []
  for root, dirs, files in os.walk('.'):
    for file in files:
      if file in dirtyFiles:
        filesToClean.append(join(root, file))
  Clean('.', filesToClean)
  if ARGUMENTS.get('fullclean', 0):
    env.Clean('.', ['$BinDir', '$ObjDir', '$LibDir'])

# make a tar of the tree tree to
#    tar/projname--YYYYMMDD-HH-MM.tgz.
# .svn will be omitted, as well as dirtyFiles and XxxDir's
# (BinDir, ObjDir etc...)
# type 'scons tar' to trigger
if env['TarDir'].lstrip('#') in COMMAND_LINE_TARGETS:
  env.Append(TARFLAGS = ['-c', '-z', '-v'], TARSUFFIX = '.tgz')
  filesToTar = []
  for root, dirs, files in os.walk('.'):
    if root == '.':
      for exclude in [env['BinDir'], env['ObjDir'], env['LibDir'],
                      env['TarDir'], env['HdrDir'], env['DocDir']]:
        if exclude.lstrip('#') in dirs:
          dirs.remove(exclude.lstrip('#'))
    for file in files:
      if not file in dirtyFiles:
        filesToTar.append(os.path.normpath(join(root, file)))
    if '.svn' in dirs:
        dirs.remove('.svn')
  env.Tar(join(env['TarDir'].lstrip('#'),
               os.path.basename(os.getcwd())
               + '--'
               + time.strftime('%Y%m%d-%Hh%M', time.localtime())),
          filesToTar)

#######################################
## walk all parts and get them built ##
#######################################

# generation of a .pc file
pkgConfigAction = env.Action(buildPkgConfigFile, verbosePkgConfigFile,
                             varlist = [ 'projectName', 'PkgCCflags', 'PkgCLibs', 'LibInstallDir', 'HdrInstallDir', 'prefix' ])
pcfile = env.Command(join(env['BuildDirectory'], env['projectName'] + '-' + env['PLATFORM'] + '_' + env['BuildType'] + '.pc'), None, pkgConfigAction)
installer = env.InstallAs(target = join(env['LibInstallDir'], 'pkgconfig', env['projectName'] + '.pc'), source = pcfile)
if env['BranchInBuildDir']: AlwaysBuild(installer)

# generation of a .cmake file
cmakeConfigAction = env.Action(buildCMakeConfigFile, verboseCMakeConfigFile,
                             varlist = [ 'projectName', 'PkgCompilerOptions', 'PkgIncludeDirectories', 'PkgLinkDirectories', 'PkgLibraries', 'PCLVersion', 'UseOpenMP', 'UsePCL', 'LibInstallDir', 'HdrInstallDir', 'prefix' ])
cmakefile = env.Command(join(env['BuildDirectory'], env['projectName'] + '-' + env['PLATFORM'] + '_' + env['BuildType'] + '.cmake'), None, cmakeConfigAction)
installer = env.InstallAs(target = join(env['LibInstallDir'], 'cmake', env['projectName'], env['projectName'] + '-config.cmake'), source = cmakefile)
if env['BranchInBuildDir']: AlwaysBuild(installer)

env.Prepend(LIBPATH = [ '$LibDir' ])

objects = []

# for every part, call its SConscript
for part in parts:
  VariantDir(join(env['ObjDir'], part), part, 0)
  names = os.listdir(part)
  part_headers = join(part, env['projectName'])
  if os.path.exists(part_headers):
    for root, dirs, files in os.walk(part_headers):
      for f in files:
        names.append(join(os.path.relpath(root, part), f))
      if '.svn' in dirs:
          dirs.remove('.svn')
  headers = []
  SConscript(join(env['ObjDir'], part, SConscriptName),
             exports = 'env names addfiles objects headers')
  for header in headers:
    installer = env.InstallAs(target = join('$HdrInstallDir', header),
                              source = join(part, header))
    if env['BranchInBuildDir']: AlwaysBuild(installer)

libtarget = os.path.join(env['LibDir'], env['projectName'])
if env['BuildStaticExecutable']:
  library = env.StaticLibrary(source = objects,
                              target = libtarget)
else:
  library = env.SharedLibrary(source = objects,
                              target = libtarget)
installer = env.Install(dir = '$LibInstallDir', source = library)
if env['BranchInBuildDir']: AlwaysBuild(installer)
AlwaysBuild(env.Alias('check'))
AlwaysBuild(env.Alias('examples'))

################
## misc stuff ##
################

#dict = env.Dictionary()
#for K in dict.keys():
#    print '*************************************'
#    print '** ', K, ' *****'
#    print dict[K]
