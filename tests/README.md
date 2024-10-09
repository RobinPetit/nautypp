# Unit Tests

## Install

To run the tests, you will need [Catch2 (v2.x)](https://github.com/catchorg/Catch2/tree/v2.x).
This is a header-only library that you can install as follows (replace `CATCH2_PATH` by whatever
path you want the header to be in).

```bash
CATCH2_PATH=~/includes/catch2/
mkdir -p $CATCH2_PATH
wget https://github.com/catchorg/Catch2/releases/download/v2.13.10/catch.hpp $CATCH2_PATH
```

## Build

Make sure you have set the environment variables `CPATH` and `LIBRARY_PATH`
so that you have access to `nautypp/nauty.hpp` and `libnautypp.a`.
Be sure that `catch2/catch.hpp` is in `CPATH`.

Build the tests using `make build`.

## Run

To run the tests, execute the command `make run` which will make sure the tests
are built at first.

**Note:** when debugging, it is easier to go back and run `make install_debug`
in the root directory to have the debug symbols, and then to change `-O3` to `-g`
in the test Makefile. This makes life way easier when running `gdb` on the tests.
