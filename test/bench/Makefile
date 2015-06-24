INCLUDE_DIR?=../../build/include

SRC_DIR=src
BUILD_BASE_DIR:=build
BUILD_DIR:=${BUILD_BASE_DIR}/$(shell readlink -nf ${INCLUDE_DIR} | md5sum | cut -f 1 -d ' ' | head -c 10)
OBJ_DIR:=${BUILD_DIR}/obj
BIN_DIR:=${BUILD_DIR}/bin
DEP_DIR:=${BUILD_DIR}/dep
RESULT_DIR:=${BUILD_DIR}/result
BUILD_SUBDIRS:=${OBJ_DIR} ${BIN_DIR} ${DEP_DIR} ${RESULT_DIR}

BENCHMARK_SRCS:=$(wildcard ${SRC_DIR}/*.cpp)
BENCHMARK_OBJS:=$(patsubst ${SRC_DIR}/%.cpp,${OBJ_DIR}/%.o,${BENCHMARK_SRCS})
BENCHMARK_DEPS:=$(patsubst ${OBJ_DIR}/%.o,${DEP_DIR}/%.o.d,${BENCHMARK_OBJS})
BENCHMARK_BINS:=$(patsubst ${OBJ_DIR}/%.o,${BIN_DIR}/%,${BENCHMARK_OBJS})
BENCHMARK_RUNS:=$(addprefix _run.,${BENCHMARK_BINS})

COMPILE_FLAGS:=-DBOOST_POOL_NO_MT -DBOOST_THREAD_MUTEX_HPP # TODO: Only needed for some of the sources.


TIMESTAMP:=$(shell date +'%s%N')


all: run


build: ${BUILD_SUBDIRS} _link_latest.${BUILD_DIR} ${BENCHMARK_BINS}


run: build ${BENCHMARK_RUNS}


$(addprefix _link_latest.,${BUILD_DIR}): _link_latest.%: %
	rm -f $</../latest
	ln -sr $< $</../latest


${BUILD_DIR}:
	mkdir -p $@
	echo '${INCLUDE_DIR}' >$@/include_dir.txt


${BUILD_SUBDIRS}: ${BUILD_DIR}
	mkdir -p $@


${BENCHMARK_RUNS}: _run.%: %
	time -p -v ./$^ 2>&1 | tee -a ${RESULT_DIR}/result_${TIMESTAMP}.txt


${BENCHMARK_OBJS}: ${OBJ_DIR}/%.o: ${SRC_DIR}/%.cpp
	mkdir -p $(dir $(patsubst ${OBJ_DIR}/%.o,${DEP_DIR}/%.o.d,$@))
	g++ -c -o $@ -MMD -MF $(patsubst ${OBJ_DIR}/%.o,${DEP_DIR}/%.o.d,$@) -O3 -DNDEBUG -Wall ${COMPILE_FLAGS} -I${INCLUDE_DIR} $<


${BENCHMARK_BINS}: ${BIN_DIR}/%: ${OBJ_DIR}/%.o
	g++ -o $@ $<


clean:
	rm -rf ${BUILD_BASE_DIR}


.PHONY: all build run clean _link_latest.% _run.%


-include ${BENCHMARK_DEPS}

