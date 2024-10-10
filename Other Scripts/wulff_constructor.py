from ase.visualize import view
from wulffpack import SingleCrystal
from ase.io import write
surface_energies = {(1, 0, 0): 1} 
natoms = 3700
particle = SingleCrystal(surface_energies, natoms = natoms)
write('wulff.xyz', particle.atoms)
particle.view()


