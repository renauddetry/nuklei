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
print 'Scanning ' + wd

# gather all files to be included in a product.
# We can choose here to include those manually,
# or using a pattern-based selection in the
# current directory.
sources = []
addfiles(sources, names, '*.cpp')

env.Prepend(CPPPATH = [ '.' ])
env.Prepend(CPPPATH = [ 'nuklei/Wm5' ])

addfiles(headers, names, 'nuklei/Wm5/*.h')
addfiles(headers, names, 'nuklei/Wm5/*.inl')

deepObjList = []
for source in sources:
  deepObjList += env.SharedObject(source)
objects += Flatten(deepObjList)
