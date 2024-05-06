#include <GL/glut.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#define NUM_GHOSTS 4

// Define the grid size
#define GRID_WIDTH 15
#define GRID_HEIGHT 15
sem_t ghostSemaphore;

int key=1;
int permit=1;
// Initial square position and size
float x = 20.0f;
float y = 20.0f;
float side = 20.0f;

struct Ghost {
    int x;
    int y;
};

struct Ghost ghosts[NUM_GHOSTS] = {
    {7, 5},
    {7, 6},
    {6, 6},
    {8, 6}
};

pthread_mutex_t ghostMutex = PTHREAD_MUTEX_INITIALIZER;
bool running = true;

// Maze configuration
bool maze[GRID_HEIGHT][GRID_WIDTH] = {
    {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},	//1
    {0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0},	//2
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	//3
    {0, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0},	//4
    {0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0},	//5
    {1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1},	//6
    {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},	//7
    {1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1},	//8
    {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},	//9
    {0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0},	//10
    {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},	//11
    {1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 1},	//12
    {0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0},	//13
    {0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 0},	//14
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	//15
};

struct {
    int x;
    int y;
} pacman = {7, 10}; // initial x and y position
//======================================================================================
// Function to draw a circle
void drawCircle(float cx, float cy, float r, int num_segments) {
    glBegin(GL_POLYGON);
    for (int i = 0; i < num_segments; i++) {
        float theta = 2.0f * 3.1415926f * (float)i / (float)(num_segments); // Get the current angle
        float x = r * cosf(theta); // Calculate the x component
        float y = r * sinf(theta); // Calculate the y component
        glVertex2f(x + cx, y + cy); // Output vertex
    }
    glEnd();
}
//======================================================================================
void drawPacman() {
    glColor3f(1.0f, 1.0f, 0.0f); // Set Pacman color to yellow
    float pacmanX = pacman.x * 20 + 10; // Calculate pixel x position from grid position
    float pacmanY = pacman.y * 20 + 10; // Calculate pixel y position from grid position
    drawCircle(pacmanX, pacmanY, 10, 20); // Draw 
}
//======================================================================================
void moveGhost(int index) {
    while (running) {
        usleep(500000); // Delay between moves (500 milliseconds)
        int direction = rand() % 4; // Random direction

        int dx = 0, dy = 0;	//positions
        switch (direction) {
            case 0: dx = 1;  break; // Right
            case 1: dx = -1; break; // Left
            case 2: dy = 1;  break; // Down
            case 3: dy = -1; break; // Up
        }

     //   pthread_mutex_lock(&ghostMutex);
        sem_wait(&ghostSemaphore);
        
       // int newX = (ghosts[index].x + dx ) ;	//new position of ghost
       //	int newY = (ghosts[index].y + dy ) ;
	
//	printf("The value of number is: %d\n", ghosts[index].x);
//	printf("The value of number is: %d\n", ghosts[index].y);
        int newX = (ghosts[index].x + dx + GRID_WIDTH) % GRID_WIDTH;
        int newY = (ghosts[index].y + dy + GRID_HEIGHT) % GRID_HEIGHT;
        if (!maze[newY][newX]) { // Check if new position is not a wall if wall no move
            ghosts[index].x = newX;
            ghosts[index].y = newY;
        }
        sem_post(&ghostSemaphore);
     //   pthread_mutex_unlock(&ghostMutex);
    }
}
//======================================================================================
void drawGhosts() {
    // Define the colors for the ghosts
    float colors[4][3] = {
        {1.0f, 0.5f, 0.0f}, // Orange
        {0.56f, 0.0f, 1.0f}, // Blue
        {1.0f, 0.0f, 0.0f}, // Red
        {1.0f, 0.5f, 0.8f}  // Pink
    };

    pthread_mutex_lock(&ghostMutex);
    for (int i = 0; i < 4; ++i) {
        glColor3f(colors[i][0], colors[i][1], colors[i][2]); // Set the color for each ghost
        float ghostX = ghosts[i].x * 20 + 10;
        float ghostY = ghosts[i].y * 20 + 10;
        drawCircle(ghostX, ghostY, 10, 20); // Draw ghost as a circle
    }
    pthread_mutex_unlock(&ghostMutex);
}
//====================================================================================

//======================================================================================
void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    // Existing code to draw walls and pellets
    float wallThickness=1.0f;
    float blockSize = 20.0f; // Size of each block in the grid

    // Draw maze walls
    glColor3f(0.0f, 0.0f, 0.5f); // Blue walls
    for (int i = 0; i < GRID_HEIGHT; ++i) {
        for (int j = 0; j < GRID_WIDTH; ++j) {
            if (maze[i][j]) {  // If there's a wall, draw it
                glBegin(GL_QUADS);
                glVertex2f(j * blockSize, i * blockSize);
                glVertex2f((j + 1) * blockSize, i * blockSize);
                glVertex2f((j + 1) * blockSize, (i+wallThickness) * blockSize);
                glVertex2f(j * blockSize, (i+wallThickness) * blockSize);
                glEnd();
            }
        }
    }

    // Draw regular pellets
    glColor3f(1.0f, 1.0f, 0.0f); // Yellow for pellets
    glPointSize(5.0f); // Size of each pellet
    glBegin(GL_POINTS);
    for (int i = 0; i < GRID_HEIGHT; ++i) {
        for (int j = 0; j < GRID_WIDTH; ++j) {
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
//======================================================================================
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

    // Wrap around environment
    if (nextY == 6 && nextX < 0) {
        nextX = 14;
    } else if (nextY == 6 && nextX >= 14) {
        nextX = 0;
    } else if (nextX < 0 || nextX >= GRID_WIDTH || nextY < 0 || nextY >= GRID_HEIGHT || maze[nextY][nextX]) {
        return;
    }

    pacman.x = nextX;
    pacman.y = nextY;

    glutPostRedisplay();
}
//=============================================================
void initOpenGL() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 300, 300, 0);
    glMatrixMode(GL_MODELVIEW);
}
pthread_mutex_t menuMutex = PTHREAD_MUTEX_INITIALIZER;
void menu(int value) {
    switch (value) {
        case 1:
            printf("Resume game\n");
            break;
        case 2:
            printf("Exit game\n");
            exit(0);
            break;
        case 3:
            printf("Check Score\n");
         //   exit(0);
            break;
		case 4:
            printf("Rules\n");
          //  exit(0);
            break;
    }
}

//=============
void* uithreadfunc(void* arg)
{
	
    int menu_id = glutCreateMenu(menu);
    glutAddMenuEntry("Resume Game", 1);
    glutAddMenuEntry("Exit Game", 2);
    glutAddMenuEntry("Check Score", 3);
    glutAddMenuEntry("Rules", 4);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
   

}


//====================================================================================
int main(int argc, char** argv) {
    

	
   
    srand(time(0)); // Seed random number generator


    pthread_t ghostThreads[NUM_GHOSTS];

    for (int i = 0; i < NUM_GHOSTS; ++i) {
        pthread_create(&ghostThreads[i], NULL, (void *(*)(void *))moveGhost, (void *)(intptr_t)i);
    }
    sem_init(&ghostSemaphore,0,1);
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(600,500);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Pacman");
 	initOpenGL();
    glutDisplayFunc(display);
    glutSpecialFunc(keyboard);
    
    
    pthread_t uithread;
    pthread_mutex_lock(&menuMutex);
    pthread_create(&uithread,NULL,&uithreadfunc,NULL);
    pthread_mutex_unlock(&menuMutex);


  
		
    glutMainLoop();
    
    running = false; // Stop threads after exiting the main loop

 
    sem_destroy(&ghostSemaphore); 

    return 0;
}
