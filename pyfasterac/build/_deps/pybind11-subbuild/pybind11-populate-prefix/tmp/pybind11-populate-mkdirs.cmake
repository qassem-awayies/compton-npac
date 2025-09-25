# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/Users/qassem.awayies/Projects/compton-npac/pyfasterac/build/_deps/pybind11-src")
  file(MAKE_DIRECTORY "/Users/qassem.awayies/Projects/compton-npac/pyfasterac/build/_deps/pybind11-src")
endif()
file(MAKE_DIRECTORY
  "/Users/qassem.awayies/Projects/compton-npac/pyfasterac/build/_deps/pybind11-build"
  "/Users/qassem.awayies/Projects/compton-npac/pyfasterac/build/_deps/pybind11-subbuild/pybind11-populate-prefix"
  "/Users/qassem.awayies/Projects/compton-npac/pyfasterac/build/_deps/pybind11-subbuild/pybind11-populate-prefix/tmp"
  "/Users/qassem.awayies/Projects/compton-npac/pyfasterac/build/_deps/pybind11-subbuild/pybind11-populate-prefix/src/pybind11-populate-stamp"
  "/Users/qassem.awayies/Projects/compton-npac/pyfasterac/build/_deps/pybind11-subbuild/pybind11-populate-prefix/src"
  "/Users/qassem.awayies/Projects/compton-npac/pyfasterac/build/_deps/pybind11-subbuild/pybind11-populate-prefix/src/pybind11-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/qassem.awayies/Projects/compton-npac/pyfasterac/build/_deps/pybind11-subbuild/pybind11-populate-prefix/src/pybind11-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/qassem.awayies/Projects/compton-npac/pyfasterac/build/_deps/pybind11-subbuild/pybind11-populate-prefix/src/pybind11-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
