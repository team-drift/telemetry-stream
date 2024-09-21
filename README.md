# Drift Telemetry Stream (DTS)

This repo contains libraries that can be used to configure and connect to
a drone via the MAVSDK.
This library will automate many complicated operations and will provide a standard
interface for other DRIFT components.

## Python Bindings

This project contains python bindings that allow python code to interact with drones via the MAVSDK.
It is recommended to create and distribute a so-called 'source distribution',
which contains the files necessary for building a 'wheel'.
Wheels contain the final built binaries/metadata that pip can then install.
The problem with wheels is that they are built for a specific version and architecture,
so it is better to work with source distributions as developers can build it themselves on their own machine.

To generate these distributions, do the following:

1. Ensure python and pip are installed on your system
2. Create a virtual environment: `python -m venv venv/`
3. Activate the virtual environment: `source venv/bin/active`
4. Install the `build` package: `pip install build`
5. Invoke the build package: `python -m build`
6. Results are in `dist/` directory. The `.tar.gz` is the source distribution and can be freely shared.
The `.whl` is the wheel file built for your system, and can be installed in the next step.
7. Install the package via pip: `pip install [NAME].whl` to install the prebuilt wheel. For developers that were given the source distribution, they can run the same command, but specify the `.tar.gz` file instead of the `.whl` file.
