// Standard libraries added
#include <sys/socket.h>			//Contains data definition and socket structure
#include <netinet/in.h>			//Contains internet family protocol definitions
#include <stdio.h>				//Prototypes of standard input, output functions
#include <stdint.h>				//using for fixed width integer types
#include <stdlib.h>				//To use functions like bzero, exit, atoi etc
#include <unistd.h>				//For timer methods
#include <string.h>				//Contains useful string functions


// Reply code definition
#define NOT_PAID 0XFFF9
#define NOT_EXIST 0XFFFA
#define ACCESS_OK 0XFFFB


// defining data structure of response packet
//using unsigned variables as below values will always be positive
struct response_packet {
	uint16_t start_of_packetID;				
	uint8_t client_ID;						
	uint16_t response_code;							
	uint8_t segment_No;							
	uint8_t length;							//length of payload
	uint8_t technology;						
	unsigned long Source_subscriber_No;		 
	uint16_t end_of_packet_ID;						
};

// defining data structure of request packet
struct request_packet{
	uint16_t start_of_packetID;				
	uint8_t client_ID;						
	uint16_t access_per;						
	uint8_t segment_No;							
	uint8_t length;							
	uint8_t technology;						
	unsigned long Source_subscriber_No;		
	uint16_t end_of_packet_ID;						
};

// printing received packet
void print_packet(struct request_packet request_packet ) {
	printf("\nPacket ID \t\t: %#X\n",request_packet.start_of_packetID);
	printf("Client ID \t\t: %#X\n",request_packet.client_ID);
	printf("Access Permission \t: %#X\n",request_packet.access_per);
	printf("Segment number \t\t: %d \n",request_packet.segment_No);
	printf("Length of the Packet \t: %d\n",request_packet.length);
	printf("Technology \t\t: %d \n", request_packet.technology);
	printf("Subscriber number \t: %lu \n",request_packet.Source_subscriber_No);
	printf("End of Packet ID \t: %#X \n",request_packet.end_of_packet_ID);
}


// Function to initialize request packet
struct request_packet Initialize () {
	struct request_packet request_packet;
	request_packet.start_of_packetID = 0XFFFF;
	request_packet.client_ID = 0XFF;
	request_packet.access_per = 0XFFF8;
	request_packet.end_of_packet_ID = 0XFFFF;
	return request_packet;

}


// this function writes server packet data to a log file
void log_server_packet(const struct response_packet* packet) {
    FILE* log_file = fopen("server_log.txt", "a");

	// using fprintf() method as we are logging
    if (log_file != NULL) {
        fprintf(log_file, "Response Packet:\n");
        fprintf(log_file, "Start Packet ID \t: %#X\n", packet->start_of_packetID);
        fprintf(log_file, "Client ID \t\t: %#X\n", packet->client_ID);
        fprintf(log_file, "Response code \t\t\t: %#X\n", packet->response_code);
        fprintf(log_file, "Segment Number \t\t: %d\n", packet->segment_No);
        fprintf(log_file, "Length of the packet \t: %d\n", packet->length);
        fprintf(log_file, "Technology \t\t: %d\n", packet->technology);
        fprintf(log_file, "Subscriber Number \t: %lu\n", packet->Source_subscriber_No);
        fprintf(log_file, "End of Packet ID \t: %#X\n", packet->end_of_packet_ID);
        fprintf(log_file, "---------------------------------------------------\n");
        fclose(log_file);
    }
}

int main(int argc, char**argv){

	//Arguments are ./client - where client program's compiler output is stored, local host and port number (server will LISTEN)
	if (argc != 3)
    {
        printf("please give command: ./client server_name port_number(localhost 8080)\n");
        exit(1);
    }


	struct request_packet request_packet;			// storing request packet
	struct response_packet response_packet;		// storing response packet
	char line_buffer[30];							// this buffer stores every line read from the file
	int i = 1;
	FILE *file_pointer;									
	int sockfd,b = 0;								// socket file descriptor and bytes number received
	struct sockaddr_in clnt_addr;			    	// client address information
	socklen_t addr_size;							// storing client address structure size
	sockfd = socket(AF_INET,SOCK_DGRAM,0);			// UDP socket creation
	struct timeval timeVal;						    // setting ack_timer
	timeVal.tv_sec = 3;           				    // setting timer to 3seconds
	timeVal.tv_usec = 0;							// setting timer to 0 ms

	// checking socket creation status
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeVal,sizeof(struct timeval));
	int counter = 0;
	if(sockfd < 0) {
		printf("Connection Failed\n");
	}
	bzero(&clnt_addr,sizeof(clnt_addr));		    	// setting all the bytes in clnt_addr to zero
	clnt_addr.sin_family = AF_INET;				    	//Address family is set to IPV4
	clnt_addr.sin_addr.s_addr = htonl(INADDR_ANY);		// setting server to listen from all network interfaces
	clnt_addr.sin_port=htons(atoi(argv[2]));			// port number to big-endian network byte order conversion
	addr_size = sizeof clnt_addr ;						// stores client address size to use in recvfrom() method

    // initializing request packet
	request_packet = Initialize();

	// opening the file in read text mode 
	file_pointer = fopen("Subscriber_data.txt", "rt");

	// checking if the file is opened successfully and return in case of error
	if(file_pointer == NULL)
	{
		printf("Error: Failure in opening the file\n");
	}

	// Loop to read each line of the file untill the end of file is reached
	while(fgets(line_buffer, sizeof(line_buffer), file_pointer) != NULL) {
		counter = 0;																			    // counter of number of attempts
		b = 0;																						// number of bytes received from recvfrom() method
		printf(" \n\n -------------Start of a New Packet------------ \n\n");
		char * token;																				//this pointer stores each token after splitting
		token = strtok(line_buffer," ");															//splits the line to different tokens separated by space
		request_packet.length = strlen(token);														
		request_packet.Source_subscriber_No = (unsigned long) strtol(token, (char **)NULL, 10);	// converting token subscriber number and storing in request_packet
		token = strtok(NULL," ");																	// moving token to next line
		request_packet.length += strlen(token);													    // updating LENGTH field by adding the length of the token(second) technology
		request_packet.technology = atoi(token);													// converting technology token and storing in request_packet
		token = strtok(NULL," ");																	// moving token to next line
		request_packet.segment_No = i;																// updating segment number
		int attempt=1;																				//keeping track of number of failed responses from server

		print_packet(request_packet);
		
		// Looping to send packets to server
		// Maximum of three attempts for any packet transmission 
		// server supports maximum of 10 segments

		while(b <= 0 && counter < 3) { 

			// sending packet to server
			sendto(sockfd,&request_packet,sizeof(struct request_packet),0,(struct sockaddr *)&clnt_addr,addr_size);

			// storing received packet in response_packet
			b = recvfrom(sockfd,&response_packet,sizeof(struct response_packet),0,NULL,NULL);

			// In case of no response from server, it will be retried 3 times and number of attempts will be printed
			if(b <= 0 ) {
				printf("No response received from server. Ack_timer expired. Attempt: %d\n",attempt++);
				counter ++;
			}

			// When response from server is received, test cases will be checked
			else if(b > 0) {
				printf("Status = ");
				if(response_packet.response_code == NOT_PAID) {
					printf("\n.............................\n");
					printf("Subscriber has not paid\n" );
					printf(".............................\n");
				}
				else if(response_packet.response_code == NOT_EXIST ) {
					printf("\n.........................\n");
					printf("Subscriber does not exist\n");
					printf(".........................\n");
				}
				else if(response_packet.response_code == ACCESS_OK) {
					printf("\n.....................................\n");
					printf("Subscriber is permitted to access network\n");
					printf("......................................\n");

				}
				// logging received server packet into a text file
            	log_server_packet(&response_packet);
				
			}
		}
		// program exists when there is no response from server after three failed attempts
		if(counter >= 3 ) {
			printf("Server is not responding.");
			exit(0);
		}
		i++;			// updating segment number
		
	}
	fclose(file_pointer);	// closing the file
};