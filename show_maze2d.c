/*  2D maze visulation.
 *
 *  For use in Wesleyan University COMP 212, Spring 2008.
 *  N. Danner.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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

#include "maze.h"

#define DEFAULT_WINDOW_WIDTH 500
#define DEFAULT_WINDOW_HEIGHT 500
#define DEFAULT_WINDOW_XORIG 0
#define DEFAULT_WINDOW_YORIG 0
#define WINDOW_TITLE "2D maze visualization"

/*  The maze itself
 */
maze_t* maze ;

// GL initialization.
void init_gl(int, int) ;

// Display callback:  draws the maze.
void initialize_maze(int, int) ;
void draw_maze() ;


int main(int argc, char **argv)
{
    // Handle command line arguments.

    // Set up the window---notice these are all GLUT calls.
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB) ;
    glutInitWindowSize(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT) ;
    glutInitWindowPosition(DEFAULT_WINDOW_XORIG, DEFAULT_WINDOW_YORIG) ;
    glutInit(&argc, argv) ;

    glutCreateWindow(WINDOW_TITLE) ;

    // Display callback---draw_sg is the function that will be called
    // every time the window system requests that the window be
    // re-drawn.
    glutDisplayFunc(draw_maze) ;

    // Initialize the maze.
    int maze_width = atoi(argv[1]) ;
    int maze_height = atoi(argv[2]) ;
    initialize_maze(maze_height, maze_width) ;

    // Initialize GL.
    init_gl(maze_width, maze_height) ;

    // Enter the main event-handling loop---this does almost nothing
    // for us right now except keep the program from terminating.
    glutMainLoop() ;

    return 0 ;
}

/* Initialize OpenGL.  Set our world frame to be the width and height
 * of the maze.
 */
void init_gl(int maze_height, int maze_width) {
    // Background color.
    glClearColor(1.0, 1.0, 1.0, 1.0) ;

    // 2D orthographic projection.
    glMatrixMode(GL_PROJECTION) ;
    glLoadIdentity() ;
    gluOrtho2D(-1.0, maze_width+1, -1.0, maze_height+1) ;
}

/*  Initialize the maze by building all possible walls.
 */
void initialize_maze(int maze_height, int maze_width) {

    maze = make_maze(maze_height, maze_width, time(NULL)) ;

}

void draw_maze() {
    glClear(GL_COLOR_BUFFER_BIT) ;

    cell_t* maze_start = get_start(maze) ;
    cell_t* maze_end = get_end(maze) ;

    int maze_width = get_ncols(maze) ;
    int maze_height = get_ncols(maze) ;

    // Draw the start and end cell.
    glBegin(GL_QUADS) ;
    glColor3f(0, 1, 0) ;
    glVertex2i(maze_start->c, maze_start->r) ;
    glVertex2i(maze_start->c+1, maze_start->r) ;
    glVertex2i(maze_start->c+1, maze_start->r+1) ;
    glVertex2i(maze_start->c, maze_start->r+1) ;
    glColor3f(.1, .1, .1) ;
    glVertex2i(maze_end->c, maze_end->r) ;
    glVertex2i(maze_end->c+1, maze_end->r) ;
    glVertex2i(maze_end->c+1, maze_end->r+1) ;
    glVertex2i(maze_end->c, maze_end->r+1) ;
    glEnd() ;

    // Draw the walls.  First draw the west and south exterior walls, then
    // draw any north or west walls of each cell.
    glColor3f(1.0, 0.0, 0.0) ;
    glBegin(GL_LINES) ;
    glVertex2i(0, 0) ;
    glVertex2i(0, maze_height) ;
    glVertex2i(0, 0) ;
    glVertex2i(maze_width, 0) ;
    for (int i=0; i<maze_width; ++i) {
        for (int j=0; j<maze_height; ++j) {
            if (has_wall(maze, get_cell(maze, j, i), NORTH)) {
                glVertex2i(i, j+1) ;
                glVertex2i(i+1, j+1) ;
            }
            if (has_wall(maze, get_cell(maze, j, i), EAST)) {
                glVertex2i(i+1, j+1) ;
                glVertex2i(i+1, j) ;
            }
        }
    }
    glEnd() ;

    glFlush() ;

}




