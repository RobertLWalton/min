# MIN Language Implementation Src Directory Makefile
#
# File:		Makefile
# Author:	Bob Walton (walton@deas.harvard.edu)
# Date:		Sat May 20 11:29:43 EDT 2006
#
# The authors have placed this program in the public
# domain; they make no warranty and accept no liability
# for this program.
#
# RCS Info (may not be true date or author):
#
#   $Author: walton $
#   $Date: 2006/05/20 20:47:15 $
#   $RCSfile: Makefile,v $
#   $Revision: 1.9 $

.SUFFIXES:
.SUFFIXES: .out .test .diff

INCLUDE_FILES = \
	../include/min_parameters.h \
	../include/min.h

all:		min_interface_test_loose.diff \
		min_interface_test_compact.diff \
		min_interface_test_loose_stretch.diff \
		min_interface_test_compact_stretch.diff

test:		min_interface_test_loose.test \
		min_interface_test_compact.test \
		min_interface_test_loose_stretch.test \
		min_interface_test_compact_stretch.test

.out.test:
	rm -f $*.test
	sed -e '/\.cc:[0-9]* assert/s//.cc:XXXX assert/' \
	    -e '/\.cc:[0-9]* desire/s//.cc:XXXX desire/' \
            < $*.out > $*.test
.out.diff:
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
	rm -f min_interface_test_loose \
	      min_interface_test_compact \
	      min_interface_test_loose_stretch \
	      min_interface_test_compact_stretch \
	      *.o *.out
