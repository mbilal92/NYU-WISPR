#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>

int main(int argc, char *argv[])
{

  if (argc > 1)
  {
    int charID = atoi(argv[1]);
    printf("%s\n", argv[1] );  
    printf("%d\n", charID );  
    kill( charID, SIGKILL );
    printf("%d\n", charID );  

  } else {
    pid_t childPID = fork();
    printf("*************  %d ******* \n", childPID);

    if ( childPID == -1 )
    {
      printf( "failed to fork child\n" );
      _exit( 1 );
    }
    else if ( childPID == 0 )
    {

      // char *args[] = { "ping", "localhost", 0 };
      // execv( "/bin/ping", args );
      umask(0);
      //set new session
      sid = setsid();
      if(sid < 0)
      {
      // Return failure
      exit(1);
      }
      // Change the current working directory to root.
      chdir("/");
      // Close stdin. stdout and stderr
      close(STDIN_FILENO);
      close(STDOUT_FILENO);
      close(STDERR_FILENO);
    } else {
      //unmask the file mode
    }

  }
  
  return 0;
} 