# ncurses notes

## Intro

### Initialisation and clean-up

```c
initscr(); // initialise the screen and memory
endwin(); // end and destroy ncurses.
```

Everything we do in ncurses should be between these two functions.

### Other important functions

```c
refresh(); // refresh the screen to match what is in memory
getch();  // blocking, returns int value of key pressed
printw("Hello World"); // prints to the standard screen, use format specifiers
                       // as with printf
clear(); // clears the standard screen
```

## Cursors

By default, cursors start at 0,0 which is the top left of the screen. Note, that
coordinates are always y,x with ncurses.

```c
move(y,x); // moves the cursor
mvprintw(y,x,"", ...); // move the cursor and then print
```

## Windows

```c
newwin(h,w,y,x); // create a window
box(win, 0, 0) // creates a border around the box, the ints are the border chars
               // or 0 for default
wborder(win, l,r,t,b,tlc,trc,blc,brc); // box with more ctl

wprintw(win, "text") // prints to the window
mvwprintw(win, y, x, "text") // prints to the window
```

When we make changes to a window. We can refresh after creating the window to
get it rendered on the screen. After this we can just use refresh window to
update specific windows:

```c
wrefresh(win);
```

## Borders and options

```c
cbreak(); // enable ctl-c to exit program (default)
raw(); // all input is raw input, so ctl-c will not exit
noecho(); // Input typed will not be echoed to the screen
```

## Attributes

```c
//ATTRIBUTES
/*
A_NORMAL
A_STANDOUT          gnerally same as reverse
A_REVERSE           reverse background and foreground colour
A_BLINK
A_DIM
A_PROTECT
A_INVIS
A_ALTCHARSET
A_CHARTEXT
*/

attron(att); // Apply an attribute
attroff(att); // Turn off an attribute
```

Note that the attributes are environment specific.

## Colours

```c
// COLOURS
/*
COLOR_PAIR(n)
COLOR_BLACK
COLOR_RED
COLOR_GREEN
COLOR_YELLOW
COLOR_BLUE
COLOR_MAGENTA
COLOR_CYAN
COLOR_WHITE
*/

has_colors() // bool indicating if terminal supports colors
start_color(); // set-up colors for the terminal

init_pair(pair_id, fg, bg); // Pair id is used to identify a colour pair
atton(COLOR_PAIR(pair_id));

can_change_color();
init_color(color, r,g,b); // Change a color, rgb values are 0-999
```

## Terminal info

```c
// note use stdscr for main window
getyx(win, y, x) // update y and x to values in the window
getbegyx(win, y, x) // starting y and x
getmaxyx(win, y, x) // ending y and x
```

These are useful where we have a separate window within stdscr. We can use these
functions to determine the dimensions.

## User input

```c
wgetch(win); // get char from a given window
keypad(win, on_or_off) // We can use things like KEY_UP or KEY_F(n) for fn keys
```

## Input modes

```c
cbreak(); // Provides input immediately rather than adding first to a line buffer
halfdelay(tenths); // Like c break, but waits for the delay. Returns -1 if nothing
                   // entered. Non blocking as after delay, continues
nodelay(win, on_or_off); // As above, but no delay
timeout(delay); // If -1 blocking input, if 0 like no delay, with a delay then
                // similar to halfdelay but ms rather than tenths of a second
```

## Key Combinations

There is a different range of values to represent ctl + char starting at 0. For
a ctl char we can use:

```c
ch & 0x1F // char plus control character (upper or lower)
```
