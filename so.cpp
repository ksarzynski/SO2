#include <GL/glut.h>
#include <thread>
#include <chrono>
#include <vector>
#include <list>
#include <iterator>
#include <cstdlib>
#include <time.h>
#include <algorithm>
#include <math.h>
#include <mutex>

/*
	1. Wektor zmieniony na liste
*/

const std::chrono::milliseconds SLEEP_TIME
	= std::chrono::milliseconds { 50 };
const int MIN_BALLS_COOLDOWN_TIME = 500;
const int MAX_BALLS_COOLDOWN_TIME = 1000;
const int WINDOW_WIDTH = 400;
const int WINDOW_HEIGHT = 300;
const int WINDOW_X = 100;
const int WINDOW_Y = 100;
const char* WINDOW_TITLE = "SO2 projekt etap 1";
const int B_R_COLOR = 255;
const int B_G_COLOR = 255;
const int B_B_COLOR = 255;
const int PANEL_RED = 255;
const int PANEL_GREEN = 0;
const int PANEL_BLUE = 0;
const GLint PANEL_MIN_SPEED = 3;
const GLint PANEL_MAX_SPEED = 12;
const GLint BALL_MIN_SPEED = 3;
const GLint BALL_MAX_SPEED = 12;
const int BOUNCES = 6;
std::thread panelThread;
std::thread redisplayThread;
std::thread ballThrowerThread;
std::list<std::thread> ballsThreads;
bool gameOver = false;
GLint panelX1 = 100;
GLint panelY1 = 200;
GLint panelX2 = 200;
GLint panelY2 = 250;
GLint panelSpeed = rand() % PANEL_MAX_SPEED + PANEL_MIN_SPEED;
bool panelGoesRight = true;
int ballCount = 0;
std::mutex panelMutex;

class Ball {

public:
	int id;
	float ballX;
	float ballY;
	float radius;
	int ballRed;
	int ballGreen;
	int ballBlue;
	GLint verticalSpeed;
	GLint horizontalSpeed;
	int bounces;
	bool isDead;
	Ball() {
		ballCount++;
		id = ballCount;
		ballX = 100;
		ballY = 0;
		radius = 3;
		ballRed = ((10 * id) + 1 * id) % 255;
		ballGreen = ((20 * id) + 2 * id) % 255;
		ballBlue = ((30 * id) + 3 * id) % 255;
		verticalSpeed = rand() % BALL_MAX_SPEED + BALL_MIN_SPEED;
		horizontalSpeed = 2 + rand() % BALL_MAX_SPEED + BALL_MIN_SPEED;
		int r = rand() % 2;
		if(r == 0)
			verticalSpeed = (-1) * verticalSpeed;
		bounces = BOUNCES;
		isDead = false;
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

void endThreads() {
	panelThread.join();
	redisplayThread.join();
	ballThrowerThread.join();
	std::list<std::thread>::iterator it;
	for(it = ballsThreads.begin(); it != ballsThreads.end(); it++){
		(*it).join();
	}
}

void checkIfGameOver(unsigned char key, int x, int y) {
	if(key == 27) {
		gameOver = true;
		endThreads();
		exit(0);
	}
}

// to be updated
void redisplayFunc() {
	while(!gameOver)
		glutPostRedisplay();
}

void drawBall(Ball* ball) {
	if(!ball->isDead) {
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		glTranslatef(ball->ballX, ball->ballY, 0.0f);
		int circlePoints = 100;
		float angle = 2.0f * 3.14f / circlePoints;
		glBegin(GL_POLYGON);
		glColor3ub(ball->ballRed, ball->ballGreen, ball->ballBlue);
		double currentAngle = 0.0;
		glVertex2d(ball->radius * std::cos(0.0), ball->radius * std::sin(0.0));
		for(int i = 0; i < circlePoints; i++) {
			glVertex2d(ball->radius * std::cos(currentAngle), ball->radius * std::sin(currentAngle));
			currentAngle += angle;
		}
		glEnd();
		glPopMatrix();
	}
}

void drawRectangle(int x1, int y1, int x2, int y2, int r, int g, int b) {
	glBegin(GL_QUADS);
	glColor3ub(r, g, b);
	glVertex2i(x1, y1);
	glVertex2i(x2, y1);
	glVertex2i(x2, y2);
	glVertex2i(x1, y2);
	glEnd();
}

void drawAll() {
	clear();
	glLoadIdentity();
	drawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, B_R_COLOR, B_G_COLOR, B_B_COLOR);
	drawRectangle(panelX1, panelY1, panelX2, panelY2, PANEL_RED, PANEL_GREEN, PANEL_BLUE);
	for(int i = 0; i < ballCount; i++)
		drawBall(balls[i]);
	glutSwapBuffers();
}

void movePanel() {
	while(!gameOver) {
		panelMutex.lock();
		GLint newPanelX1 = panelX1;
		GLint newPanelX2 = panelX2;
		if(panelGoesRight) {
			newPanelX1 +=  panelSpeed;
			newPanelX2 +=  panelSpeed;
			if(newPanelX2 > WINDOW_WIDTH) {
				panelGoesRight = !panelGoesRight;
				newPanelX1 -= panelSpeed;
				newPanelX2 -= panelSpeed;
				panelSpeed = rand() % PANEL_MAX_SPEED + PANEL_MIN_SPEED;
			}
		}
		else {
			newPanelX1 -= panelSpeed;
			newPanelX2 -= panelSpeed;
			if(newPanelX1 < 0) {
				panelGoesRight = !panelGoesRight;
				newPanelX1 += panelSpeed;
				newPanelX2 += panelSpeed;
				panelSpeed = rand() % PANEL_MAX_SPEED + PANEL_MIN_SPEED;
			}
		}
		panelX1 = newPanelX1;
		panelX2 = newPanelX2;
		panelMutex.unlock();
		std::this_thread::sleep_for(SLEEP_TIME);
	}
}

void moveBall(Ball* ball) {
	while(!gameOver && ball->bounces && !ball->isDead) {
		GLint newBallX = ball->ballX + ball->verticalSpeed;
		GLint newBallY = ball->ballY + ball->horizontalSpeed;
		if(newBallX + ball->radius >= WINDOW_WIDTH ||
			newBallX - ball->radius <= 0) {
			ball->verticalSpeed = (-1) * ball->verticalSpeed;
			ball->bounces--;
		}
		if(newBallY + ball->radius >= WINDOW_HEIGHT ||
			newBallY - ball->radius <= 0) {
			ball->horizontalSpeed = (-1) * ball->horizontalSpeed;
			ball->bounces--;
		}
		if(newBallX - ball->radius >= panelX1 && newBallX + ball->radius <= panelX2 &&
			newBallY + ball->radius >= panelY1 && newBallY - ball->radius <= panelY2){
			ball->verticalSpeed = (-1) * ball->verticalSpeed;
			ball->horizontalSpeed = (-1) * ball->horizontalSpeed;
			continue;
		}
		ball->ballX = newBallX;
		ball->ballY = newBallY;
		std::this_thread::sleep_for(SLEEP_TIME);
	}
	ball->isDead = true;
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
	int timeBreak = 0;
	while(!gameOver) {
		std::this_thread::sleep_for(std::chrono::milliseconds { timeBreak });
		Ball* ball = new Ball();
		balls.push_back(ball);
		ballsThreads.push_back(std::thread(moveBall, ball));
		timeBreak = rand() % MAX_BALLS_COOLDOWN_TIME + MIN_BALLS_COOLDOWN_TIME;
	}
}

void initThreads() {
	panelThread = std::thread(movePanel);
	redisplayThread = std::thread(redisplayFunc);
	ballThrowerThread = std::thread(ballThrowerFunc);
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

// kulki nie beda wchodzilo do prostokata tylko sie od niego odbijaly
// jak jedna kulka uderzy i sie odbije to dla nastepnej prostokat jest przezroczysty
// nastepna sie odbija normalnie
