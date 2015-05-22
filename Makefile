SRC_DIR:=src
TEST_DIR:=test
BUILD_DIR:=build
INCLUDE_DIR:=${PWD}/${BUILD_DIR}/include
export INCLUDE_DIR


all: build test


build: $(wildcard ${SRC_DIR}/*.hpp)
	mkdir -p ${INCLUDE_DIR}
	cp -r ${SRC_DIR}/* ${INCLUDE_DIR}/


test: unittest benchmark


unittest: build
	$(MAKE) -C ${TEST_DIR}/unit


benchmark: build
	$(MAKE) -C ${TEST_DIR}/bench


clean:
	rm -rf ${BUILD_DIR}
	$(MAKE) -C ${TEST_DIR}/unit clean
	$(MAKE) -C ${TEST_DIR}/bench clean


.PHONY: all build test unittest benchmark clean

