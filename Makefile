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
	CXXFLAGS += -O2
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

ifeq ($(ARCH),)
ARCH="aarch64"
endif

UNAME := $(shell uname -n)

ifeq ($(ARCH), aarch64)
CXX = clang++ -Xclang -fcolor-diagnostics
CXXFLAGS += -march=native
endif

ifeq ($(ARCH), mte)
CXXFLAGS += -march=armv8.5-a+memtag -DMTE
CXX = clang++ -Xclang -fcolor-diagnostics
endif

ifeq ($(ARCH), cheri)
#CXX = $(CLANG_PURECAP_PATH)/bin/clang++ -Xclang -fcolor-diagnostics
#CXXFLAGS += -march=morello -mabi=purecap -Wcheri --target=aarch64-linux-musl_purecap --sysroot $(PURECAP_LIBC) -I$(PURECAP_LIBC)/include $(GCC_INCLUDES) -isystem $(CLANG_PURECAP_PATH)/include/ -I$(CLANG_PURECAP_PATH)/lib/clang/15.0.0/include -D_LIBCPP_HAS_MUSL_LIBC -DCHERI -stdlib=libc++
#LDFLAGS += -L$(PURECAP_LIBC)/lib -L$(NIX_LD_LIBRARY_PATH) -L$(LLVM_PATH)/lib
CXX = $(CLANG_PURECAP_PATH)/bin/clang++ -Xclang -fcolor-diagnostics
CXXFLAGS += -lunwind -lc++abi
endif

CXXFLAGS += -std=c++20 -Wall -pthread $(EXTRA_CXXFLAGS) -fsigned-char -Wno-deprecated-volatile -I./
CXXFLAGS += -I../../datastructures/include -I../../utils
LDFLAGS += $(EXTRA_LDFLAGS) -lpthread
LDFLAGS += -L../../datastructures/lib -lclht_lf_$(ARCH) 
SOURCES += $(wildcard core/*.cc)
SOURCES += $(wildcard memsafedb_bench/*.cc)
#memsafedb_bench/art_db.cc memsafedb_bench/hashtable.cc memsafedb_bench/serialize.cc memsafedb_bench/clht_db.cc memsafedb_bench/skiplist_db.cc memsafedb_bench/btree_db.cc memsafedb_bench/link_list_db.cc
OBJECTS += $(SOURCES:.cc=.o)
DEPS += $(SOURCES:.cc=.d)
EXEC = ycsb

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

ifeq ($(ARCH), cheri)
$(EXEC): $(OBJECTS)
	$(CXX) -fuse-ld=ldd -march=morello -mabi=purecap --target=aarch64-linux-musl_purecap \
		-Wl,-rpath,$(PURECAP_LIB)/lib --sysroot $(PURECAP_LIB) -lunwind -lc++abi \
		-rtlib=compiler-rt $^ -o $@ -Wl,--dynamic-linker=$(PURECAP_LIB)/lib/libc.so

.cc.o:
	$(CXX) -c -g -march=morello -mabi=purecap --target=aarch64-linux-musl_purecap --sysroot $(PURECAP_LIB) -o $@ $<

%.d: %.cc
	@$(CXX) -c -g -march=morello -mabi=purecap --target=aarch64-linux-musl_purecap --sysroot $(PURECAP_LIB) -MM -MT '$(<:.cc=.o)' -o $@ $<
else
$(EXEC): $(OBJECTS)
	@$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@
	@echo "  LD      " $@

.cc.o:
	@$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<
	@echo "  CC      " $@

%.d: %.cc
	@$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MM -MT '$(<:.cc=.o)' -o $@ $<
endif

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
