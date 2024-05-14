#include <GL/glut.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
//#include <cstring>
#include <string.h>
#include <stdio.h>

#define NUM_GHOSTS 4
#define GHOST_HOUSE_X 7
#define GHOST_HOUSE_Y 6
#define GRID_WIDTH 15
#define GRID_HEIGHT 15
#define NUM_SPEED_BOOSTS 2
sem_t speedBoosts;
sem_t fastGhost;
// Mutex for printing to avoid garbling in stdout
pthread_mutex_t print_mutex;

// Ghost vulnerability state
bool ghostsVulnerable = false;
sem_t ghostSemaphore;
bool powerPelletActive = false;
int vulnerabilityTimeLeft = 0; // in milliseconds

// Initial square position and size
float x = 20.0f;
float y = 20.0f;
float side = 20.0f;

struct Ghost
{
    int x;
    int y;
};

struct Ghost ghosts[NUM_GHOSTS] = {
    {7, 5},
    {7, 6},
    {6, 6},
    {9, 6}
    };

pthread_mutex_t ghostMutex = PTHREAD_MUTEX_INITIALIZER;
bool running = true;
int score = 0;
int lives = 3;

// Maze configuration
int maze[GRID_HEIGHT][GRID_WIDTH] = {
    {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0}, //1
    {0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0}, //2
    {2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2}, //3
    {0, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0}, //4
    {0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0}, //5
    {1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1}, //6
    {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0}, //7
    {1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1}, //8
    {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0}, //9
    {0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0}, //10
    {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0}, //11
    {1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 1}, //12
    {2, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 2}, //13
    {0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 0}, //14
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //15
};

struct
{
    int x;
    int y;
} pacman = {7, 10}; // initial x and y position

//=======================================================================================
/*void drawSpeedBooster() {
    // Check if a speed booster is present in the maze
    bool speedBoosterExists = false;
    for (int i = 0; i < GRID_HEIGHT; ++i) {
        for (int j = 0; j < GRID_WIDTH; ++j) {
            if (maze[i][j] == 3) {
                speedBoosterExists = true;
                break;
            }
        }
        if (speedBoosterExists) {
            break;
        }
    }

    // If a speed booster exists, draw it
    if (speedBoosterExists) {
        // Define the color for the speed booster
        float color[3] = {0.0f, 1.0f, 0.0f};  // Black color for the speed booster

        // Loop through the maze to find the speed booster and draw it
        for (int i = 0; i < GRID_HEIGHT; ++i) {
            for (int j = 0; j < GRID_WIDTH; ++j) {
                if (maze[i][j] == 3) { // Check if the current cell contains a speed booster
                    // Calculate the center position of the speed booster
                    float centerX = j * 20 + 10; // 20 is the size of each block, and 10 is half of 20
                    float centerY = i * 20 + 10; // Same logic applies for the Y coordinate
                    
                    // Draw the speed booster as a small square or any other desired shape
                    glColor3f(color[0], color[1], color[2]);
                    glBegin(GL_QUADS);
                    glVertex2f(centerX - 5, centerY - 5); // Top-left corner
                    glVertex2f(centerX + 5, centerY - 5); // Top-right corner
                    glVertex2f(centerX + 5, centerY + 5); // Bottom-right corner
                    glVertex2f(centerX - 5, centerY + 5); // Bottom-left corner
                    glEnd();
                }
            }
        }
    }
}*/

//=======================================================================================
// Function to draw a circle
void drawCircle(float cx, float cy, float r, int num_segments)
{
    glBegin(GL_POLYGON);
    for (int i = 0; i < num_segments; i++)
    {
        float theta = 2.0f * 3.1415926f * (float)i / (float)(num_segments); // Get the current angle
        float x = r * cosf(theta);                                            // Calculate the x component
        float y = r * sinf(theta);                                            // Calculate the y component
        glVertex2f(x + cx, y + cy);                                           // Output vertex
    }
    glEnd();
}
//======================================================================================
void drawPacman()
{
    glColor3f(1.0f, 1.0f, 0.0f); // Set Pacman color to yellow
    float pacmanX = pacman.x * 20 + 10; // Calculate pixel x position from grid position
    float pacmanY = pacman.y * 20 + 10; // Calculate pixel y position from grid position
    drawCircle(pacmanX, pacmanY, 10, 20); // Draw
}
//====================================================================================
void displayGameOver()
{

    running = false;
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(1.0, 1.0, 1.0);
    const char *message = "Game Over!";
    glRasterPos2f(150 - strlen(message) * 4.5, 150);
    // Print each character with a bitmap font
    for (const char *c = message; *c != '\0'; c++)
    {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *c);
    }
    // Swap buffers to display the message
    glutSwapBuffers();

    // Properly end the application
    exit(0);
}

//=====================================================================================
bool isPositionOccupiedByAnotherGhost(int x, int y, int currentGhostIndex)
{
    for (int i = 0; i < NUM_GHOSTS; i++)
    {
        if (i != currentGhostIndex && ghosts[i].x == x && ghosts[i].y == y)
        {
            return true;
        }
    }
    return false;
}
//====================================================================================
void* fastGhostRoutine(int id) {
    int ghost_id = id;
    while (running) {
        usleep(500000); // Normal move delay

        pthread_mutex_lock(&print_mutex);
        printf("Fast Ghost %d is trying to get a speed boost.\n", ghost_id);
        pthread_mutex_unlock(&print_mutex);

        // Try to acquire a speed boost
        sem_wait(&speedBoosts);

        pthread_mutex_lock(&print_mutex);
        printf("Fast Ghost %d has received a speed boost.\n", ghost_id);
        pthread_mutex_unlock(&print_mutex);

        // Move faster
        usleep(250000); 

        sem_post(&speedBoosts);

        pthread_mutex_lock(&print_mutex);
        printf("Fast Ghost %d has released the speed boost.\n", ghost_id);
        pthread_mutex_unlock(&print_mutex);

        // (same as in moveGhost function)
        int direction = rand() % 4; // Random direction
        int dx = 0, dy = 0;
        switch (direction) {
            case 0: dx = 2;  break; // Right
            case 1: dx = -2; break; // Left
            case 2: dy = 2;  break; // Down
            case 3: dy = -2; break; // Up
        }

	}
    return NULL;
}
//==============================================
bool canBeFast(int index)
{
	//if((pacman.x == 12 && pacman.y == 0) || (pacman.x == 3 && pacman.y ==14) ||(ghosts[index].x ==12 && ghosts[index].y== 0) ||(ghosts[index].x == 3 && ghosts[index].y == 14))
		return 1;
	//return 0;
	

}
void increaseSpeed(int index)
{

	while (ghostsVulnerable) {
		usleep(5000); // Determines speed of the ghosts
		printf("the val: %d\n",index);   
		int direction = rand() % 4; // Random direction
		//bool isVulnerable = ghostsVulnerable;
   
        int dx = 0, dy = 0;	//positions
        switch (direction) {
            case 0: dx = 2;  break; // Right
            case 1: dx = -2; break; // Left
            case 2: dy = 2;  break; // Down
            case 3: dy = -2; break; // Up
        }
        
       // printf("sf");
        sem_wait(&ghostSemaphore);
        //13--12,0
        //3,14

        int newX = (ghosts[index].x + dx + GRID_WIDTH) % GRID_WIDTH;
        int newY = (ghosts[index].y + dy + GRID_HEIGHT) % GRID_HEIGHT;
        if (!maze[newY][newX] && !isPositionOccupiedByAnotherGhost(newX, newY, index)) { // Check if new position is not a wall if wall no move
            ghosts[index].x = newX;
            ghosts[index].y = newY;
        }
        
		
        
         if(ghosts[0].x==pacman.x && ghosts[0].y == pacman.y  || ghosts[1].x==pacman.x && ghosts[1].y == pacman.y  || ghosts[2].x==pacman.x && ghosts[2].y == pacman.y  || 
         ghosts[3].x==pacman.x && ghosts[3].y == pacman.y  ){
            if (ghostsVulnerable) {
                score += 200;  // Pacman eats  ghost
                ghosts[index].x = GHOST_HOUSE_X;
                ghosts[index].y = GHOST_HOUSE_Y;
            } else {
                //ghost eat pacman
                lives -= 1;  // lives decreaes
                pacman.x = 7;  // back to start position 
                pacman.y = 10;
                if (lives <= 0) {
                    running = false;  // Game over
                    displayGameOver();  
                    sem_post(&ghostSemaphore);
                    break;  // Exit the loop
                }
            }
        }
        sem_post(&ghostSemaphore);
		
	}
	sem_post(&fastGhost);
}
	
//======================================================================================
void moveGhost(int index)
{
	int val;
    while (running)
    {
    	int direction = rand() % 4; // Random direction
        bool isVulnerable = ghostsVulnerable;
        int dx = 0, dy = 0; 
        usleep(500000); // speed decrease this to make faster
	
		printf("main\n");
	//	printf("the val: %d\n",index);   
	    //positions
        switch (direction)
        {
        case 0:
            dx = 1;
            break; // Right
        case 1:
            dx = -1;
            break; // Left
        case 2:
            dy = 1;
            break; // Down
        case 3:
            dy = -1;
            break; // Up
        }
        sem_wait(&ghostSemaphore);
        if(canBeFast(index)==1 && ghostsVulnerable == 1 &&  sem_trywait(&fastGhost) == 0)	//make two ghost faster 4th wala
		{
			//sem_wait(&fastGhost);
			sem_post(&ghostSemaphore);
			increaseSpeed(index);
			continue;
			///sem_post(&fastGhost);
		
		}

       
        int newX = (ghosts[index].x + dx + GRID_WIDTH) % GRID_WIDTH;
        int newY = (ghosts[index].y + dy + GRID_HEIGHT) % GRID_HEIGHT;
        if (!maze[newY][newX] && !isPositionOccupiedByAnotherGhost(newX, newY, index))
        { // Check if new position is not a wall if wall no move
            ghosts[index].x = newX;
            ghosts[index].y = newY;
        }
       
        
        if(ghosts[0].x==pacman.x && ghosts[0].y == pacman.y  || ghosts[1].x==pacman.x && ghosts[1].y == pacman.y  || ghosts[2].x==pacman.x && ghosts[2].y == pacman.y  || 
        ghosts[3].x==pacman.x && ghosts[3].y == pacman.y  )
        {
            if (ghostsVulnerable)
            {
                score += 200; // Pacman eats vulnerable ghost
                ghosts[index].x = GHOST_HOUSE_X;
                ghosts[index].y = GHOST_HOUSE_Y;
            }
            else
            {
                lives -= 1; 
                pacman.x = 7; 
                pacman.y = 10;
                if (lives <= 0)
                {
                    running = false; 
                    displayGameOver(); 
                    sem_post(&ghostSemaphore);
                    break;            
                }
            }
        }
       
       

        sem_post(&ghostSemaphore);
    }
}
//======================================================================================//
void drawGhosts()
{
    //colors 
    float colors[4][3] = {
        {1.0f, 0.5f, 0.0f}, // Orange
        {0.56f, 0.0f, 1.0f}, // purple
        {1.0f, 0.0f, 0.0f}, // Red
        {1.0f, 0.5f, 0.8f}  // Pink
    };

    sem_wait(&ghostSemaphore);
    if (ghostsVulnerable)
    {
        for (int i = 0; i < NUM_GHOSTS; ++i)
        {
            colors[i][0] = 0.0f; // Blue when vulnerable
            colors[i][1] = 0.0f;
            colors[i][2] = 1.0f;
        }
    }
    for (int i = 0; i < NUM_GHOSTS; ++i)
    {
        glColor3f(colors[i][0], colors[i][1], colors[i][2]); // Set the color for each ghost
        float ghostX = ghosts[i].x * 20 + 10;
        float ghostY = ghosts[i].y * 20 + 10;
        drawCircle(ghostX, ghostY, 10, 20); // Draw ghost as a circle
    }
    sem_post(&ghostSemaphore);
}

//==================================================================================
void resetVulnerability(int value)
{
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

    // Draw maze walls
    float wallThickness = 1.0f;
    float blockSize = 20.0f; 
    glColor3f(0.0f, 0.0f, 0.5f); // walls
    for (int i = 0; i < GRID_HEIGHT; ++i) {
        for (int j = 0; j < GRID_WIDTH; ++j) {
            if (maze[i][j] == 1) { // 1 represents wall
                glBegin(GL_QUADS);
                glVertex2f(j * blockSize, i * blockSize);
                glVertex2f((j + 1) * blockSize, i * blockSize);
                glVertex2f((j + 1) * blockSize, (i + wallThickness) * blockSize);
                glVertex2f(j * blockSize, (i + wallThickness) * blockSize);
                glEnd();
            }
        }
    }

    glColor3f(1.0f, 1.0f, 0.0f); // Yellow for pellets
    glPointSize(5.0f); // Size of each pellet
    glBegin(GL_POINTS);
    for (int i = 0; i < GRID_HEIGHT; ++i) {
        for (int j = 0; j < GRID_WIDTH; ++j) {
            if (!maze[i][j]) {
                glColor3f(1.0f, 1.0f, 0.0f); // Yellow for pellets
                glVertex2f(j * blockSize + blockSize / 2, i * blockSize + blockSize / 2);
            } else if (maze[i][j] == 2) { // Draw power pellets
                glColor3f(1.0f, 0.0f, 0.0f); // Red for power pellets
                glVertex2f(j * 20 + 10, i * 20 + 10); // Center the power pellet
            } 
        }
    }
    
   /* for (int i = 0; i < GRID_HEIGHT; ++i) {
        for (int j = 0; j < GRID_WIDTH; ++j) {
           if (maze[i][j] == 3) { 
                drawSpeedBooster();
            }
        }
    }*/
    glEnd();

    drawPacman(); 
    drawGhosts();
    glColor3f(1.0, 1.0, 1.0); // White color
    glRasterPos2f(5, 285);
    char scoreText[50];
    char livesText[50];
    sprintf(livesText, "Lives: %d", lives);
    sprintf(scoreText, "Score: %d", score);
    for (char *c = scoreText; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *c);
    }
    glRasterPos2f(5, 300);
    for (char *c = livesText; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *c);
    }

    glutSwapBuffers();
}


//======================================================================================
void keyboard(int key, int xx, int yy)
{
    int nextX = pacman.x;
    int nextY = pacman.y;

    switch (key)
    {
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
    if (nextY == 6 && nextX < 0)
    {
        nextX = 14;
    }
    else if (nextY == 6 && nextX >= 14)
    {
        nextX = 0;
    }
    else if (nextX < 0 || nextX >= GRID_WIDTH || nextY < 0 || nextY >= GRID_HEIGHT || maze[nextY][nextX] == 1)
    {
        return;
    }

    // Check for yellow pellets
    if (maze[nextY][nextX] == 0)
    {
        maze[nextY][nextX] = -1; 
        score += 10;             //Increment the score
    }

    //Check for red power pellets
    if (maze[nextY][nextX] == 2 && !powerPelletActive)
    {
        maze[nextY][nextX] = -1; 
        score += 50;
        sem_wait(&ghostSemaphore);
        ghostsVulnerable = true;
        powerPelletActive = true;
        sem_post(&ghostSemaphore);
        glutTimerFunc(10000, resetVulnerability, 0); // 10 seconds blue ghosts appear 
    }
  
    pacman.x = nextX;
    pacman.y = nextY;
	//drawBooster();
    glutPostRedisplay();
}
//=============================================================
void initOpenGL()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 300, 300, 0);
    glMatrixMode(GL_MODELVIEW);
}

//========================================================

pthread_mutex_t menuMutex = PTHREAD_MUTEX_INITIALIZER;
//RULELESSS
void displayRules() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

   
    glColor3f(0.90f, 0.80f, 0.95f);	//text color

	int windowWidth = glutGet(GLUT_WINDOW_WIDTH);
    int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);
    int textWidth = 200; // Adjust as needed
    int textHeight = 20; // Adjust as needed
    int x = (windowWidth - textWidth) / 2-30; // Center horizontally
    int y = (windowHeight - textHeight)/2 +50; // 10 pixels from the top

   
    glRasterPos2i(x, y); // Set the raster position
    char text[] = "Welcome to Pacman! "; 
	char text2[] = "1. Eat pellets and avoid ghosts.";
    char text3[] = "2. Use arrow keys to move.";
    char text4[] = "3. You have three lives:)";


	//displaying text on screen 
	
    for (int i = 0; text[i] != '\0'; i++) {
        glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24 , text[i]);
    }
	
	y -= textHeight + 5; // 1st line

    glRasterPos2i(x, y);
    for (int i = 0; text2[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, text2[i]);
    }
	
    y -= textHeight + 5; // 2nd line

    glRasterPos2i(x, y); 
    for (int i = 0; text3[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, text3[i]);
    }
    
    y -= textHeight + 5; //third line

    glRasterPos2i(x, y); 
    for (int i = 0; text4[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, text4[i]);
    }

	
    glutSwapBuffers();
}
//==============================================================
void reshapeRules(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, width, 0, height);
    glMatrixMode(GL_MODELVIEW);
}
//==============================================================
void menu(int value) {
    switch (value) {
     
        case 3:
            printf("Exit game\n");
            exit(0);
            break;
        case 1:
            printf("Play\n");
         //   exit(0);
            break;
		case 2:
            printf("Rules\n");
            glutInitWindowSize(400, 400);
            glutInitWindowPosition(100, 100);
            glutCreateWindow("Game Rules");
            glutDisplayFunc(displayRules);
            glutReshapeFunc(reshapeRules);
            break;
    }
}



//==============================================================
void* uithreadfunc(void* arg)
{
	
    int menu_id = glutCreateMenu(menu);
    glutAddMenuEntry("Play", 1);
    glutAddMenuEntry("Rules", 2);
    glutAddMenuEntry("Exit Game", 3);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
   

}
//=========================================================================//
int main(int argc, char **argv) {


	sem_init(&ghostSemaphore, 0, 1);
    sem_init(&fastGhost, 0, 2);
    sem_init(&speedBoosts, 0, NUM_SPEED_BOOSTS);
    
    pthread_mutex_init(&print_mutex, NULL);
    srand(time(0)); // Seed random number generator

	pthread_t ghostThreads[NUM_GHOSTS];
   
    for (int i = 0; i < NUM_GHOSTS; ++i) 
    {
        pthread_create(&ghostThreads[i], NULL, (void *(*)(void *))moveGhost, (void *)(intptr_t)i);
  
    }

    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(600, 500);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("22i-0846_22i-1036_J_Project");
    initOpenGL(); // Ensure OpenGL is initialized properly

   
    glutDisplayFunc(display);
    glutSpecialFunc(keyboard);
    
    //-----------------------------------------
	pthread_t uithread;
    pthread_mutex_lock(&menuMutex);
    //ui thread
    pthread_create(&uithread,NULL,&uithreadfunc,NULL);
    pthread_mutex_unlock(&menuMutex);

	//-----------------------------------------
    glutMainLoop();

    running = false; // Stop threads after exiting the main loop

   

    sem_destroy(&ghostSemaphore);
    sem_destroy(&fastGhost);
    pthread_mutex_destroy(&print_mutex);

    return 0;
}
