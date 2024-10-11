# nautypp

*NautyPP* is a wrapper for [nauty](https://pallini.di.uniroma1.it/) in multithreaded C++ context.
It allows you to test/compute properties on every graphs generated by `geng` or every tree generated by `gentreeg`.
More precisely, you can have one thread running `geng` (or `gentreeg`) and feeding the generated graphs to workers
running some computations.

See the repo on [GitHub](https://github.com/RobinPetit/nautypp).

## Install

Be sure you have downloaded and compiled `nauty` at first (see the README in the downloaded zip for more informations).
**Remark:** when executing `./configure`, be sure to provide the parameter `--enable-tls` to enable *thread local storage*
and to make sure that nauty behaves as expected in a multithreaded context!

Your environment variable `CPATH` must also contain a path to a directory containing a subdirectory `nauty` containing
all the headers of the `nauty` source code.

Make sure you also have set an environment variable `NAUTY_SRC_PATH` containing the path to the source and compiled
files of `nauty`. Example: `export NAUTY_SRC_PATH=~/includes/nauty/`.

If you don't want/can't set an environment variable, you can add it as a variable in the Makefile.

Compilation will create directories `./obj/` and `./lib/` for respectively object files and the compiled static libraries.

Choose where to install the static library and the include files, and modify the variables `INSTALL_INCLUDES_PATH`and
`INSTALL_LIB_PATH` accordingly in the `Makefile`. If the directories do not exist, they will be created.

Then simply run `$ make install_release` to install `nautypp`.

## Examples

When compiling, be sure to correctly link the library and the header files, either using the `-I` and `-L` parameters
with `g++` or by updating the environment variables `CPATH` and `LIBRARY_PATH` (using `export`).

Examples of use can be found in the [`examples`](https://github.com/RobinPetit/nautypp/tree/main/examples) directory.