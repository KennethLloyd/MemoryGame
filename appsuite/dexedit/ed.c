#include "dexsdk.h"

typedef struct _editor_line {
int line,length;
char *lineptr;
struct _editor_line *prev;
struct _editor_line *next;
} editor_line;

typedef struct _editor_document {
char filename[255], path[255];
editor_line *visible_lines[255];
int start_col;
int curx,cury;
} editor_document;

editor_document *current_doc;

void editor_newline(editor_line *line);

void editor_new (editor_document *doc, char *filename)
{
	int i;
   editor_line *first_line;
   dtrcpy(doc->filename,filename);
   for (i=0;i<25;i++) doc->visible_lines[i] = 0;
   doc->start_col = 0;
   doc -> curx = 0; doc -> cury = 0;
   doc->visible_lines[0]= malloc(sizeof(editor_line));
   first_line = doc->visible_lines[0];
   editor_newline(first_line);
   first_line->prev = 0;
   first_line->next = 0;
}

void editor_newline(editor_line *line)
  {
    line->length = 0;
    line ->lineptr = malloc(2);
    strcpy(line->lineptr,"");
  };

void editor_showdoc(editor_document *doc)
{
	int i,i2;
   clrscr();
   for (i=0; i <25; i++)
   {
      if (doc->visible_lines[i]!=0)
        {
          int char_count = 0;
          char *lineptr = doc->visible_lines[i]->lineptr;
          for (i2 = doc->start_col;i2 < doc->visible_lines[i]->length &&
                lineptr[i2] && char_count < 80; i2 ++)
                 {
                    if (lineptr[i2]=='\n') break;
                    printf("%c",lineptr[i2]); char_count++;
                 };
          printf("\n");
        };

   };
   gotoxy(doc->curx+1,doc->cury+1);
};

int editor_loadfile( editor_document *doc, const char *filename)
{
  eidotr_line *ptr;
  FILE *f = fopen(filename,"r");
  doc = (editor_document*) malloc(sizeof(editor_document));
  strcpy(doc->filename, filename);
  doc->firstline = 0;
};

void main2(int c, char *p[])
{
  FILE *f;
  char filename[255];
  char *buffer;
  char inp;
  int size,ptr=0;
  if (!p[1])
    {
     printf("usage:\n");
     printf("ed <filename> -where filename is the file to create\n");
     exit(1);
    };

 buffer=(char*)malloc(1000000); //allocate 1 MB of memory

 strcpy(filename,p[1]);
 f=fopen(filename,"w");
 clrscr();
 printf("DEX32 quick text file creator\n");
 printf("You may now enter your data, CTRL-Z to exit:\n");
 do {
   char line[255];
   gets(line);
   strcat(buffer,line);

 } while (inp!=25&&ptr<1000000);

 fputs(buffer,1,ptr-1,f);
 fclose(f);

};
