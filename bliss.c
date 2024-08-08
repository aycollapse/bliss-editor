/*** Includes ***/
#include <stdio.h>
#include <errno.h> //error handling
#include <stdlib.h> //general functions
#include <ctype.h>  //character classification
#include <unistd.h> //access POSIX operating system API functions
#include <termios.h> //configure Unix system terminal options

/*** defines ***/
#define CTRL_KEY(k) ((k) & 0x1f)

/*** Data ***/
//save original termios attributes
struct termios og_termios; 

/*** Terminal ***/
//error handling
void die(const char *s) 
{
    //check global errno variable and describe error
    perror(s);
    //exit process
    exit(1);
}

//return to default terminal attributes
void disableRawMode()
{
    //write original attributes
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &og_termios) == -1)
    die("tcsetattr");
}

//ovverride terminal canonical mode
void enablerRawMode() 
{
    /*save terminal attributes on struct */

    if(tcgetattr(STDIN_FILENO, &og_termios)==-1) die("tcgetattr");
    //called at exit of process regardless of position in code
    atexit(disableRawMode); 
    struct termios raw = og_termios;

    /* modify attributes */

    //Disable ECHO, canonical mode, ctrl+ combinations, new lines (\n) and carriage return (\r)
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    //disable terminal post processing in output
    raw.c_oflag &= ~(OPOST); 
    //CS8 is a bit mask, sets the char size to 8 bits per byte, which is default on most systems
    raw.c_cflag |= (CS8);

    /* Add settings for timers */

    //min value of bytes to read before read can return, with 0 it returns as soon as there is input
    raw.c_cc[VMIN] = 0; 
    //max amount of time in tenths of second before read returns
    raw.c_cc[VTIME] = 1;

    /* write new terminal attributes */

    if(tcsetattr(STDIN_FILENO, TCSAFLUSH ,&raw)==-1) die("tcsetattr");
    /*TCSAFLUSH specifies when to apply the change: in this case, 
    it waits for all pending output to be written to the terminal, 
    and also discards any input that hasn’t been read.
    */
}

/*** Init ***/
int main()
{
    enablerRawMode();   
    char c;
    while (1)
    {
        c='\0';
        //check if there is an error that is not EAGAIN, but something more serious
        if(read(STDIN_FILENO, &c, 1)==-1 && errno != EAGAIN)die("read");
        
        // \r carrier return, puts cursor on the leftmost position on newline
        //if char is control char print only ASCII value
        if(iscntrl(c)) printf("%d\r\n",c); 
        else printf("%d ('%c')\r\n", c, c); 
        
        if(c=='q')break;
    }
    return 0;
}
