# Redis Makefile
# Copyright (C) 2009 Salvatore Sanfilippo <antirez at gmail dot com>
# This file is released under the BSD license, see the COPYING file

OPTIMIZATION?=
CFLAGS?=-std=c++0x $(OPTIMIZATION) -Wall $(PROF)
CCLINK?=
DEBUG?=-g -ggdb
INCLUDE=-I.
CCOPT= $(CFLAGS) $(ARCH) $(PROF)

CCCOLOR="\033[34m"
LINKCOLOR="\033[34;1m"
SRCCOLOR="\033[33m"
BINCOLOR="\033[37;1m"
MAKECOLOR="\033[32;1m"
ENDCOLOR="\033[0m"

ifndef V
QUIET_CC = @printf '    %b %b\n' $(CCCOLOR)CXX$(ENDCOLOR) $(SRCCOLOR)$@$(ENDCOLOR);
QUIET_LINK = @printf '    %b %b\n' $(LINKCOLOR)LINK$(ENDCOLOR) $(BINCOLOR)$@$(ENDCOLOR);
endif

TESTPRGNAME = bpt_unit_test bpt_vec_unit_test zbpt_unit_test

OBJ = bpt.o util/benchmark_utils.o zbpt.o
PRGNAME = bpt_cli bpt_dump_numbers data_process bpt_benchmark zbpt_benchmark dbgen zdbgen

DUMP_OBJ = bpt.o util/dump_numbers.o

all: $(PRGNAME)

test:
	$(MAKE) test_bpt test_vec test_zm

test_bpt:
	@-rm -f bpt_unit_test
	$(MAKE) TEST="-DUNIT_TEST" bpt_unit_test
	./bpt_unit_test

test_vec:
	@-rm -f bpt_vec_unit_test
	$(MAKE) TEST="-DUNIT_TEST" bpt_vec_unit_test 
	./bpt_vec_unit_test

test_zm:
	@-rm -f zbpt_unit_test
	$(MAKE) TEST="-DUNIT_TEST" zbpt_unit_test
	./zbpt_unit_test

dbgen:$(OBJ)
	$(QUIET_LINK)$(CXX) -o dbgen $(ORDER) $(SOURCE) $(INCLUDE) $(CCOPT) $(DEBUG) dbgen.cc $(OBJ) $(TEST) $(CCLINK)

zdbgen:$(OBJ)
	$(QUIET_LINK)$(CXX) -o zdbgen  $(ORDER) $(SOURCE) $(INCLUDE) $(CCOPT) $(DEBUG) zdbgen.cc $(OBJ) $(TEST) $(CCLINK)

data_process:
	$(CXX) util/data_process.cpp -o data_process

benchmark: $(OBJ)
	$(QUIET_LINK)$(CXX) -o benchmark  $(ORDER) $(INCLUDE) $(CCOPT) $(DEBUG) benchmark.cc $(OBJ) $(TEST) $(CCLINK)

bpt_benchmark: $(OBJ)
	$(QUIET_LINK)$(CXX) -o bpt_benchmark $(INCLUDE) $(CCOPT) $(DEBUG) bpt_benchmark.cc $(OBJ) $(TEST) $(CCLINK)

zbpt_benchmark: $(OBJ)
	$(QUIET_LINK)$(CXX) -o zbpt_benchmark $(INCLUDE) $(CCOPT) $(DEBUG) zbpt_benchmark.cc $(OBJ) $(TEST) $(CCLINK)

gprof:
	$(MAKE) PROF="-pg"

gcov:
	$(MAKE) PROF="-fprofile-arcs -ftest-coverage"

noopt:
	$(MAKE) OPTIMIZATION=""

clean:
	rm -rf $(PRGNAME) $(TESTPRGNAME) *.o util/*.o

dep:
	$(CC) -MM *.cc

bpt_cli: $(OBJ) util/cli.o
	$(QUIET_LINK)$(CXX) -o bpt_cli $(CCOPT) $(DEBUG) util/cli.o $(OBJ) $(CCLINK)

bpt_unit_test:
	$(QUIET_LINK)$(CXX) -o bpt_unit_test $(INCLUDE) $(CCOPT) $(DEBUG) util/unit_test.cc bpt.cc util/benchmark_utils.cc $(TEST) $(CCLINK)

bpt_vec_unit_test:
	$(QUIET_LINK)$(CXX) -o bpt_vec_unit_test $(INCLUDE) $(CCOPT) $(DEBUG) util/unit_test_vec.cc bpt.cc util/benchmark_utils.cc $(TEST) $(CCLINK)  

bpt_dump_numbers: $(DUMP_OBJ)
	$(QUIET_LINK)$(CXX) -o bpt_dump_numbers $(CCOPT) $(DEBUG) $(DUMP_OBJ) $(CCLINK)

bench_unit_test: ./util/benchmark_utils_test.cc
	$(QUIET_LINK)$(CXX) -o bench_unit_test $(DEBUG) util/benchmark_utils_test.cc util/benchmark_utils.cc $(CCLINK) 

zbpt_unit_test: $(OBJ)
	$(QUIET_LINK)$(CXX) -o zbpt_unit_test $(INCLUDE) $(CCOPT) $(DEBUG) $(TEST) util/unit_test_zvec.cc $(OBJ) $(CCLINK)


%.o: %.cc
	$(QUIET_CC)$(CXX) -o $@ -c $(CFLAGS) $(TEST) $(DEBUG) $(COMPILE_TIME) $<

# Deps (use make dep to generate this)
bpt.o: bpt.cc bpt.h predefined.h
zbpt.o: zbpt.cc zbpt.h
cli.o: cli.cc bpt.h predefined.h
dump_numbers.o: dump_numbers.cc bpt.h predefined.h
unit_test.o: unit_test.cc bpt.h predefined.h
