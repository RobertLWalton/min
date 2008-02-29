/*
<applet code="applet_test" width=400 height=400></applet>
*/

import java.awt.*;
import java.applet.*;

public class applet_test extends Applet {
    String msg;

    public void init() {
        setBackground ( Color.green );
        setForeground ( Color.black );
	msg = "init done";
    }

    public void start() {
	msg += "+ start done";
    }

    public void paint ( Graphics g ) {
	msg += "+ paint done";
	g.drawString ( msg, 100, 100 );
    }
}
