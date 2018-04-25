# SimpleStupidShell

## Current Working Functionality
- clear
- cd
- Run shell commands using execvp
- piping
- history
- IO redirection
- Wildcards\
    Usage : 
    + ls *.c		// Matches all the c files
    + ls *hell* 	// Matches all files that has hell between like shell.c, shell_main.o etc
- Alias \
    Usage : 
    + alias [command to alias] [aliased command]
    + alias show // to show the aliased command list
- logs
    Usage :
        logs begin
        logs end
        logs show
- hypercat
    cat pdf files
- Editor With networking capabilites
    + editor host [filename]
    + editor listen [ip addr of host]
## Instructions to run 

`$ make`\
`$ ./shell`

# Shell 

## Parsing commands

command is a character array that can accommodate a maximum of 64 characters

```c
char command[MAX_COMMAND_LENGTH];
```

Next, we define our parameter. It a array of strings (array of array of characters) and stores the
parsed command spilt either based of a space(‘ ‘) or a pipe(‘|’).\
It can accomodate 16 parameters of length 64.

```c
params = (char **)malloc(MAX_PARAM_LENGTH * sizeof(char *));
for(int i=0; i<MAX_PARAM_LENGTH; i++){
    params[i] = (char *)malloc(MAX_COMMAND_LENGTH * sizeof(char));
}
```
If the command is singular then only one split occurs based on the spaces. If it is a piped
command then first the split occurs based on the (‘|’) and then each param is again split based on
spaces to be passed to the execvp.

The function commandToParams takes 3 arguments : command, pointer to the array of character
arrays to which the parsed input should be put to and the split character which is either (‘ ‘) or (‘|’).
Return value : Number of parameters

```c
int _commandToParams(char *command, char **params, char split){
    int paramCount = 0, count = 0;
    for(int i=0; i<strlen(command); i++){
        if(command[i] == split && command[i-1]!=command[i]){
            if(split == ' '){
                params[paramCount][count] = '\0';
            }else if(split == '|'){
                count--;
                params[paramCount][count] = '\0';
            }
            paramCount++;   
            count = 0;
        }else{
            params[paramCount][count] = command[i];
            count++;
        }
    }
    return paramCount;
}
```

The structure essentially looks as below : 

![](struct.png)

## Executing a single command

The system call execvp() is used to achieve this. As execvp is a system call hence to regain control
back to our controlling shell the process is forked.\
The usage of a single execed command is : 

```c
execvp(params[0],params);
```

## Executing a piped command
The approach for piping of a command is to to create two pipe files one for reading and the other
one for writing. Here first the split occurs at the (‘|’) and then each command is split into
parameters with a (‘ ’).\
The first n-1 commands are taken in in the child process and the STDOUT is duped with the
pipeDescriptor1 and the command is execed under a fork lastly, STDIN is duped with
pipeDescriptor2 in the parent process\
The nth command is just execed with the last set of parameters as their IO need not be redirected.

## IO Redirection
There are two files that need to be directed or redirected to/from the program. The Input and
Output files can be redirected using the dup2 command along with the STDIN and STDOUT file
descriptors.\
Just before the execution of the command if the command parser identifies an IO the files are
opened and dups are called and the output instead of printing on the screen pushes it to a file.

```c
outfd = open(outfle, O_RDWR | O_CREAT | O_TRUNC, __RRU_R | __RWU_R);
dup2(outfd, 1);
infd = open(infle, O_RDWR | O_CREAT | O_TRUNC, __RRU_R | __RWU_R);
dup2(infd, 0);
```

## Persistent Aliasing
Aliasing is essentially just naming of a functional command into something shorter and more
intuitive for a better user experience.\
This aliasing is persistent across sessions of the terminal invocation. A hidden file “.alia” is
maintained on the user system to keep track of this.\
The syntax to use aliasing is as follows : 

```c
alias <command to alias> <aliased command>
alias show // to show the aliased command list
Example :
alias ps -ef proc // Alises the ps -ef command to proc
```
These aliased commands are stored the file .alia in the following format : 

```c
command =value
Example :
ps -ef =proc
```
On the startup of the program the hidden file is looked at and the alias table is populated and on an
aliased command call this table is looked up.

## Wildcards
These are essentially special characters attached to the query instance to get more results on
incomplete searches. Currently supported regular characters are * and ?

Example : 
```c
// Yields all the fles in the directory
$ ls
_hell_main.c shell.o shell.h shell.c a.out Makefle shell_main.o shell readme.md
//Yields all the fle that have hell in between
$ ls -l *hell*
-rw-rw-r-- 1 pratik pratik 13219 Wed Apr 25 16:56:14 2018 shell_main.c
-rw-rw-r-- 1 pratik pratik 14800 Wed Apr 25 06:29:22 2018 shell.o
-rw-rw-r-- 1 pratik pratik 1622 Tue Apr 24 20:19:20 2018 shell.h
-rw-rw-r-- 1 pratik pratik 10583 Wed Apr 25 06:29:19 2018 shell.c
-rw-rw-r-- 1 pratik pratik 15736 Wed Apr 25 15:26:35 2018 shell_main.o
-rwxrwxr-x 1 pratik pratik 27288 Wed Apr 25 16:34:56 2018 shell
```

# Editor 

For our project we made a minimal editor which allowed the user to open existing files,
add, delete or modify its content, and save it back to disk. The editor allowed the user to
control the terminal cursor using the arrow keys and allowed the user to make edits on the
fly without entering a specific mode (as in Vim). The editor consists of a few main modules
performing the following tasks, and each section hence deals with the implementation
details of these modules.
1. Terminal Raw mode
2. File read from disk
3. Display
4. Cursor movement
5. User interaction
6. File write to disk

## Terminal raw mode
Terminals that we use typically exist in “cooked” mode. That is, the terminal performs some
basic input processing and sanitisation before it is sent to out program. Our first step
before displaying anything or taking in user input is to disable this. This is required so that
we can read arrow keys and save keys without anything being printed to the screen. This
involves turning off automatic echoing and canonical mode. The following code snippet
achieves this

```c
tcgetattr(STDIN_FILENO, &orig_termios);
struct termios raw = orig_termios;
raw.c_lflag &= ~(ECHO | ICANON);
tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
```

## Read from disk (+ structural details)
As a first step, when the user launches the program, they are required to specify a file to
open as a command line argument. The function which carries out this process is
`populate_file_buffer`. Before going into the implementation details of this however, it
is imperative to understand the format in which we maintain the file in RAM. We maintain
the file as a large doubly linked list of lines. Each node in the DLL consists of next and prev
pointers, a pointer to a string containing a single line from the file, and metadata detailing
the space used by the string.
This is implemented as a C structure as below

```c
typedef struct row_dll{
char *text;
int used_len;
int avlbl_len;
struct row_dll *next;
struct row_dll *prev;
} row;
```

The entire file is read in line-by-line and stored in this DLL. The line-wise reading is
achieved using the `getline` function provided in the C standard I/O library. It reads
characters from a file until it reaches a \n character, and allocates space for the same on
the heap. Our text field within the DLL node is handled slightly differently from a standard
fixed width array of characters. We implement it as a dynamic table. That is, a data
structure which behaves like an array, but offers amortized O(1) time complexity for
appends. We use a growth factor of 2.0 and an initial size of 4096 bytes. A growth factor of
2 means that anytime we use up all the allocated storage for the text, and we wish to add
another character, we double the space allocated to it, using the standard library’s
`realloc` function. The `populate_file_buffer` function handles all the operations
detailed above.

## Display
For the display function to work accurately, it is not essential that the terminal is in raw
mode. It works just the same in “cooked” mode. Two essential variables are used to
determine what content must be displayed on the screen. 

```c
row *top_line;
int screen_rows;
```

The first is a pointer to the first line, which must be displayed at the top of the screen. The
next is the number of rows which must be displayed.
The display function, then begins iterating forwards over the DLL, beginning at `top_line`
and continuing for `screen_rows` number of lines. At each step, it prints the text field of
the structure to screen. As it proceeds, it also advances the pointer `bot_row` which at the
end of the loop will be a pointer to the last line which was displayed. This becomes
essential when we discuss handling cursor movement.

## Cursor Movement
The most intuitive manner to move a text cursor over a terminal is the arrow keys. Thus is
was imperative that we allow for this basic functionality. The user simply presses left and
right arrow keys to move the cursor along a line, and presses up or down to move to the
previous or next line.
The primary challenge here is to get the user’s key down event, without it echoing garbage
to the screen. This is achieved using raw mode as described earlier. Each time a user
presses an arrow key, the following bytestream is placed in the input buffer : `“\033[A”`.
Here \033 represents the escape sequence, which tells us that what follows next is not a
printable character. The next is an open square bracket and then is a single uppercase
telling us which key was pressed

Once the event is captured, we need to move the cursor to the required position. A simple
printf allows us to move the cursor to any coordinate we desire.

```c
printf("\x1b[%d;%dH", cur_y, cur_x);
fflush(stdout);
```

Here `cur_y` and `cur_x` are the desired coordinates and the first argument is a string
which specifies that the cursor must be moved to a particular coordinate. The `fflush` is
required since the previous line does not include a newline character and the line buffer of
the terminal must be cleared.
There are some special cases we must take care of. Such as, when we reach the end of a
line and press right, when we reach the beginning and press left, when we press up at the
first displayed line and when we press down at the last displayed line. Simple if statements
and careful management of the DLL pointers allow us to get around this issue.

## User interaction
Users can insert characters or delete characters using backspace at any point in the file
without having to enter a special mode. Insertion is handled fairly simply. Each time the
user presses a character which does not have some specific action associated with it, we
know the line where the cursor was, and the index into the same. We shift characters at
this point and to it’s right to the right by one step. The entered character is then placed in
this gap. (At this stage, the dynamic table may be grown if required.)
Backspace is handled similarly, except, here characters are moved to the left.
The special case here is when the user presses enter, which causes a newline to be
inserted. This involves splitting the current line at the cursor’s X index and allocating a new
node for the second half. The standard shortcut CTRL_S is supported for saving.

`NOTE`: Many terminals have CTRL+S reserved as a key sequence for entering a sort of
paused state. This can be removed by editing the .bashrc file.

## File write to disk
When the user presses CTRL+S, we write the file back to disk, one line at a time. We first
truncate the file while opening, since this helps us avoid comparing the old contents with
the new for length and equivalence. This is slightly inefficient since the entire file must be
rewritten regardless how much was changed. This is necessary however, since UNIX file
systems do not allow arbitrary insertion of data into the middle of a file without overwriting.

# Unique features 

 we created two new features, one each for the terminal and the
editor. For the terminal, we provided a new function called `Hypercat`, which
allows a user to cat regular files and also PDF files which is not currently
supported by the cat function.\
For the editor, we provided `remote viewing`.
That is, the host system runs the editor and a user can interact with it
normally, and a client system can remotely connect to the host and see all the
updates to the file live, as they are being made.

## Hypercat
A key deficiency that we see in the cat system is that is simply gives a dump
of the contents of a file, regardless of its type. If it happens to be a regular
text file, this works well. But if it is a file type that includes some amount of
formatting, the results are meaningless.
Hypercat is a simple python script which uses the PyPDF2 library to extract
text content from a PDF file. As a first step, we accept the user’s filename as
a command line argument. Then we open the file in binary reading mode.
Once it is open, we use PyPDF2 to create a special object to encode all the
file’s contents. Once this is done, we query this object to determine the
number of pages in the file. Then, we iterate over each page, and for each we
get its text content using the function `extractText` which returns the text as
a string. This string is then printed to screen as a normal cat program would do. 

## Remote editor view
Our remote viewing functionality consists of two separate programs. One is the
host which runs the actual instance of the editor. Here the user can interact with
the editor and use all the functionality described above, including saving the
edits to disk. The other program is a much simpler program, which simply
establishes a connection to the host which acts as a server. It prints to screen
all the text it receives from the host editor.

In the host program, the frst step on startup is to set up an IP server to listen for
read requests on a port. To do this, we launch a separate thread which
performs the primary TCP/IP port setup. Once the server is established and the
socket is created, the thread listens on the port for a connection request. Once
the connection is established, the thread sets up a fle descriptor for the specifc
connection, and terminates itself. This connection fle descriptor, connfd, is a
global variable which is shared with the main thread. A golbal fag is also set,
which indicates to the display function that a client is listening, and that all data
being written to the screen must also be written to the network port. This code is
rather simple and adds very few lines to the display function, ensuring low
latency. 

```c
if(has_client){
    write(connfd, curr->text, strlen(curr->text));
}
```

The client program is also a very lightweight program, which simply attempts to
establish a connection with the host. Once this connection is established, it
listens on the port for incoming data. Whatever data it reads it prints to screen.
Even clearing of the screen on the client side when refreshing the display
content can be achieved by sending a particular bytestream on the connection. 

```c
write(connfd, "\e[1;1H\e[2J", 10)
```