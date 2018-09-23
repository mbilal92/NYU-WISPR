#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

/**
 * TCP Uses 2 types of sockets, the connection socket and the listen socket.
 * The Goal is to separate the connection phase from the data exchange phase.
 * */

struct timeval receivedtime;
typedef struct __attribute__((packed, aligned(2))) m {
  uint32_t seq;
  uint32_t mili;
  uint32_t seconds;  
  // uint8_t data[1300];
} udp_packet_t;

int main(int argc, char *argv[]) {
	// port to start the server on
	int SERVER_PORT = 8877;
	udp_packet_t *pdu;
	float delay;
	float delay1;
	float delay2;
	float delay3;
	float delay4;
	float delay5;
	int packet_to_receive = atoi(argv[1]);
	int * packet_to_receive_array = (int*) malloc(4*(packet_to_receive +2));
	

	// socket address used for the server
	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;

	// htons: host to network short: transforms a value in host byte
	// ordering format to a short value in network byte ordering format
	server_address.sin_port = htons(SERVER_PORT);

	// htonl: host to network long: same as htons but to long
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);

	// create a TCP socket, creation returns -1 on failure
	int listen_sock;
	if ((listen_sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("could not create listen socket\n");
		return 1;
	}

	// bind it to listen to the incoming connections on the created server
	// address, will return -1 on error
	if ((bind(listen_sock, (struct sockaddr *)&server_address,
	          sizeof(server_address))) < 0) {
		printf("could not bind socket\n");
		return 1;
	}

	int wait_size = 16;  // maximum number of waiting clients, after which
	                     // dropping begins
	if (listen(listen_sock, wait_size) < 0) {
		printf("could not open socket for listening\n");
		return 1;
	}

	// socket address used to store client address
	struct sockaddr_in client_address;
	int client_address_len = 0;
	// FILE *fptr;
 //    fptr = fopen("delayTCP.txt","w");
 //    if (fptr == NULL)
 //    {
 //    	printf("error File\n");
 //    }
	// run indefinitely
	while (true) {
		// open a new socket to transmit data per connection
		int sock;
		if ((sock =
		         accept(listen_sock, (struct sockaddr *)&client_address,
		                &client_address_len)) < 0) {
			printf("could not open a socket to accept data\n");
			return 1;
		}

		int n = 0;
		int len = 0, maxlen = 100;
		// char buffer[maxlen];
		// char *pbuffer = buffer;

		printf("client connected with ip address: %s:%d\n",
		       inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
		// keep running as long as the client keeps the connection open
		int counter = 0;
		int duplicateCount = 0;
		// memset(packet_to_receive_array, 0, sizeof(packet_to_receive*4));
		pdu = (udp_packet_t*)malloc(sizeof(udp_packet_t));
		memset(pdu, 0, sizeof(udp_packet_t));
		float FinalDelay = 0;
		int  i = 0;
		for (i = 0; i < packet_to_receive; ++i)
		{
			packet_to_receive_array[i] = 0;
		}
		int PacketReceived = 0;
		while (pdu->seq != -1) {
			// n = recvfrom(sock,pdu,sizeof(udp_packet_t),0,NULL,NULL);
			memset(pdu, 0, sizeof(udp_packet_t));
			// printf("Hello1\n");
			n = recv(sock, pdu, sizeof(udp_packet_t), 0);
		    if ((int)pdu->seq == -1)
		    {
		    	break;
		    }
		    memset(&receivedtime ,0, sizeof(receivedtime));
		    gettimeofday(&receivedtime,NULL);
			// printf("Hello2\n");

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
		    
			// printf("Hello3\n");
			printf("%d\n",(int)pdu->seq);
		    if (packet_to_receive_array[(int)pdu->seq] == 1)
		    {
		    	duplicateCount++;
		    	// printf("duplicate: %d\n", pdu->seq);
		    } else {
		    	// printf("%d %d \n",(int)pdu->seq, ++PacketReceived);
		    	packet_to_receive_array[(int)pdu->seq] = 1;
		    	// PacketReceived++;
		    	printf("%d %d,\t %d,%ld   .   %d,%ld --- %f %f %f %f %f\n",n, pdu->seq,pdu->seconds, receivedtime.tv_sec,pdu->mili, receivedtime.tv_usec, delay, delay1, delay2, delay3, delay4);
			    FinalDelay += delay4;
			    // fprintf(fptr, "%f\n", delay4);       
			}
		    // printf("%d %d  %d,%ld --- %f \n",n, pdu->seq,pdu->mili, receivedtime.tv_usec, delay2);
			// printf("Hello4\n");
		}

		int lostPacket = 0;
		// int i = 0;
		for (i = 0; i < packet_to_receive; ++i)
		{
			if (packet_to_receive_array[i] != 1){
				lostPacket++;
			}
		}

			// printf("Hello5\n");
		for (i = 0; i < packet_to_receive; ++i)
		{
			printf("%d ",  packet_to_receive_array[i]);
		}
		printf("\n");

		// printf("Hello1\n");
		printf("%d %d %f\n",lostPacket, duplicateCount, FinalDelay/(packet_to_receive - lostPacket) );
		close(sock);
		free(pdu);
		// fclose(fptr);
	}
		
	close(listen_sock);
	return 0;
}
