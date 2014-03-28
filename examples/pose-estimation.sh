#!/bin/sh

echo "This script aligns the model bottle.xyz to the bottle of tabletop.xyz."

nuklei pe -c 2 data/bottle.xyz data/tabletop.xyz --aligned /tmp/aligned-bottle.txt
nuklei conv -w txt --remove_normals /tmp/aligned-bottle.txt data/tabletop.xyz /tmp/tabletop+bottle.xyz

if hash meshlab 2>/dev/null; then
  echo "Displaying the bottle model aligned to the tabletop bottle."
  meshlab /tmp/tabletop+bottle.xyz
else
  echo "Install Meshlab to view the bottle model aligned to the tabletop bottle."
fi
