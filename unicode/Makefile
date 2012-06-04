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

all:	make_unicode_class.diff

make_unicode_class:	make_unicode_class.cc
	rm -f make_unicode_class
	g++ ${WFLAGS} -o make_unicode_class \
	    make_unicode_class.cc

make_unicode_class.out:	make_unicode_class
	rm -f unicode_class.h make_unicode_class.out
	make_unicode_class > make_unicode_class.out
	cat make_unicode_class.out

make_unicode_class.test:	make_unicode_class.out
	rm -f make_unicode_class.test
	cp -p make_unicode_class.out \
	      make_unicode_class.test

make_unicode_class.diff:	make_unicode_class.out
	echo DIFFING make_unicode_class.test \
	             make_unicode_class.out
	@diff make_unicode_class.test \
	      make_unicode_class.out

clean:
	rm -f make_unicode_class.out make_unicode_class




