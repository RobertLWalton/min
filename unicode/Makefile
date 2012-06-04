# UNICODE Makefile
#
# File:		Makefile
# Author:	Bob Walton (walton@acm.org)
# Date:		Mon Jun  4 04:46:50 EDT 2012
#
# The authors have placed this program in the public
# domain; they make no warranty and accept no liability
# for this program.
#

# O3 is needed to inline functions.
#
WFLAGS = -Wall -Wextra \
         -Wno-unused-parameter \
	 -Wno-char-subscripts

all:	unicode_class.h

make_unicode_class:	make_unicode_class.cc
	rm -f make_unicode_class
	g++ ${WFLAGS} -o make_unicode_class \
	    make_unicode_class.cc

unicode_class.h:	make_unicode_class
	rm -f unicode_class.h
	make_unicode_class


