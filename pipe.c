/*-
	A. Abdulla, January 2008
*/

/*--------------- Third pipe example using O_NDELAY ---------------*/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define MSGSIZE 16

/*------ Function prototypes ----------*/
void parent (int p[]);
void child (int p[]);
void fatal (char *);

/*------- Message Strings -------------*/
char *msgT = "Sent to translate";
char *msgO = "Sent to output";
char *msg2 = "Goodbye World";

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
  switch(fork()) {
      case -1:
        fatal("error creating translate child");
      case 0:
        translate(translatepipe, outputpipe);
      default:
        switch(fork()) {
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
    int count;

    close(translate_pipe[0]);    /* close the read descriptor */
    close(echo_pipe[0]);    /* close the read descriptor */

    for (count = 0; count < 3; count ++)
    {
      write (translate_pipe[1], msgT, MSGSIZE);
      write (echo_pipe[1], msgO, MSGSIZE);
      sleep (3);
    }

    write (translate_pipe[1], msg2, MSGSIZE);
    write (echo_pipe[1], msg2, MSGSIZE);
    exit(0);
}

void translate(int translate_pipe[2], int output_pipe[2]) {
    int nread;
    char buf[MSGSIZE];

    close(translate_pipe[1]);    /* close the write descriptor */
    close(output_pipe[0]);    /* close the read descriptor */

    for (;;)
    {
      switch (nread = read(translate_pipe[0], buf, MSGSIZE))
      {
        case -1:
        case 0:
  	          printf ("(translate pipe empty)\n");
  	          sleep(1);
              break;
        default:
            if (strcmp (buf, msg2) == 0)
            {
          	  printf ("End of Translate\n");
          	  exit(0);
            }
        else
  	         printf ("MSG = %s\n", buf);
      }
    }
}

void output(int echo_pipe[2], int output_pipe[2]) {
    int nread;
    char buf[MSGSIZE];

    close(echo_pipe[1]);    /* close the write descriptor */
    close(output_pipe[1]);    /* close the write descriptor */

    for (;;)
    {
      switch (nread = read(echo_pipe[0], buf, MSGSIZE))
      {
        case -1:
        case 0:
  	          printf ("(Output pipe empty)\n");
  	          sleep(1);
              break;
        default:
            if (strcmp (buf, msg2) == 0)
            {
          	  printf ("End of Output\n");
          	  exit(0);
            }
        else
  	         printf ("MSG = %s\n", buf);
      }
    }
}

/*---------- Error function ------*/
void fatal (char *s)
{
  perror (s);    /* print error msg and die */
  exit(1);
}
