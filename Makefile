#
#  Makefile
#  YCSB-cpp
#
#  Copyright (c) 2020 Youngjae Lee <ls4154.lee@gmail.com>.
#  Copyright (c) 2014 Jinglei Ren <jinglei@ren.systems>.
#  Modifications Copyright 2023 Chengye YU <yuchengye2013 AT outlook.com>.
#


#---------------------build config-------------------------

# Database bindings
BIND_WIREDTIGER ?= 0
BIND_LEVELDB ?= 0
BIND_ROCKSDB ?= 0
BIND_LMDB ?= 0
BIND_SQLITE ?= 0

# Extra options
DEBUG_BUILD ?=
EXTRA_CXXFLAGS ?=
EXTRA_LDFLAGS ?=

# HdrHistogram for tail latency report
BIND_HDRHISTOGRAM ?= 0
# Build and statically link library, submodule required
BUILD_HDRHISTOGRAM ?= 0

#----------------------------------------------------------

ifeq ($(DEBUG_BUILD), 1)
	CXXFLAGS += -g -O0 -fno-omit-frame-pointer#-fsanitize=address -fsanitize=undefined 
else
	CXXFLAGS += -O2 -g
	CPPFLAGS += -DNDEBUG
endif

ifeq ($(BIND_WIREDTIGER), 1)
	LDFLAGS += -lwiredtiger
	SOURCES += $(wildcard wiredtiger/*.cc)
endif

ifeq ($(BIND_LEVELDB), 1)
	LDFLAGS += -lleveldb
	SOURCES += $(wildcard leveldb/*.cc)
endif

ifeq ($(BIND_ROCKSDB), 1)
	LDFLAGS += -lrocksdb
	SOURCES += $(wildcard rocksdb/*.cc)
endif

ifeq ($(BIND_LMDB), 1)
	LDFLAGS += -llmdb
	SOURCES += $(wildcard lmdb/*.cc)
endif

ifeq ($(BIND_SQLITE), 1)
	LDFLAGS += -lsqlite3
	SOURCES += $(wildcard sqlite/*.cc)
endif

CXXFLAGS += -std=c++20 -Wall $(EXTRA_CXXFLAGS) -pthread -fsigned-char -Wno-deprecated-volatile -I./
CXXFLAGS += -I../include -I../../utils
LDFLAGS += $(EXTRA_LDFLAGS) -lpthread
LDFLAGS += -L../lib -lclht_lf_$(ARCH) 
SOURCES += $(wildcard core/*.cc)
SOURCES += $(wildcard memsafedb_bench/*.cc)
OBJECTS += $(SOURCES:.cc=.o)
DEPS += $(SOURCES:.cc=.d)
EXEC = ycsb

ifeq ($(ARCH),)
ARCH="aarch64"
endif

UNAME := $(shell uname -n)

ifeq ($(ARCH), aarch64)
CXX = clang++ -Xclang -fcolor-diagnostics
CXXFLAGS += -march=native
LDFLAGS += -L/tmp
ifeq ($(UNAME), ace)
CXXFLAGS += -static -stdlib=libc++ -I$(LIBCXX_HDR)/include/c++/v1/ -L$(MUSL_PATH)/lib -nostdlib $(MUSL_PATH)/lib/crt1.o $(MUSL_PATH)/lib/crti.o -DAARCH64_ACE -Wno-unused-command-line-argument 
LDFLAGS += -lc++ -lunwind -lc++abi -Wl,--start-group -lc -lgcc -Wl,--end-group $(MUSL_PATH)/lib/crtn.o
endif
endif

ifeq ($(ARCH), mte)
CXXFLAGS += -march=armv8.5-a+memtag -DMTE
CXX = clang++ -Xclang -fcolor-diagnostics
endif

ifeq ($(ARCH), cheri)
ifeq ($(MORELLO_HOME),)
$(error "Please source /morello/env/morello-sdk first")
endif
CXX = $(MORELLO_HOME)/llvm/bin/clang++ -Xclang -fcolor-diagnostics
CXXFLAGS += -march=morello -mabi=purecap -Wcheri --target=aarch64-linux-musl_purecap -DCHERI --sysroot $(MUSL_HOME)
ADDITIONAL_CXXFLAGS = -nostdlib -static -L$(MORELLO_HOME)/musl/lib -L$(MORELLO_HOME)/gnu/lib/gcc/aarch64-none-linux-gnu/10.1.0/purecap/c64 
ADDITIONAL_CXXFLAGS += $(MORELLO_HOME)/musl/lib/crt1.o $(MORELLO_HOME)/musl/lib/crti.o
LDFLAGS += -lc++ -lunwind -lc++abi -lc -L$(MORELLO_HOME)/llvm/lib/clang/14.0.0/lib/aarch64-unknown-linux-musl_purecapi -lgcc $(MORELLO_HOME)/musl/lib/crtn.o
endif

HDRHISTOGRAM_DIR = HdrHistogram_c
HDRHISTOGRAM_LIB = $(HDRHISTOGRAM_DIR)/src/libhdr_histogram_static.a

ifeq ($(BIND_HDRHISTOGRAM), 1)
ifeq ($(BUILD_HDRHISTOGRAM), 1)
	CXXFLAGS += -I$(HDRHISTOGRAM_DIR)/include
	OBJECTS += $(HDRHISTOGRAM_LIB)
else
	LDFLAGS += -lhdr_histogram
endif
CPPFLAGS += -DHDRMEASUREMENT
endif

all: $(EXEC)

$(EXEC): $(OBJECTS)
	@$(CXX) $(CXXFLAGS) $(ADDITIONAL_CXXFLAGS) $^ $(LDFLAGS) -o $@
	@echo "  LD      " $@

.cc.o:
	@$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<
	@echo "  CC      " $@

%.d: %.cc
	@$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MM -MT '$(<:.cc=.o)' -o $@ $<

$(HDRHISTOGRAM_DIR)/CMakeLists.txt:
	@echo "Download HdrHistogram_c"
	@git submodule update --init

$(HDRHISTOGRAM_DIR)/Makefile: $(HDRHISTOGRAM_DIR)/CMakeLists.txt
	@cmake -DCMAKE_BUILD_TYPE=Release -S $(HDRHISTOGRAM_DIR) -B $(HDRHISTOGRAM_DIR)


$(HDRHISTOGRAM_LIB): $(HDRHISTOGRAM_DIR)/Makefile
	@echo "Build HdrHistogram_c"
	@make -C $(HDRHISTOGRAM_DIR)

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS)
endif

clean:
	find . -name "*.[od]" -delete
	$(RM) $(EXEC)

.PHONY: clean
