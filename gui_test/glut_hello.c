// gcc -o glut_hello -L /usr/X11R6/lib glut_hello.c -lglut -lGLU -lGL -lXi -lXmu

#include <GL/gl.h>
#include <GL/glut.h>
#include <unistd.h>
#include <assert.h>

void finish (unsigned char key, int x, int y)
{
    exit (0);
}

void display (void)
{
    glClear (GL_COLOR_BUFFER_BIT);
    glColor3f (1.0, 0.0, 0.0);
    glBegin (GL_POLYGON);
    	glVertex3f (0.25, 0.25, 0.0);
    	glVertex3f (0.75, 0.25, 0.0);
    	glVertex3f (0.75, 0.75, 0.0);
    	glVertex3f (0.25, 0.75, 0.0);
    glEnd();

    glFlush();
}

void init (void)
{
    glClearColor (0.0, 0.0, 1.0, 0.0);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    glOrtho (0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
}

int main (int argc, char** argv)
{
    glutInit (&argc, argv);
    glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);
    // glutInitWindowSize (250, 250);
    // glutInitWindowPosition (100, 100);
    glutCreateWindow ("hello");
    init ();
    glutDisplayFunc (display);
    glutKeyboardFunc (finish);
    glutMainLoop();
    return 0;
}
