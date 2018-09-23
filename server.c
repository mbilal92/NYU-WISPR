/*
    Simple udp server
*/
#include<stdio.h> //printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include<arpa/inet.h>
#include<sys/socket.h>
#include <sys/time.h>


#define BUFLEN 512  //Max length of buffer
#define PORT 8888   //The port on which to listen for incoming data
struct timeval receivedtime;
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
 
int main(int argc, char *argv[]) 
{
    struct sockaddr_in si_me, si_other;
    udp_packet_t *pdu;     
    int packet_to_receive = atoi(argv[1]);
    int * packet_to_receive_array = (int*) malloc(4*packet_to_receive);
    float delay,delay1,delay2,delay3,delay4;
    int s, i, slen = sizeof(si_other) , recv_len;
    char buf[BUFLEN];
     
    //create a UDP socket
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
     
    // zero out the structure
    memset((char *) &si_me, 0, sizeof(si_me));
     
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
     
    //bind socket to port
    if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
    {
        die("bind");
    }
    pdu = (udp_packet_t*)malloc(sizeof(udp_packet_t));
    int n = 0;
    //keep listening for data
        // while (pdu->seq != -1) {
    // FILE *fptr;
    // fptr = fopen("UDP.txt","w");
    
    while(1)
    {
        // printf("Waiting for data...");
        // fflush(stdout);

        int duplicateCount = 0;
        // memset(packet_to_receive_array, 0, sizeof(packet_to_receive*4));
        memset(pdu, 0, sizeof(udp_packet_t));
        // int  i = 0;
        for (i = 0; i < packet_to_receive; ++i)
        {
            packet_to_receive_array[i] = 0;
        }

        int lostPacket = 0;
        float finalDelay = 0;
        while(1) {
            //try to receive some data, this is a blocking call
            memset(pdu, 0, sizeof(udp_packet_t));
            if ((n = recv_len = recvfrom(s, pdu, sizeof(udp_packet_t), 0, (struct sockaddr *) &si_other, &slen)) == -1)
            {
                die("recvfrom()");
            }
             
            //print details of the client/peer and the data received
            // printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
            gettimeofday(&receivedtime,NULL);
            if (pdu->seq == -1)
            {
                printf("Done\n");
                break;
            }

            // delay = (receivedtime.tv_sec-pdu->seconds)*100000.0+(receivedtime.tv_usec-pdu->mili)/100000.0;
            delay = receivedtime.tv_sec - pdu->seconds;
            delay1 = delay *100000.0;
            if (receivedtime.tv_usec < pdu->mili)
            {
                delay2 = (receivedtime.tv_usec + 1000000) - pdu->mili;
            }else {
                delay2 = receivedtime.tv_usec - pdu->mili;
            }

            delay3 = delay1 + delay2;
            delay4 = delay3/100000;
            // printf("%d\n", packet_to_receive_array[(int)pdu->seq]);
            if (packet_to_receive_array[(int)pdu->seq] == 1)
            {
                duplicateCount++;
                // printf("duplicate: %d\n", pdu->seq);
            } else {
                packet_to_receive_array[(int)pdu->seq] = 1;
                printf("%d %d,\t %d,%ld   .   %d,%ld --- %f %f %f %f %f\n",n, pdu->seq,pdu->seconds, receivedtime.tv_sec,pdu->mili, receivedtime.tv_usec, delay, delay1, delay2, delay3, delay4);
                finalDelay += delay4;     
                // fprintf(fptr, "%f\n", delay4);       
            }
            
            // printf("%d %d,\t %d,%ld   .   %d,%ld --- %f %f %f %f %f\n",n, pdu->seq,pdu->seconds, receivedtime.tv_sec,pdu->mili, receivedtime.tv_usec, delay, delay1, delay2, delay3, delay4);
            // int i = 0;  
            // for (i = 0; i < packet_to_receive; ++i)
            // {
            //     printf("%d ",  packet_to_receive_array[i]);
            // }
            // printf("\n");
            // int i = 0;

        }

        for (i = 0; i < packet_to_receive; ++i)
        {
            printf("%d ",  packet_to_receive_array[i]);
        }
        printf("\n");
        for (i = 0; i < packet_to_receive; ++i)
        {
            if (packet_to_receive_array[i] != 1){
                lostPacket++;
            }
        }
        printf("%d %d %f\n",lostPacket, duplicateCount, finalDelay/(packet_to_receive- lostPacket));
        // if (fptr != NULL)
        // {
        //     fclose(fptr);
        //     fptr = NULL;
        // }
        // //now reply the client with the same data
        // if (sendto(s, buf, recv_len, 0, (struct sockaddr*) &si_other, slen) == -1)
        // {
        //     die("sendto()");
        // }
    }
    
    close(s);
    return 0;
}