/*
 * hello.cc -- A minimal Tcl C++ extension.
 */
extern "C" {
#include <tcl.h>
}

static int
Hello_Cmd(ClientData cdata, Tcl_Interp *interp, int objc,
          Tcl_Obj * CONST objv[])
{
    Tcl_SetObjResult(interp, Tcl_NewStringObj("Hello, World!", -1));
    return TCL_OK;
}

/*
* Hello_Init -- Called when Tcl loads your extension.
 */
extern "C" int DLLEXPORT Tcl_test_cc_Init(Tcl_Interp *interp);
int DLLEXPORT
Tcl_test_cc_Init(Tcl_Interp *interp)
{
    if (Tcl_InitStubs(interp, TCL_VERSION, 0) == NULL) {
	return TCL_ERROR;
    }
    /* changed this to check for an error - GPS */
    if (Tcl_PkgProvide(interp, "Hello", "1.0") == TCL_ERROR) {
	return TCL_ERROR;
    }
    Tcl_CreateObjCommand(interp, "hello", Hello_Cmd, NULL, NULL);
    return TCL_OK;
}
