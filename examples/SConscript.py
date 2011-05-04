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

sources = [ 'evaluate.cpp' ]

target_name = 'evaluate'
target  = os.path.join(env['BinDir'], 'examples', target_name)
product = env.Program(source = sources, target = target)
env.Alias('check', [ target ], 'cd examples && ' + product[0].abspath + '>/tmp/' + target_name + '.output'
          + ' && diff -q /tmp/' + target_name + '.output ../test/data/' + target_name + '.output')
env.Alias('examples', [ target ])

## sample ################
env = origEnv.Clone()

sources = [ 'sample.cpp' ]

target_name = 'sample'
target  = os.path.join(env['BinDir'], 'examples', target_name)
product = env.Program(source = sources, target = target)
env.Alias('check', [ target ], 'cd examples && ' + product[0].abspath + '>/tmp/' + target_name + '.output'
          + ' && test `cat /tmp/' + target_name + '.output | wc -l` -eq `cat ../test/data/' + target_name + '.output | wc -l`')
env.Alias('examples', [ target ])
