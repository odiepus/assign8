#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/stat.h>
#include<string.h>

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

    printf("\n+--> ");      //print our prompt
    gets(holdInput);        //get user input and put in our buffer
    holdInput[strcspn(holdInput, "\n")] = 0; //place null at end of input just incase

    while( (strcmp(holdInput, "exit")) != 0 ) //loop till user inputs exit
    {
        if(cmdscan(holdInput, c1) == -1)
        {
            printf("Error Parsing line\n");
            printf("Please try again.\n");
            contine;
        }

        if(c1.pipe == 1)                        //check if we need to pipe
        {
            if(pipe(inNout) == -1) //create this programs pipe
            {
                perror("pipe");
                exit -1;
            }

        }


        sitch(fork())
        {
            case -1:
                perror("fork");
                exit -1;
        }

    return 0;
}
