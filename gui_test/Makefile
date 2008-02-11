gtk_test: gtk_test.c
	gcc `pkg-config --cflags gtk+-2.0 --libs gtk+-2.0` \
	    -o gtk_test gtk_test.c

tcl_test.so: tcl_test.c
	gcc -shared -o tcl_test.so -DUSE_TCL_STUBS tcl_test.c -ltclstub8.4

tcl_test_cc.so: tcl_test_cc.cc
	g++ -shared -o tcl_test_cc.so -DUSE_TCL_STUBS tcl_test_cc.cc -ltclstub8.4

