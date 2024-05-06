#include <GL/glut.h>
#include <vector>
#include <cstdlib>  
#include <ctime>   
#include <cmath>
#include <thread>
#include <array>
#include<mutex>
	// Define the grid size
	const int gridWidth = 15;
	const int gridHeight = 15;
	
	// Initial square position and size
	float x = 20.0f;
	float y = 20.0f;
	float side = 20.0f;
struct Ghost {
int x, y;
};

	std::array<Ghost, 4> ghosts = {{{7, 5}, {7, 6}, {7, 7}, {7, 8}}};
	std::mutex ghostMutex;
	bool running = true;

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
    int x = 7; // initial x position
    int y = 10; // initial y position
} pacman;
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

void drawPacman() {
    glColor3f(1.0f, 1.0f, 0.0f); // Set Pacman color to yellow
    float pacmanX = pacman.x * 20 + 10; // Calculate pixel x position from grid position
    float pacmanY = pacman.y * 20 + 10; // Calculate pixel y position from grid position
    drawCircle(pacmanX, pacmanY, 10, 20); // Draw 
}



void moveGhost(int index) {
    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Delay between moves
        int direction = std::rand() % 4; // Random direction

        int dx = 0, dy = 0;
        switch (direction) {
            case 0: dx = 1;  break; // Right
            case 1: dx = -1; break; // Left
            case 2: dy = 1;  break; // Down
            case 3: dy = -1; break; // Up
        }

        std::lock_guard<std::mutex> lock(ghostMutex);
        int newX = (ghosts[index].x + dx + gridWidth) % gridWidth;
        int newY = (ghosts[index].y + dy + gridHeight) % gridHeight;
        if (!maze[newY][newX]) { // Check if the new position is not a wall
            ghosts[index].x = newX;
            ghosts[index].y = newY;
        }
    }
}
void drawGhosts() {
    // Define the colors for the ghosts
    std::array<std::tuple<float, float, float>, 4> colors = {
        std::make_tuple(1.0f, 0.5f, 0.0f), // Orange
        std::make_tuple(0.0f, 0.0f, 1.0f), // Blue
        std::make_tuple(1.0f, 0.0f, 0.0f), // Red
        std::make_tuple(1.0f, 0.5f, 0.8f)  // Pink
    };

    std::lock_guard<std::mutex> lock(ghostMutex);
    for (int i = 0; i < ghosts.size(); ++i) {
        auto [r, g, b] = colors[i];
        glColor3f(r, g, b); // Set the color for each ghost
        float ghostX = ghosts[i].x * 20 + 10;
        float ghostY = ghosts[i].y * 20 + 10;
        drawCircle(ghostX, ghostY, 10, 20); // Draw ghost as a circle
    }
}


void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    // Existing code to draw walls and pellets
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
    drawPacman(); // Draw pacman
    drawGhosts(); // Draw ghosts

    glutSwapBuffers();
}

void keyboard(int key, int xx, int yy) {
    int nextX = pacman.x;
    int nextY = pacman.y;

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

	///ye wrap around envirmnment hao
    if (nextY == 6 && nextX < 0) {  
        nextX = 14; 
    } else if (nextY == 6 && nextX >= 14) {  
        nextX = 0;
    } else if (nextX < 0 || nextX >= gridWidth || nextY < 0 || nextY >= gridHeight || maze[nextY][nextX]) {
       
        return; 
    }

    pacman.x = nextX;
    pacman.y = nextY;

    glutPostRedisplay();
}


void initOpenGL() {
glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
glMatrixMode(GL_PROJECTION);
glLoadIdentity();
gluOrtho2D(0, 300, 300, 0);
glMatrixMode(GL_MODELVIEW);
}
int main(int argc, char** argv) {
    std::srand(std::time(0)); // Seed random number generator
    std::array<std::thread, 4> ghostThreads;
    for (int i = 0; i < ghostThreads.size(); ++i) {
        ghostThreads[i] = std::thread(moveGhost, i);
    }

  glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(600, 400);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("22i-0846_22i-1036_J_PACMAN");
    initOpenGL();
    glutDisplayFunc(display);
    glutSpecialFunc(keyboard);
    glutMainLoop();
    running = false; // Stop threads after exiting the main loop

    for (auto& thread : ghostThreads) {
        thread.join(); // Wait for all threads to finish
    }

    return 0;
}

