#include "../../sdk/dexsdk.h"

void erase(int x, int y, int w, int h); //basically covers an area with a black rectangle
void print_board(int x, int y); //set up initial board
void print_card(int r, int c, int x, int y);
void gray(int x, int y);
void setup_level(); 

#define FLIP 'l'
#define UP_KEY 'w'
#define DOWN_KEY 's'
#define LEFT_KEY 'a'
#define RIGHT_KEY 'd'
#define QUIT 'x'
#define RESET 'r'
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

int level, flips;
char board[5][5]; 

int main() {
	char keypress = START_GAME;
	set_graphics(VGA_320X200X256);

	do {
		erase(1,1,400,220);
		header(); //print menu

		level = 1;

		keypress = (char)getch();
		erase(1,1,400,220); //erase menu

		if(keypress == START_GAME){
			do {
				if (keypress == START_GAME) {
					setup_level();

					//update level display
					erase(25,125,40,30);
					write_text("0",25,125,WHITE,0);
				}
				print_board(X,Y);
				do {

				}while(keypress !=  QUIT_GAME && keypress != RESET);
			}while(keypress != YES);
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

void print_card(int r, int c, int x, int y){ //print a card

	gray(x, y);
	//light(r, c, x, y);
	
}

void gray(int x, int y){ //prints a gray card socket 
int i;
	for (i=0;i<30;i++)write_pixel(i+x,0+y,56);
	for (i=0;i<30;i++)write_pixel(i+x,1+y,56);
	for (i=0;i<30;i++)write_pixel(i+x,2+y,56);
	for (i=0;i<30;i++)write_pixel(i+x,3+y,56);
	for (i=0;i<30;i++)write_pixel(i+x,4+y,56);
	for (i=0;i<30;i++)write_pixel(i+x,5+y,56);
	for (i=0;i<30;i++)write_pixel(i+x,6+y,56);
	for (i=0;i<30;i++)write_pixel(i+x,7+y,56);
	for (i=0;i<30;i++)write_pixel(i+x,8+y,56);
	for (i=0;i<30;i++)write_pixel(i+x,9+y,56);
	for (i=0;i<30;i++)write_pixel(i+x,10+y,56);
	for (i=0;i<30;i++)write_pixel(i+x,11+y,56);
	for (i=0;i<30;i++)write_pixel(i+x,12+y,56);
	for (i=0;i<30;i++)write_pixel(i+x,13+y,56);
	for (i=0;i<30;i++)write_pixel(i+x,14+y,56);
	for (i=0;i<30;i++)write_pixel(i+x,15+y,56);
	for (i=0;i<30;i++)write_pixel(i+x,16+y,56);
	for (i=0;i<30;i++)write_pixel(i+x,17+y,56);
	for (i=0;i<30;i++)write_pixel(i+x,18+y,56);
}

void setup_level(){ 

}

void print_board(int x, int y){ //set up initial board 

	int i, j, a, b;
 	a=x;
 	b=y;
 	char str[15];

	//display level
   	write_text("Level",135,5,WHITE,0); 
	sprintf(str,"%d",level);
   	write_text(str,190,5,WHITE,0);

	//print the 25 cards
	for(i=0; i<5; i++, b+=24){
		for(j=0; j<5; j++, a+=31)
			print_card(i, j, a, b);
		a=x;
	}
	
	//display legend
	write_text("Up-W",5,35,WHITE,0);
	write_text("Dn-S",5,45,WHITE,0);
	write_text("Lf-A",5,55,WHITE,0);
	write_text("Rt-D",5,65,WHITE,0);
	
	write_text("Flip-L",5,75,WHITE,0);
	write_text("Exit-X",5,85,WHITE,0);
	write_text("Reset-R",5,95,WHITE,0);

	//show number of flips
	write_text("Flips:",5,115,WHITE,0);
}

//displays header
header(){
	
	write_text("MEMORY GAME!",100,40,WHITE,1); //title

	//menu options
	write_text("1 - Start",40,160,WHITE,0); 
	write_text("2 - Quit",200,160,WHITE,0);
}