# MIN Language Implementation Src Directory Makefile
#
# File:		Makefile
# Author:	Bob Walton (walton@deas.harvard.edu)
# Date:		Fri Feb 24 03:09:47 EST 2006
#
# The authors have placed this program in the public
# domain; they make no warranty and accept no liability
# for this program.
#
# RCS Info (may not be true date or author):
#
#   $Author: walton $
#   $Date: 2006/02/24 08:53:53 $
#   $RCSfile: Makefile,v $
#   $Revision: 1.6 $

INCLUDE_FILES = \
	../include/min_parameters.h \
	../include/min.h

all:		min_interface_test_loose \
		min_interface_test_compact \
		min_interface_test_loose_stretch \
		min_interface_test_compact_stretch
	min_interface_test_loose
	min_interface_test_compact
	min_interface_test_loose_stretch
	min_interface_test_compact_stretch

min.o:	min.cc
	g++ -I ../include -c min.cc

min_interface_test_loose:	\
		min_interface_test.cc min.cc \
		${INCLUDE_FILES}
	g++ -g -DMIN_IS_COMPACT=0 \
	    -I ../include \
	    -o min_interface_test_loose \
	    min_interface_test.cc min.cc

min_interface_test_compact:	\
		min_interface_test.cc min.cc \
		${INCLUDE_FILES}
	g++ -g -DMIN_IS_COMPACT=1 \
	    -I ../include \
	    -o min_interface_test_compact \
	    min_interface_test.cc min.cc

min_interface_test_loose_stretch:	\
		min_interface_test.cc min.cc \
		${INCLUDE_FILES}
	g++ -g -DMIN_IS_COMPACT=0 -DMIN_STRETCH=44 \
	    -I ../include \
	    -o min_interface_test_loose_stretch \
	    min_interface_test.cc min.cc

min_interface_test_compact_stretch:	\
		min_interface_test.cc min.cc \
		${INCLUDE_FILES}
	g++ -g -DMIN_IS_COMPACT=1 -DMIN_STRETCH=36 \
	    -I ../include \
	    -o min_interface_test_compact_stretch \
	    min_interface_test.cc min.cc

clean:
	rm -f min_interface_test_loose \
	      min_interface_test_compact \
	      min_interface_test_loose_stretch \
	      min_interface_test_compact_stretch
