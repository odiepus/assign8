#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/stat.h>
#include<string.h>
#include<fcntl.h>

#define BUFF 1024


struct cmd
{
    int redirect_in;        //Any stdin redirection?
    int redirect_out;       //Any stdout redirection?
    int redirect_append;    //Append stdout redirection?
    int background;         //Put process in background?
    int piping;             //Pipe prog1 to prog2
    char *infile;           //Name of stdin redirect file
    char *outfile;          //Name of stdout redirect file
    char *argv1[10];        //First program to execute
    char *argv2[10];        //Second program to execute
};


int main(void)
{
    char holdInput[BUFF];   //will hold inputs from users
    int inNout[2];          //fd for our pipe
    struct cmd c1;          //struct to hold our inputs and have Prof's func parse for us
    int inFile;
    int outFile;

    printf("\n+--> ");      //print our prompt
    gets(holdInput);        //get user input and put in our buffer
    holdInput[strcspn(holdInput, "\n")] = 0; //place null at end of input just incase

    while( (strcmp(holdInput, "exit")) != 0 ) //loop till user inputs exit
    {
        if(cmdscan(holdInput, c1) == -1)
        {
            printf("Error Parsing line\n");
            printf("Please try again.\n");
            continue;
        }

        if(c1.piping == 1)                        //check if we need to pipe
        {
            if(pipe(inNout) == -1) //create this programs pipe
            {
                perror("pipe");
                exit -1;
            }

        }


        switch(fork())
        {
            case -1:
                perror("fork");
                exit -1;
                /*
                 * check if there are files. this allows me to redirect the std's first
                 * then if later I can redirect std's to the fd's if needed. else I can still
                 * redirect fd's later without having to worry about files
                 */
            case 0:         //first child
                switch(strcmp(c1.infile, " "))                          //check if there is an in file
                {
                    case 0:
                        if( (inFile = open(c1.infile, O_READONLY)) == -1) //if there is open it and redirect stdin from file
                        {
                            perror("open");
                            exit -1;
                        }
                        dup2(inNout[0], STDIN_FILENO);
                }
                switch(strcmp(c1.outfile, " "))                 //check if there is an outfile
                {
                    case 0:
                        if(c1.redirect_append == 1 && c1.redirect_out == 1)  //if there is an outfile then check its to be truncated or appended
                        {
                            outFile = open(c1.outfile, O_CREAT | O_WRONLY | O_APPEND, 0600);
                        }
                        else if(c1.redirect_out == 1)
                        {
                            outFile = open(c1.outfile, O_CREAT | O_WRONLY | O_TRUNC, 0600);
                        }
                        dup2(inNout[1], STDOUT_FILENO);                     //redirect stdout to our file
                }

                /*
                 * check if processes must run in the background
                 */
                switch(c1.background)
                {
                    case 0:         //if no background and no piping then there is only one program to run
                        swtich(c1.piping)
                        {
                            case 0:
                                execvp(c1.argv1[0], c1.argv1);
                                perror("exec");
                                exit -1;
                            case 1:     //if no BG and there is piping then we run our second prog in child1 and our first prog in child2(grandchild)
                                dup2(inNout[0], STDIN_FILENO);
                                close(inNout[0]);
                                close(inNout[1]);

                                switch(fork())
                                {
                                    case -1:
                                        perror("fork");
                                        exit -1;
                                    case 0:         //child2 (grandchild) this process will run first prog and pass to parent(child1)
                                        dup2(inNout[1], STDOUT_FILENO);
                                        close(inNout[1]);
                                        close(inNout[0]);       //child2 [pipes closed]

                                        execvp(c1.argv1[0], c1.argv1);
                                        perror("exec");
                                        exit -1;
                                    default:                //child1 takes output from child2 after it executes [pipes closed earlier]
                                        execvp(c1.argv2[0], c1.argv2);
                                        perror("exec");
                                        exit -1;
                                }
                        }
                    case 1:  //if there is to be BG
                        swtich(c1.piping)               //check for piping
                        {
                            case 0:                     //if ther is no piping
                                execvp(c1.argv1[0], c1.argv1);
                                perror("exec");
                                exit -1;
                            case 1:                     //if there is piping then have child 2(grandchild) fork and it and child3 can run progs
                                swtich(fork())
                                {
                                    case -1:
                                        perror("fork");
                                        exit -1;
                                    case 0:         //child4 (great grandchild) this process will run first prog and pass to parent(child3)
                                        dup2(inNout[1], STDOUT_FILENO);
                                        close(inNout[1]);
                                        close(inNout[0]);       //child4 [pipes closed]

                                        execvp(c1.argv1[0], c1.argv1);
                                        perror("exec");
                                        exit -1;
                                    default:                //child3 takes output from child4 after it executes [pipes closed earlier]
                                        execvp(c1.argv2[0], c1.argv2);
                                        perror("exec");
                                        exit -1;
                                }
                        }
                }
        }

    return 0;
}
