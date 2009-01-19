all:	glXtest glut_hello gtk_test tcl_test.so tcl_test_cc.so

glXtest: glXtest.c
	gcc -o glXtest -L /usr/X11R6/lib glXtest.c -lX11 -lGL

glut_hello: glut_hello.c
	gcc -o glut_hello -L /usr/X11R6/lib glut_hello.c \
	    -lglut -lGLU -lGL -lXi -lXmu

gtk_test: gtk_test.c
	gcc `pkg-config --cflags gtk+-2.0 --libs gtk+-2.0` \
	    -o gtk_test gtk_test.c

tcl_test.so: tcl_test.c
	gcc -shared -o tcl_test.so -DUSE_TCL_STUBS tcl_test.c -ltclstub8.4

tcl_test_cc.so: tcl_test_cc.cc
	g++ -shared -o tcl_test_cc.so -DUSE_TCL_STUBS tcl_test_cc.cc -ltclstub8.4

clean:
	rm -rf glXtest glut_hello gtk_test tcl_test_cc.so tcl_test.so *.class

