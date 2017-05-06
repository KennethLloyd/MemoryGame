#include "../../sdk/dexsdk.h"

void erase(int x, int y, int w, int h); //basically covers an area with a black rectangle
void printBoard(int x, int y); //set up initial board
void printCard(int r, int c, int x, int y); //set up initial cards
void updateBoard(); //after moving
void setupLevel(); 
void setCoordinates(int x, int y); //position of individual cards

#define SHOW 1
#define HIDE 2
#define SELECT 3
 
#define FLIP 'l'
#define UP_KEY 'w'
#define DOWN_KEY 's'
#define LEFT_KEY 'a'
#define RIGHT_KEY 'd'
#define QUIT 'x'
#define RESTART 'r'
#define YES 'y' 
#define NO 'n'
#define START_GAME '1'
#define QUIT_GAME '2'

#define YELLOW 54
#define PALE_YELLOW 62
#define ROYAL_BLUE 1
#define DARK_BLUE 8
#define GRAY 56
#define WHITE 63

#define X 85
#define Y 35
#define MAXROW 6
#define MAXCOL 6

int level, flips;
char board[MAXROW][MAXCOL]; 
int cards_x[MAXROW][MAXCOL];
int cards_y[MAXROW][MAXCOL];
int row, col, oldrow = 0, oldcol = 0;

int main() {
	char keypress = START_GAME;
	set_graphics(VGA_320X200X256);
	setCoordinates(X, Y); //initialize coordinates

	do {
		erase(1,1,400,220); //empty screen
		header(); //print menu

		level = 1;

		keypress = (char)getch();
		erase(1,1,400,220); //erase menu

		if(keypress == START_GAME){
			do {
				if (keypress == START_GAME) {
					setupLevel(); //set up initial states and current selected position
					//needed to position the cursor at the first card during quit or restart
					row = 0;
					col = 0;

					//update level display
					erase(25,125,40,30);
					write_text("0",25,125,WHITE,0);
				}
				printBoard(X,Y);
				do {
					int hasMoved = 0; //flag for pressing up, down, left or right only (avoids unnecessary presses)
					if (keypress=(char)getch()) { //catch movements (circular)
						if (keypress == RIGHT_KEY) {
							oldrow = row;
							oldcol = col;
							col = (col + 1) % MAXCOL;
							hasMoved = 1;
						}
						else if (keypress == LEFT_KEY) {
							oldrow = row;
							oldcol = col;
							if(col <= 0) {
								col = MAXCOL - 1;
							}
							else col--;
							hasMoved = 1;
						}
						else if (keypress == UP_KEY) {
							oldrow = row;
							oldcol = col;
							if(row <= 0) {
								row = MAXROW - 1;
							}
							else row--;
							hasMoved = 1;
						}
						else if (keypress == DOWN_KEY) {
							oldrow = row;
							oldcol = col;
							row = (row + 1) % MAXROW;
							hasMoved = 1;
						}

						if(hasMoved == 1) { //up, down, left or right
							board[row][col] = board[row][col] == HIDE? SELECT: HIDE; //select the moved card
							board[oldrow][oldcol] = board[oldrow][oldcol] == SELECT? HIDE: SELECT; //deselect the previous card (hide)
							updateBoard();
						}
					}
				}while(keypress != QUIT && keypress != RESTART);
				if(keypress == QUIT){
					//prompt confirmation then erase message
					write_text("Do you want to exit? y/n ",60,160,WHITE,0); 
					keypress = (char)getch();
					erase(60,160,220,40);			
				}
				else if(keypress == RESTART){
					//prompt confirmation then erase message
					write_text("Do you want to restart? y/n ",50,160,WHITE,0);
					keypress = (char)getch();
					if(keypress == YES) keypress = START_GAME;
					erase(50,160,260,40);			
				}
			}while(keypress != YES); //will continue if the user selects no in quit or restart
		}

	}while(keypress != QUIT_GAME);

	set_graphics(VGA_TEXT80X25X16);
	clrscr();

	return 0;
}

void erase(int x, int y, int w, int h){ //basically covers an area with a black rectangle 
   int i,j;
   for (i=y;i<=(y+h);i++)
      for (j=x;j<=(x+w);j++)
         write_pixel(j,i,100);
}

void setCoordinates(int x, int y){ //initialize card coordinates
	int i, j, a, b;
	
	a = x; //top position
	b = y; //bottom position
	
	//adjust the coordinates for individual positioning (different positions but same sizes)
	for(i=0; i<MAXROW; i++, b+=24){
		for(j=0; j<MAXCOL; j++, a+=31){
			cards_x[i][j] = a;
			cards_y[i][j] = b;
		}
		a=x;
	}
}

void printCard(int r, int c, int x, int y){ //print a card
	int i, color;

	switch(board[r][c]){ //sets the card color based on state
		case HIDE: color = GRAY; break;
		case SELECT: color = WHITE; break;
		case SHOW: color = DARK_BLUE; break;
	}
	
	for (i=0;i<30;i++)write_pixel(i+x,0+y,color);
	for (i=0;i<30;i++)write_pixel(i+x,1+y,color);
	for (i=0;i<30;i++)write_pixel(i+x,2+y,color);
	for (i=0;i<30;i++)write_pixel(i+x,3+y,color);
	for (i=0;i<30;i++)write_pixel(i+x,4+y,color);
	for (i=0;i<30;i++)write_pixel(i+x,5+y,color);
	for (i=0;i<30;i++)write_pixel(i+x,6+y,color);
	for (i=0;i<30;i++)write_pixel(i+x,7+y,color);
	for (i=0;i<30;i++)write_pixel(i+x,8+y,color);
	for (i=0;i<30;i++)write_pixel(i+x,9+y,color);
	for (i=0;i<30;i++)write_pixel(i+x,10+y,color);
	for (i=0;i<30;i++)write_pixel(i+x,11+y,color);
	for (i=0;i<30;i++)write_pixel(i+x,12+y,color);
	for (i=0;i<30;i++)write_pixel(i+x,13+y,color);
	for (i=0;i<30;i++)write_pixel(i+x,14+y,color);
	for (i=0;i<30;i++)write_pixel(i+x,15+y,color);
	for (i=0;i<30;i++)write_pixel(i+x,16+y,color);
	for (i=0;i<30;i++)write_pixel(i+x,17+y,color);
	for (i=0;i<30;i++)write_pixel(i+x,18+y,color);
}

void setupLevel() { //setup cards all face down
	int i, j;
   
	for(i=0;i<MAXROW;i++) {
		for(j=0;j<MAXCOL;j++) {
			board[i][j] = HIDE;
		}
	}

	board[0][0] = SELECT; //first card as the current (selected) for the cursor
}

void printBoard(int x, int y){ //set up initial board 

	int i, j, a, b;
 	a=x;
 	b=y;
 	char str[15];

	//display level
   	write_text("Level",135,5,WHITE,0); 
	sprintf(str,"%d",level);
   	write_text(str,190,5,WHITE,0);

	//print the 36 cards
	for(i=0; i<6; i++, b+=24){
		for(j=0; j<6; j++, a+=31)
			printCard(i, j, a, b);
		a=x;
	}
	
	//display legend
	write_text("Up-W",5,35,WHITE,0);
	write_text("Dn-S",5,45,WHITE,0);
	write_text("Lt-A",5,55,WHITE,0);
	write_text("Rt-D",5,65,WHITE,0);
	
	write_text("Flip-L",5,75,WHITE,0);
	write_text("Exit-X",5,85,WHITE,0);
	write_text("Rstrt-R",5,95,WHITE,0);

	//show number of turns
	write_text("Turns:",5,115,WHITE,0);
}

void updateBoard() { //after pressing a key, update for the changes in positions
	printCard(oldrow, oldcol, cards_x[oldrow][oldcol], cards_y[oldrow][oldcol]); //previous position
	printCard(row, col, cards_x[row][col], cards_y[row][col]); //current position
}

//displays header
header(){
	
	write_text("MEMORY MAGIC!",100,40,WHITE,1); //title

	write_text("-a memory game-",90,80,WHITE,0); //title

	//menu options
	write_text("1 - Start",40,160,WHITE,0); 
	write_text("2 - Quit",200,160,WHITE,0);
}