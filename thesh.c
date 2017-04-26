#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/stat.h>
#include<string.h>
#include<fcntl.h>
#include<wait.h>

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
    gets(holdInput); //get user input and put in our buffer
    //holdInput[strcspn(holdInput, "\r\n")] = 0; //place null at end of input just incase

    while( (strcmp(holdInput, "exit")) != 0 ) //loop till user inputs exit
    {
        if(cmdscan(&holdInput, &c1) == -1)
        {
            printf("Error Parsing line\n");
            exit (-1);
        }

        if(c1.piping == 1)                        //check if we need to pipe
        {
            if(pipe(inNout) == -1) //create this programs pipe
            {
                perror("pipe");
                exit (-1);
            }

        }


        switch(fork())
        {
            case -1:
                perror("fork");
                exit (-1);
                /*
                 * check if there are files. this allows me to redirect the std's first
                 * then if later I can redirect std's to the fd's if needed. else I can still
                 * redirect fd's later without having to worry about files
                 */
            case 0:         //first child
                if(c1.redirect_in == 1)
                {
                    if( (inFile = open(c1.infile, O_RDONLY)) == -1) //if there is open it and redirect stdin from file
                    {
                        perror("openFFFFFF");
                        exit (-1);
                    }
                    dup2(inFile, STDIN_FILENO);
                }

                if(c1.redirect_append == 1 && c1.redirect_out == 1)  //if there is an outfile then check its to be truncated or appended
                {
                    if( (outFile = open(c1.outfile, O_CREAT | O_WRONLY | O_APPEND, 0600)) == -1 )
                    {
                        perror("openhhhhhhh");
                        exit (-1);

                    }
                    dup2(outFile, STDOUT_FILENO);                     //redirect stdout to our file
                }
                else if(c1.redirect_out == 1)
                {
                    if( (outFile = open(c1.outfile, O_CREAT | O_WRONLY | O_TRUNC, 0600)) == -1)
                    {
                        perror("openHHHHHHH");
                        exit (-1);
                    }
                    dup2(outFile, STDOUT_FILENO);                     //redirect stdout to our file
                }


                /*
                 * check if processes must run in the background
                 */
                switch(c1.background)
                {
                    case 0:         //if no background and no piping then there is only one program to run
                        switch(c1.piping)
                        {
                            case 0:
                                execvp(c1.argv1[0], c1.argv1);
                                perror("exec");
                                exit (-1);
                            case 1:     //if no BG and there is piping then we run our second prog in child1 and our first prog in child2(grandchild)
                                switch(fork())          //child1 forks another child
                                {
                                    case -1:
                                        perror("fork");
                                        exit (-1);
                                    case 0:         //child2 (grandchild) this process will run first prog and pass to parent(child1)
                                        close(inNout[0]);       //child2 [pipes closed]
                                        dup2(inNout[1], STDOUT_FILENO);
                                        close(inNout[1]);

                                        execvp(c1.argv1[0], c1.argv1);
                                        perror("exec BBBBBBB");
                                        exit (-1);
                                    default:                //child1 takes output from child2 after it executes [pipes closed earlier]
                                        close(inNout[1]);
                                        dup2(inNout[0], STDIN_FILENO);
                                        close(inNout[0]);

                                        execvp(c1.argv2[0], c1.argv2);
                                        perror("exec GDAFDSDSAF");
                                        exit (-1);
                                }
                            default:
                                exit(0);
                        }
                    case 1:  //if there is to be BG
                        switch(c1.piping)               //check for piping
                        {
                            case 0:                     //if ther is no piping then give to child 3 (grandchild) and child2 leaves
                                switch(fork())
                                {
                                    case -1:
                                        perror("fork");
                                        exit (-1);
                                    case 0:                 //child3 execs the 1 prog cuz there is no piping
                                        execvp(c1.argv1[0], c1.argv1);
                                        perror("exec");
                                        exit (-1);
                                }
                            case 1:                     //if there is piping then have child 2(grandchild) fork and it and child3 can run progs
                                switch(fork())          //child1 forks another child
                                {
                                    case -1:
                                        perror("fork");
                                        exit (-1);
                                    case 0:         //child4 (great grandchild) this process will run first prog and pass to parent(child3)
                                        dup2(inNout[1], STDOUT_FILENO);
                                        close(inNout[1]);
                                        close(inNout[0]);       //child4 [pipes closed]

                                        execvp(c1.argv1[0], c1.argv1);
                                        perror("exec");
                                        exit (-1);
                                    default:                //child3 takes output from child4 after it executes [pipes closed earlier]
                                        dup2(inNout[0], STDIN_FILENO);
                                        close(inNout[0]);
                                        close(inNout[1]);

                                        execvp(c1.argv2[0], c1.argv2);
                                        perror("exec ,MMMMMMMM");
                                        exit (-1);
                                }
                        }
                    default:        //child 1 exits cuz there is no piping
                        exit (0);
                }
            default:                            //parent closes its pipe and doesnt wait for child
                if(c1.piping == 1)
                {
                    close(inNout[0]);
                    close(inNout[1]);
                }
                wait(NULL);
        }                               //end of first fork
        printf("\n+--> ");      //print our prompt
        gets(holdInput);        //get user input and put in our buffer
        holdInput[strcspn(holdInput, "\n")] = 0; //place null at end of input just incase

    } // end of while loop




    return 0;
}
