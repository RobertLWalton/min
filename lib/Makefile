# MIN Lib Makefile
#
# File:		Makefile
# Author:	Bob Walton (walton@acm.org)
# Date:		Sat Nov 28 02:21:05 EST 2020
#
# The authors have placed this program in the public
# domain; they make no warranty and accept no liability
# for this program.

.SUFFIXES:

MAKE = make --no-print-directory

min-all:
	@${MAKE} -C ../unicode
	@${MAKE} libmin.a

# O3 is needed to inline functions.
#
OPTIMIZE = -O3

WFLAGS = -Wall -Wextra \
         -Wno-unused-parameter \
	 -Wno-char-subscripts

O_FILES = min.o min_os.o min_assert.o \
          min_unicode.o min_acc.o

min_assert.o:	../src/min_assert.cc \
		../include/min_parameters.h \
		Makefile
	rm -f min_assert.o
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include \
	    -c ../src/min_assert.cc \

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

min_acc.o:	../src/min_acc.cc \
		../include/min_acc.h \
		../include/min_acc_parameters.h \
                ../include/min_parameters.h \
		../include/min.h \
	        ../include/min_unicode.h \
		../unicode/unicode_data.h \
		Makefile
	rm -f min_acc.o
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include \
	    -c ../src/min_acc.cc \

min.o:		../src/min.cc \
                ../include/min_parameters.h \
	        ../include/min.h \
	        ../include/min_unicode.h \
		../unicode/unicode_data.h \
		Makefile
	rm -f min.o
	g++ -g ${GFLAGS} ${WFLAGS} \
	    -I ../include \
	    -c ../src/min.cc \

libmin.a:	${O_FILES}
	rm -f libmin.a
	ar rs libmin.a ${O_FILES}

clean:
	rm -f *.o *.a
