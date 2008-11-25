
import java.lang.*;
import java.awt.*;
import java.applet.*;

public class frame_test extends Frame implements Runnable {
    String msg;
    Thread thread = null;
    int x = 100;

    public void paint ( Graphics g ) {
	msg = "paint done";
	g.drawString ( msg, x, 100 );
    }
    public void start ( ) {
	thread = new Thread ( this );
	thread.start();
    }

    public void run ( ) {
        for ( ; ; ) {
	    try {
		repaint();
		Thread.sleep (2000 );
		if ( x > 250 ) x = 100;
		else x += 10;
	    }
	    catch ( InterruptedException e )
	        { thread.destroy(); }
	}
    }
        
    public static void main ( String args[] ) {
        frame_test frame = new frame_test();
	frame.setSize ( new Dimension ( 400, 400 ) );
	frame.setTitle ( "Frame Test" );
        frame.setBackground ( Color.cyan );
        frame.setForeground ( Color.red );
	frame.setVisible ( true );
	frame.start();
    }
}
