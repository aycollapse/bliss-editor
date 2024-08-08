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

    /* Disable ECHO, canonical mode, ctrl+ combinations, new lines (\n) and carriage return (\r) */
    //bitwise NOT ~ and AND & operators to switch values of flags
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST); //disable terminal post processing in output
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    
    //CS8 is a bit mask, sets the char size to 8 bits per byte, which is default on most systems
    raw.c_cflag |= (CS8);

    /* Add settings for timers */
    raw.c_cc[VMIN] = 0; //min value of bytes to read before read can return, with 0 it returns as soon as there is input
    raw.c_cc[VTIME] = 1; //max amount of time in tenths of second before read returns

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
    while (1)
    {
        c='\0';
        read(STDIN_FILENO, &c, 1);
        
        //(\r) needed to begin from the leftmost position on newline (try without)
        if(iscntrl(c)) printf("%d\r\n",c); //if char is control char print only ASCII value
        else printf("%d ('%c')\r\n", c, c); 
        
        if(c=='q')break;
    }
    return 0;
}
