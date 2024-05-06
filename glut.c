#include <GL/glut.h>
#include <vector>
#include <cstdlib>  
#include <ctime>   
#include <cmath>

// Define the grid size
	const int gridWidth = 15;
	const int gridHeight = 15;
	
// Initial square position and size
	float x = 20.0f;
	float y = 20.0f;
	float side = 20.0f;
	
// Maze configuration
bool maze[gridHeight][gridWidth] = {
    {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},	//1
    {0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0},	//2
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	//3
    {0, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0},	//4
    {0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0},	//5
    {1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1},	//6
    {0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0},	//7
    {1, 1, 0, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 1, 1},	//8
    {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},	//9
    {0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0},	//10
    {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},	//11
    {1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 1},	//12
    {0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0},	//13
    {0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 0},	//14
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	//15
};

struct {
    int x = 6; // initial x position
    int y = 3; // initial y position
} cherry;
// Function to draw a circle
void drawCircle(float cx, float cy, float r, int num_segments) {
    glBegin(GL_POLYGON);
    for(int i = 0; i < num_segments; i++) {
        float theta = 2.0f * 3.1415926f * float(i) / float(num_segments); // Get the current angle
        float x = r * cosf(theta); // Calculate the x component
        float y = r * sinf(theta); // Calculate the y component
        glVertex2f(x + cx, y + cy); // Output vertex
    }
    glEnd();
}
void drawCherry() {
    glColor3f(1.0f, 0.0f, 0.0f); // Set cherry color to red
    float cherryX = cherry.x * 20 + 10; // Calculate pixel x position from grid position
    float cherryY = cherry.y * 20 + 10; // Calculate pixel y position from grid position
    drawCircle(cherryX, cherryY, 10, 20); // Draw cherry as a circle
}


void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    // Define the size of each block and pellet
    float blockSize = 20.0f; // Size of each block in the grid

    // Draw maze walls
    glColor3f(0.0f, 0.0f, 1.0f); // Blue walls
    for (int i = 0; i < gridHeight; ++i) {
        for (int j = 0; j < gridWidth; ++j) {
            if (maze[i][j]) {  // If there's a wall, draw it
                glBegin(GL_QUADS);
                glVertex2f(j * blockSize, i * blockSize);
                glVertex2f((j + 1) * blockSize, i * blockSize);
                glVertex2f((j + 1) * blockSize, (i + 1) * blockSize);
                glVertex2f(j * blockSize, (i + 1) * blockSize);
                glEnd();
            }
        }
    }

    // Draw regular pellets
    glColor3f(1.0f, 1.0f, 0.0f); // Yellow for pellets
    glPointSize(5.0f); // Size of each pellet
    glBegin(GL_POINTS);
    for (int i = 0; i < gridHeight; ++i) {
        for (int j = 0; j < gridWidth; ++j) {
            if (!maze[i][j]) { // Only draw pellets where there's no wall
                glVertex2f(j * blockSize + blockSize / 2, i * blockSize + blockSize / 2);
            }
        }
    }
    glEnd();
 drawCherry();
    glutSwapBuffers();
}



void keyboard(int key, int xx, int yy) {
    int nextX = cherry.x;
    int nextY = cherry.y;

    switch (key) {
        case GLUT_KEY_RIGHT:
            nextX++;
            break;
        case GLUT_KEY_LEFT:
            nextX--;
            break;
        case GLUT_KEY_UP:
            nextY--;
            break;
        case GLUT_KEY_DOWN:
            nextY++;
            break;
    }

    // Check for boundaries and walls before updating the position
    if (nextX >= 0 && nextX < gridWidth && nextY >= 0 && nextY < gridHeight && !maze[nextY][nextX]) {
        cherry.x = nextX;
        cherry.y = nextY;
    }

    glutPostRedisplay();
}


void initOpenGL() {
glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
glMatrixMode(GL_PROJECTION);
glLoadIdentity();
gluOrtho2D(0, 300, 300, 0);
glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) 
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(600, 400);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("22i-0846_22i-1036_J_PACMAN");
    initOpenGL();
    glutDisplayFunc(display);
    glutSpecialFunc(keyboard);
    glutMainLoop();
    return 0;
}

