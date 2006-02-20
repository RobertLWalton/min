# MIN Language Implementation Src Directory Makefile
#
# File:		Makefile
# Author:	Bob Walton (walton@deas.harvard.edu)
# Date:		Mon Feb 20 14:11:52 EST 2006
#
# The authors have placed this program in the public
# domain; they make no warranty and accept no liability
# for this program.
#
# RCS Info (may not be true date or author):
#
#   $Author: walton $
#   $Date: 2006/02/20 19:48:07 $
#   $RCSfile: Makefile,v $
#   $Revision: 1.3 $

INCLUDE_FILES = \
	../include/min_parameters.h \
	../include/min.h

all:		min_interface_test_loose \
		min_interface_test_compact \
		min_interface_test_loose_vsns \
		min_interface_test_compact_vsns
	min_interface_test_loose
	min_interface_test_compact
	min_interface_test_loose_vsns
	min_interface_test_compact_vsns

min.o:	min.cc
	g++ -I ../include -c min.cc

min_interface_test_loose:	\
		min_interface_test.cc ${INCLUDE_FILES}
	g++ -g -DMIN_IS_COMPACT=0 -DMIN_USES_VSNS=0 \
	    -I ../include \
	    -o min_interface_test_loose \
	    min_interface_test.cc

min_interface_test_compact:	\
		min_interface_test.cc ${INCLUDE_FILES}
	g++ -g -DMIN_IS_COMPACT=1 -DMIN_USES_VSNS=0 \
	    -I ../include \
	    -o min_interface_test_compact \
	    min_interface_test.cc

min_interface_test_loose_vsns:	\
		min_interface_test.cc ${INCLUDE_FILES}
	g++ -g -DMIN_IS_COMPACT=0 -DMIN_USES_VSNS=1 \
	    -I ../include \
	    -o min_interface_test_loose_vsns \
	    min_interface_test.cc

min_interface_test_compact_vsns:	\
		min_interface_test.cc ${INCLUDE_FILES}
	g++ -g -DMIN_IS_COMPACT=1 -DMIN_USES_VSNS=1 \
	    -I ../include \
	    -o min_interface_test_compact_vsns \
	    min_interface_test.cc

clean:
	rm -f min_interface_test_loose \
	      min_interface_test_compact \
	      min_interface_test_loose_vsns \
	      min_interface_test_compact_vsns
