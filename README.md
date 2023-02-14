# Burnback-qt

Analysis of burn surfaces for solid propellant rockets using Burnback as an alternative of the Top Level Method

Supports both light and dark theme. Should use accordingly to your system theme. If you want dark theme and it isn't, add `QT_QUICK_CONTROLS_MATERIAL_THEME=Dark` to your environment variables.

## Compiling

Can be compiled by either using command line or using the QtCreator. Binaries should be found at `<Project Dir>/target/debug|release`. When building with QtCreator, `<Project Dir>` equals to where the build location is set.

### Using command line

The project can be easily built with the `Makefile`. The `Makefile` is written with multiplatform compilation in mind, it should work with Linux, macOS, or Microsoft Windows:

- `make run`: Build the debug binary and run
- `make debug`: Build the debug binary
- `make release`: Build the release binary

### Using QtCreator

Open `burnback-qt.pro` with QtCreator, set your compiling options if needed, and runs directly by clicking the play button at bottom-left.

