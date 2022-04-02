#include <ncurses.h>
#include <thread>
#include <chrono>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

struct Point{
	int posx, posy;
};

WINDOW *create_newwin(int height, int width, int starty, int startx);
void destroy_win(WINDOW *local_win);
void moveWin(WINDOW *win, int startx);
void clearScreen();

static int position = 4;
static bool end = false;
static Point balls[5];

void printBall(int posy, int posx)
{
	attron(COLOR_PAIR(1));
	mvprintw(posy-1, posx, "*");
	mvprintw(posy, posx-1, "***");
	mvprintw(posy+1, posx, "*");
	attroff(COLOR_PAIR(1));
	refresh();
}

Point initBall()
{
	Point ball;
	ball.posy = 4;
	ball.posx = 4 + rand()%(COLS/2) + COLS/4;
	return ball;
}

void moveBall(int id)
{
	while(not end){
		balls[id] = initBall();
		int velocity = 20 + 10 * rand()%10;
		std::pair<int, int> direction{0,0};
		direction.first = rand() % 3 + 1;
		direction.second = rand() % 7 - 3;
		int bounce = 4 + rand() % 3;
		while(not end and bounce >= 0)
		{
			int posy, posx;
			posy = balls[id].posy + direction.first;
			posx = balls[id].posx + direction.second;
			if(posy > LINES - 9)
			{
				if(bounce == 0)
				{
					break;
				}
				else
				{
					if(posx < position or posx > position + 25)
						break;
					bounce--;
					posy = 2 * (LINES - 8) - posy;
					direction.first = -direction.first;
				}
			}
			else if(posy < 4)
			{
				if(bounce == 0)
				{
					break;
				}
				else
				{
					bounce--;
					posy = 8 - posy;
					direction.first = -direction.first;
				}
			}
			if(posx < 4)
			{
				if(bounce == 0)
				{
					break;
				}
				else
				{
					bounce--;
					posx = 8 - posx;
					direction.second = -direction.second;
				}
			}
			if(posx > COLS - 5)
			{
				if(bounce == 0)
				{
					break;
				}
				else
				{
					bounce--;
					posx = 2 * (COLS - 5) - posx;
					direction.second = -direction.second;
				}
			}
			balls[id].posx = posx;
			balls[id].posy = posy;
			std::this_thread::sleep_for(std::chrono::milliseconds(velocity));
		}
	}
}

void panelMovement()
{
	bool direction = true; //true = move to the right
	while(not end)
	{
		if(direction)
		{
			if(position < COLS - 4 - 25)
				position++;
			else
			{
				direction = not direction;
				position--;
			}
		}
		else
		{
			if(position > 6)
				position--;
			else
			{
				direction = not direction;
				position++;
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
}

void printAll(){
	WINDOW *my_win, *another_win;
	int startx, starty, width, height;
	int ch;
	balls[0].posx = 15;
	balls[0].posy = 8;
	initscr();			/* Start curses mode 		*/
	cbreak();			/* Line buffering disabled, Pass on
					 * everty thing to me 		*/
	keypad(stdscr, TRUE);		/* I need that nifty F1 	*/
	noecho();
	start_color();
	init_pair(1, COLOR_BLUE, COLOR_RED);
	timeout(20);
	height = LINES - 4;
	width = COLS - 4;
	starty = 2;
	startx = 2;
	refresh();
	my_win = newwin(height, width, starty, startx);
	box(my_win, 0, 0);
	wrefresh(my_win);
	another_win = newwin(3, 25, LINES - 7, position);
	box(another_win, 0, 0);
	wrefresh(another_win);
	std::thread t1(panelMovement);
	std::thread t2(moveBall, 0);
	std::thread t3(moveBall, 1);
	std::thread t4(moveBall, 2);
	std::thread t5(moveBall, 3);
	std::thread t6(moveBall, 4);
	while((ch = getch()) != KEY_F(1))
	{
		clearScreen();
		moveWin(another_win, position);
		wrefresh(another_win);
		for(int i = 0; i<5; i++){
			
			printBall(balls[i].posy, balls[i].posx);
			refresh();
		}
	}
	end = true;
	t1.join();
	t2.join();
	t2.join();
	t3.join();
	t4.join();
	t5.join();
	t6.join();
	delwin(my_win);
	delwin(another_win);
}

int main(int argc, char *argv[])
{	
	srand(time(NULL));
	printAll();

	
	endwin();			/* End curses mode		  */
	return 0;
}

void moveWin(WINDOW *win, int startx){
	wborder(win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
	wrefresh(win);
	mvwin(win, LINES - 7, startx);
	box(win, 0, 0);
	wrefresh(win);
}

WINDOW *create_newwin(int height, int width, int starty, int startx)
{	WINDOW *local_win;

	local_win = newwin(height, width, starty, startx);
	box(local_win, 0 , 0);		/* 0, 0 gives default characters 
					 * for the vertical and horizontal
					 * lines			*/
	wrefresh(local_win);		/* Show that box 		*/

	return local_win;
}

void destroy_win(WINDOW *local_win)
{	
	/* box(local_win, ' ', ' '); : This won't produce the desired
	 * result of erasing the window. It will leave it's four corners 
	 * and so an ugly remnant of window. 
	 */
	wborder(local_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
	/* The parameters taken are 
	 * 1. win: the window on which to operate
	 * 2. ls: character to be used for the left side of the window 
	 * 3. rs: character to be used for the right side of the window 
	 * 4. ts: character to be used for the top side of the window 
	 * 5. bs: character to be used for the bottom side of the window 
	 * 6. tl: character to be used for the top left corner of the window 
	 * 7. tr: character to be used for the top right corner of the window 
	 * 8. bl: character to be used for the bottom left corner of the window 
	 * 9. br: character to be used for the bottom right corner of the window
	 */
	wrefresh(local_win);
	delwin(local_win);
}

void clearScreen(){
	for(int row = 3; row <= LINES - 8; row++){
		move(row, 3);
		for(int col = 3; col <= COLS - 4; col++)
			addch(' ');
	}
}