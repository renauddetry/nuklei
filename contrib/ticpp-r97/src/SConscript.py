import os
import fnmatch
import time

Import([ 'env', 'names', 'addfiles', 'objects', 'headers' ])
# env: the basic env created in the SConstruct
# names: list of the names of all files in the
#        current directory
# addfiles: procedure
#           addfiles(sources, names, pattern)
#           adds to 'sources' all names in 'names'
#           that match 'pattern'
env = env.Clone()

wd = Dir('.').srcnode().path
absWd = '#' + wd
if env['UseTICPP']:
  print 'Scanning ' + wd

  # gather all files to be included in a product.
  # We can choose here to include those manually,
  # or using a pattern-based selection in the
  # current directory.
  sources = [ 'ticpp.cpp',
              'tinystr.cpp',
              'tinyxml.cpp',
              'tinyxmlerror.cpp',
              'tinyxmlparser.cpp']

  env.Prepend(CPPPATH = [ '.' ])
  env.Append(CPPDEFINES = [ 'TIXML_USE_TICPP' ])

  deepObjList = []
  for source in sources:
    deepObjList += env.SharedObject(source)
  objects += Flatten(deepObjList)
