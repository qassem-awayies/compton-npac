# pyfasterac

C++ wrapper around the FasterAC library, enabling seamless integration with Python through pybind11. 
The wrapper exposes the functionality of the FasterAC library to Python users, allowing for efficient execution of C++ code while maintaining the simplicity and flexibility of Python.
The pybind11 and fasterac dependencies can be downloaded and installed as part of this package (via FetContent and ExternalProject) or local versions can be found.

## Requirements

- cmake 3.16+
- pkgconfig
- python-devel

Note: see Dockferfile for an up-to-date list of dependencies

## Getting started

```bash
mkdir build install
cd build
cmake .. -D CMAKE_INSTALL_PREFIX=../install
make install
```

If you prefer to use a local version of pybind11 or fasterac (for development purposes), you should use the `BUILTIN_XXX` cmake flags instead:

```bash
mkdir build install
cd build
cmake .. -D CMAKE_INSTALL_PREFIX=../install -D BUILTIN_PYBIND11=OFF -D BUILTIN_FASTERAC=OFF
make install
```


## Run the example

```bash
export PYTHONPATH=<path to this package>/install/lib
python test_pyfasterac.py # change the filename in the pythin script before
```