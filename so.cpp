#include <GL/glut.h>
#include <thread>
#include <chrono>

// CONSTANTS //
const std::chrono::milliseconds SLEEP_TIME
	= std::chrono::milliseconds { 50 };
	// WINDOW //
const int WINDOW_WIDTH = 400;
const int WINDOW_HEIGHT = 300;
const int WINDOW_X = 100;
const int WINDOW_Y = 100;
const char* WINDOW_TITLE = "SO2 projekt etap 1";
	// ------ //
	// PANEL //
const int PANEL_RED = 255;
const int PANEL_GREEN = 0;
const int PANEL_BLUE = 0;
const GLint panelMaxSpeed = 10;
	// ----- //
// --------- //

// GLOBAL VARIABLES //
std::thread panelThread;
bool gameOver = false;
	// PANEL //
GLint panelX1 = 100;
GLint panelY1 = 200;
GLint panelX2 = 200;
GLint panelY2 = 250;
GLint panelSpeed = 5;
bool panelGoesRight = true;
	// ----- //
// ---------------- //

// FUNCTIONS //
void clear() { glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); }

	// INIT //
void initGlut(int argc, char** argv) {
	glutInit(&argc, argv);
   	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
}

void initWindow() {
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitWindowPosition(WINDOW_X, WINDOW_Y);
	glutCreateWindow(WINDOW_TITLE);
}

void initRestOfGlut() {
	glClearColor(0, 0, 0, 0);
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, WINDOW_WIDTH - 1, WINDOW_HEIGHT - 1, 0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
}

void init(int argc, char** argv) {
	initGlut(argc, argv);
	initWindow();
	initRestOfGlut();
}
	// ---- //

	// PANEL //
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
		glutPostRedisplay();
		std::this_thread::sleep_for(SLEEP_TIME);
	}
}

void drawAll() {
		// PANEL //
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
		// ----- //
}
	// ----- //

void checkIfGameOver(unsigned char key, int x, int y) {
	if(key == 27) {
		panelThread.join();
		gameOver = true;
		exit(0);
	}
}
// -------- //

int main (int argc, char** argv) {
	init(argc, argv);
	panelThread = std::thread(movePanel);
	glutDisplayFunc(drawAll);
	glutKeyboardFunc(checkIfGameOver);
	glutMainLoop();
	return 0;
}
