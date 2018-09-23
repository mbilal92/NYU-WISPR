#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include<stdlib.h> //exit(0);



struct timeval current_time;

typedef struct __attribute__((packed, aligned(2))) m {
  	uint32_t seq;
	uint32_t mili;
	uint32_t seconds;
	// uint8_t data[1300];
} udp_packet_t;


int main(int argc, char** argv) {
	const char* server_name = "10.0.0.1";
	const int server_port = 8877;
	udp_packet_t *pdu;
	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
    float TTI = atof(argv[1])/1000.0;

    printf("%f\n", atof(argv[1]));

	// creates binary representation of server name
	// and stores it as sin_addr
	// http://beej.us/guide/bgnet/output/html/multipage/inet_ntopman.html
	inet_pton(AF_INET, server_name, &server_address.sin_addr);

	// htons: port in network order format
	server_address.sin_port = htons(server_port);

	// open a stream socket
	int sock;
	if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("could not create socket\n");
		return 1;
	}

	// TCP is connection oriented, a reliable connection
	// **must** be established before any data is exchanged
	if (connect(sock, (struct sockaddr*)&server_address,
	            sizeof(server_address)) < 0) {
		printf("could not connect to server\n");
		return 1;
	}

	// send

	// data that will be sent to the server
	const char* data_to_send = "Gangadhar Hi Shaktimaan hai";
    int packet_counter = 0; 
    int n = 0;
    // int packet_to_send = atoi(argv[1]);
    pdu = (udp_packet_t*)malloc(sizeof(udp_packet_t));
    long int i_end = (int)((30*60)/TTI);
    printf("%ld %f\n", i_end, TTI);

    while(packet_counter < i_end)
    {
		memset(pdu, 0, sizeof(udp_packet_t));
		memset(&current_time ,0, sizeof(current_time));
	    gettimeofday(&current_time,NULL);
	    pdu->seconds = current_time.tv_sec;
	    pdu->mili = current_time.tv_usec;
	    pdu->seq = packet_counter;
        packet_counter++;
		// printf("%d,\t  %d,%ld \n", current_time.tv_sec,pdu->mili, current_time.tv_usec);
		n = send(sock, pdu, sizeof(udp_packet_t), 0);
		printf("%d,\t %d,%ld   .   %d,%ld \n", pdu->seq,pdu->seconds, current_time.tv_sec,pdu->mili, current_time.tv_usec);
		printf ("%d %d\n", n, packet_counter);
        // usleep(TTI*1000000.0);
		usleep(100000);
		if (n == -1)
		{
			printf("chud gae\n");
			break;
		}
	}
	pdu->seq = -1;
	n = send(sock, pdu, sizeof(udp_packet_t), 0);
	// receive

	// int n = 0;
	// int len = 0, maxlen = 100;
	// char buffer[maxlen];
	// char* pbuffer = buffer;

	// // will remain open until the server terminates the connection
	// while ((n = recv(sock, pbuffer, maxlen, 0)) > 0) {
	// 	pbuffer += n;
	// 	maxlen -= n;
	// 	len += n;

	// 	buffer[len] = '\0';
	// 	printf("received: '%s'\n", buffer);
	// }

	// close the socket
	close(sock);
	return 0;
}