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
#include <time.h>

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
int theta;			// The angle the look direction makes with the x-axis.    
point3_t camera_position;
point3_t jump_look_at;	  // To be set at the beginning of every jump
						  // and used as the look-at point throughout the
						  // animation.
vector3_t up_dir = {0.0, 1.0, 0.0};
#define EYE_THETA_INCR 5
#define CAMERA_POSN_INCR 0.1
#define JUMP_INCR 0.1
#define NORM_HEIGHT 0.75
#define JUMP_HEIGHT 19.5
#define COLLISION_THRESHOLD 0.1f

#define D2R(x) ((x)*M_PI/180.0)

// The maze and associated data.
maze_t *maze;
int maze_width;
int maze_height;
cell_t *start;
cell_t *end;
bool *visited;
unsigned char wall_dirs[] = {NORTH, SOUTH, EAST, WEST};
#define NUM_WALL_DIRS 4
#define WALL_THICKNESS .25

// View-volume specification in camera frame basis.
float view_plane_near = 0.1f;
float view_plane_far = 100.0f;

// Materials and lights.
typedef struct _material_t {
	GLfloat ambient[4];
	GLfloat diffuse[4];
	GLfloat specular[4];
	GLfloat phong_exp;
} material_t;

typedef enum _movement_dir_t {
    Forward,
    Backward
} movement_dir_t;

// Material setting function.

typedef struct _light_t {
	GLfloat position[4];
	GLfloat color[4];
} light_t;

GLfloat BLACK[4] = {0.0, 0.0, 0.0, 1.0};

// Spot light
light_t far_light = {
    {2.0, 20.0, 2.0, 1.0},
    {0.75, 0.75, 0.75, 1.0}
};

// Ambient light
GLfloat global_ambient[] = {0.5f, 0.5f, 0.5f, 1.0f};

material_t blue_plastic = {
    {0.0f, 0.0f, 1.0f, 1.0f},
    {0.0f, 0.0f, 1.0f, 1.0f},
    {1.0f, 1.0f, 1.0f, 1.0f},
    1000.0f
};

material_t bright_gold = {
	{0.0f, 0.0f, 0.0f, 1.0f},
	{10.0f, 10.0f, 0.0f, 1.0f},
	{0.0f, 0.0f, 0.0f, 1.0f},
	0.0f
};

material_t bright_green = {
	{0.0f, 0.0f, 0.0f, 1.0f},
	{0.0f, 10.0f, 0.0f, 1.0f},
	{0.0f, 0.0f, 0.0f, 1.0f},
	0.0f
};

material_t bright_red = {
	{0.0f, 0.0f, 0.0f, 1.0f},
	{10.0f, 0.0f, 0.0f, 1.0f},
	{0.0f, 0.0f, 0.0f, 1.0f},
	0.0f
};

// Callbacks.
void animate_jump();
void animate_fall();
void handle_display(void);
void handle_key_norm(unsigned char, int, int);
void handle_key_jumped(unsigned char, int, int);
void handle_resize(int, int);
void handle_special_key(int, int, int);

// Initialization functions.
void gl_init();
void init();
void initialize_maze();

// Application functions.
void draw_breadcrumbs();
void draw_maze();
void draw_square(material_t*);
void draw_start_end();
void draw_string(char*);
void draw_wall();
void get_new_posn(movement_dir_t, point3_t*);
bool is_collision(point3_t*);
bool is_visited(int, int);
void print_position_heading();
void process_cell();
void set_camera();
void set_jump_look_at();
void set_lights();
void set_material(material_t*);
void set_projection_viewport();
void set_visited(int, int);

// XXX: SHOULD WE KEEP THIS??? AT THE VERY LEAST WE NEED TO CHANGE THE MATERIAL
void draw_floor();

int main(int argc, char **argv) {	
	// Parse the width and height of the maze.
	maze_width = atoi(argv[1]);
	maze_height = atoi(argv[2]);

	// Initialize the drawing window.
	glutInitWindowSize(DEFAULT_WIN_WIDTH, DEFAULT_WIN_HEIGHT);
	glutInitWindowPosition(0, 0);
	// Double buffering seems to make the animation smoother, so use it.
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); 
	glutInit(&argc, argv);

	// Create the main window.
	glutCreateWindow(WINDOW_TITLE);

	// Set callbacks.
	glutReshapeFunc(handle_resize);
	glutDisplayFunc(handle_display);
	glutKeyboardFunc(handle_key_norm);
	glutSpecialFunc(handle_special_key);

	// GL initialization.
	gl_init();

    // Application initialization.
    init();

	// Enter the main loop.
	glutMainLoop();

    return EXIT_SUCCESS;
}

// GLUT CALLBACKS.

/** Animate falling back to the normal in-maze view from the overhead view.
 */
void animate_fall() {
	debug("animate_fall()");

	if (camera_position.y > NORM_HEIGHT) {
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		camera_position.y -= JUMP_INCR;
		gluLookAt(camera_position.x, camera_position.y, camera_position.z,
				  jump_look_at.x, jump_look_at.y, jump_look_at.z,
				  up_dir.x, up_dir.y, up_dir.z);
		glutPostRedisplay();
	} else {
		// Unregister the idle callback and return control to the player.
		glutKeyboardFunc(handle_key_norm);
		glutSpecialFunc(handle_special_key);
		glutIdleFunc(NULL);
	}
}

/** Animate jumping to the overhead view.
 */
void animate_jump() {
	debug("animate_jump()");

	// If the y-coordinate of the camera is less than JUMP_HEIGHT,
	// increment it by JUMP_INCR and redisplay. Otherwise, we're done animating,
	// so unregister the idle callback and set the keyboard callback to allow
	// the player to return to the normal view.
	if (camera_position.y < JUMP_HEIGHT) {
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		camera_position.y += JUMP_INCR;
		gluLookAt(camera_position.x, camera_position.y, camera_position.z,
				  jump_look_at.x, jump_look_at.y, jump_look_at.z,
				  up_dir.x, up_dir.y, up_dir.z);
		glutPostRedisplay();
	} else {
		glutKeyboardFunc(handle_key_jumped);
		glutIdleFunc(NULL);
	}
}

/** Handle a display request by clearing the screen, drawing the maze, and
 * printing the player's position and heading.
 */
void handle_display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Light position.
    glLightfv(GL_LIGHT0, GL_POSITION, far_light.position)
    
	// Display the maze.
	draw_maze();

	// Display the player's position and heading.
	print_position_heading();

    glFlush();
	glutSwapBuffers();
}

/** Handle keyboard events when in the normal in-maze view:
 *  
 *  - SPACE: Animate jumping to an overhead view of the maze. Movement 
 *			 is	disabled
 *			 until the player switches back to the in-maze view.
 *
 *  @param key the key that was pressed.
 *  @param x the mouse x-position when <code>key</code> was pressed.
 *  @param y the mouse y-position when <code>key</code> was pressed.
 */
void handle_key_norm(unsigned char key, int x, int y) {
	debug("handle_key_norm()");

    if (key == ' ') {
		glutKeyboardFunc(NULL);
		glutSpecialFunc(NULL);
		set_jump_look_at();
		glutIdleFunc(animate_jump);
    }
}

/** Handle keyboard events when in the overhead view:
 *  
 *  - SPACE: Animate falling back into the maze.
 *
 *  @param key the key that was pressed.
 *  @param x the mouse x-position when <code>key</code> was pressed.
 *  @param y the mouse y-position when <code>key</code> was pressed.
 */
void handle_key_jumped(unsigned char key, int x, int y) {
	debug("handle_key_jumped");
	if (key == ' ') {
		debug("Space pressed");
		glutIdleFunc(animate_fall);
	}
}

/** Handle keyboard events when in the normal maze view:
 *
 *	-LEFT: Rotate the camera <code>EYE_THETA_INCR</code> degrees to the left.
 *	-RIGHT: Rotate the camera <code>EYE_THETA_INCR</code> degrees to the right.
 *	-UP: Move the camera forward if possible.
 *	-DOWN: Move the camera backwards if possible.
 *
 *	@param key the key that was pressed.
 *	@param x the mouse x-position when <code>key</code> was pressed.
 *	@param y the mouse y-position when <code>key</code> was pressed.
 */
void handle_special_key(int key, int x, int y) {
	switch (key) {
		case GLUT_KEY_LEFT:
			theta += EYE_THETA_INCR;
			if (theta >= 360) theta -= 360;
			break;
		case GLUT_KEY_RIGHT:
			theta -= EYE_THETA_INCR;
		    if (theta < 0) theta += 360;
			break;
		case GLUT_KEY_UP: {
			point3_t new_posn;
			get_new_posn(Forward, &new_posn);
            if (!is_collision(&new_posn))
				camera_position = new_posn;
			break;
		}
		case GLUT_KEY_DOWN: {
			point3_t new_posn;
			get_new_posn(Backward, &new_posn);
		    if (!is_collision(&new_posn))
				camera_position = new_posn;
			break;
		}
		default:
			break;
	}

	process_cell();
	set_camera();
	glutPostRedisplay();
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




// INITIALIZATION FUNCTIONS

/** Basic GL initialization.
 */
void gl_init() {
	glEnable(GL_NORMALIZE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
}

/** Create the maze and visited array, set the lights, and set the camera.
 */
void init() {
    debug("init()");

	initialize_maze();
	visited = calloc(maze_width*maze_height, sizeof(bool));
	debug("total cells: %d", maze_width*maze_height);

	cell_t *start = get_start(maze);

	// Viewpoint position.
    theta = 0;
    camera_position.x = start->r+0.5;
    camera_position.y = NORM_HEIGHT;
    camera_position.z = start->c+0.5;

    // Set the viewpoint.
    set_camera();

	// Set the lights
	set_lights();
}

/*  Initialize the maze by building all possible walls and set the global
 *  start and end cell pointers.
 */
void initialize_maze() {
    maze = make_maze(maze_height, maze_width, time(NULL));
	start = get_start(maze);
	end = get_end(maze);
}

// APPLICATION FUNCTIONS

/** Draw bright gold square markers on the floor of all visited cells.
 */
void draw_breadcrumbs() {
	glMatrixMode(GL_MODELVIEW);

	for (int i=0; i<maze_width; i++) {
		for (int j=0; j<maze_height; j++) {
			if (is_visited(j, i)) {
				glPushMatrix();
				glTranslatef(j+.5, 0.0, i+.5);
				glScalef(.25, 1.0, .25);
				draw_square(&bright_gold);
				glPopMatrix();
			}
		}
	}
}

/** Draw the maze by first drawing the west and south exterior walls, then
 * drawing any north or east walls of each cell.
 */
void draw_maze() {
	debug("draw_maze()");
	
	glMatrixMode(GL_MODELVIEW);

	// Draw the start and end cell markers.
	draw_start_end();

	// Draw the breadcrumbs.
	draw_breadcrumbs();	

	// Draw the floor.
	draw_floor();
	
	// Draw the west and south exterior walls. 
	glPushMatrix();
	glTranslatef(maze_height/2.0, 0.5, 0.0);
	glScalef(maze_height+0.25, 1.0, 1.0);
	draw_wall();
	glPopMatrix();
	glPushMatrix();
	glTranslatef(0.0, 0.5, maze_width/2.0);
	glScalef(1.0, 1.0, maze_width+0.25);
	glRotatef(90, 0.0, 1.0, 0.0);
	draw_wall();
	glPopMatrix();

	// Draw any north or east walls of each cell.
	for (int i=0; i<maze_width; i++) {
		for (int j=0; j<maze_height; j++) {
			if (has_wall(maze, get_cell(maze, j, i), NORTH)) {
				glPushMatrix();
				glTranslatef(j+1, 0.5, i+0.5);
				glScalef(1.0, 1.0, 1.25);
				glRotatef(90, 0.0, 1.0, 0.0);
				draw_wall();
				glPopMatrix();
			}
			if (has_wall(maze, get_cell(maze, j, i), EAST)) {
				glPushMatrix();
				glTranslatef(j+0.5, 0.5, i+1);
				glScalef(1.25, 1.0, 1.0);
				draw_wall();
				glPopMatrix();
			}
		}
	}
}

/** Draw a sqaure of side length 2 in the xz plane centered at the origin
 *
 * @param material the material to use.
 */
void draw_square(material_t *material) {
	// Specify the material for the square.
	set_material(material);

	// Draw the square.
	glBegin(GL_QUADS);

	glNormal3f(0.0, 1.0, 0.0);
	glVertex3f(1.0, 0.0, 1.0);
	glVertex3f(-1.0, 0.0, 1.0);
	glVertex3f(-1.0, 0.0, -1.0);
	glVertex3f(1.0, 0.0, -1.0);

	glEnd();
}

/** Draw a green square marker on the start cell and a red one on the end cell.
 */
void draw_start_end() {
	glPushMatrix();
	glTranslatef(start->r+0.5, 0.0, start->c+0.5);
	glScalef(0.5, 0.0, 0.5);
	draw_square(&bright_green);
	glPopMatrix();
	
	glPushMatrix();
	glTranslatef(end->r+0.5, 0.0, end->c+0.5);
	glScalef(0.5, 0.0, 0.5);
	draw_square(&bright_red);
	glPopMatrix();
}

/* Draw a string at the current raster position.
 * 
 *@param the_string the string to display.
 */
void draw_string(char *the_string) {
	for (int i=0; i<strlen(the_string); i++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, the_string[i]);
	}
}

/** Draw a canonical rectangular solid of length 1, height 1, and width .25
 * along the x-axis centered at the origin.
 */
void draw_wall() {
    // Specify the material for the wall.
	set_material(&blue_plastic);

	// Draw the wall as a sequence of GL_QUADS
	glBegin(GL_QUADS);

	// x=.5 plane
	glNormal3f(1.0, 0.0, 0.0);
	glVertex3f(0.5, -0.5, 0.125);
	glVertex3f(0.5, 0.5, 0.125);
	glVertex3f(0.5, 0.5, -0.125);
	glVertex3f(0.5, -0.5, -0.125);

	// x=-.5 plane
	glNormal3f(-1.0, 0.0, 0.0);
	glVertex3f(-0.5, -0.5, -0.125);
	glVertex3f(-0.5, 0.5, -0.125);
	glVertex3f(-0.5, 0.5, 0.125);
	glVertex3f(-0.5, -0.5, 0.125);
	
	// y=.5 plane
	glNormal3f(0.0, 1.0, 0.0);
	glVertex3f(0.5, 0.5, 0.125);
	glVertex3f(-0.5, 0.5, 0.125);
	glVertex3f(-0.5, 0.5, -0.125);
	glVertex3f(0.5, 0.5, -0.125);
	
	
	// y=-.5 plane
	glNormal3f(0.0, -1.0, 0.0);
	glVertex3f(0.5, -0.5, -0.125);
	glVertex3f(-0.5, -0.5, -0.125);
	glVertex3f(-0.5, -0.5, 0.125);
	glVertex3f(0.5, -0.5, 0.125);

	
	// z=.125 plane
	glNormal3f(0.0, 0.0, 1.0);
	glVertex3f(-0.5, -0.5, 0.125);
	glVertex3f(-0.5, 0.5, 0.125);
	glVertex3f(0.5, 0.5, 0.125);
	glVertex3f(0.5, -0.5, 0.125);

	// z=-.125 plane
	glNormal3f(0.0, 0.0, -1.0);
	glVertex3f(0.5, -0.5, -0.125);
	glVertex3f(0.5, 0.5, -0.125);
	glVertex3f(-0.5, 0.5, -0.125);
	glVertex3f(-0.5, -0.5, -0.125);

	glEnd();
}

/** Set a <code>point3_t</code> representing the result of moving the camera
 * <code>CAMERA_POSN_INCR</code> forward or backward.
 *
 * @param dir the direction.
 * @param new_posn the <code>point3_t</code> to be filled in with the result of
 *		moving <code>CAMERA_POSN_INCR</code> in direction <code>dir</code>.
 */
void get_new_posn(movement_dir_t dir, point3_t *new_posn) {
	float x_incr = CAMERA_POSN_INCR*cos(D2R(theta));
	float z_incr = CAMERA_POSN_INCR*sin(D2R(-theta));
	new_posn->y = camera_position.y;

	if (dir == Forward) {
		new_posn->x = camera_position.x+x_incr;
		new_posn->z = camera_position.z+z_incr;
	} else {
		new_posn->x = camera_position.x-x_incr;
		new_posn->z = camera_position.z-z_incr;
	}
}

/** Check if a point collides with any walls.
 *
 * @param posn the point for which to check collision.
 * @return true if <code>posn</code> collides with any walls, false otherwise.
 */
bool is_collision(point3_t *posn) {    
    // Get the cell in which posn is located.
    int r = floor(posn->x);
    int c = floor(posn->z);
    cell_t *cell = get_cell(maze, r, c);

	// Iterate through all wall directions and check collision with each
	// wall that this cell has. If posn collides with any of them,
	// return true.
	unsigned char current_dir;
	point3_t closest_wall_pt = {0.0, NORM_HEIGHT, 0.0};
	for (int i=0; i<NUM_WALL_DIRS; i++) {
		current_dir = wall_dirs[i];
		if (has_wall(maze, cell, current_dir)) {
			switch(current_dir) {
				case NORTH:
					closest_wall_pt.x = cell->r+1-WALL_THICKNESS/2;
					closest_wall_pt.z = posn->z;
					break;
				case SOUTH:
					closest_wall_pt.x = cell->r+WALL_THICKNESS/2;
					closest_wall_pt.z = posn->z;
					break;
				case EAST:
					closest_wall_pt.x = posn->x;
					closest_wall_pt.z = cell->c+1-WALL_THICKNESS/2;
					break;
				case WEST:
					closest_wall_pt.x = posn->x;
					closest_wall_pt.z = cell->c+WALL_THICKNESS/2;
			}
			if (dist(posn, &closest_wall_pt) < COLLISION_THRESHOLD)
				return true;
		}
	}
	// We've checked all the walls without returning, so
	// there are no collisions.
	return false;
}

/** Determine whether or not a given cell in the maze has been visted.
 *
 * @param r the row of the cell.
 * @param c the column of the cell.
 */
bool is_visited(int r, int c) {
	return *(visited+r*maze_height+c);
}

/** Print the position (camera_position) and heading (theta) of the player.
 */
void print_position_heading() {
	debug("print_position_heading()");
	glColor3f(1.0f, 1.0f, 1.0f);

	glWindowPos2s(10, 30);
	char *s;
	asprintf(&s, "Location: (%f, %f, %f)", camera_position.x, 
			camera_position.y, camera_position.z);
	draw_string(s);
	free(s);

	glWindowPos2s(10, 10);
	asprintf(&s, "Heading: %d", theta);
	draw_string(s);
	free(s);
}

/** Determine if the current cell is a newly visited cell. If so, and it is not
 * the start nor end cell, set it as visited so that a breadcrumb will be
 * drawn on it.
 */
void process_cell() {
	// Get the current cell.
	int r = floor(camera_position.x);
	int c = floor(camera_position.z);
	cell_t *cell = get_cell(maze, r, c);
	
	// If this is a newly visited cell that isn't the start or end cell, 
	// set it as visited.
	if (!is_visited(r, c) && cell_cmp(cell, start) != 0 && 
			cell_cmp(cell, end) != 0) { 
		set_visited(r, c);
	}
}

/** Set the camera transform. The viewpoint is given by the eye coordinates,
 * and we look in angle theta-90 around the y-axis (theta is the angle the
 * view direction makes with the x-axis).
 */
void set_camera() {
	debug("set_camera()");

    // Set the camera transform.
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
    glRotatef(360-(theta-90), 0.0, 1.0, 0.0);
    glTranslatef(-camera_position.x, -camera_position.y, -camera_position.z);
}

/** Set the look-at point for a jump animation. We look at a point that is
 * 1 unit away from the current camera position in the view direction (theta).
 */
void set_jump_look_at() {
	jump_look_at.x = camera_position.x + cos(D2R(theta));
	jump_look_at.y = camera_position.y;
	jump_look_at.z = camera_position.z + sin(D2R(-theta));
}

/** Set the light colors.  Since the position of the light
 *  is subject to the current model-view transform, and we have
 *  specified the light position in world-frame coordinates,
 *  we want to set the light position after setting the camera
 *  transformation; since the camera transformation may change in response
 *  to keyboard events, we ensure this by setting the light position
 *  in the display callback.
 */
void set_lights() {
    debug("set_lights()");

    // Colors
    glLightfv(GL_LIGHT0, GL_DIFFUSE, far_light.color);
    glLightfv(GL_LIGHT0, GL_AMBIENT, BLACK);
    glLightfv(GL_LIGHT0, GL_SPECULAR, far_light.color);
}

/** Set a material as the current material.
 *
 * @param material the material to set as the current material.
 */
void set_material(material_t *material) {
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material->diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, material->specular);
    glMaterialf(GL_FRONT, GL_SHININESS, material->phong_exp);

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

/** Mark a cell as visited.
 *
 * @param r the row of the cell.
 * @param c the column of the cell.
 */
void set_visited(int r, int c) {
	debug("set_visited()");
	*(visited+r*maze_height+c) = true;
}

void draw_floor() {
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(maze_height/2.0, 0.0, maze_width/2.0);
	glScalef(maze_height/2.0, 1.0, maze_width/2.0);
    // draw_square(&blue_plastic);
	glPopMatrix();
}

