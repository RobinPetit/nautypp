CXXFLAGS=-D_Thread_local=thread_local -DHAVE_TLS -Wall -Wextra -std=c++20 -MMD \
		 -fPIC -pthread -fconcepts-diagnostics-depth=3 -Iincludes/
CXXDEBUGFLAGS=-g -DNAUTYPP_DEBUG -fsanitize=address
CXXRELEASEFLAGS=-O3

NAUTY_DEPENDENCIES=${NAUTY_SRC_PATH}/gtnauty.o ${NAUTY_SRC_PATH}/gtools.o ${NAUTY_SRC_PATH}/gutil2.o \
                   ${NAUTY_SRC_PATH}/naugraph.o ${NAUTY_SRC_PATH}/naugroup.o ${NAUTY_SRC_PATH}/naurng.o \
                   ${NAUTY_SRC_PATH}/nausparse.o ${NAUTY_SRC_PATH}/nautil.o ${NAUTY_SRC_PATH}/nautinv.o \
                   ${NAUTY_SRC_PATH}/naututil.o ${NAUTY_SRC_PATH}/nautycliquer.o ${NAUTY_SRC_PATH}/nauty.o \
                   ${NAUTY_SRC_PATH}/schreier.o ${NAUTY_SRC_PATH}/traces.o ${NAUTY_SRC_PATH}/planarity.o

ifeq (${INSTALL_INCLUDES_PATH},)
INSTALL_INCLUDES_PATH=~/includes/nautypp/
endif
ifeq (${INSTALL_LIB_PATH},)
INSTALL_LIB_PATH=~/lib/
endif

DOC_DIR=doc/
REMOTE_ORIGIN=git@github.com:RobinPetit/nautypp.git

ensure_dir=mkdir -p $(@D)

build: build_debug build_release
build_debug: lib/debug/libnautypp.a
build_release: lib/release/libnautypp.a

install: install_release

install_release: build_release
	mkdir -p ${INSTALL_LIB_PATH} && cp lib/release/libnautypp.a ${INSTALL_LIB_PATH}
	mkdir -p ${INSTALL_INCLUDES_PATH} && cp includes/nautypp/*pp ${INSTALL_INCLUDES_PATH}

install_debug: build_debug
	mkdir -p ${INSTALL_LIB_PATH} && cp lib/debug/libnautypp.a ${INSTALL_LIB_PATH}
	mkdir -p ${INSTALL_INCLUDES_PATH} && cp includes/nautypp/*pp ${INSTALL_INCLUDES_PATH}

lib/%/libnautypp.a: obj/%/nautypp.o obj/nauty/geng.o obj/nauty/gentreeg.o obj/nauty/planarity.o ${NAUTY_DEPENDENCIES} ${NAUTY_SRC_PATH}/nauty.a
	$(ensure_dir)
	ar rvs $@ $(filter %.o,$^) $(filter %.a,$^)

obj/debug/%.o: src/%.cpp
	$(ensure_dir)
	${CXX} -c -o $@ $(CXXFLAGS) $(CXXDEBUGFLAGS) $<

obj/release/%.o: src/%.cpp
	$(ensure_dir)
	${CXX} -c -o $@ $(CXXFLAGS) $(CXXRELEASEFLAGS) $<

obj/nauty/geng.o: ${NAUTY_SRC_PATH}/geng.c
	$(ensure_dir)
	${CXX} -c -o $@ $< $(CXXFLAGS) -DGENG_MAIN=_geng_main -DOUTPROC=_geng_callback

obj/nauty/gentreeg.o: ${NAUTY_SRC_PATH}/gentreeg.c
	$(ensure_dir)
	${CXX} -c -o $@ $< $(CXXFLAGS) -DGENTREEG_MAIN=_gentreeg_main -DOUTPROC=_gentreeg_callback

obj/nauty/planarity.o: ${NAUTY_SRC_PATH}/planarity.c
	$(ensure_dir)
	${CXX} -c -o $@ $< $(CXXFLAGS) -g

-include obj/**/*.d

clear:
	rm -f obj/**/*.o
	rm -f obj/**/*.d
	rm -f lib/**/*.a

doc:
	doxygen Doxyfile
	touch ${DOC_DIR}/html/.nojekyll

pushdoc: doc
	cd ${DOC_DIR}/html && \
	if [ ! -d ".git" ]; then git init; git remote add origin ${REMOTE_ORIGIN}; fi && \
	git add . && git commit -m "Build the doc" && git push -f origin HEAD:gh-pages

.PHONY: build clear doc pushdoc
