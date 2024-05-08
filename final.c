#include <GL/glut.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include<unistd.h>
#include <string.h>
#include <stdio.h>


#define NUM_GHOSTS 4
#define GHOST_HOUSE_X 7
#define GHOST_HOUSE_Y 6
#define NUM_KEYS 2
#define NUM_PERMITS 2

// Semaphores for keys and exit permits
sem_t keys;
sem_t exit_permits;
bool menuactive=false;
// Mutex for printing to avoid garbling in stdout
pthread_mutex_t print_mutex;

// Define the grid size
#define GRID_WIDTH 15
#define GRID_HEIGHT 15

// Ghost vulnerability state
bool ghostsVulnerable = false;
sem_t ghostSemaphore;
bool powerPelletActive = false;
int vulnerabilityTimeLeft = 0;  // in milliseconds


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
    {3, 3}
};

pthread_mutex_t ghostMutex = PTHREAD_MUTEX_INITIALIZER;
bool running = true;
int score=0;
int lives=3;

// Maze configuration
int maze[GRID_HEIGHT][GRID_WIDTH] = {
    {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},	//1
    {0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0},	//2
    {2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2},	//3
    {0, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0},	//4
    {0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0},	//5
    {1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1},	//6
    {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},	//7
    {1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1},	//8
    {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},	//9
    {0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0},	//10
    {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},	//11
    {1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 1},	//12
    {2, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 2},	//13
    {0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 0},	//14
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	//15
};

struct {
    int x;
    int y;
} pacman = {7, 10}; // initial x and y position
//=======================================================================================

//======================================================================================
void* ghost_routine(void* id) {
    int ghost_id = *((int*)id);
    while (1) {
        pthread_mutex_lock(&print_mutex);
        printf("Ghost %d is trying to get a key and an exit permit.\n", ghost_id);
        pthread_mutex_unlock(&print_mutex);

        // Try to acquire a key
        sem_wait(&keys);
        pthread_mutex_lock(&print_mutex);
        printf("Ghost %d has taken a key.\n", ghost_id);
        pthread_mutex_unlock(&print_mutex);

        // Try to acquire an exit permit
        sem_wait(&exit_permits);
        pthread_mutex_lock(&print_mutex);
        printf("Ghost %d has taken an exit permit and is leaving the house.\n", ghost_id);
        pthread_mutex_unlock(&print_mutex);

        // Simulate the ghost leaving
        pthread_mutex_lock(&print_mutex);
        printf("Ghost %d has left the ghost house.\n", ghost_id);
        pthread_mutex_unlock(&print_mutex);

        // Release both resources
        sem_post(&keys);
        sem_post(&exit_permits);

        pthread_mutex_lock(&print_mutex);
        printf("Ghost %d has returned both the key and the exit permit.\n", ghost_id);
        pthread_mutex_unlock(&print_mutex);
    }

    return NULL;
}
//=======================================================================================
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
//====================================================================================
void displayGameOver() {
    
    running = false;
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(1.0, 1.0, 1.0);
    const char *message = "Game Over!";
    glRasterPos2f(150 - strlen(message) * 4.5, 150);  
    // Print each character with a bitmap font
    for (const char *c = message; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *c);
    }
    // Swap buffers to display the message
    glutSwapBuffers();

    // Properly end the application
    exit(0);
}


//=====================================================================================
bool isPositionOccupiedByAnotherGhost(int x, int y, int currentGhostIndex) {
    for (int i = 0; i < NUM_GHOSTS; i++) {
        if (i != currentGhostIndex && ghosts[i].x == x && ghosts[i].y == y) {
            return true;
        }
    }
    return false;
}
//======================================================================================
void moveGhost(int index) {
    while (running) {
        usleep(500000); // Delay between moves (500 milliseconds)
         
        int direction = rand() % 4; // Random direction
	bool isVulnerable = ghostsVulnerable;
   
        int dx = 0, dy = 0;	//positions
        switch (direction) {
            case 0: dx = 2;  break; // Right
            case 1: dx = -2; break; // Left
            case 2: dy = 2;  break; // Down
            case 3: dy = -2; break; // Up
        }

     //   pthread_mutex_lock(&ghostMutex);
        sem_wait(&ghostSemaphore);
        
       // int newX = (ghosts[index].x + dx ) ;	//new position of ghost
       //	int newY = (ghosts[index].y + dy ) ;
	
//	printf("The value of number is: %d\n", ghosts[index].x);
//	printf("The value of number is: %d\n", ghosts[index].y);
        int newX = (ghosts[index].x + dx + GRID_WIDTH) % GRID_WIDTH;
        int newY = (ghosts[index].y + dy + GRID_HEIGHT) % GRID_HEIGHT;
        if (!maze[newY][newX] && !isPositionOccupiedByAnotherGhost(newX, newY, index)) { // Check if new position is not a wall if wall no move
            ghosts[index].x = newX;
            ghosts[index].y = newY;
        }
         if (ghosts[index].x == pacman.x && ghosts[index].y == pacman.y) {
            if (ghostsVulnerable) {
                score += 200;  // Pacman eats vulnerable ghost
                ghosts[index].x = GHOST_HOUSE_X;
                ghosts[index].y = GHOST_HOUSE_Y;
            } else {
                // Pacman collides with a non-vulnerable ghost
                lives -= 1;  // Decrease lives
                pacman.x = 7;  // Reset Pacman position to starting position
                pacman.y = 10;
                if (lives <= 0) {
                    running = false;  // Game over
                     displayGameOver();  // Call to display game over screen
                    break;  // Exit the loop
                }
            }
        }

        sem_post(&ghostSemaphore);
    
    }
}
//======================================================================================//
void drawGhosts() {
    // Define the colors for the ghosts
    float colors[4][3] = {
        {1.0f, 0.5f, 0.0f}, // Orange
        {0.56f, 0.0f, 1.0f}, // Blue
        {1.0f, 0.0f, 0.0f}, // Red
        {1.0f, 0.5f, 0.8f}  // Pink
    };
    
    sem_wait(&ghostSemaphore);
    if (ghostsVulnerable) {
        for (int i = 0; i < NUM_GHOSTS; ++i) {
            colors[i][0] = 0.0f; // Blue when vulnerable
            colors[i][1] = 0.0f;
            colors[i][2] = 1.0f;
        }
    }
    for (int i = 0; i < NUM_GHOSTS; ++i) {
        glColor3f(colors[i][0], colors[i][1], colors[i][2]); // Set the color for each ghost
        float ghostX = ghosts[i].x * 20 + 10;
        float ghostY = ghosts[i].y * 20 + 10;
        drawCircle(ghostX, ghostY, 10, 20); // Draw ghost as a circle
    }
    sem_post(&ghostSemaphore);
}

//==================================================================================
void resetVulnerability(int value) {
    sem_wait(&ghostSemaphore);
    
        ghostsVulnerable = false;
        powerPelletActive = false;
    sem_post(&ghostSemaphore);
    glutPostRedisplay();
}


//======================================================================================
void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();



	if(menuactive)
	{
		glutSwapBuffers();
		return;
	}




    // Existing code to draw walls and pellets
    float wallThickness=1.0f;
    float blockSize = 20.0f; // Size of each block in the grid

    // Draw maze walls
    glColor3f(0.0f, 0.0f, 0.5f); // Blue walls
    for (int i = 0; i < GRID_HEIGHT; ++i) {
        for (int j = 0; j < GRID_WIDTH; ++j) {
            if (maze[i][j]==1) {  // If there's a wall, draw it
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
            glColor3f(1.0f, 1.0f, 0.0f); // Reset color to yellow
            glPointSize(5.0f); // Reset size to normal
                glVertex2f(j * blockSize + blockSize / 2, i * blockSize + blockSize / 2);
            }
              else if (maze[i][j] == 2) {
            // Draw special power pellets
            glColor3f(1.0f, 0.0f, 0.0f); // Red for power pellets
            glPointSize(10.0f); // Larger size for power pellets
            glVertex2f(j * 20 + 10, i * 20 + 10); // Center the power pellet
             glutTimerFunc(10000, resetVulnerability, 0);  
        } 
        }
    }
    glEnd();
    drawPacman(); // Draw pacman
    drawGhosts(); // Draw ghosts
 glColor3f(1.0, 1.0, 1.0); // White color
    glRasterPos2f(5, 285);
    char scoreText[50];
    char livesText[50];
    sprintf(livesText,"Lives: %d",lives);
    sprintf(scoreText, "Score: %d", score);
    for (char* c = scoreText; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *c);
    }
    glRasterPos2f(5, 300); 
     for (char* c = livesText; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *c);
    }
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
    } else if (nextX < 0 || nextX >= GRID_WIDTH || nextY < 0 || nextY >= GRID_HEIGHT || maze[nextY][nextX]==1) {
        return;
    }
      
	
	 // Check for yellow pellets
    if (maze[nextY][nextX] == 0) {
        maze[nextY][nextX] = -1; // Mark the pellet as consumed
        score += 10; // Increment the score
    }

    // Check for power pellets
    if (maze[nextY][nextX] == 2 && !powerPelletActive) {
        maze[nextY][nextX] = -1; // Consume the power pellet
        score += 50;
        sem_wait(&ghostSemaphore);
        ghostsVulnerable = true; 
        powerPelletActive = true;
        sem_post(&ghostSemaphore);
        glutTimerFunc(10000, resetVulnerability, 0);  // 10 seconds of vulnerability
    
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
// Function to display rules
void displayRules() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    // Set the color for the text (white in this case)
    glColor3f(0.90f, 0.80f, 0.95f);

	int windowWidth = glutGet(GLUT_WINDOW_WIDTH);
    int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);
    int textWidth = 200; // Adjust as needed
    int textHeight = 20; // Adjust as needed
    int x = (windowWidth - textWidth) / 2-30; // Center horizontally
    int y = (windowHeight - textHeight)/2 +50; // 10 pixels from the top

   
    glRasterPos2i(x, y); // Set the raster position
    char text[] = "Welcome to Pacman! "; // Change this to your actual rules text
	char text2[] = "1. Eat pellets and avoid ghosts.";
    char text3[] = "2. Use arrow keys to move.";
    char text4[] = "3. You have three lives:)";
    // Loop through the characters in the text and draw them
    for (int i = 0; text[i] != '\0'; i++) {
        glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24 , text[i]);
    }
	
	y -= textHeight + 5; // Adjust the vertical spacing as needed

    glRasterPos2i(x, y); // Set the raster position for the second line
    for (int i = 0; text2[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, text2[i]);
    }
	//(0.01f, 0.32f, 0.14f);
    // Move to the next line
    y -= textHeight + 5; // Adjust the vertical spacing as needed

    glRasterPos2i(x, y); // Set the raster position for the third line
    for (int i = 0; text3[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, text3[i]);
    }
    
    y -= textHeight + 5; // Adjust the vertical spacing as needed

    glRasterPos2i(x, y); // Set the raster position for the third line
    for (int i = 0; text4[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, text4[i]);
    }

	
    glutSwapBuffers();
}

void reshapeRules(int width, int height) {
    // Set viewport and projection matrix
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, width, 0, height);
    glMatrixMode(GL_MODELVIEW);
}
void menu(int value) {
    switch (value) {
       // case 1:
         //   printf("Resume game\n");
           // menuactive=false;
           // break;
        case 1:
            printf("Exit game\n");
            exit(0);
            break;
        case 2:
            printf("Check Score\n");
         //   exit(0);
            break;
		case 3:
            printf("Rules\n");
          //  exit(0);
         //   rulesWindowActive = true;
            glutInitWindowSize(400, 400);
            glutInitWindowPosition(100, 100);
            glutCreateWindow("Game Rules");
            glutDisplayFunc(displayRules);
            glutReshapeFunc(reshapeRules);
            break;
            break;
    }
}



//=============
void* uithreadfunc(void* arg)
{
	
    int menu_id = glutCreateMenu(menu);
  //  glutAddMenuEntry("Resume Game", 1);
    glutAddMenuEntry("Exit Game", 1);
    glutAddMenuEntry("Check Score", 2);
    glutAddMenuEntry("Rules", 3);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
   

}












//=========================================================================//
int main(int argc, char** argv) {
    
 // Initialize the semaphore
    sem_init(&ghostSemaphore, 0, 1);

    sem_init(&keys, 0, NUM_KEYS);
    sem_init(&exit_permits, 0, NUM_PERMITS);
    pthread_mutex_init(&print_mutex, NULL);
   
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
    glutCreateWindow("22i-0846_22i-1036_J_Project");
 
    initOpenGL();
    glutDisplayFunc(display);
    glutSpecialFunc(keyboard);
    
//-----------------------------------------
	pthread_t uithread;
    pthread_mutex_lock(&menuMutex);
    pthread_create(&uithread,NULL,&uithreadfunc,NULL);
    pthread_mutex_unlock(&menuMutex);

//-----------------------------------------

    glutMainLoop();
    
    running = false; // Stop threads after exiting the main loop

    sem_destroy(&ghostSemaphore); 
sem_init(&ghostSemaphore, 0, 1);
 sem_destroy(&exit_permits);
    pthread_mutex_destroy(&print_mutex);

    return 0;
}
