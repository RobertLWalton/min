import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

class swing_test
{
    JLabel label = null;

    class canvas extends JPanel
    {
        canvas ()
	{
	    setBorder
	        ( BorderFactory.createLineBorder
		      ( Color.RED, 5 ) );
	}
    }

    canvas c = null;

    swing_test ()
    {
	JFrame frame = new JFrame ( "Swing Test" );
	label = new JLabel ( "hello" );
	c = new canvas();
	JButton button = new JButton ( "Test" );
	Box hbox = Box.createHorizontalBox();
	Box vbox = Box.createVerticalBox();

	frame.setSize ( 400, 400 );
	frame.setDefaultCloseOperation
	    ( JFrame.EXIT_ON_CLOSE );

	frame.getContentPane().add ( vbox );
	vbox.add ( hbox );
	hbox.add ( label );
	hbox.add ( button );
	vbox.add ( c );

	button.addActionListener ( new ActionListener() {
	    public void actionPerformed ( ActionEvent e )
	    {
	        label.setText ( "height " + c.getHeight()
				+ " width " + c.getWidth() );
	    } } );

	frame.setVisible ( true );
    }

    public static void main ( String argv[] )
    {
        SwingUtilities.invokeLater ( new Runnable() {
	    public void run() { new swing_test(); } } );
    }
}
