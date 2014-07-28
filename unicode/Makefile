# UNICODE Makefile
#
# File:		Makefile
# Author:	Bob Walton (walton@acm.org)
# Date:		Mon Jul 28 14:30:18 EDT 2014
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

FROM = http://www.unicode.org/Public/UCD/latest/ucd

DATA = NameAliases.txt \
       PropList.txt \
       UnicodeData.txt


all:	make_unicode_data.diff \
	utf8look

curl:
	rm -f ${DATA}
	make ${DATA}

${DATA}:
	rm -f $@
	curl ${FROM}/$@ >$@

# Derived stuff is no longer used
#
${DERIVED}:
	rm -f $@
	curl ${FROM}/extracted/$@ >$@

make_unicode_data:	make_unicode_data.cc \
			output_unicode_data.cc \
			unicode_data.h \
			Makefile
	rm -f make_unicode_data
	g++ -g ${WFLAGS} -o make_unicode_data \
	    make_unicode_data.cc

make_unicode_data.out \
unicode_data.cc \
unicode_data.dump:	make_unicode_data \
			${DATA}
	rm -f unicode_data.cc unicode_data.dump \
	      make_unicode_data.out
	make_unicode_data unicode_data.cc \
	                  unicode_data.dump \
	                  unicode_data_support_sets.h \
			  > make_unicode_data.out
	cat make_unicode_data.out

make_unicode_data.test:	make_unicode_data.out
	rm -f make_unicode_data.test
	cp -p make_unicode_data.out \
	      make_unicode_data.test

make_unicode_data.diff:	make_unicode_data.out
	echo DIFFING make_unicode_data.test \
	             make_unicode_data.out
	@diff make_unicode_data.test \
	      make_unicode_data.out

print_unicode:	print_unicode.cc
	rm -f print_unicode
	g++ -g -o print_unicode print_unicode.cc \
	    -lfontconfig

utf8look:	utf8look.cc
	rm -f utf8look
	g++ -g -o utf8look utf8look.cc -lfontconfig

latin1look:	latin1look.cc
	rm -f latin1look
	g++ -g -o latin1look latin1look.cc

latin1-utf8:	utf8look
	@utf8look 00A0 00A1 00A2 00A3 00A4 00A5 00A6 00A7
	@utf8look 00A8 00A9 00AA 00AB 00AC 00AD 00AE 00AF
	@utf8look 00B0 00B1 00B2 00B3 00B4 00B5 00B6 00B7
	@utf8look 00B8 00B9 00BA 00BB 00BC 00BD 00BE 00BF
	@utf8look 00C0 00C1 00C2 00C3 00C4 00C5 00C6 00C7
	@utf8look 00C8 00C9 00CA 00CB 00CC 00CD 00CE 00CF
	@utf8look 00D0 00D1 00D2 00D3 00D4 00D5 00D6 00D7
	@utf8look 00D8 00D9 00DA 00DB 00DC 00DD 00DE 00DF
	@utf8look 00E0 00E1 00E2 00E3 00E4 00E5 00E6 00E7
	@utf8look 00E8 00E9 00EA 00EB 00EC 00ED 00EE 00EF
	@utf8look 00F0 00F1 00F2 00F3 00F4 00F5 00F6 00F7
	@utf8look 00F8 00F9 00FA 00FB 00FC 00FD 00FE 00FF

latin1-latin1:	latin1look
	@latin1look 00A0 00A1 00A2 00A3 00A4 00A5 00A6 00A7
	@latin1look 00A8 00A9 00AA 00AB 00AC 00AD 00AE 00AF
	@latin1look 00B0 00B1 00B2 00B3 00B4 00B5 00B6 00B7
	@latin1look 00B8 00B9 00BA 00BB 00BC 00BD 00BE 00BF
	@latin1look 00C0 00C1 00C2 00C3 00C4 00C5 00C6 00C7
	@latin1look 00C8 00C9 00CA 00CB 00CC 00CD 00CE 00CF
	@latin1look 00D0 00D1 00D2 00D3 00D4 00D5 00D6 00D7
	@latin1look 00D8 00D9 00DA 00DB 00DC 00DD 00DE 00DF
	@latin1look 00E0 00E1 00E2 00E3 00E4 00E5 00E6 00E7
	@latin1look 00E8 00E9 00EA 00EB 00EC 00ED 00EE 00EF
	@latin1look 00F0 00F1 00F2 00F3 00F4 00F5 00F6 00F7
	@latin1look 00F8 00F9 00FA 00FB 00FC 00FD 00FE 00FF

clean:
	rm -f make_unicode_data.out \
	      make_unicode_data \
	      utf8look \
	      latin1look \
	      print_unicode
