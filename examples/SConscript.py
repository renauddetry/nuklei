# 
# 
# 
# 
# This file defines how the Nuklei build system should build
# the examples. Users (you) do not need to worry about this
# file to compile your own nuklei-dependent programs, or the
# examples. Each example source file contains a simple
# command line which can be used to build it.
# 
# 
# 
# 

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

## kde_evaluate ################
env = origEnv.Clone()

sources = [ 'kde_evaluate.cpp' ]

target_name = 'kde_evaluate'
target  = os.path.join(env['BinDir'], 'examples', target_name)
product = env.Program(source = sources, target = target)
env.Alias('check', [ 'install', target ], 'cd examples && '
          + product[0].abspath + ' >/tmp/' + target_name + '.output'
          + ' && diff -q /tmp/' + target_name + '.output output/'
          + target_name + '.output')
env.Alias('examples', [ target ])

## kde_sample ################
env = origEnv.Clone()

sources = [ 'kde_sample.cpp' ]

target_name = 'kde_sample'
target  = os.path.join(env['BinDir'], 'examples', target_name)
product = env.Program(source = sources, target = target)
env.Alias('check', [ 'install', target ], 'cd examples && '
          + product[0].abspath + ' >/tmp/' + target_name + '.output'
          + ' && test `cat /tmp/' + target_name
          + '.output | wc -l` -eq `cat output/'
          + target_name + '.output | wc -l`')
env.Alias('examples', [ target ])

## klr_classify ################
env = origEnv.Clone()

sources = [ 'klr_classify.cpp' ]

target_name = 'klr_classify'
target  = os.path.join(env['BinDir'], 'examples', target_name)
product = env.Program(source = sources, target = target)
env.Alias('examples', [ target ])
