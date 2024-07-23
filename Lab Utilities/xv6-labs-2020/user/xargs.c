#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

#define MAXN 1000

int
main(int argc, char *argv[])
{
    //If there are no other parameters except for the program name, output an error message and exit the program.
    if (argc < 2) {
        fprintf(2, "usage: xargs command error\n");
        exit(1);
    }

    char * argvs[MAXARG];
    int index = 0;
    for (int i = 1; i < argc; ++i) {
        argvs[index++] = argv[i];
    }

    // The buf [] array is used to store data read from standard input (pipeline).
    char buf[MAXN] = {"\0"};
    
    int n;
	// Read data from pipeline loop
    while((n = read(0, buf, MAXN)) > 0 ) {
        // The temp [] array is used to store each row of data read from buf [] as an additional parameter to the exec function.
		char temp[MAXN] = {"\0"};
        argvs[index] = temp;
        // Inner loop retrieves appended parameters and creates child processes to execute commands
        for(int i = 0; i < strlen(buf); ++i) {
            //To read individual lines of input, read a character at a time until a newline ('\n') appears.
            if(buf[i] == '\n') {
                if (fork() == 0) {
                    exec(argv[1], argvs);
                }
                wait(0);
            } 
            else {
                temp[i] = buf[i];
            }
        }
    }
    
    exit(0);
}
