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
sources = [ "ICP.cc",
            "KDtree.cc",
            "TriMesh_bounding.cc",
            "TriMesh_connectivity.cc",
            "TriMesh_curvature.cc",
            "TriMesh_grid.cc",
            "TriMesh_io.cc",
            "TriMesh_normals.cc",
            "TriMesh_pointareas.cc",
            "TriMesh_stats.cc",
            "TriMesh_tstrips.cc",
            "conn_comps.cc",
            "diffuse.cc",
            "edgeflip.cc",
            "faceflip.cc",
            "filter.cc",
            "lmsmooth.cc",
            "overlap.cc",
            "remove.cc",
            "reorder_verts.cc",
            "shared.cc",
            "subdiv.cc"]

env.Prepend(CPPPATH = [ '#contrib/trimesh2-2.12/include/trimesh' ])

if env['PartialView']:
  deepObjList = []
  for source in sources:
    deepObjList += env.SharedObject(source)
  objects += Flatten(deepObjList)

