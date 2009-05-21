# MIN Makefile
#
# File:		Makefile
# Author:	Bob Walton (walton@deas.harvard.edu)
# Date:		Thu May 21 15:14:02 EDT 2009
#
# The authors have placed this program in the public
# domain; they make no warranty and accept no liability
# for this program.
#
# RCS Info (may not be true date or author):
#
#   $Author: walton $
#   $Date: 2009/05/21 19:19:17 $
#   $RCSfile: Makefile,v $
#   $Revision: 1.18 $

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

PROGRAMS = \
    ${foreach c,${TEST_CASES},min_interface_test_$c}

.SECONDARY:	${PROGRAMS:=.out}

all:	diff

diff:	${PROGRAMS:=.diff}

out:	${PROGRAMS:=.out}

test:	${PROGRAMS:=.test}


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
	$* > $*.out

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

min_interface_test_loose_stretch:	\
	GFLAGS = -DMIN_IS_COMPACT=0 -DMIN_STRETCH=1 \
	         -DMIN_USES_OBJ_AUX_STUBS=1 \
		 -DMIN_STUB_BASE=0x100

min_interface_test_compact_stretch:	\
	GFLAGS = -DMIN_IS_COMPACT=1 -DMIN_STRETCH=1 \
	         -DMIN_USES_OBJ_AUX_STUBS=1 \
		 -DMIN_STUB_BASE=0x100

min_interface_test_loose_partial:	\
	GFLAGS = -DMIN_IS_COMPACT=0 \
		 -DMIN_ALLOW_PARTIAL_ATTRIBUTE_LABELS=1 \
	         -DMIN_USES_OBJ_AUX_STUBS=1

min_interface_test_compact_partial:	\
	GFLAGS = -DMIN_IS_COMPACT=1 \
		 -DMIN_ALLOW_PARTIAL_ATTRIBUTE_LABELS=1 \
	         -DMIN_USES_OBJ_AUX_STUBS=1

${PROGRAMS}:		\
		min_interface_test.cc ../src/min.cc \
		${INCLUDE_FILES} Makefile
	g++ -g ${GFLAGS} \
	    -I ../include \
	    -o $@ \
	    min_interface_test.cc ../src/min.cc

clean:
	rm -f ${PROGRAMS} ${PROGRAMS:=.out} min.o
