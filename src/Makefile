# MIN Language Implementation Src Directory Makefile
#
# File:		Makefile
# Author:	Bob Walton (walton@deas.harvard.edu)
# Date:		Mon Feb  6 10:01:38 EST 2006
#
# The authors have placed this program in the public
# domain; they make no warranty and accept no liability
# for this program.
#
# RCS Info (may not be true date or author):
#
#   $Author: walton $
#   $Date: 2006/02/06 15:36:36 $
#   $RCSfile: Makefile,v $
#   $Revision: 1.1 $

INCLUDE_FILES = \
	../include/min_parameters.h \
	../include/min.h

all:		min_test_loose \
		min_test_compact \
		min_test_loose_vsns \
		min_test_compact_vsns
	min_test_loose
	min_test_compact
	min_test_loose_vsns
	min_test_compact_vsns

min.o:	min.cc
	g++ -I ../include -c min.cc

min_test_loose:	min_test.cc ${INCLUDE_FILES}
	g++ -DMIN_IS_COMPACT=0 -DMIN_USES_VSNS=0 \
	    -I ../include -o min_test_loose \
	    min_test.cc

min_test_compact:	min_test.cc ${INCLUDE_FILES}
	g++ -DMIN_IS_COMPACT=1 -DMIN_USES_VSNS=0 \
	    -I ../include -o min_test_compact \
	    min_test.cc

min_test_loose_vsns:	min_test.cc ${INCLUDE_FILES}
	g++ -DMIN_IS_COMPACT=0 -DMIN_USES_VSNS=1 \
	    -I ../include -o min_test_loose_vsns \
	    min_test.cc

min_test_compact_vsns:	min_test.cc ${INCLUDE_FILES}
	g++ -DMIN_IS_COMPACT=1 -DMIN_USES_VSNS=1 \
	    -I ../include -o min_test_compact_vsns \
	    min_test.cc
