CC     = gcc
CXX    = g++
RM     = rm -rf
MKDIR  = mkdir -p
CP     = cp -r
MV     = mv
LN     = ln -s

#name of executable file to generate
OUTPUT   = 5greplay

#directory where probe will be installed on
ifndef MMT_BASE
  MMT_BASE             := /opt/mmt
  NEED_ROOT_PERMISSION := 1
else
  $(info INFO: Set default folder of MMT to $(MMT_BASE))
endif

# directory where MMT-DPI was installed
MMT_DPI_DIR := $(MMT_BASE)/dpi


#get git version abbrev
GIT_VERSION := $(shell git log --format="%h" -n 1)
VERSION     := 0.0.7

CACHE_LINESIZE := 64 #$(shell getconf LEVEL1_DCACHE_LINESIZE)

#always embeddes libconfuse
LIBS += -l:libconfuse.a

#set of library
LIBS     += -ldl -lpthread
#Jul. 19 2021: we currently use directly the function inside libmmt_tmobile.so

ifdef STATIC_LINK
  CFLAGS += -DSTATIC_LINK
  LIBS   += -l:libxml2.a -l:libicuuc.a -l:libz.a -l:liblzma.a -l:libicudata.a #xml2 and its dependencies
  LIBS   +=  -l:libmmt_core.a -l:libmmt_tcpip.so -l:libmmt_tmobile.a  -l:libsctp.a -l:libpcap.a -l:libmmt_tcpip.a
else
  LIBS   += -l:libmmt_core.so -l:libmmt_tmobile.so -l:libxml2.so -l:libsctp.so -l:libpcap.so -l:libmmt_tcpip.a
endif


CFLAGS   += -fPIC -Wall -DVERSION_NUMBER=\"$(VERSION)\" -DGIT_VERSION=\"$(GIT_VERSION)\" -DLEVEL1_DCACHE_LINESIZE=$(CACHE_LINESIZE) \
				-Wno-unused-variable -Wno-unused-function -Wuninitialized\
				-I/usr/include/libxml2/  -I$(MMT_DPI_DIR)/include  -I$(MMT_DPI_DIR)/include/dpi/mobile 

CLDFLAGS += -L$(MMT_DPI_DIR)/lib -L./plugins -L/usr/local/lib  -L/opt/mmt/plugins

#a specific flag for each .o file
CFLAGS += $(CFLAGS-$@)

#for debuging
ifdef DEBUG
  CFLAGS   += -g -DDEBUG_MODE -O0 -fstack-protector-all
else
  CFLAGS   += -O3
endif

ifdef VALGRIND
  CFLAGS += -DVALGRIND_MODE
endif

#Enable update_rules if this parameter is different 0 
ifndef UPDATE_RULES 
else
  ifneq "$(UPDATE_RULES)" "0"
  		CFLAGS += -DMODULE_ADD_OR_RM_RULES_RUNTIME
  endif
endif

#folders containing source files
SRCDIR := $(abspath ./src )

#list all .c files inside ./src and its sub-folders
LIB_SRCS  := $(shell find $(SRCDIR)/ -type f -name '*.c')
#exclude main.c
#LIB_SRCS  := $(filter-out $(SRCDIR)/main.c, $(LIB_SRCS))
LIB_OBJS  := $(patsubst %.c,%.o, $(LIB_SRCS))

ifndef VERBOSE
  QUIET := @
endif

#make starts with the first target (not targets whose names start with ‘.’). 
# This is called the default goal.
# https://www.gnu.org/software/make/manual/html_node/How-Make-Works.html
all: main


#this is useful when running the tools, such as, gen_dpi, compile_rule
# but libmmt_core, ... are not found by ldd
export LD_LIBRARY_PATH=$(MMT_DPI_DIR)/lib

# check if there exists the folder of MMT-DPI 
$(MMT_DPI_DIR):
	@echo "ERROR: Not found MMT-DPI at folder $(MMT_DPI_DIR).\n"
	@exit 1
	
%.o: %.c
	@echo "[COMPILE] $(notdir $@)"
	$(QUIET) $(CC) -lm $(CFLAGS) $(CLDFLAGS) -c -o $@ $<
	
main:  $(MMT_DPI_DIR) $(LIB_OBJS)
	@echo "[COMPILE] $@"
# When compiling using static link, we need to use g++ as DPI uses stdc++
ifdef STATIC_LINK
	$(QUIET) $(CXX) -std=c++11 -Wl,--export-dynamic -o $(OUTPUT) $(CLDFLAGS) $(LIB_OBJS) $(LIBS)
else
	$(QUIET) $(CC) -Wl,--export-dynamic -o $(OUTPUT) $(CLDFLAGS) $(LIB_OBJS) $(LIBS)
endif
	

# list of sample rules inside ./rules folder
RULE_XML  := $(sort $(wildcard rules/*.xml))
RULE_OBJS := $(patsubst %.xml,%.so, $(RULE_XML))

rules/%.so: main
	$(QUIET) ./$(OUTPUT) compile rules/$*.so rules/$*.xml
	
sample-rules: $(RULE_OBJS)

clean-rules:
	$(QUIET) $(RM) rules/*.so rules/*.o rules/*.c
	
clean: clean-rules
	$(QUIET) $(RM) $(LIB_OBJS) $(OUTPUT) test.* \
			$(RULE_OBJS)

#environment variables
SYS_NAME    = $(shell uname -s)
SYS_VERSION = $(shell uname -p)

#name of package file
ZIP_NAME  ?= 5greplay-$(VERSION)_$(SYS_NAME)_$(SYS_VERSION).tar.gz
DIST_NAME ?= 5greplay-$(VERSION)

dist: sample-rules
	@[ "${STATIC_LINK}" ] || ( echo ">> STATIC_LINK is not set. Do 'make STATIC_LINK=1 dist"; exit 1 )
	$(MKDIR) $(DIST_NAME)
	$(CP) /opt/mmt/plugins ./rules mmt-5greplay.conf $(OUTPUT) $(DIST_NAME)/
	tar -czf $(ZIP_NAME) $(DIST_NAME)

docker-image:
	docker build --tag ${OUTPUT}:${VERSION} . 

