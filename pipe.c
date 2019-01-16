/*-
	A. Abdulla, January 2008
*/

/*--------------- Third pipe example using O_NDELAY ---------------*/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#define MSGSIZE 16

/*------ Function prototypes ----------*/
void parent (int p[]);
void child (int p[]);
void fatal (char *);

/*------- Message Strings -------------*/
char *msgT = "Sent to translate";
char *msgO = "Sent to output";
char *msg2 = "T";

pid_t translate_pid;
pid_t output_pid;

void input(int translate_pipe[2], int echo_pipe[2]);
void translate(int translate_pipe[2], int output_pipe[2]);
void output(int echo_pipe[2], int output_pipe[2]);

int main (void)
{
  int translatepipe[2];
  int echopipe[2];
  int outputpipe[2];

  /*----- Open the pipe -----------*/
  // TODO: reduce code with : ?
  if (pipe(translatepipe) < 0)
  {
    fatal ("error creating translate pipe");
    exit(1);
  }

  if (pipe(echopipe) < 0)
  {
    fatal ("error creating echo pipe");
    exit(1);
  }

  if (pipe(outputpipe) < 0)
  {
    fatal ("error creating output pipe");
    exit(1);
  }


  /*---- Set the O_NDELAY flag for p[0] -----------*/
  // TODO: simplify
  if (fcntl (translatepipe[0], F_SETFL, O_NDELAY) < 0) {
      fatal ("fcntl call");
  }
  if (fcntl (echopipe[0], F_SETFL, O_NDELAY) < 0) {
      fatal ("fcntl call");
  }
  if (fcntl (outputpipe[0], F_SETFL, O_NDELAY) < 0) {
      fatal ("fcntl call");
  }


  /*-------- fork ---------------*/
  switch(translate_pid = fork()) {
      case -1:
        fatal("error creating translate child");
      case 0:
        translate(translatepipe, outputpipe);
      default:
        switch(output_pid = fork()) {
            case -1:
              fatal("error creating output child");
            case 0:
              output(echopipe, outputpipe);
            default:
              input(translatepipe, echopipe);
        }
  }
}

void input(int translate_pipe[2], int echo_pipe[2]) {
    char input;
    char buffer[100];
    char *inputptr;

    close(translate_pipe[0]);    /* close the read descriptor */
    close(echo_pipe[0]);    /* close the read descriptor */

    system("stty raw igncr -echo");

    while(1) {
        if((input=getchar()) != EOF) {
            if(input == 'E') {
                break;
            }
            else {
                inputptr = &input;
                write (echo_pipe[1], inputptr, 1);
            }
        }
    }
    //
    // while((input=getchar())!='E') {
    //     if(!inputptr){
    //         inputptr = &input;
    //         write (echo_pipe[1], inputptr, 1);
    //     }
    // }

    system("stty -raw -igncr echo");
    kill(translate_pid, SIGTERM);
    kill(output_pid, SIGTERM);
    exit(0);
}

void translate(int translate_pipe[2], int output_pipe[2]) {
    int nread;
    char buf[1];

    close(translate_pipe[1]);    /* close the write descriptor */
    close(output_pipe[0]);    /* close the read descriptor */

    for (;;)
    {
      switch (nread = read(translate_pipe[0], buf, 1))
      {
        case -1: break;
        case 0: break;
        default:
            if (strcmp (buf, msg2) == 0)
            {
          	  printf ("End of Translate\n");
          	  exit(0);
            }
            else{}

      }
    }
}

void output(int echo_pipe[2], int output_pipe[2]) {
    int nread;
    char buf[1];

    close(echo_pipe[1]);    /* close the write descriptor */
    close(output_pipe[1]);    /* close the write descriptor */

     while (1){
         nread = read(echo_pipe[0], buf, 1);
         if(nread == -1) {
             // printf("read is %d", nread);
             putchar(*buf);
        }
     }
}

/*---------- Error function ------*/
void fatal (char *s)
{
  perror (s);    /* print error msg and die */
  exit(1);
}
