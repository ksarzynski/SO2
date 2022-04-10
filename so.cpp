#include <GL/glut.h>
#include <thread>
#include <chrono>
#include <vector>
#include <cstdlib>
#include <time.h>

const std::chrono::milliseconds SLEEP_TIME
	= std::chrono::milliseconds { 50 };
const std::chrono::milliseconds BALLS_COOLDOWN_TIME
	= std::chrono::milliseconds { 5000 };
const int WINDOW_WIDTH = 400;
const int WINDOW_HEIGHT = 300;
const int WINDOW_X = 100;
const int WINDOW_Y = 100;
const char* WINDOW_TITLE = "SO2 projekt etap 1";
const int PANEL_RED = 255;
const int PANEL_GREEN = 0;
const int PANEL_BLUE = 0;
const GLint PANEL_MIN_SPEED = 2;
const GLint PANEL_MAX_SPEED = 10;
const GLint BALL_MIN_SPEED = 5;
const GLint BALL_MAX_SPEED = 20;
const int BOUNCES = 6;
std::thread panelThread;
std::thread redisplayThread;
std::vector<std::thread> ballsThreads;
bool gameOver = false;
GLint panelX1 = 100;
GLint panelY1 = 200;
GLint panelX2 = 200;
GLint panelY2 = 250;
GLint panelSpeed = 5;
bool panelGoesRight = true;

class Ball {

public:
	const int bounces = BOUNCES;
	static int id;
	GLint ballX;
	GLint ballY;
	GLint radius;
	int ballRed;
	int ballGreen;
	int ballBlue;
	GLint verticalSpeed;
	GLint horizontalSpeed;

	Ball() {
		id++;
		ballX = 100;
		ballY = 0;
		radius = 3;
		ballRed = ((10 * id) + id) % 255;
		ballRed = ((10 * id) + id) % 255;
		ballRed = ((10 * id) + id) % 255;
		verticalSpeed = rand() % BALL_MAX_SPEED + BALL_MIN_SPEED;
		horizontalSpeed = rand() % BALL_MAX_SPEED + BALL_MIN_SPEED;
	}
};

std::vector<Ball*> balls;

void clear() { glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); }

void initGlut(int argc, char** argv) {
	glutInit(&argc, argv);
   	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
}

void initWindow() {
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitWindowPosition(WINDOW_X, WINDOW_Y);
	glutCreateWindow(WINDOW_TITLE);
}

void checkIfGameOver(unsigned char key, int x, int y) {
	if(key == 27) {
		gameOver = true;
		panelThread.join();
		redisplayThread.join();
		exit(0);
	}
}

void redisplayFunc() {
	while(!gameOver)
		glutPostRedisplay();
}

void drawAll() {
	clear();
	glLoadIdentity();
	glBegin(GL_QUADS);
	glColor3ub(PANEL_RED, PANEL_GREEN, PANEL_BLUE);
	glVertex2i(panelX1, panelY1);
	glVertex2i(panelX2, panelY1);
	glVertex2i(panelX2, panelY2);
	glVertex2i(panelX1, panelY2);
	glEnd();
	glutSwapBuffers();
}

void movePanel() {
	while(!gameOver) {
		GLint newPanelX1 = panelX1;
		GLint newPanelX2 = panelX2;
		if(panelGoesRight) {
			newPanelX1 += panelSpeed;
			newPanelX2 += panelSpeed;
			if(newPanelX2 > WINDOW_WIDTH) {
				panelGoesRight = !panelGoesRight;
				newPanelX1 -= panelSpeed;
				newPanelX2 -= panelSpeed;
			}
		}
		else {
			newPanelX1 -= panelSpeed;
			newPanelX2 -= panelSpeed;
			if(newPanelX1 < 0) {
				panelGoesRight = !panelGoesRight;
				newPanelX1 += panelSpeed;
				newPanelX2 += panelSpeed;
			}
		}
		panelX1 = newPanelX1;
		panelX2 = newPanelX2;
		std::this_thread::sleep_for(SLEEP_TIME);
	}
}

void moveBall(Ball* ball) {
	while(!gameOver && ball->bounces) {
		GLint newBallX = ball->ballX + ball->verticalSpeed;
		GLint newBallY = ball->ballY + ball->horizontalSpeed;
		if(newBallX + ball->radius > WINDOW_WIDTH ||
			newBallX - ball->radius < 0) {
			ball->verticalSpeed = (-1) * ball->verticalSpeed;
			continue;
		}
		if(newBallY + ball->radius > WINDOW_HEIGHT ||
			newBallY - ball->radius < 0) {
			ball->horizontalSpeed = (-1) * ball->horizontalSpeed;
			continue;
		}
		ball->ballX = newBallX;
		ball->ballY = newBallY;
	}
}

void initRestOfGlut() {
	glClearColor(0, 0, 0, 0);
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, WINDOW_WIDTH - 1, WINDOW_HEIGHT - 1, 0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glutDisplayFunc(drawAll);
	glutKeyboardFunc(checkIfGameOver);
}

void ballThrowerFunc() {
	while(!gameOver) {
		std::this_thread::sleep_for(BALLS_COOLDOWN_TIME);
	}
}

void initThreads() {
	panelThread = std::thread(movePanel);
	redisplayThread = std::thread(redisplayFunc);
}

void init(int argc, char** argv) {
	initGlut(argc, argv);
	initWindow();
	initRestOfGlut();
	initThreads();
	glutMainLoop();
}

int main (int argc, char** argv) {
	init(argc, argv);
	return 0;
}
