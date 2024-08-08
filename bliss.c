/*** Includes ***/
#include <stdio.h>
#include <errno.h> //error handling
#include <stdlib.h> //general functions
#include <ctype.h>  //character classification
#include <sys/ioctl.h> //get terminal window size
#include <unistd.h> //access POSIX operating system API functions
#include <termios.h> //configure Unix system terminal options

/*** defines and signatures ***/
#define CTRL_KEY(k) ((k) & 0x1f) //every CTRL_KEY(k) is replaced with (k & 0x1f) where k is a char
void editorClear();

/*** Data ***/
struct editorConfig
{
    //terminal windows size
    int screenrows;
    int screencols;
    //save original termios attributes
    struct termios og_termios;
};
 
 struct editorConfig Config;

/*** Terminal ***/
//error handling
void die(const char *s) 
{
    editorClear();
    //check global errno variable and describe error
    perror(s);
    //exit process failure
    exit(1);
}

//return to default terminal attributes
void disableRawMode()
{
    //write original attributes
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &Config.og_termios) == -1)
    die("tcsetattr");
}

//ovverride terminal canonical mode
void enableRawMode() 
{
    /*save terminal attributes on struct */

    if(tcgetattr(STDIN_FILENO, &Config.og_termios)==-1) die("tcgetattr");
    //called at exit of process regardless of position in code
    atexit(disableRawMode); 
    struct termios raw = Config.og_termios;

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

char editorReadKey()
{
    int num_read;
    char c;
    //reads an exact byte of data and then returns if there are no errors
    while ((num_read = read(STDIN_FILENO, &c, 1))!=1)
    {
        //check if there is an error that is not EAGAIN, but something more serious
        if(num_read == -1 && errno != EAGAIN) die("read");
    }
    return c;
}

int getWindowSize(int *rows, int *cols) {
    struct winsize ws;
    //retrieve terminal size if possible, otherwise return error to initEditor
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) return -1;
    else 
    {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}

/*** Input ***/
void editorProcessKeypress()
{
    char c = editorReadKey();

    switch (c)
    {
        case CTRL_KEY('q'):
            editorClear();
            //exit process success
            exit(0);
            break;
    }
}

/*** Output ***/
void editorDrawRows()
{
    for(int i=0;i < Config.screenrows; i++)
    {
        write(STDOUT_FILENO, "~\r\n", 3); // return carrier \r is always needed in raw mode
    }
    write(STDOUT_FILENO, "~", 1); //last line
}

void editorClear() {
    //clear terminal
    write(STDOUT_FILENO, "\x1b[2J", 4);
    //reset cursor position
    write(STDOUT_FILENO, "\x1b[H", 3);
}

void editorRefreshScreen()
{
    editorClear();
    editorDrawRows();
    //reset cursor position after draw
    write(STDOUT_FILENO, "\x1b[H", 3);
}

/*** Init ***/
void initEditor()
{
    //set terminal size in config at startup if possible
    if(getWindowSize(&Config.screenrows, &Config.screencols)==-1) die("getWindowsize");
}

int main()
{
    enableRawMode();
    initEditor(); 
    while (1)
    {
        editorRefreshScreen();
        editorProcessKeypress();
    }
    return 0;
}
