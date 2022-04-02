#include <ncurses.h>
#include <thread>
#include <chrono>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <vector>

static int PANEL_X = 4;
static const int PANEL_SLEEP = 50;
static bool END = false;
static const int MAIN_WINDOW_X = 2;
static const int MAIN_WINDOW_Y = 2;

struct Point{
	int x, y;
};

static const int BALLS = 6;
static Point balls[BALLS];
static std::vector<std::thread> threads;

Point initBall(){
	Point ball;
	ball.x = 4 + rand() % (COLS / 2) + COLS / 4;
	ball.y = 4;
	return ball;
};

void printBall(Point ball) {
	attron(COLOR_PAIR(1));
	mvprintw(ball.y - 1, ball.x, "*");
	mvprintw(ball.y, ball.x - 1, "***");
	mvprintw(ball.y + 1, ball.x, "*");
	attroff(COLOR_PAIR(1));
	refresh();
}

void moveBall(int id) {
	while(not END) {
		balls[id] = initBall();
		int velocity = 100;
		std::pair<int, int> direction{0,0};
		direction.first = rand() % 3 + 1;
		direction.second = rand() % 7 - 3;
		int bounce = 10 + rand() % 3;
		while(not END and bounce >= 0) {
			int y, x;
			y = balls[id].y + direction.first;
			x = balls[id].x + direction.second;
			if(y > LINES - 9) {
				if(bounce == 0)
					break;
				else {
					if(x < PANEL_X or x > PANEL_X + 25)
						break;
					bounce--;
					y = 2 * (LINES - 8) - y;
					direction.first = -direction.first;
				}
			}
			else if(y < 4) {
				if(bounce == 0)
					break;
				else {
					bounce--;
					y = 8 - y;
					direction.first = -direction.first;
				}
			}
			if(x < 4) {
				if(bounce == 0)
					break;
				else {
					bounce--;
					x = 8 - x;
					direction.second = -direction.second;
				}
			}
			if(x > COLS - 5) {
				if(bounce == 0)
					break;
				else {
					bounce--;
					x = 2 * (COLS - 5) - x;
					direction.second = -direction.second;
				}
			}
			balls[id].x = x;
			balls[id].y = y;
			std::this_thread::sleep_for(std::chrono::milliseconds(velocity));
		}
	}
}

void moveWindow(WINDOW* window, int startX) {
	wborder(window, ' ', ' ', ' ',' ',' ',' ',' ',' ');
	wrefresh(window);
	mvwin(window, LINES - 7, startX);
	box(window, 0, 0);
	wrefresh(window);
}

WINDOW* createNewWindow(int height, int width, int startY, int startX) {
	WINDOW* localWindow;
	localWindow = newwin(height, width, startY, startX);
	box(localWindow, 0 , 0);
	wrefresh(localWindow);
	return localWindow;
}

void clearScreen() {
	for(int row = 3; row <= LINES - 8; row++){
		move(row, 3);
		for(int col = 3; col <= COLS - 4; col++)
			addch(' ');
	}
}

void panelMovement() {
	bool direction = true;
	while(not END) {
		if(direction) {
			if(PANEL_X < COLS - 4 - 25)
				PANEL_X++;
			else {
				direction = not direction;
				PANEL_X--;
			}
		}
		else {
			if(PANEL_X > 6)
				PANEL_X--;
			else {
				direction = not direction;
				PANEL_X++;
			}
		}
		std::this_thread::sleep_for(
			std::chrono::milliseconds(PANEL_SLEEP));
	}
}

void printAll() {
	WINDOW* mainWindow;
	WINDOW* panel;
	int startX, startY, width, height;
	int ch;
	initscr();
	cbreak();
	keypad(stdscr, TRUE);
	noecho();
	start_color();
	init_pair(1, COLOR_BLUE, COLOR_RED);
	timeout(20);
	height = LINES - 4;
	width = COLS - 4;
	startY = 2;
	startX = 2;
	refresh();
	mainWindow = createNewWindow(
		height,
		width,
		MAIN_WINDOW_Y,
		MAIN_WINDOW_X);
	panel = createNewWindow(3, 25, LINES - 7, PANEL_X);
	std::thread panelThread(panelMovement);
	for(int i = 0; i < BALLS; i++)
		threads.push_back(std::thread(moveBall, i));
	while((ch = getch()) != KEY_F(2)) {
		clearScreen();
		moveWindow(panel, PANEL_X);
		wrefresh(panel);
		for(int i = 0; i < BALLS; i++) {
			printBall(balls[i]);
			refresh();
		}
	}
	END = true;
	for(int i = 0; i < BALLS; i++)
		threads[i].join();
	panelThread.join();
	delwin(mainWindow);
	delwin(panel);
}

int main(int argc, char *argv[]) {
	srand(time(NULL));
	printAll();
	endwin();
	return 0;
}
