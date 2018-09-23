#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>

//#define PORT 4311

static int quit = 0;
static FILE *fp;
static int sockfd;
struct timeval receivedtime;

typedef struct __attribute__((packed, aligned(2))) m {
  uint32_t seq;
  long long seconds;
  long long mili;
} udp_packet_t;

/* SIGINT handler: set quit to 1 for graceful termination */
void
handle_sigint(int signum) {
  quit = 1;
  gettimeofday(&receivedtime,NULL);
  fprintf(fp, "END TIMESTAMP %ld.%3.6ld\n", receivedtime.tv_sec, receivedtime.tv_usec);

  fclose (fp);
  close(sockfd);

  printf ("\n Finished the experiement \n");
  exit(0);
}

int main(int argc, char**argv)
{
  int n, bytes_sent, PORT;
  float delay;
  struct sockaddr_in servaddr, to_addr;
  struct timeval lastreceivedtime;
  char filename[256];
  udp_packet_t *pdu;

  if (argc != 3)
    {
      printf ("Usage : %s <Output File name> <Port number>\n", argv[0]);
      exit(0);
    }

  sprintf(filename, "./%s", argv[1]);
  PORT = atoi(argv[2]);
  fp=fopen(filename, "w+");

//  gettimeofday(&receivedtime,NULL);
//  fprintf(fp, "START TIMESTAMP %ld.%3.6ld\n", receivedtime.tv_sec, receivedtime.tv_usec);

  // creating the remote struct for sending the packet initialization from the user side
  sockfd=socket(AF_INET,SOCK_DGRAM,0);

  // bind the socket to the the port number
  bzero(&servaddr,sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
  servaddr.sin_port=htons(PORT);
  bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));

  printf("SEQ \t received time\n");
  fprintf(fp, "SEQ \t received time\n");

  signal(SIGINT, handle_sigint);

  pdu = (udp_packet_t *) malloc(sizeof(udp_packet_t));

  while ( !quit )
    {
      n = recvfrom(sockfd,pdu,1000000,0,NULL,NULL);
      gettimeofday(&receivedtime,NULL);
      delay = (receivedtime.tv_sec-pdu->seconds)*1000.0+(receivedtime.tv_usec-pdu->mili)/1000.0;

      //printf("%3.9u,\t %ld.%3.6ld\n", pdu->seq, receivedtime.tv_sec, receivedtime.tv_usec);
      fprintf(fp, "%3.9u,\t %ld.%3.6ld\n", pdu->seq, receivedtime.tv_sec, receivedtime.tv_usec);
      //fprintf(fp, "%3.9u,\t %ld.%3.6d,\t %lld.%3.6lld,\t %f,\t %d, \t", pdu->seq, receivedtime.tv_sec, receivedtime.tv_usec, pdu->seconds, pdu->mili, delay,n);

//      delay = (receivedtime.tv_sec-lastreceivedtime.tv_sec)*1000.0+(receivedtime.tv_usec-lastreceivedtime.tv_usec)/1000.0;
//      lastreceivedtime = receivedtime;

//      printf ("%f \n", delay);
//      fprintf (fp, "%f \n", delay);
    }
  gettimeofday(&receivedtime,NULL);
  fprintf(fp, "End %ld.%3.6ld\n", receivedtime.tv_sec, receivedtime.tv_usec);

  close(sockfd);
  fclose (fp);
  return 0;
}
