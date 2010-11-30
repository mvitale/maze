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

#include "debug.h"

// Window data.
const int DEFAULT_WIN_WIDTH = 800;
const int DEFAULT_WIN_HEIGHT = 600;
int win_width;
int win_height;

// Viewing data.
int look_direction ;
point3_t camera_position ;
#define EYE_DIR_INCR 5 ;
#define CAMERA_POSN_INCR .1 ;

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
    return EXIT_SUCCESS;
}

/** Handle a resize event by recording the new width and height.
 *  
 *  @param width the new width of the window.
 *  @param height the new height of the window.
 */
void handle_resize(int width, int height) {
    debug("handle_resize(%d, %d)\n", width, height);

    win_width = width;
    win_height = height;

    glutPostRedisplay();
}

/** Handle a display request by clearing the screen.
 */
void handle_display() {
    debug("handle_display()") ;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;

    glFlush() ;
}
