# MIN Makefile
#
# File:		Makefile
# Author:	Bob Walton (walton@acm.org)
# Date:		Mon Jul 21 14:39:06 EDT 2014
#
# The authors have placed this program in the public
# domain; they make no warranty and accept no liability
# for this program.
#
# We need the following to mask RCS lines
#
D = $$

.SUFFIXES:
.SUFFIXES: .out .test .diff

# O3 is needed to inline functions.
#
OPTIMIZE = -O3

WFLAGS = -Wall -Wextra \
         -Wno-unused-parameter \
	 -Wno-char-subscripts

INCLUDE_FILES = \
	../include/min_parameters.h \
	../include/min.h \
	../include/min_unicode.h

LIBRARIES =

TEST_CASES = loose loose_no_aux compact \
             loose_stretch compact_stretch \
	     loose_partial compact_partial

INTERFACE_PROGRAMS = \
    ${foreach c,${TEST_CASES},min_interface_test_$c} 
OS_PROGRAMS = \
    min_os_test
UNICODE_PROGRAMS = \
    min_unicode_test
BUILTIN_PROGRAMS = \
    min_builtin_test min_builtin_replacement_test
ACC_PROGRAMS = \
    min_acc_test
OPTIMIZATION_NON_PROGRAMS = \
    min_relocation_optimization_test

PROGRAMS = ${INTERFACE_PROGRAMS} \
           ${OS_PROGRAMS} \
           ${UNICODE_PROGRAMS} \
           ${BUILTIN_PROGRAMS} \
           ${ACC_PROGRAMS}

NON_PROGRAMS = ${OPTIMIZATION_NON_PROGRAMS}

TESTS = ${PROGRAMS} ${NON_PROGRAMS}

.SECONDARY:	${TESTS:=.out}

all:		diff
interface:	interface_diff
os:		os_diff
builtin:	builtin_diff
acc:		acc_diff
optimization:	optimization_diff

diff:		${TESTS:=.diff}
interface_diff:	${INTERFACE_PROGRAMS:=.diff}
os_diff:	${OS_PROGRAMS:=.diff}
builtin_diff:	${BUILTIN_PROGRAMS:=.diff}
acc_diff:	${ACC_PROGRAMS:=.diff}
optimization_diff:	${OPTIMIZATION_NON_PROGRAMS:=.diff}

out:		${PROGRAMS:=.out}
interface_out:	${INTERFACE_PROGRAMS:=.out}
os_out:		${OS_PROGRAMS:=.out}
builtin_out:	${BUILTIN_PROGRAMS:=.out}
acc_out:	${ACC_PROGRAMS:=.out}
optimization_out:	${OPTIMIZATION_NON_PROGRAMS:=.out}

test:		${PROGRAMS:=.test}
interface_test:	${INTERFACE_PROGRAMS:=.test}
os_test:	${OS_PROGRAMS:=.test}
builtin_test:	${BUILTIN_PROGRAMS:=.test}
acc_test:	${ACC_PROGRAMS:=.test}
optimization_test:	\
                ${OPTIMIZATION_NON_PROGRAMS:=.test}


%.test:	%.out Makefile
	rm -f $*.test
	sed \
	    -e '/\.cc:[0-9]* assert/s//.cc:XXXX assert/' \
	    -e '/\.h:[0-9]* assert/s//.h:XXXX assert/' \
	    -e '/\.cc:[0-9]* desire/s//.cc:XXXX desire/' \
	    -e '/\.L/s/\(\.L[A-Z][A-Z]*\)[0-9][0-9]*/\1XXX/g' \
	    -e '/\$DDate: .*\$D/s//\$DRCS Date: XXX\$D/' \
	    -e '/\$DRevision: .*\$D/s//\$DRCS Revision: XXX\$D/' \
	    -e '/\$DRCSfile: .*\$D/s//\$DRCS RCSfile: XXX\$D/' \
	    -e '/TRACE: inaccess_pool/s/0x[0-9a-f]*/0xXXXXXXXX/' \
	    -e '/stub [0-9][0-9]*/s//stub XXX/' \
            < $*.out > $*.test

%.diff:	%.out
	@echo DIFFING $*.test $*.out
	-@sed \
	    -e '/\.cc:[0-9]* assert/s//.cc:XXXX assert/' \
	    -e '/\.h:[0-9]* assert/s//.h:XXXX assert/' \
	    -e '/\.cc:[0-9]* desire/s//.cc:XXXX desire/' \
	    -e '/\.L/s/\(\.L[A-Z][A-Z]*\)[0-9][0-9]*/\1XXX/g' \
	    -e '/\$DDate: .*\$D/s//\$DRCS Date: XXX\$D/' \
	    -e '/\$DRevision: .*\$D/s//\$DRCS Revision: XXX\$D/' \
	    -e '/\$DRCSfile: .*\$D/s//\$DRCS RCSfile: XXX\$D/' \
	    -e '/TRACE: inaccess_pool/s/0x[0-9a-f]*/0xXXXXXXXX/' \
	    -e '/stub [0-9][0-9]*/s//stub XXX/' \
            < $*.out | diff -u3 $*.test -

%.out:	%
	rm -f $*.out
	${RUNENV} $* > $*.out

# Note: MIN_STUB_BASE if set must be LESS than any
#       data address.

min_interface_test_loose:	\
	GFLAGS = -DMIN_IS_COMPACT=0 \
	         -DMIN_USE_OBJ_AUX_STUBS=1

min_interface_test_compact:	\
	GFLAGS = -DMIN_IS_COMPACT=1 \
	         -DMIN_USE_OBJ_AUX_STUBS=1

min_interface_test_loose_no_aux:	\
	GFLAGS = -DMIN_IS_COMPACT=0

N=MIN_MAX_NUMBER_OF_STUBS
min_interface_test_loose_stretch:	\
	GFLAGS = -DMIN_IS_COMPACT=0 \
	         -D$N=0xFFFFFFF0000ull \
		 -DMIN_STUB_BASE=0x100 \
	         -DMIN_USE_OBJ_AUX_STUBS=1

min_interface_test_compact_stretch:	\
	GFLAGS = -DMIN_IS_COMPACT=1 \
	         -D$N=0xDFFFFFFF \
		 -DMIN_STUB_BASE=0x100 \
	         -DMIN_USE_OBJ_AUX_STUBS=1

min_interface_test_loose_partial:	\
	GFLAGS = \
	    -DMIN_IS_COMPACT=0 \
	    -DMIN_ALLOW_PARTIAL_ATTR_LABELS=1 \
	    -DMIN_USE_OBJ_AUX_STUBS=1

min_interface_test_compact_partial:	\
	GFLAGS = \
	    -DMIN_IS_COMPACT=1 \
	    -DMIN_ALLOW_PARTIAL_ATTR_LABELS=1 \
	    -DMIN_USE_OBJ_AUX_STUBS=1

min_os_test.out:	\
	RUNENV = \
	    MIN_CONFIG="parameter1=123 parameter2=3.21"

min_os_test.dump:	\
	RUNENV = \
	    MIN_CONFIG="parameter1=123 parameter2=3.21"

min_os_test.dump:	min_os_test
	rm -f min_os_test.out
	${RUNENV} min_os_test dump

min_acc_test.out:	\
	RUNENV = \
	    MIN_CONFIG="debug=pmc"

min_unicode.o:	../src/min_unicode.cc \
		../include/min_unicode.h \
		../unicode/unicode_data.h \
		../unicode/unicode_data.cc
	rm -f min_unicode.o
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include \
	    -c ../src/min_unicode.cc \

min_unicode_test:	min_unicode_test.cc \
			min_unicode.o
	rm -f min_unicode_test
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include \
	    -o min_unicode_test \
	    min_unicode_test.cc min_unicode.o

# We need to make unicode_data.cc and unicode_data_
# support_sets.h using their own names because their
# names are embedded in the files themselves, and
# them mv them to min_unicode_test.{code,header}.
#
min_unicode_test.code \
min_unicode_test.header \
min_unicode_test.dump:	min_unicode_test
	rm -rf min_unicode_test.code \
	       min_unicode_test.header \
	       min_unicode_test.dump \
	       unicode_data.cc \
	       unicode_data_support_sets.h
	min_unicode_test
	mv unicode_data.cc min_unicode_test.code
	mv unicode_data_support_sets.h \
	   min_unicode_test.header


min_unicode_test.diff:	../unicode/unicode_data.cc \
			../unicode/unicode_data.dump \
			min_unicode_test.code \
			min_unicode_test.header \
			min_unicode_test.dump
	@echo DIFFING ../unicode/unicode_data.cc \
	              min_unicode_test.code
	-@diff -u3 \
	              ../unicode/unicode_data.cc \
		      min_unicode_test.code
	@echo DIFFING \
		../unicode/unicode_data_support_sets.h \
	        min_unicode_test.header
	-@diff -u3 \
	        ../unicode/unicode_data_support_sets.h \
	        min_unicode_test.header
	@echo DIFFING ../unicode/unicode_data.dump \
	              min_unicode_test.dump
	-@diff -u3 \
	              ../unicode/unicode_data.dump \
	              min_unicode_test.dump

${INTERFACE_PROGRAMS}:		\
		min_interface_test.cc ../src/min.cc \
		min_unicode.o ${INCLUDE_FILES} Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include \
	    -o $@ \
	    min_interface_test.cc \
	    ../src/min.cc ../src/min_os.cc \
	    min_unicode.o

min_os_test:		\
		min_os_test.cc \
		../src/min_os.cc \
		${INCLUDE_FILES} Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include \
	    -o $@ \
	    min_os_test.cc

min_builtin_test:		\
		min_builtin_test.cc \
		${INCLUDE_FILES} Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include \
	    -o $@ \
	    min_builtin_test.cc

min_builtin_replacement_test:		\
		min_builtin_test.cc \
		${INCLUDE_FILES} Makefile
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include \
	    -o $@ \
	    -DMIN_USE_GNUC_BUILTINS=0 \
	    min_builtin_test.cc

min_acc_test:		\
		min_acc_test.cc \
		../src/min_acc.cc \
		../include/min_acc.h \
		../include/min_acc_parameters.h \
		${INCLUDE_FILES} Makefile \
		../src/min.cc ../src/min_os.cc \
		min_unicode.o
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include \
	    -o $@ \
	    ../src/min_acc.cc ../src/min.cc \
	    min_unicode.o \
	    ../src/min_os.cc min_acc_test.cc

min_relocation_optimization_test.out:	\
		min_relocation_optimization_test.cc \
		${INCLUDE_FILES} Makefile
	rm -f min_relocation_optimization_test.s \
	      min_relocation_optimization_test.out
	g++ -S ${OPTIMIZE} -I ../include \
	       min_relocation_optimization_test.cc
	mv min_relocation_optimization_test.s \
	   min_relocation_optimization_test.out

clean:
	rm -f ${PROGRAMS} ${TESTS:=.out} \
	      *.dump *.code *.header *.o
