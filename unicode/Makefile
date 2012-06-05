# UNICODE Makefile
#
# File:		Makefile
# Author:	Bob Walton (walton@acm.org)
# Date:		Tue Jun  5 04:19:32 EDT 2012
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

all:	make_unicode_class.diff utf8test

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

utf8test:	utf8test.cc
	rm -f utf8test
	g++ -o utf8test utf8test.cc -lfontconfig

latin1:
	@utf8test 00A0 00A1 00A2 00A3 00A4 00A5 00A6 00A7
	@utf8test 00A8 00A9 00AA 00AB 00AC 00AD 00AE 00AF
	@utf8test 00B0 00B1 00B2 00B3 00B4 00B5 00B6 00B7
	@utf8test 00B8 00B9 00BA 00BB 00BC 00BD 00BE 00BF
	@utf8test 00C0 00C1 00C2 00C3 00C4 00C5 00C6 00C7
	@utf8test 00C8 00C9 00CA 00CB 00CC 00CD 00CE 00CF
	@utf8test 00D0 00D1 00D2 00D3 00D4 00D5 00D6 00D7
	@utf8test 00D8 00D9 00DA 00DB 00DC 00DD 00DE 00DF
	@utf8test 00E0 00E1 00E2 00E3 00E4 00E5 00E6 00E7
	@utf8test 00E8 00E9 00EA 00EB 00EC 00ED 00EE 00EF
	@utf8test 00F0 00F1 00F2 00F3 00F4 00F5 00F6 00F7
	@utf8test 00F8 00F9 00FA 00FB 00FC 00FD 00FE 00FF

clean:
	rm -f make_unicode_class.out \
	      make_unicode_class utf8test




