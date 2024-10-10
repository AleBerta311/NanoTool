from ase import Atoms
from ase.io import write
import numpy as np

# Platinum lattice constant in angstroms (Pt FCC lattice constant ~3.92 Å)
a = 3.92
scale_factor = a / np.sqrt(3)  # Scale based on platinum's lattice constant

# Define the tetrahedron vertices (scaled)
vertices = np.array([
    [0, 0, 0],     # Vertex A
    [1, 1, 1],     # Vertex B
    [1, 0, 1],     # Vertex C
    [0, 1, 1]      # Vertex D
]) * scale_factor

# Initialize a list for the atom positions
tetrahedron_atoms = []

# Function to interpolate between two points
def interpolate(p1, p2, num_points):
    return np.linspace(p1, p2, num_points)

# Function to generate atoms within the tetrahedron
def generate_filled_tetrahedron(vertices, num_layers):
    for i in range(len(vertices)):
        for j in range(i + 1, len(vertices)):
            # Create midpoints between each pair of vertices
            mid_points = interpolate(vertices[i], vertices[j], num_layers + 2)[1:-1]  # Skip endpoints
            tetrahedron_atoms.extend(mid_points)

    # Add interior points by creating layers
    for layer in range(1, num_layers + 1):
        for x in np.linspace(0, 1, layer + 2)[1:-1]:
            for y in np.linspace(0, 1, layer + 2)[1:-1]:
                for z in np.linspace(0, 1, layer + 2)[1:-1]:
                    point = np.array([x, y, z])
                    if is_inside_tetrahedron(point):
                        tetrahedron_atoms.append(point * scale_factor)

# Function to check if a point is inside the tetrahedron
def is_inside_tetrahedron(pos):
    A, B, C, D = vertices
    v0 = B - A
    v1 = C - A
    v2 = D - A
    p = pos - A

    d00 = np.dot(v0, v0)
    d01 = np.dot(v0, v1)
    d02 = np.dot(v0, v2)
    d11 = np.dot(v1, v1)
    d12 = np.dot(v1, v2)
    d20 = np.dot(v2, v0)
    d21 = np.dot(v2, v1)

    denom = d00 * (d11 * d20 - d12 * d12) - d01 * (d01 * d20 - d02 * d12) + d02 * (d01 * d21 - d11 * d12)

    if denom == 0:
        return False  # Degenerate case

    # Barycentric coordinates
    v = (d00 * (d11 * np.dot(p, v2) - d12 * np.dot(p, v1)) - d01 * (d01 * np.dot(p, v2) - d02 * np.dot(p, v0)) + d02 * (d01 * np.dot(p, v1) - d11 * np.dot(p, v0))) / denom
    w = (d00 * (d11 * np.dot(p, v1) - d01 * np.dot(p, v2)) - d01 * (d01 * np.dot(p, v0) - d20 * np.dot(p, v2)) + d20 * (d01 * np.dot(p, v1) - d11 * np.dot(p, v0))) / denom
    u = 1 - v - w

    return (v >= 0) and (w >= 0) and (u >= 0)

# Generate the filled tetrahedron
num_layers = 5  # You can adjust this for more or fewer interior atoms
generate_filled_tetrahedron(vertices, num_layers)

# Create an ASE Atoms object with the tetrahedron positions
tetrahedron = Atoms('Pt' * len(tetrahedron_atoms), positions=tetrahedron_atoms)

# Write the filled tetrahedron structure to an xyz file
write('platinum_filled_tetrahedron.xyz', tetrahedron)

print(f'Generated filled tetrahedron with {len(tetrahedron)} atoms.')
