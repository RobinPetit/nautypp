CXXFLAGS=-Wall -Wextra -std=c++20 -MMD -fPIC -pthread -fconcepts-diagnostics-depth=3 -Iincludes/ -I./
CXXDEBUGFLAGS=-g -DPOLYMAKE_DEBUG=1 -DNAUTYPP_DEBUG
CXXRELEASEFLAGS=-O3 -DPOLYMAKE_DEBUG=0

NAUTY_DEPENDENCIES=nauty/gtnauty.o nauty/gtools.o nauty/gutil2.o nauty/naugraph.o nauty/naugroup.o nauty/naurng.o \
                   nauty/nausparse.o nauty/nautil.o nauty/nautinv.o nauty/naututil.o nauty/nautycliquer.o nauty/nauty.o \
				   nauty/schreier.o nauty/traces.o

INSTALL_INCLUDES_PATH=~/includes/
INSTALL_LIB_PATH=~/lib/

build: build_debug build_release
build_debug: lib/debug/libnautypp.a
build_release: lib/release/libnautypp.a

install_release: build_release
	cp lib/release/libnautypp.a ${INSTALL_LIB_PATH}
	cp includes/*.hpp ${INSTALL_INCLUDES_PATH}/nautypp/
	cp nauty/*.h ${INSTALL_INCLUDES_PATH}/nauty/

lib/%/libnautypp.a: obj/%/nautypp.o obj/nauty/geng.o obj/nauty/gentreeg.o ${NAUTY_DEPENDENCIES} includes/nauty.hpp
		ar rvs $@ $(filter %.o,$^) $(filter %.a,$^)

obj/debug/%.o: src/%.cpp
	g++ -c -o $@ $(CXXFLAGS) $(CXXDEBUGFLAGS) $<

obj/release/%.o: src/%.cpp
	g++ -c -o $@ $(CXXFLAGS) $(CXXRELEASEFLAGS) $<

obj/nauty/geng.o: nauty/geng.c
	g++ -c -o $@ $< $(CXXFLAGS) -DGENG_MAIN=_geng_main -DOUTPROC=_geng_callback

obj/nauty/gentreeg.o: nauty/gentreeg.c
	g++ -c -o $@ $< $(CXXFLAGS) -DGENTREEG_MAIN=_gentreeg_main -DOUTPROC=_gentreeg_callback

-include obj/**/*.d

clear:
	rm -f obj/**/*.o
	rm -f lib/**/*.a

.PHONY: build clear
