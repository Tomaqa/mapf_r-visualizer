#!/bin/make
OF_ROOT = ${CURDIR}/third_party/openFrameworks
OF_INCLUDE = $(OF_ROOT)/libs/openFrameworksCompiled/project/makefileCommon/compile.project.mk
PROJECT_EXTERNAL_SOURCE_PATHS = ${CURDIR}/third_party/mapf_r
PROJECT_EXTERNAL_SOURCE_PATHS += ${CURDIR}/third_party/ofxGifEncoder
PROJECT_CFLAGS =
PROJECT_EXCLUSIONS = ${CURDIR}/third_party/mapf_r/data% ${CURDIR}/third_party/mapf_r/misc% ${CURDIR}/third_party/mapf_r/test% ${CURDIR}/third_party/mapf_r/tools% ${CURDIR}/third_party/mapf_r/src/main% ${CURDIR}/third_party/mapf_r/src/test%
PROJECT_EXCLUSIONS += ${CURDIR}/third_party/mapf_r/external/tomaqa/src/main% ${CURDIR}/third_party/mapf_r/external/tomaqa/src/test%
PROJECT_EXCLUSIONS += ${CURDIR}/third_party/mapf_r/external/opensmt%
PROJECT_EXCLUSIONS += ${CURDIR}/third_party/ofxGifEncoder/example%
PROJECT_LDFLAGS = -lz3 -lgomp
PROJECT_LDFLAGS += -lgmpxx -lgmp ${CURDIR}/third_party/mapf_r/lib/libmathsat.a
PROJECT_LDFLAGS += ${CURDIR}/third_party/mapf_r/lib/release/libopensmt.a

include $(OF_INCLUDE)
