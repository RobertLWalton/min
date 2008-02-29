
import java.awt.*;
import java.applet.*;

public class frame_test extends Frame {
    String msg;

    public void paint ( Graphics g ) {
	msg = "paint done";
	g.drawString ( msg, 100, 100 );
    }

    public static void main ( String args[] ) {
        frame_test frame = new frame_test();
	frame.setSize ( new Dimension ( 400, 400 ) );
	frame.setTitle ( "Frame Test" );
        frame.setBackground ( Color.green );
        frame.setForeground ( Color.black );
	frame.setVisible ( true );
    }
}
