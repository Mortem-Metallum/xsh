Its X shell. Its a UNIX shell.

It combines commands like echo, whoami, and pwd that are usually separate programs, into the shell directly, like BusyBox.
It is unable to do (as of now):

    
    1. You cannot use the arrow keys

    2. Piping or using && operators

    3. Variables

    4. Loops
    
BUT I will implement these things in a later release eventually (maybe v2.0.0-stable?).

DEPENDENCIES:

    1. curses or ncurses (most UNIX systems have this)
    2. Basic UNIX header files