/** 3D maze drawing algorithm.
 *
 *  Homework #4
 *  Carlo Francisco (WesID: 774066), Michael Vitale (WesID: 777332)
 *  jfrancisco@wesleyan.edu
 *  mvitale@wesleyan.edu
 */

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __MACOSX__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#elif defined __LINUX__ || defined __CYGWIN__
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

#include "geom356.h"
#include "maze.h"

// Window data.
#define DEFAULT_WIN_WIDTH 800
#define DEFAULT_WIN_HEIGHT 600
#define WINDOW_TITLE "3D maze"
int win_width;
int win_height;

// Viewing data.
int theta ;			
point3_t camera_position ;
#define EYE_DIR_INCR 5 ;
#define CAMERA_POSN_INCR .1 ;

// The maze.
maze_t *maze ;

// View-volume specification in camera frame basis.
float view_plane_near = 4.0f;
float view_plane_far = 100.0f;

// Callbacks.
void handle_display(void);
void handle_resize(int, int);

// Materials and lights.
typedef struct _material_t {
	GLfloat ambient[4] ;
	GLfloat diffuse[4] ;
	GLfloat specular[4] ;
	GLfloat phong_exp ;
} material_t ;

typedef struct _light_t {
	GLfloat position[4] ;
	GLfloat color[4] ;
} light_t ;

GLfloat BLACK[4] = {0.0, 0.0, 0.0, 1.0} ;

material_t blue_plastic = {
    {0.0f, 0.0f, 1.0f, 1.0f},
    {0.0f, 0.0f, 1.0f, 1.0f},
    {1.0f, 1.0f, 1.0f, 1.0f},
    1000.0f
} ;

int main(int argc, char **argv) {
	// Initialize the drawing window.
	glutInitWindowSize(DEFAULT_WIN_WIDTH, DEFAULT_WIN_HEIGHT) ;
	glutInitWindowPosition(0, 0) ;
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH) ;
	glutInit(&argc, argv) ;

	// Create the main window.
	glutCreateWindow(WINDOW_TITLE) ;

	glutReshapeFunc(handle_resize) ;
	glutDisplayFunc(handle_display) ;

	// GL initialization.
	glEnable(GL_DEPTH_TEST) ;
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f) ;

	glutMainLoop() ;

    return EXIT_SUCCESS;
}

/** Set the camera transform. The viewpoint is given by the eye coordinates,
 * and we look in angle theta around the y-axis.
 */
void set_camera() {

	glMatrixMode(GL_MODELVIEW) ;
	glLoadIdentity() ;

	// Set the camera transform.
	glRotatef(-theta, 0.0, 1.0, 0.0) ;
	glTranslatef(-camera_position.x, -camera_position.y, -camera_position.z) ;

}

/** Set the projection and viewport transformations.  We use perspective
 *  projection and always match the aspect ratio of the screen window
 *  with vertical field-of-view 60 degrees and always map to the entire
 *  screen window.
 */
void set_projection_viewport() {

	// Set perspective projection transform.
	glMatrixMode(GL_PROJECTION) ;
	glLoadIdentity() ;
	gluPerspective(60.0, (GLdouble)win_width/win_height, view_plane_near,
			view_plane_far) ;

	// Set the viewport transform.
	glViewport(0, 0, win_width, win_height) ;	

}

/** Handle a resize event by recording the new width and height.
 *  
 *  @param width the new width of the window.
 *  @param height the new height of the window.
 */
void handle_resize(int width, int height) {

    win_width = width;
    win_height = height;

    glutPostRedisplay();
}

/** Handle a display request by clearing the screen.
 */
void handle_display() {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;

    glFlush() ;
}
