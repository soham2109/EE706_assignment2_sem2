#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <ncurses.h>

/* Global Variables */
int  rno=0, max_rno=1, gui_row_2 = 5, gui_row_1 = 5,i=0;
char scrap[100];

/* Function Declarations */
void custom_print(int screen, char *format, char* string);	// prints string on screen1/2
void gui_clear(int screen);	// clears the gui screen
void initial_display(int n_router);	// display initial layout

void custom_print(int screen, char *format, char* string){
    if(screen == 1){  // If left half of screen (screen 1)
        if(gui_row_1 >= LINES-2){
            gui_clear(1);
        }
        mvprintw(gui_row_1, 2, format, string);
        gui_row_1++;
    }else{
        if(gui_row_2 >= LINES-2){
            gui_clear(2);
        }
        mvprintw(gui_row_2, 2+COLS/2, format, string);
        gui_row_2++;
    }
    refresh();
}
void gui_clear(int screen){
    int i=0;
    if(screen == 1){
        char space[COLS/2-1];
        for (i=0;i<COLS/2-1;i++){
            space[i] = ' ';
        }
        space[i] = '\0';
        for (int i=5;i<=LINES-2;i++){
            mvprintw(i, 0, "%s", space);
        }
        gui_row_1 = 5;
    }else{
        for (int i=5;i<=LINES-2;i++){
            move(i, 1+COLS/2);clrtoeol();
        }
        gui_row_2 = 5;
    }
    refresh();
}
void initial_display(int rno){
	initscr();
    move(0,0);hline(ACS_HLINE, COLS);	// draw a top horizontal Line

    // Print Text for GUI type of output
    mvprintw(1, COLS/2, "|");	// Move to 1st row, centre of the screen and print a "|" character
    sprintf(scrap, "[Router %d]-Messages Rx & Tx", rno);
    mvprintw(1, COLS/4 - strlen(scrap)/2 , "%s", scrap);	// Print Title
    sprintf(scrap, "Metrics @ Router %d [%d]",rno, 8000+rno);
    mvprintw(1, 3*COLS/4 - strlen(scrap)/2 , "%s", scrap);	// Print Title

    // draw a line again
    move(2,0);hline(ACS_HLINE, COLS);
    move(3, COLS/2);vline(ACS_VLINE, COLS);	// draw a vertical line seperator

    strcpy(scrap, "");
//    for(i=1;i<=max_rno;i++){
//        sprintf(scrap + strlen(scrap), "R%d\t", i);
//    }
//    mvprintw(3, 2+COLS/2, "%s", scrap);
    refresh();
}
