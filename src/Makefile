# MIN Language Implementation Src Directory Makefile
#
# File:		Makefile
# Author:	Bob Walton (walton@deas.harvard.edu)
# Date:		Sun May 21 04:45:41 EDT 2006
#
# The authors have placed this program in the public
# domain; they make no warranty and accept no liability
# for this program.
#
# RCS Info (may not be true date or author):
#
#   $Author: walton $
#   $Date: 2006/05/21 09:02:53 $
#   $RCSfile: Makefile,v $
#   $Revision: 1.10 $

.SUFFIXES:
.SUFFIXES: .out .test .diff

INCLUDE_FILES = \
	../include/min_parameters.h \
	../include/min.h

TEST_CASES = loose compact loose_stretch compact_stretch

PROGRAMS = \
    ${foreach c,${TEST_CASES},min_interface_test_$c}

.SECONDARY:	${PROGRAMS:=.out}

all:	${PROGRAMS:=.diff}

test:	${PROGRAMS:=.test}

%.test:	%.out
	rm -f $*.test
	sed -e '/\.cc:[0-9]* assert/s//.cc:XXXX assert/' \
	    -e '/\.cc:[0-9]* desire/s//.cc:XXXX desire/' \
            < $*.out > $*.test

%.diff:	%.out %.test
	sed -e '/\.cc:[0-9]* assert/s//.cc:XXXX assert/' \
	    -e '/\.cc:[0-9]* desire/s//.cc:XXXX desire/' \
            < $*.out | diff - $*.test

%.out:	%
	rm -f $*.out
	$* > $*.out

min.o:	min.cc
	g++ -I ../include -c min.cc

min_interface_test_loose:	\
		min_interface_test.cc min.cc \
		${INCLUDE_FILES}
	g++ -g -DMIN_IS_COMPACT=0 \
	    -DMIN_USES_OBJ_AUX_STUBS=1 \
	    -I ../include \
	    -o min_interface_test_loose \
	    min_interface_test.cc min.cc

min_interface_test_compact:	\
		min_interface_test.cc min.cc \
		${INCLUDE_FILES}
	g++ -g -DMIN_IS_COMPACT=1 \
	    -DMIN_USES_OBJ_AUX_STUBS=1 \
	    -I ../include \
	    -o min_interface_test_compact \
	    min_interface_test.cc min.cc

min_interface_test_loose_stretch:	\
		min_interface_test.cc min.cc \
		${INCLUDE_FILES}
	g++ -g -DMIN_IS_COMPACT=0 -DMIN_STRETCH=1 \
	    -DMIN_USES_OBJ_AUX_STUBS=1 \
	    -I ../include \
	    -o min_interface_test_loose_stretch \
	    min_interface_test.cc min.cc

min_interface_test_compact_stretch:	\
		min_interface_test.cc min.cc \
		${INCLUDE_FILES}
	g++ -g -DMIN_IS_COMPACT=1 -DMIN_STRETCH=1 \
	    -DMIN_USES_OBJ_AUX_STUBS=1 \
	    -I ../include \
	    -o min_interface_test_compact_stretch \
	    min_interface_test.cc min.cc

clean:
	rm -f ${PROGRAMS} ${PROGRAMS:=.out} min.o
