import ase
import ase.cluster
from ase.visualize import view
from ase.cluster.cubic import FaceCenteredCubic
from ase.io import write
surfaces = [(1, 1, 1), (1, -1, 1), (-1, 1, 1), (1, 1, -1), (-1,1,1), (-1,-1,-1)]
# Adjust layers to match tetrahedral symmetry. Use truncation (-1) to cut the unwanted facets.
layers = [30, 5, 5, 5, 5 , 5]  # Choose an appropriate number of layers for each surface
atoms = FaceCenteredCubic('Pt', surfaces, layers)
write('Pt_tetra.xyz', atoms)