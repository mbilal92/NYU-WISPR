/*
    Simple udp client
*/
#include<stdio.h> //printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include<arpa/inet.h>
#include<sys/socket.h>
#include <sys/time.h>
#include <time.h>


#define SERVER "10.0.0.1"
#define BUFLEN 512  //Max length of buffer
#define PORT 8888   //The port on which to send data
struct timeval current_time;


typedef struct __attribute__((packed, aligned(2))) m {
  uint32_t seq;
  uint32_t seconds;
  uint32_t mili;
} udp_packet_t;



void die(char *s)
{
    perror(s);
    exit(1);
}
 
int main(int argc, char** argv)
{
    struct sockaddr_in si_other;
    int s, i, slen=sizeof(si_other);
    char buf[BUFLEN];
    char message[BUFLEN];
    udp_packet_t *pdu; 

    float TTI = atof(argv[1])/1000.0;

    if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
 
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);
     
    if (inet_aton(SERVER , &si_other.sin_addr) == 0) 
    {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }
    int packet_counter = 0; 
    pdu = (udp_packet_t*)malloc(sizeof(udp_packet_t));
    int n = 0;
    // int packet_to_send = atoi(argv[1]);
    long int i_end = (int)((30*60)/TTI);
    printf("%ld %f\n", i_end, TTI);
    clock_t start = clock();
    while(packet_counter < i_end)
    {
        // printf("Enter message : ");
        // scanf("%s", message);
        memset(pdu, 0, sizeof(udp_packet_t));
        memset(&current_time ,0, sizeof(current_time));
        gettimeofday(&current_time,NULL);
        pdu->seconds = current_time.tv_sec;
        pdu->mili = current_time.tv_usec;
        pdu->seq = packet_counter;

        packet_counter ++;
        //send the message
        printf("%d,\t %d,%ld   .   %d,%ld \n", pdu->seq,pdu->seconds, current_time.tv_sec,pdu->mili, current_time.tv_usec);
        if (sendto(s, pdu, sizeof(udp_packet_t) , 0 , (struct sockaddr *) &si_other, slen)==-1)
        {
            die("sendto()");
        }
        
        usleep(TTI*1000000.0);
        // usleep(100000);
        //receive a reply and print it
        //clear the buffer by filling null, it might have previously received data
        //try to receive some data, this is a blocking call
        // if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen) == -1)
        // {
        //     die("recvfrom()");
        // }
         
        // puts(buf);
    }
    clock_t stop = clock();
    double elapsed = (double)(stop - start) * 1000.0 / CLOCKS_PER_SEC;
    printf("Time elapsed in ms: %f", elapsed);

    usleep(5000000);
    printf("chal bae\n");
    pdu->seq = -1;
    sendto(s, pdu, sizeof(udp_packet_t) , 0 , (struct sockaddr *) &si_other, slen);
    printf("chal bae1\n");
    close(s);
    return 0;
}