# MIN Makefile
#
# File:		Makefile
# Author:	Bob Walton (walton@deas.harvard.edu)
# Date:		Tue Nov 25 20:51:18 EST 2008
#
# The authors have placed this program in the public
# domain; they make no warranty and accept no liability
# for this program.
#
# RCS Info (may not be true date or author):
#
#   $Author: walton $
#   $Date: 2008/11/26 03:14:00 $
#   $RCSfile: Makefile,v $
#   $Revision: 1.14 $

# We need the following to mask RCS lines
#
D = $$

.SUFFIXES:
.SUFFIXES: .out .test .diff

INCLUDE_FILES = \
	../include/min_parameters.h \
	../include/min.h

LIBRARIES =

TEST_CASES = loose compact loose_stretch compact_stretch

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
	@sed \
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

min_interface_test_loose:	\
		min_interface_test.cc ../src/min.cc \
		${INCLUDE_FILES} Makefile
	g++ -g -DMIN_IS_COMPACT=0 \
	    -DMIN_USES_OBJ_AUX_STUBS=1 \
	    -I ../include \
	    -o min_interface_test_loose \
	    min_interface_test.cc ../src/min.cc

min_interface_test_compact:	\
		min_interface_test.cc ../src/min.cc \
		${INCLUDE_FILES} Makefile
	g++ -g -DMIN_IS_COMPACT=1 \
	    -DMIN_USES_OBJ_AUX_STUBS=1 \
	    -I ../include \
	    -o min_interface_test_compact \
	    min_interface_test.cc ../src/min.cc

min_interface_test_loose_stretch:	\
		min_interface_test.cc ../src/min.cc \
		${INCLUDE_FILES} Makefile
	g++ -g -DMIN_IS_COMPACT=0 -DMIN_STRETCH=1 \
	    -DMIN_USES_OBJ_AUX_STUBS=1 \
	    -I ../include \
	    -o min_interface_test_loose_stretch \
	    min_interface_test.cc ../src/min.cc

min_interface_test_compact_stretch:	\
		min_interface_test.cc ../src/min.cc \
		${INCLUDE_FILES} Makefile
	g++ -g -DMIN_IS_COMPACT=1 -DMIN_STRETCH=1 \
	    -DMIN_USES_OBJ_AUX_STUBS=1 \
	    -I ../include \
	    -o min_interface_test_compact_stretch \
	    min_interface_test.cc ../src/min.cc

clean:
	rm -f ${PROGRAMS} ${PROGRAMS:=.out} min.o
