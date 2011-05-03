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
build_dir = 'scons_build'

# files to be removed when cleaning at top dir
# those files will also not be included in tar backups
dirtyFiles = ['.DS_Store', '.gdb_history']

enable_gpl=1

# Search for all subdirectories containing a SConscript.
parts = []
for root, dirs, files in os.walk('.'):
  if SConscriptName in files:
      parts.append(os.path.normpath(root))
  if '.svn' in dirs:
      dirs.remove('.svn')

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
   build_dir),
  EnumVariable('bt', 'Sets the build type', 'deploy',
               allowed_values = ('deploy', 'develop', 'profile'),
               map={'opt':'deploy', 'debug':'develop'}),
  ('CXX', 'Sets the c++ compiler command', cxx),
  EnumVariable('gpl', 'Activates GPL code', 'yes',
               allowed_values = ('yes', 'no')),
  EnumVariable('qpl', 'Activates QPL code', 'no',
               allowed_values = ('yes', 'no')),
  EnumVariable('use_opencv', 'Enables functions that depend on OpenCV', 'no',
               allowed_values = ('yes', 'no'))
  # nice -n 18 distcc nice -n 18 i686-apple-darwin8-g++-4.0.1
)

opts.Update(env)

env['BuildType'] = env['bt']
env['EnableGPL'] = env['gpl'] == 'yes'
env['EnableQPL'] = env['qpl'] == 'yes'
env['UseOpenCV'] = env['use_opencv'] == 'yes'

# this is obsolete, should not be used.
env['InstallPrefix'] = env['prefix']

red = ''
green = ''
blue = ''
defColor = ''

if 'TERM' in os.environ and os.environ['TERM'] != 'dumb':
  red = '\033[0;31m'
  green = '\033[0;32m'
  blue = '\033[0;34m'
  defColor = '\033[0m'

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

if not env['EnableGPL']:
  print red + "The whole Nuklei codebase is currently distributed under GPL. GPL code thus cannot be disabled." + defColor
  Exit(1)
if env['EnableQPL'] and env['EnableGPL']:
  print red + "Warning: GPL and QPL are incompatible. Programs built with these settings break both GPL and QPL terms." + defColor


opts.Save(conf_file, env)

Help(opts.GenerateHelpText(env))


###############################################################
## basic environment setup (to be completed in SConscript's) ##
###############################################################

sconsign_file = join(env['build_dir'],
                     'sconsign-' + env['PLATFORM'] + '_' + env['BuildType'])
configure_dir = join(env['build_dir'],
                     'sconf_temp-' + env['PLATFORM'] + '_' + env['BuildType'])
configure_log_file = join(env['build_dir'],
                          'config-' + env['PLATFORM'] + '_' + env['BuildType'] + '.log')

env.SConsignFile(sconsign_file)
if ARGUMENTS.get('fullclean', 0):
  dirtyFiles.append(sconsign_file)
  dirtyFiles.append(configure_dir)
  dirtyFiles.append(configure_log_file)

#env.SetOption('implicit_cache', 1)
#env.SetOption('max_drift', 4)

# local products recipients
env['BinDir'] = join('#' + env['build_dir'], 'bin')
env['ObjDir'] = join('#' + env['build_dir'], 'obj')
env['HdrDir'] = join('#' + env['build_dir'], 'include')
env['LibDir'] = join('#' + env['build_dir'], 'lib')
env['DocDir'] = join('#' + env['build_dir'], 'doc')
env['TarDir'] = join('#' + env['build_dir'], 'tar')

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

env['PkgCCflags'] = ''
env['PkgCLibs'] = ' -lnuklei'

if env['EnableGPL']:
  env.Append(CPPDEFINES = 'NUKLEI_ENABLE_GPL')
  env['PkgCCflags'] += " -DNUKLEI_ENABLE_GPL"

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
  ret = conf.TryCompile("""
#include <CGAL/Monge_via_jet_fitting.h>

#ifndef CGAL_USE_LAPACK
#error We need LAPACK-enabled CGAL!
#endif
  int main()
  {
      return 0;
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

env.Append(CPPDEFINES = [ ('BUILDTYPE', "\\\"" + env['BuildType'] + "\\\"") ])

# darwin specific stuff
if env['PLATFORM'] == 'darwin':
  pass
elif env['PLATFORM'] == 'posix':
  env.Append(CCFLAGS = [ '-pthread' ])
  env.Append(LINKFLAGS = [ '-pthread' ])
  # Required by Wm5:
  conf.env.Append(CPPDEFINES = "__LINUX__")
  conf.env['PkgCCflags'] += ' -D__LINUX__'

# build type

extra_cxx_args = [];
if env['CXX'].find('clang++') >= 0:
  extra_cxx_args = [ '-Wno-mismatched-tags', '-Wno-gnu-designator' ]

if env['BuildType'] == 'deploy':
  env.Append(CCFLAGS = [ '-pipe', '-O3', '-Wall', '-Wno-sign-compare', '-Wno-deprecated' ])
  env.Append(CCFLAGS = extra_cxx_args)
# May bring a 10% speedup:
#  env.Append(CPPDEFINES = [ 'NDEBUG' ])
elif env['BuildType'] == 'profile':
  env.Append(CCFLAGS = [ '-pipe', '-O3' ])
  env.Append(CCFLAGS = [ '-pg' ])
  env.Append(LINKFLAGS = [ '-pg' ])
elif env['BuildType'] == 'develop':
  env.Append(CCFLAGS = [ '-pipe', '-g', '-Wall', '-Wno-sign-compare', '-Wno-deprecated' ])
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
                             varlist = [ 'projectName', 'PkgCCflags', 'PkgCLibs' ])
pcfile = env.Command(join(env['build_dir'], env['projectName'] + '.pc'), None, pkgConfigAction)
env.Install(dir = join(env['LibInstallDir'], 'pkgconfig'), source = pcfile)

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
    env.InstallAs(target = join('$HdrInstallDir', header),
                  source = join(part, header))

libtarget = os.path.join(env['LibDir'], env['projectName'])
library = env.SharedLibrary(source = objects,
                            target = libtarget)
env.Install(dir = '$LibInstallDir', source = library)
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
