from ase import Atom, Atoms, io 
from ase.build import surface, fcc111, bulk
from ase.io import write, read

# Define lattice constants for HCP
a = 2.75  # Lattice constant a in Å
c = 4.34  # Lattice constant c in Å

# Create an hcp bulk structure for Gold (Au) with specified parameters
hcp_bulk = bulk('Ru', 'hcp', a=a, c=c)

# Repeat the structure to create a larger supercell
hcp_bulk = hcp_bulk.repeat((3, 3, 3))

# Generate the hcp(1100) surface
hcp_surface = surface(hcp_bulk, (2, 1, 0), 9)

# Center the surface and add vacuum
hcp_surface.center(vacuum=10, axis=2)

# Write the surface to an XYZ file
write('hcp_surface_210.xyz', hcp_surface)