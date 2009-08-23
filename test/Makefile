# MIN Makefile
#
# File:		Makefile
# Author:	Bob Walton (walton@deas.harvard.edu)
# Date:		Sun Aug 23 10:13:29 EDT 2009
#
# The authors have placed this program in the public
# domain; they make no warranty and accept no liability
# for this program.
#
# RCS Info (may not be true date or author):
#
#   $Author: walton $
#   $Date: 2009/08/23 14:45:28 $
#   $RCSfile: Makefile,v $
#   $Revision: 1.25 $

# We need the following to mask RCS lines
#
D = $$

.SUFFIXES:
.SUFFIXES: .out .test .diff

INCLUDE_FILES = \
	../include/min_parameters.h \
	../include/min.h

LIBRARIES =

TEST_CASES = loose compact \
             loose_stretch compact_stretch \
	     loose_partial compact_partial

INTERFACE_PROGRAMS = \
    ${foreach c,${TEST_CASES},min_interface_test_$c} 
OS_PROGRAMS = \
    min_os_test
ACC_PROGRAMS = \
    min_acc_test

PROGRAMS = ${INTERFACE_PROGRAMS} \
           ${OS_PROGRAMS} \
           ${ACC_PROGRAMS}

.SECONDARY:	${PROGRAMS:=.out}

all:		diff
interface:	interface_diff
os:		os_diff
acc:		acc_diff

diff:		${PROGRAMS:=.diff}
interface_diff:	${INTERFACE_PROGRAMS:=.diff}
os_diff:	${OS_PROGRAMS:=.diff}
acc_diff:	${ACC_PROGRAMS:=.diff}

out:		${PROGRAMS:=.out}
interface_out:	${INTERFACE_PROGRAMS:=.out}
os_out:		${OS_PROGRAMS:=.out}
acc_out:	${ACC_PROGRAMS:=.out}

test:		${PROGRAMS:=.test}
interface_test:	${INTERFACE_PROGRAMS:=.test}
os_test:	${OS_PROGRAMS:=.test}
acc_test:	${ACC_PROGRAMS:=.test}


%.test:	%.out Makefile
	rm -f $*.test
	sed \
	    -e '/\.cc:[0-9]* assert/s//.cc:XXXX assert/' \
	    -e '/\.h:[0-9]* assert/s//.h:XXXX assert/' \
	    -e '/\.cc:[0-9]* desire/s//.cc:XXXX desire/' \
	    -e '/\$DDate: .*\$D/s//\$DRCS Date: XXX\$D/' \
	    -e '/\$DRevision: .*\$D/s//\$DRCS Revision: XXX\$D/' \
	    -e '/\$DRCSfile: .*\$D/s//\$DRCS RCSfile: XXX\$D/' \
            < $*.out > $*.test

%.diff:	%.out
	@echo DIFFING $*.test $*.out
	-@sed \
	    -e '/\.cc:[0-9]* assert/s//.cc:XXXX assert/' \
	    -e '/\.h:[0-9]* assert/s//.h:XXXX assert/' \
	    -e '/\.cc:[0-9]* desire/s//.cc:XXXX desire/' \
	    -e '/\$DDate: .*\$D/s//\$DRCS Date: XXX\$D/' \
	    -e '/\$DRevision: .*\$D/s//\$DRCS Revision: XXX\$D/' \
	    -e '/\$DRCSfile: .*\$D/s//\$DRCS RCSfile: XXX\$D/' \
            < $*.out | diff --minimal $*.test -

%.out:	%
	rm -f $*.out
	${RUNENV} $* > $*.out

min.o:	../src/min.cc
	g++ -I ../include -c ../src/min.cc

# Note: MIN_STUB_BASE if set must be LESS than any
#       data address.

min_interface_test_loose:	\
	GFLAGS = -DMIN_IS_COMPACT=0 \
	         -DMIN_USES_OBJ_AUX_STUBS=1

min_interface_test_compact:	\
	GFLAGS = -DMIN_IS_COMPACT=1 \
	         -DMIN_USES_OBJ_AUX_STUBS=1

N=MIN_ABSOLUTE_MAX_NUMBER_OF_STUBS
min_interface_test_loose_stretch:	\
	GFLAGS = -DMIN_IS_COMPACT=0 \
	         -D$N=0xFFFFFFF0000ull \
		 -DMIN_STUB_BASE=0x100 \
	         -DMIN_USES_OBJ_AUX_STUBS=1

min_interface_test_compact_stretch:	\
	GFLAGS = -DMIN_IS_COMPACT=1 \
	         -D$N=0xDFFFFFFF \
		 -DMIN_STUB_BASE=0x100 \
	         -DMIN_USES_OBJ_AUX_STUBS=1

min_interface_test_loose_partial:	\
	GFLAGS = \
	    -DMIN_IS_COMPACT=0 \
	    -DMIN_ALLOW_PARTIAL_ATTRIBUTE_LABELS=1 \
	    -DMIN_USES_OBJ_AUX_STUBS=1

min_interface_test_compact_partial:	\
	GFLAGS = \
	    -DMIN_IS_COMPACT=1 \
	    -DMIN_ALLOW_PARTIAL_ATTRIBUTE_LABELS=1 \
	    -DMIN_USES_OBJ_AUX_STUBS=1

min_os_test.out:	\
	RUNENV = \
	    MIN_CONFIG="parameter1=123 parameter2=3.21"

min_os_test.dump:	\
	RUNENV = \
	    MIN_CONFIG="parameter1=123 parameter2=3.21"

min_os_test.dump:	min_os_test
	rm -f min_os_test.out
	${RUNENV} min_os_test dump

${INTERFACE_PROGRAMS}:		\
		min_interface_test.cc ../src/min.cc \
		${INCLUDE_FILES} Makefile
	g++ -g ${GFLAGS} \
	    -I ../include \
	    -o $@ \
	    min_interface_test.cc ../src/min.cc

min_os_test:		\
		min_os_test.cc \
		../src/min_os.cc \
		${INCLUDE_FILES} Makefile
	g++ -g ${GFLAGS} \
	    -I ../include \
	    -o $@ \
	    min_os_test.cc

min_acc_test:		\
		min_acc_test.cc \
		../src/min_acc.cc \
		../include/min_acc.h \
		../include/min_acc_parameters.h \
		${INCLUDE_FILES} Makefile
	g++ -g ${GFLAGS} \
	    -I ../include \
	    -o $@ \
	    ../src/min_acc.cc ../src/min.cc \
	    ../src/min_os.cc min_acc_test.cc

clean:
	rm -f ${PROGRAMS} ${PROGRAMS:=.out} min.o
