from ase.visualize import view
from wulffpack import SingleCrystal
from ase.io import write
from ase.build import bulk
prim = bulk('Ru',
            crystalstructure='hcp',
            a=2.75, c =4.34 )
surface_energies = {(0,0,1): 2.959,
                    (1,0,0): 3.316,
                    (1,0,1): 3.282,
                    (1,0,2): 3.482,
                    (1,0,3): 3.456,
                    (1,1,0):3.786,
                    (1,1,1):3.716,
                    (1,1,2):3.560,
                    (2,0,1): 3.390,
                    (2,1,0):3.725,
                    (2,1,1):3.709}
natoms = 6500
particle = SingleCrystal(surface_energies, primitive_structure = prim, natoms = natoms)
write('wulff.xyz', particle.atoms)
particle.view()