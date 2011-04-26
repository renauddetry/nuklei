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

origEnv.Prepend(CPPPATH = [ '.' ])
origEnv.Prepend(LIBS = [ 'nuklei' ])

## evaluate ################
env = origEnv.Clone()

sources = [ 'nuklei_example_evaluate.cpp' ]

target_name = 'nuklei_example_evaluate'
target  = os.path.join(env['BinDir'], target_name)
product = env.Program(source = sources, target = target)
env.Install(dir = '$BinInstallDir', source = product)
env.Alias(target_name, [ target ])

## sample ################
env = origEnv.Clone()

sources = [ 'nuklei_example_sample.cpp' ]

target_name = 'nuklei_example_sample'
target  = os.path.join(env['BinDir'], target_name)
product = env.Program(source = sources, target = target)
env.Install(dir = '$BinInstallDir', source = product)
env.Alias(target_name, [ target ])

