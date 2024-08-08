#include <stdio.h>
#include <stdlib.h> //general functions
#include <ctype.h>  //character classification
#include <unistd.h> //access POSIX operating system API functions
#include <termios.h> //configure Unix system terminal options

struct termios og_termios; //save original termios attributes

void disableRawMode()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &og_termios); //write original attributes
}

void enablerRawMode() //ovverride terminal canonical mode
{
    /* save terminal attributes on struct */
    tcgetattr(STDIN_FILENO, &og_termios);
    atexit(disableRawMode); //called at exit of process regardless of position in code
    struct termios raw = og_termios;

    /* modify struct */
    raw.c_lflag &= ~(ECHO | ICANON); //bitwise not on the flag BITS we want to change and then AND operator with c_lflag bits

    /* write struct on terminal attributes */
    tcsetattr(STDIN_FILENO, TCSAFLUSH ,&raw);
    /*TCSAFLUSH specifies when to apply the change: in this case, 
    it waits for all pending output to be written to the terminal, 
    and also discards any input that hasn’t been read.
    */

}

int main()
{
    enablerRawMode();   
    char c;
    while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q');
    return 0;
}
