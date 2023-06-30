# Burnback-qt

Analysis of 2D burn surfaces (3D only when axisymmetric) for solid propellant rockets using Time Marching Method as an alternative of the Level Set Method. Key features:

- Combustion time computation with triangular meshes for solid propellant rockets
- Choose from triangle based or edge based computation
- Supports per node configuration of the recession speed
- Supports isotropic and anisotropic propellants
- Graphical interface to evaluate the results
- Import and export data of any mesh formats with a python script

For 3D meshes, see [burnback-3d](https://codeberg.org/iff/burnback-3d). It is also advisable to use the 3D version for study purposes, as code is much more cleaner and simpler due to the tetrahedra based implementation.

Built binaries for Windows and MacOS can be found at [releases](https://github.com/iffse/burnback-qt/releases). For Linux is advisable to compile from source as Qt has no compatibility across different distributions (it is also possible to run the Windows binary through Wine, with minor flickers).

Supports both light and dark theme. Should use accordingly to your system theme. If you want dark theme, and it isn't, add `QT_QUICK_CONTROLS_MATERIAL_THEME=Dark` to your environment variables.

![burnback-qt interface](img/interface.png)

## Usage

First, you will need a mesh in order to use the program for analysis. For instance, [Gmsh](https://gmsh.info/) is an open source meshing software that can generate 2D and 3D finite element mesh. The input files for Gmsh is preferably in the `.brep` or `.stp` formats (which most of the CAD software supports). Be sure to place your mesh in the XY plane as the Z coordinate will not be read by Burnback.

Once you have the mesh, you will have to convert it into a Json file with edge based information. A python script that converts Gmsh mesh to this format using [meshio](https://github.com/nschloe/meshio) can be found at tools directory: [mesh_convert.py](./tools/mesh_convert.py). The script is optimized for speed and can convert a mesh with 200K nodes in seconds. If you are using another meshing tool, feel free to edit it (probable you will only need to change the cell names).

When using Gmsh and the script, you can define boundary conditions with physical groups with the following naming conventions in curves:

- `inlet 0`: The boundary is an inlet, where the propellant starts to burn. The number after the condition is the initial condition applied in that boundary.
- `outlet`: Used for boundaries where the combustion ends, like the shell of the container.
- `symmetry 45`: Used to indicate that a boundary defines a symmetry. Numbers after the condition is the angle of the symmetry line with respect to the x asis in degrees.
- `condition`: Placeholder for conditions that should be changed later in Burnback GUI (will be treated as outlet).

In surfaces, you can define recession velocities:

- `recession 1 [0.5 45]` (optional): Used to indicate the recession velocity of a node, defaults to 1. When 3 number are specified, the recession is considered to be anisotropic. The first 2 numbers are the recession velocity in the `x` and `y` direction respectively, and the last is the rotation of the angle (counterclockwise) in degrees.

Everything after the names above will be added to a description field.

When using the axisymmetric option, the axis is the horizontal line (`y = 0`).

Example files of Gmsh can be found at [examples/gmsh](./examples/gmsh). The commands to be executed to obtain the Json file are:
```shell
gmsh -2 mesh.geo
python mesh_convert.py mesh.msh
```

If you want to use the exported results to another format other than Json (for instance `.CGNS`, or `.dat` for TecPlot/ParaView, etc.) you can use the [result_convert.py](./tools/result_convert.py) script. Usage is:
```shell
python result_convert.py result.json output.extension
```
You can provide only the extension name for the output file. In this case the name is inferred from the input file.


## Compiling

Can be compiled by either using command line or using the QtCreator. Binaries should be found at `<Project Dir>/target/debug|release`. When building with QtCreator, `<Project Dir>` equals to where the build location is set.

Qt modules dependencies:

- qt-charts
- qt-declaratives
- qt-quickcontrols2

### Using command line

The project can be easily built with the `Makefile`. The `Makefile` is written with multiplatform compilation in mind, it should work with Linux, macOS, or Microsoft Windows:

- `make run`: Build the debug binary and run
	- `make run-sanitizer`: Build the debug binary with sanitizer and run
- `make debug`: Build the debug binary
- `make release`: Build the release binary

### Using QtCreator

Open `burnback-qt.pro` with QtCreator, set your compiling options if needed, and runs directly by clicking the play button at bottom-left.

