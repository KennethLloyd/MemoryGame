/*
  Name: dex file editor applictaion
  Copyright: 
  Author: Joseph Emmanuel Dl Dayo
  Date: 02/04/04 18:53
  
  Description: A simple file editor for a simple OS with a simple API.
  no ncurses, curses, mouse support or whatever . This file editor is 
  not only for dex-os but for other simple os'es as well. There are still some
  issues when creating a new file, and random bugs all over the place.
  
  DexEdit - The simple text editor for dex-os
  Copyright (C) 2004  Joseph Emmanuel DL Dayo

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. 
  
  
  
*/

#define TAB_MARK 253
#define MAX_SCREEN_HEIGHT  40

#include "dexsdk.h"


/**********OS Dependent values: Modify depending on what your OS returns
 in getch*/
#define KEY_UP  151
#define KEY_DOWN 152
#define KEY_LEFT  150
#define KEY_RIGHT 153
#define KEY_PAGEDOWN 149
#define KEY_PAGEUP 148
#define KEY_HOME 146
#define KEY_END 147
 
 

unsigned int SCREEN_WIDTH  = 80;
unsigned int SCREEN_HEIGHT = 24;
int x_adj = 0, y_adj = 1;
int tab_stop = 8;
int editor_highlight = 0;
int editor_lineno = 0;
int editor_paperwhite = 0;
int default_background = BLACK , default_foreground = WHITE;


typedef struct _editor_line {
    int line, length;
    char *lineptr;
    struct _editor_line *prev,*next;
} editor_line;

typedef struct _editor_document {
    char filename[255], path[255];
    editor_line *visilines[MAX_SCREEN_HEIGHT];
    int modified;
    int start_col;
    int start_row;
    int curx,cury;
    int lines;
    editor_line *first_line;
    } editor_document;





void movecursor(int x,int y);
void editor_showheader();

//copies a line while converting special characters
int editor_copyline(char *dest,const char *source,int char_limit)
{
int i = 0, i2, j = 0;
for (i=0; source[i] && j < char_limit && i < char_limit;i++)
  {
     //tab, convert to tab marks
     if ( source[i] == '\t')
        {
               int j_tab = j;

               dest[j++]='\t';
               for (i2 = 1; i2 < tab_stop - ( j_tab % tab_stop); i2 ++)
                    {
                      dest[j++] = TAB_MARK;
                    };
        }
     else
    if (source[i] != '\r')
    dest[j++] = source[i];
  };
dest[j] = 0;
};

int editor_savefile(editor_document *doc, const char *filename)
{
FILE *outfile;

editor_line *ptr = doc->first_line;

if (doc->modified==0 && (strcmp(doc->filename,filename) == 0) ) return 1;

outfile = fopen(filename,"w");

//cannot write to file?
if (outfile == 0) return -1;

while (ptr!=0)
{
   char *linebuf = ptr->lineptr;
	if (linebuf)
   	{
			int i;
         for (i=0;linebuf[i];i++)
         	{
            	if ((unsigned char)linebuf[i]!=TAB_MARK)
               	fputc(linebuf[i],outfile);
            };
      };
   ptr=ptr->next;
};
doc->modified = 0;
fclose(outfile);
return 1;
};

int editor_queryloadfile(editor_document **doc)
{
	char filename[255];
	gotoxy(1,1);
    printf("%-80s","");
	strcpy(filename,"");
    gotoxy(1,1);
	printf("load file, Enter filename:");
	gets(filename);
    if (strcmp(filename,"n")==0) return 0;
	if (strcmp(filename,"")==0) strcpy(filename,(*doc)->filename);

	
   if (editor_loadfile(doc,filename)==1)
   {
      strcpy((*doc)->filename,filename);
      return 1;
   };
   
   printf("error: unable to load file									");
   getch();
   return 0;
};

int editor_querysavefile(editor_document *doc)
{
	char filename[255];
	gotoxy(1,1);
   printf("%-80s","");
	strcpy(filename,"");
   gotoxy(1,1);
	printf("Save file, Enter filename? [%s] or \"n\":?",doc->filename);
	gets(filename);
   if (strcmp(filename,"n")==0) return 0;
	if (strcmp(filename,"")==0) strcpy(filename,doc->filename);

	if (editor_savefile(doc,filename)==1)
   {
      strcpy(doc->filename,filename);
      return 1;
   };
   
   printf("error: unable to save file									");
   getch();
   return 0;
};


int editor_free(editor_document *doc)
{
    editor_line *ptr = doc->first_line, *temp;
    /*Free the lines*/
    while (ptr!=0)
    {
        temp = ptr->next;
        if (ptr->lineptr!=0)
                free(ptr->lineptr);
        free(ptr);
        ptr = temp;
    };
    
    free(doc);
};

int editor_loadfile(editor_document **doc, const char *filename)
{
    FILE *infile;
    editor_line *ptr = 0;
    int i;

    infile = fopen(filename,"r");

    //file does not exist?, createa blank document

    *doc = (editor_document*) malloc(sizeof(editor_document));
    (*doc)->modified = 0;
    strcpy( (*doc)->filename, filename);
    (*doc)->first_line = 0;
	 i = 0;
    if (infile==0)
      {
          ptr = (editor_line*)malloc(sizeof(editor_line));
          ptr->lineptr = (char*)malloc(2);
          strcpy(ptr->lineptr," ");
          ptr->length = 2;
          ptr->next = 0;
          ptr->prev = 0;
          (*doc)->first_line = ptr;
          i = 1;
      };

    if (infile != 0)
    {
    while (!feof(infile))
        {
            char buf[255], buf2[255];
            char *lineptr;
            int length;
            editor_line *line;

            line = (editor_line*)malloc(sizeof(editor_line));
            fgets( buf,255,infile);
            buf[254] = 0;
            editor_copyline(buf2,buf,255);
				length = strlen(buf2);
            lineptr = (char*) malloc(length+1);

            memcpy(lineptr,buf2,length+1);

            line->length = length+1;
            line->lineptr = lineptr;
            line->next = 0;
            line->prev = ptr;

            if (ptr==0)
                {
                    (*doc)->first_line = line;
                }
            else
                {
                    ptr-> next =  line;
                };

            i++;
            ptr = line;
        };
    fclose(infile);
    };

    (*doc)->lines = i;
    ptr = (*doc)->first_line;

    for (i=0;i < SCREEN_HEIGHT ;i++)
     {
          (*doc)->visilines[i] = ptr;
          if (ptr!=0) ptr= ptr->next;
     };

    (*doc)->start_col = 0;
    (*doc)->start_row = 0;
    (*doc)->modified = 0;
    
    return 1;

};

int showfile(editor_document *doc)
{

    int i,i2,i3;
    int ox = x_adj, oy = y_adj;

    clrscr();
    textcolor(BLACK);
    textbackground(WHITE);
    for (i2=0;i2 < SCREEN_HEIGHT ;i2++)
              {
                  char *lineptr;

                  if (editor_lineno)
                  {
                  	gotoxy(1,i2 + 1 + y_adj);
                     textcolor(WHITE);
                     textbackground(CYAN);
                     printf("%4d:", i2 + 1 + doc->start_row);
							textcolor(default_foreground);
                  };

                  if (doc->visilines[i2]!=0)
                     {
                       unsigned char background = 0;
                       int adj = doc->start_col;
                       lineptr = doc->visilines[i2]->lineptr;



                       for ( i3 = 0; i3 < SCREEN_WIDTH &&
                       ( i3+adj < doc->visilines[i2]->length) && lineptr[i3 + adj]
                                      ;i3++)
                           {

                               char out;
                               unsigned char color;

                    				 if ( editor_highlight && i2 == doc->cury)
                               		background = CYAN;
                               else
                               		background = default_background;

                               if ( (unsigned char) lineptr[i3+adj] == TAB_MARK ||
                                    (unsigned char) lineptr[i3+adj] == '\t' )
                                  {out = ' '; color= combine_color(default_foreground,background);}
                               else
                               	 if (lineptr[i3+adj]!='\n' &&
                                   lineptr[i3+adj]!='\r')
                                  {out = lineptr [i3+adj]; color = combine_color(default_foreground,background);}
                               else
                                  {out = ' '; color = combine_color(default_foreground,background);};

                               directputchar(i3+ox,i2+oy,out,color);
                           };

                       for (; i3 < SCREEN_WIDTH ;i3++)
                           directputchar(i3+ox, i2+oy,' ',combine_color(default_foreground,background));

                     }
                  else
                   for ( i3 = 0 ; i3 < SCREEN_WIDTH ;i3++)
                           directputchar(i3+ox, i2+oy,' ',combine_color(default_foreground, default_background));

              };

    editor_showheader(doc);

};

void editor_showheader(editor_document *doc)
{
    char title[255];

    textcolor(WHITE);
    textbackground(BLUE);
    sprintf(title,"DexEdit - %s", doc->filename);
    if (doc->modified) strcat(title,"(modified)");
    gotoxy(1,1);
    printf("%-59s col:%-5d row:%-5d",title,doc->curx+1,doc->cury+1 + doc->start_row);
    movecursor(doc->curx,doc->cury);
};

void movecursor(int x,int y)
{
	update_cursor(y + y_adj, x + x_adj);
};

int home(editor_document *doc)
{
int i;
editor_line *ptr = 0;

ptr = doc->first_line;
for (i=0; i < SCREEN_HEIGHT;i++)
     {
          doc->visilines[i] = ptr;
          if (ptr!=0) ptr= ptr->next;
     };
};

int editor_moveup(int start_row, editor_document *doc, int force)
{
   int i2;

   if (force == 0)
   {
	   if (doc->visilines[SCREEN_HEIGHT-1] == 0) return 0;
   	if (doc->visilines[SCREEN_HEIGHT-1]->next == 0) return 0;
		doc->start_row++;
   };

	for (i2 = start_row; i2 < SCREEN_HEIGHT-1; i2 ++)
     {
           doc->visilines[i2] = doc->visilines[i2+1];
           if (doc->visilines[i2] == 0) break;
     };

   if (doc->visilines[SCREEN_HEIGHT-1] != 0)
  	doc->visilines[SCREEN_HEIGHT-1] = doc->visilines[SCREEN_HEIGHT - 1]->next;
   return 1;
};


int editor_movedown(int start_row, editor_document *doc,int force)
{
   int i2;
   if (doc->visilines[0]->prev!=0 || force)
	for (i2 = SCREEN_HEIGHT - 1 ; i2 > start_row ; i2 --)
     {
           doc->visilines[i2] = doc->visilines[i2-1];
     };

   if (doc->visilines[0]->prev!=0)
     {
      	doc->visilines[0] = doc->visilines[0]->prev;
         if (force == 0) doc->start_row--;
         return 1;
     };
     return 0;
};

int shift_up(editor_document *doc)
{
   if (doc->visilines[SCREEN_HEIGHT-1])
   if (doc->visilines[SCREEN_HEIGHT-1]->next);
   return editor_moveup(0, doc,0);
};

int shift_down(editor_document *doc)
{
   editor_movedown(0,doc,0);
};


void page_down(editor_document *doc)
{
   int i;
   for (i=0;i<SCREEN_HEIGHT;i++)
   if (shift_up(doc)==0) break;
};

void page_up(editor_document *doc)
{
   int i;
   for (i=0;i<SCREEN_HEIGHT;i++)
        shift_down(doc);
};



int editor_insertchar(editor_document *doc, unsigned char c, int lineno, int position)
{
	int i;
   editor_line *line = doc-> visilines[lineno];

	//case when enter is pressed, in this case we have a situation wherein we
   //have to split the line.
   if (line!=0)
   {
   if (c=='\b')
   {
		if (position>0)
      	editor_insertchar(doc,145,lineno,position-1);
      else
      {
      	if (lineno>0)
         {
         doc->curx = doc->visilines[lineno-1]->length-3;
         doc->cury = lineno-1;
         editor_insertchar(doc,145,lineno-1,doc->visilines[lineno-1]->length-2);
         };
      };
   }
   else
   //The delete key was pressed
   if ( c == 145)
   {
          //The case when a non-newline character was deleted
   	    if (line->lineptr[position]!='\n')
          {

        	   for (i = position; i < line->length - 1 ; i++)
     	   	line->lineptr[i] = line->lineptr[i+1];
            line->length-= 1;
   	      line->lineptr = realloc(line->lineptr,line->length);
   	      return 0;
          }
            else
          //A newline character was deleted, we now perform a merge op
          {
            int length1;
            int length2;
            if (line->next)
            	{
               	  editor_line *next_line = line->next;
                    line->lineptr[position] = 0;
                    length1 = strlen ( line->lineptr );
               	  length2 = strlen ( next_line->lineptr);
                    line->lineptr = (char*) realloc(line->lineptr,length1 + length2 + 1);
                    line->length = length1 + length2 + 1;

                    strcat(line->lineptr, line->next->lineptr);
                    
                    line->next = next_line->next;
                    if (line->next!=0)
                    line->next->prev = line;

						  //free up memory used by the other line
                    free(next_line->lineptr);
                    free(next_line);
                    editor_moveup(lineno,doc,1);
                    doc->visilines[lineno] = line;
               };
          };
   }
   else
   if (c=='\r' || c=='\n')
   {
   		 int length;
          editor_line *line2 = (editor_line*) malloc(sizeof(editor_line));
          line2->lineptr = (char*) malloc(line->length - position);
          line2->length = line->length - position;
          memcpy(line2->lineptr, line->lineptr + position, line2->length);

          line->lineptr[position]='\n';
          line->lineptr[position + 1] = 0;
          line->length = strlen ( line->lineptr ) + 1;
          line->lineptr =  (char*) realloc(line->lineptr, line->length);

          
          line2->next = line->next;
          line2->prev = line;
          if (line->next) line->next->prev = line2;
          line->next  = line2;

          editor_movedown(lineno,doc,1);

          if (lineno + 1 < SCREEN_HEIGHT)
          doc->visilines[lineno+1] = line2;

          doc->curx = 0;  doc->start_col = 0;
          doc->cury++;
          return 0;
   }
   else
   //case when there is something on the line  and c is not \n or \r
   if (line!=0 && c!='\n' && c!='\r')
   {
   	     line->length += 1;
         line->lineptr = realloc(line->lineptr,line->length);
         for (i = line->length - 1; i > position ; i--)
            	line->lineptr[i] = line->lineptr[i-1];
         line->lineptr[position] = c;
         return 1;
   };
   };
   return 0;
};

/*check if text file is modified, if it is query the user
 to save or not to save the file*/
  
void check_modified(editor_document *doc)
{
           if (doc->modified)
			   {
			   	char ans;
			     	gotoxy(1,1);
			      printf("%-80s","");
			      gotoxy(1,1);
				   printf("Do you which to save your file?, [\"y\" or \"n\"]:");
			      ans = toupper(getch());
			      if (ans == 'Y')  editor_querysavefile(doc);
			   };
 
};

void input(editor_document *doc)
{
   unsigned char key;
   movecursor(doc->curx,doc->cury);

   do {
   	int do_redraw = 0;

      key = getch();
		if (key == KEY_UP)
          doc->cury -- ;
      else
      if (key == KEY_DOWN)
          doc->cury++;
      else
      if (key == KEY_RIGHT)
          doc->curx++;
      else
      if (key == KEY_LEFT)
          doc->curx--;
      else
      if (key == KEY_PAGEDOWN)
         {
           page_down(doc);
           do_redraw = 1;
         }
      else
	   if (key == KEY_PAGEUP)
         {
           page_up(doc);
           do_redraw = 1;
         }
      else
      if (key == KEY_HOME)
         {
          doc->curx = 0;
          if (doc->start_col!=0)
          {
	          doc->start_col = 0;
   	       do_redraw = 1;
          };
         }
      else
      if (key == KEY_END)
      	{
          int line_length;
          if (doc->visilines[doc->cury])
          line_length = doc->visilines[doc->cury]->length;

          if (line_length - 2 > SCREEN_WIDTH)
          {
 	         doc->start_col = line_length - 1 - SCREEN_WIDTH;
            doc->curx = SCREEN_WIDTH - 1;
            do_redraw = 1;
          }
	          else
          doc->curx = doc->visilines[doc->cury]->length;
         }
      else
      if (key == 14) /*CTRL-O: save file*/
      	{
             editor_querysavefile(doc);
             do_redraw = 1;
        }
      else
      if ( key == 11) /*CTRL-L: load file*/
         {
            check_modified(doc);
            /*Free memory used by the editor*/
            editor_free(doc);
            /*load a new file*/
            editor_queryloadfile(&doc);
            do_redraw = 1;
         }

      else
         {
              if ( key == '\t')
              {
                   int j_tab = doc->curx + doc->start_col, i2;
    					editor_insertchar(doc,'\t', doc->cury,doc->curx + doc->start_col);
    					doc->curx++;
    
                   for (i2 = 1; i2 < tab_stop - ( j_tab % tab_stop); i2 ++)
                        {
                           editor_insertchar(doc,TAB_MARK, doc->cury,doc->curx + doc->start_col);
             				  doc->curx++;
                        };
             }
               else
             if ( key == 23) /*Exit*/
             {
                check_modified(doc);
                break;
             }
               else
             {
             if (editor_insertchar(doc,key,doc->cury,doc->curx + doc->start_col))
             doc->curx++;
             };
             do_redraw = 1;
             doc->modified = 1;
         };

         if (doc->cury<0)
          {
             doc->cury = 0;
             shift_down(doc);
             do_redraw = 1;
          };

          if (doc->cury > SCREEN_HEIGHT - 1)
          {
            doc->cury = SCREEN_HEIGHT - 1;
            shift_up(doc);
            do_redraw = 1;
          };

          if (doc->curx < 0)
          {
          doc->curx = 0;
          	if (doc->start_col>0)
	          {
   	         doc->start_col --;
      	      do_redraw = 1;
         	 };
          };

          if (doc->curx > SCREEN_WIDTH - 1 )
          {
             doc->curx = SCREEN_WIDTH - 1;
             doc->start_col ++;
             do_redraw = 1;
          };

          if (doc->visilines[doc->cury])
          {
          int line_length = doc->visilines[doc->cury]->length;

         	 if (doc->curx + doc->start_col  > line_length - 2)
      	    {

		          if (doc -> start_col > line_length - 2)
                {
	         		 doc->start_col =  doc -> start_col - line_length - 2;
                   doc->curx = line_length - doc->start_col - 2;
   	             do_redraw = 1 ;
                }
                else
                {

   		       doc->curx = line_length - 2 - doc->start_col;
                };
	          };
          };

          if (do_redraw || editor_highlight) showfile(doc);
				else
          editor_showheader(doc);

          movecursor(doc->curx,doc->cury);


   } while (key!=23);

   clrscr();
};

int main2(int argc, char *argv[])
{
  char filename[255];
  int i;
  editor_document *doc;
  printf("Dayosoft Editor/Viewer 1.0 for DEX-OS\n");
  strcpy(filename,"");
  for (i=1;i<argc; i++)
   {

   	char *opt = argv[i];

    	printf("%s ", argv[i]);
      if (opt[0]=='-')
      {
   	if ( strcmp(opt,"-line")==0 )
      	{
				  editor_lineno = 1;
				  SCREEN_WIDTH  = 75;
              x_adj = 5;
         }
      else
      if (strcmp(opt,"-paperwhite")==0)
      	{
         	editor_paperwhite = 1;
            default_background = WHITE;
            default_foreground = BLACK; 
         }
      else
      if (strcmp(opt,"-highlight")==0)
      	{
          	editor_highlight = 1;
         }
      else
      if ( strcmp(opt,"-help")==0 )
      	{
         	printf("ed.exe - A simple command line text-editor\n");
            printf("			ed.exe [-line | -highlight ] <input file>\n\n");
            printf("options:\n");
            printf("			<Programmer's Aid options>\n");
            printf("			-line         :show line numbers on the side of the screen.\n");
            printf("			-highlight    :highlights the current line.\n");
            printf("Developed by Joseph Emmanuel DL Dayo fot the DEX Operating System\n");
            exit(0);
         }
         else
         {
         	printf("error: unknown option %s.\n", opt);
            exit(0);
         };
      }
       else
	  strcpy(filename,opt);
   };

  if (strcmp(filename,"")==0)
  {
		strcpy(filename,"untitled.txt");
  };

  printf("opening %s..\n",filename);
  editor_loadfile(&doc,filename);
  printf("done.\n");
  showfile(doc);
  input(doc);
  return 0;
}
