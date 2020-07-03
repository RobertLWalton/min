# MIN Lib Makefile
#
# File:		Makefile
# Author:	Bob Walton (walton@acm.org)
# Date:		Fri Jul  3 13:21:11 EDT 2020
#
# The authors have placed this program in the public
# domain; they make no warranty and accept no liability
# for this program.

.SUFFIXES:

# O3 is needed to inline functions.
#
OPTIMIZE = -O3

WFLAGS = -Wall -Wextra \
         -Wno-unused-parameter \
	 -Wno-char-subscripts

INCLUDE_FILES = \
	../include/min_parameters.h \
	../include/min.h \
	../include/min_unicode.h

O_FILES = min.o min_os.o min_assert.o min_unicode.o

min_assert.o:	../src/min_assert.cc \
		../include/min_parameters.h \
		Makefile
	rm -f min_assert.o
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include \
	    -c ../src/min_assert.cc \

../unicode/unicode_data.cc:	\
		../unicode/make_unicode_data.cc \
		../unicode/output_unicode_data.cc \
		../unicode/unicode_data.h \
		../unicode/Makefile
	make -C ../unicode

min_unicode.o:	../src/min_unicode.cc \
		../include/min_unicode.h \
		../unicode/unicode_data.h \
		../unicode/unicode_data.cc \
		../unicode/Makefile \
		Makefile
	rm -f min_unicode.o
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include \
	    -c ../src/min_unicode.cc \

min_os.o:	../src/min_os.cc \
		../include/min_os.h \
		../include/min.h \
		Makefile
	rm -f min_os.o
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include \
	    -c ../src/min_os.cc \

min.o:		${INCLUDE_FILES}

libmin.a:
	rm -f libmin.a
	ar rs libmin.a min.o min_assert.o min_unicode.o

clean:
	rm -f *.o
