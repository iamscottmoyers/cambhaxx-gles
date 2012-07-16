#include <stdlib.h>

#if defined(__APPLE__) || defined(MACOSX)
# include <GLUT/glut.h>
#else
# include <GL/glut.h>
#endif

#include "window_system.h"
#include "utils.h"

int window_system_initialise(int argc, char ***argv)
{
	glutInit(&argc, *argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(DEFAULT_WIN_WIDTH, DEFAULT_WIN_HEIGHT);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("loads of rotating dudes");
        return 0;
}

int window_system_cleanup()
{
        /* TODO: Clear it all up */
        return 0;
}

int window_system_start_main_loop()
{
	glutMainLoop();
        return 0;
}

static void (*display)(void);
static void glut_redraw_callback(void)
{
	display();
	glutSwapBuffers();
}

int window_system_set_redraw_callback(void (*redraw_callback)(void))
{	
	display = redraw_callback;
	glutDisplayFunc(glut_redraw_callback);
        return 0;
}

int window_system_set_resize_callback(void (*resize_callback)(int,int))
{
	glutReshapeFunc(resize_callback);
        return 0;
}

static void (*rotate)(void);
static void glut_rotation_callback(int value)
{
	UNUSED( value );
	rotate();
	glutPostRedisplay();
	glutTimerFunc(value, glut_rotation_callback, value);
}

int window_system_set_rotate_callback(int rotation_speed, void (*rotation_callback)(void))
{
	rotate = rotation_callback;
	glutTimerFunc(rotation_speed, glut_rotation_callback, rotation_speed);
        return 0;
}
