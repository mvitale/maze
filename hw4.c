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
#include "debug.h"

// Window data.
#define DEFAULT_WIN_WIDTH 800
#define DEFAULT_WIN_HEIGHT 600
#define WINDOW_TITLE "3D maze"
int win_width;
int win_height;

// Viewing data.
int theta;          
point3_t camera_position;
point3_t look_at = {0.0f, 0.0f, 0.0f};         // Look-at position (world coordinates).
vector3_t up_dir = {0.0f, 1.0f, 0.0f};             // Up direction.
#define EYE_THETA_INCR 10;
#define CAMERA_POSN_INCR .1;

// The maze.
maze_t *maze;

// View-volume specification in camera frame basis.
float view_plane_near = 1.0f;
float view_plane_far = 100.0f;

// Callbacks.
void handle_display(void);
void handle_resize(int, int);
void handle_special_key(int, int, int);

// Application functions.
void init();

// Materials and lights.
typedef struct _material_t {
	GLfloat ambient[4];
	GLfloat diffuse[4];
	GLfloat specular[4];
	GLfloat phong_exp;
} material_t;

typedef struct _light_t {
	GLfloat position[4];
	GLfloat color[4];
} light_t;

GLfloat BLACK[4] = {0.0, 0.0, 0.0, 1.0};

light_t far_light = {
    {20.0, 10.0, 0.0, 0.0},
    {0.75, 0.75, 0.75, 1.0}
};

material_t blue_plastic = {
    {0.0f, 0.0f, 1.0f, 1.0f},
    {0.0f, 0.0f, 1.0f, 1.0f},
    {1.0f, 1.0f, 1.0f, 1.0f},
    1000.0f
};


int main(int argc, char **argv) {
	// Initialize the drawing window.
	glutInitWindowSize(DEFAULT_WIN_WIDTH, DEFAULT_WIN_HEIGHT);
	glutInitWindowPosition(0, 0);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
	glutInit(&argc, argv);

	// Create the main window.
	glutCreateWindow(WINDOW_TITLE);

	// Set callbacks.
	glutReshapeFunc(handle_resize);
	glutDisplayFunc(handle_display);
	glutSpecialFunc(handle_special_key);

	// GL initialization.
	glEnable(GL_DEPTH_TEST);
    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_BACK);
	glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    // Application initialization.
    init();

	glutMainLoop();

    return EXIT_SUCCESS;
}

/** Set the camera transform. The viewpoint is given by the eye coordinates,
 * and we look in angle theta around the y-axis.
 */
void set_camera() {
    // Set the camera transform.
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
    glRotatef(360-theta, 0.0, 1.0, 0.0);
    glTranslatef(-camera_position.x, -camera_position.y, -camera_position.z);
}

void handle_special_key(int key, int x, int y) {
	switch (key) {
		case GLUT_KEY_LEFT:
			theta += EYE_THETA_INCR;
			if (theta > 360) theta -= 360;
			break;
		case GLUT_KEY_RIGHT:
			theta -= EYE_THETA_INCR;
			if (theta < 360) theta += 360;
			break;
		/**
		case GLUT_KEY_UP:
			camera_position.x += CAMERA_POSN_INCR*cos(theta);
			camera_position.z += CAMERA_POSN_INCR*sin(theta);
			break;
		case GLUT_KEY_DOWN:
			camera_position.x -= CAMERA_POSN_INCR*cos(theta);
			camera_position.z -= CAMERA_POSN_INCR*sin(theta);
			break;
		*/
		default:
			break;
	}
	set_camera();
	glutPostRedisplay();
}

/** Set the projection and viewport transformations.  We use perspective
 *  projection and always match the aspect ratio of the screen window
 *  with vertical field-of-view 60 degrees and always map to the entire
 *  screen window.
 */
void set_projection_viewport() {
    debug("set_projection_viewport");
    
	// Set perspective projection transform.
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLdouble)win_width/win_height, view_plane_near,
			view_plane_far);

	// Set the viewport transform.
	glViewport(0, 0, win_width, win_height);	
}

/** Set the light colors.  Since the position of the light
 *  is subject to the current model-view transform, and we have
 *  specified the light position in world-frame coordinates,
 *  we want to set the light position after setting the camera
 *  transformation;since the camera transformation may change in response
 *  to keyboard events, we ensure this by setting the light position
 *  in the display callback.
 *
 *  It is also easy to "attach" a light to the viewer.  In that case,
 *  just specify the light position in the camera frame and make sure
 *  to set its position while the camera transformation is the identity!
 */
void set_lights() {
    debug("set_lights()");

    light_t* light = &far_light;
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light->color);
    glLightfv(GL_LIGHT0, GL_AMBIENT, BLACK);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light->color);
}

void init() {
    debug("init");

    // Viewpoint position.
    theta = 90;
    camera_position.x = 5.0;
    camera_position.y = 0.0;
    camera_position.z = 0.0;

    set_lights();
    
    // Set the viewpoint.
    set_camera();
}

/** Handle a resize event by recording the new width and height.
 *  
 *  @param width the new width of the window.
 *  @param height the new height of the window.
 */
void handle_resize(int width, int height) {
    win_width = width;
    win_height = height;
    
    set_projection_viewport();

    glutPostRedisplay();
}

void draw_cube() {
    debug("draw_cube()");
    
    // Specify the material for the cube.
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, blue_plastic.diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, blue_plastic.specular);
    glMaterialf(GL_FRONT, GL_SHININESS, blue_plastic.phong_exp);

    // Draw cube as a sequence of GL_QUADS.
    glBegin(GL_QUADS);


    // z=1 plane.
    glNormal3f(0.0, 0.0, 1.0);
    glVertex3f(-1.0, -1.0, 1.0);
    glVertex3f(1.0, -1.0, 1.0);
    glVertex3f(1.0, 1.0, 1.0);
    glVertex3f(-1.0, 1.0, 1.0);
    
    // z=-1 plane.
    glNormal3f(0.0, 0.0, -1.0);
    glVertex3f(-1.0, -1.0, -1.0);
    glVertex3f(-1.0, 1.0, -1.0);
    glVertex3f(1.0, 1.0, -1.0);
    glVertex3f(1.0, -1.0, -1.0);

    // x=1 plane.
    glNormal3f(1.0, 0.0, 0.0);
    glVertex3f(1.0, -1.0, 1.0);
    glVertex3f(1.0, -1.0, -1.0);
    glVertex3f(1.0, 1.0, -1.0);
    glVertex3f(1.0, 1.0, 1.0);

    // x=-1 plane.
    glNormal3f(-1.0, 0.0, 0.0);
    glVertex3f(-1.0, -1.0, 1.0);
    glVertex3f(-1.0, 1.0, 1.0);
    glVertex3f(-1.0, 1.0, -1.0);
    glVertex3f(-1.0, -1.0, -1.0);

    // y=1 plane.
    glNormal3f(0.0, 1.0, 0.0);
    glVertex3f(-1.0, 1.0, 1.0);
    glVertex3f(1.0, 1.0, 1.0);
    glVertex3f(1.0, 1.0, -1.0);
    glVertex3f(-1.0, 1.0, -1.0);

    // y=-1 plane.
    glNormal3f(0.0, -1.0, 0.0);
    glVertex3f(-1.0, -1.0, 1.0);
    glVertex3f(-1.0, -1.0, -1.0);
    glVertex3f(1.0, -1.0, -1.0);
    glVertex3f(1.0, -1.0, 1.0);

    glEnd();
}

/** Handle a display request by clearing the screen.
 */
void handle_display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    // Keep cube at origin for now.
    glTranslatef(0.0, 0.0, 0.0);
    draw_cube();
    glPopMatrix();
    
    glFlush();
}
