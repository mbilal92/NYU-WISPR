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
static int sockfd;
static FILE *fp;

typedef struct __attribute__((packed, aligned(2))) m {
  uint32_t seq;
  long long seconds;
  long long millis;
} udp_packet_t;

/* SIGINT handler: set quit to 1 for graceful termination */
void
handle_sigint(int signum) {
  quit = 1;
  close(sockfd);
  fclose(fp);
  printf ("\n Finished the experiement \n");
  exit(0);
}

int main(int argc, char** argv)
{
  struct sockaddr_in serv_addr, client;
  int len = sizeof(client);
  int i, j, slen=sizeof(serv_addr), PORT;
  long int i_end;
  int msglen;
  int port=0;
  int PACKETSIZE;
  char filename[256];
  char packet[256];
  float TTI;
  udp_packet_t *pdu;
  struct timeval timestamp;

  if(argc != 5)
    {
      printf("Usage : %s <UDP-Packet-Size in Bytes> <Transmission-Frequency in ms> <Output file name> \n",argv[0]);
      exit(0);
    }

  TTI = atof(argv[2])/1000.0;
  PACKETSIZE = atoi(argv[1]);
  sprintf(filename, "./%s", argv[3]);
  PORT = atoi(argv[4]);
  fp=fopen(filename, "w+");

  // openning a UDP socket
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
    {
      printf("error while openning socket");
      exit(0);
    }

  // binding the udp socekt to the port of the server
  bzero(&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);
  bind(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr));

  printf ("Server side waiting for user to initialize the connection \n");

  // waiting to receive the intialiaztion packet, to get the user gloabl ip address and the port number
  for (;;)
    {
      msglen = recvfrom(sockfd,packet,100,0,(struct sockaddr *)&client, (socklen_t *)&len);
      port = ntohs(client.sin_port);

      if (port != 0)
	{
	  serv_addr.sin_port = htons(port);
       	  memcpy(&serv_addr.sin_addr, &client.sin_addr, sizeof (client.sin_addr));
	  printf ("Recieved request from user global IP address %s and port number %d \n ", inet_ntoa(serv_addr.sin_addr), port);
	  break;
	}
    }

  printf("SEQ \t Send time \t\t UDP Size \n");

  signal(SIGINT, handle_sigint);

  i_end = (int)((30*60)/TTI);
  pdu = (udp_packet_t*)malloc(PACKETSIZE);

  for ( i=0 ; i<= i_end; i++) {

    pdu->seq = i;
    gettimeofday(&timestamp,NULL);
    pdu->seconds = timestamp.tv_sec;
    pdu->millis = timestamp.tv_usec;

    fprintf(fp, "%3.9u, \t %lld.%lld, \t %d\n", i,  pdu->seconds, pdu->millis, msglen);
//    printf("%3.9u, \t %lld.%lld, \t %d\n", i,  pdu->seconds, pdu->millis, msglen);

    msglen = sendto(sockfd, pdu , PACKETSIZE, 0, (struct sockaddr*)&serv_addr, slen);

    if (msglen==-1) {
      printf("failed to sent");
      exit(0);
    }

    usleep(TTI*1000000.0);

  }

  close(sockfd);
  fclose(fp);
  return 0;
}
