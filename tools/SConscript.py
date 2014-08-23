import os
import fnmatch
from os.path import join

Import('env names addfiles')
# env: the basic env created in the SConstruct
# names: list of the names of all files in the
#        current directory
# addfiles: procedure
#           addfiles(sources, names, pattern)
#           adds to 'sources' all names in 'names'
#           that match 'pattern'
origEnv = env.Clone()

wd = Dir('.').srcnode().path
absWd = '#' + wd
print 'Scanning ' + wd

origEnv.Prepend(CPPPATH = [ '$tclap_include' ])

origEnv.Prepend(CPPPATH = [ '.' ])
if env['UseTICPP']:
  origEnv.Prepend(CPPPATH = [ '$ticpp_include' ])

origEnv.Prepend(LIBS = [ 'nuklei' ])

## nuklei ################
env = origEnv.Clone()

sources = [ 'nuklei.cpp', env.Glob('util/*.cpp') ]

target_name = 'nuklei'
target  = os.path.join(env['BinDir'], target_name)
if env['BuildStaticExecutable']:
  env.Append(LIBPATH = [ "/usr/lib/atlas-base/atlas" ])
  env.Append(LINKFLAGS= [ "-static" ])
  product = env.Program(source = sources, target = target)
else:
  product = env.Program(source = sources, target = target)
env.Install(dir = '$BinInstallDir', source = product)
env.Alias(target_name, [ target ])

