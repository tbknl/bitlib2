INCLUDE_DIR?=../../build/include

SRC_DIR=src
BUILD_BASE_DIR:=build
BUILD_DIR:=${BUILD_BASE_DIR}/$(shell readlink -nf ${INCLUDE_DIR} | md5sum | cut -f 1 -d ' ' | head -c 10)
OBJ_DIR:=${BUILD_DIR}/obj
BIN_DIR:=${BUILD_DIR}/bin
DEP_DIR:=${BUILD_DIR}/dep
RESULT_DIR:=${BUILD_DIR}/result
BUILD_SUBDIRS:=${OBJ_DIR} ${BIN_DIR} ${DEP_DIR} ${RESULT_DIR}

TEST_SRCS:=$(wildcard ${SRC_DIR}/*.cpp)
TEST_OBJS:=$(patsubst ${SRC_DIR}/%.cpp,${OBJ_DIR}/%.o,${TEST_SRCS})
TEST_DEPS:=$(patsubst ${OBJ_DIR}/%.o,${DEP_DIR}/%.o.d,${TEST_OBJS})
TEST_BINS:=${BIN_DIR}/bitlib2_unittest
TEST_RUNS:=$(addprefix _run.,${TEST_BINS})


TIMESTAMP:=$(shell date +'%s%N')


all: run


build: ${BUILD_SUBDIRS} _link_latest.${BUILD_DIR} ${TEST_BINS}


run: build ${TEST_RUNS}


$(addprefix _link_latest.,${BUILD_DIR}): _link_latest.%: %
	rm -f $</../latest
	ln -sr $< $</../latest


${BUILD_DIR}:
	mkdir -p $@
	echo '${INCLUDE_DIR}' >$@/include_dir.txt


${BUILD_SUBDIRS}: ${BUILD_DIR}
	mkdir -p $@


${TEST_RUNS}: _run.%: %
	./$< | tee -a ${RESULT_DIR}/result_${TIMESTAMP}.txt


${TEST_OBJS}: ${OBJ_DIR}/%.o: ${SRC_DIR}/%.cpp
	mkdir -p $(dir $(patsubst ${OBJ_DIR}/%.o,${DEP_DIR}/%.o.d,$@))
	g++ -c -o $@ -MMD -MF $(patsubst ${OBJ_DIR}/%.o,${DEP_DIR}/%.o.d,$@) -O0 -g -Wall -I${INCLUDE_DIR} $<


${TEST_BINS}: ${BIN_DIR}/%: ${TEST_OBJS}
	g++ -o $@ $^


clean:
	rm -rf ${BUILD_BASE_DIR}


.PHONY: all build run clean _link_latest.% _run.%


-include ${TEST_DEPS}

