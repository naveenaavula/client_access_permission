// Standard libraries added
#include <sys/socket.h>			//Contains data definition and socket structure
#include <netinet/in.h>			//Contains internet family protocol definitions
#include <stdio.h>				//Prototypes of standard input, output functions
#include <stdint.h>				//using for fixed width integer types
#include <stdlib.h>				//To use functions like bzero, exit, atoi etc
#include <unistd.h>				//Included to use timer
#include <string.h>				//Contains useful string functions

//#define is used to declare a constant value to use in program
#define LENGTH 10 //Subscriber ID length

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

//defining data structure to store retrieved information from DB
struct subscriber_db {
	unsigned long sub_no;					//subscriber number
	uint8_t technology;						
	int status;								//status of payment/subscription
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


// Function to create response packet
struct response_packet createresponse_packet(struct request_packet request_packet) {
	struct response_packet response_packet;
	response_packet.start_of_packetID = request_packet.start_of_packetID;
	response_packet.client_ID = request_packet.client_ID;
	response_packet.segment_No = request_packet.segment_No;
	response_packet.length = request_packet.length;
	response_packet.technology = request_packet.technology;
	response_packet.Source_subscriber_No = request_packet.Source_subscriber_No;
	response_packet.end_of_packet_ID = request_packet.end_of_packet_ID;
	return response_packet;
}

// Below function reads DB file, stores in data structure for comparison and stores in the subscriber_db array.
void read_file(struct subscriber_db subscriber_db[]) {

	char line_buffer[30];									// this buffer stores every line read from the file
	int i = 0;												// tracks array current index
	FILE *file_pointer;											
	file_pointer = fopen("Verification_Database.txt", "rt");	// opens file in read mode

	// checks if the files is opened and returns in case of error
	if(file_pointer == NULL)
	{
		printf("Error: Failed to open the file\n");
		return;
	}
	// loops through each line of files until code reaches end of file
	while(fgets(line_buffer, sizeof(line_buffer), file_pointer) != NULL)
	{
		char *token=NULL;											// this pointer stores each token after splitting
		token = strtok(line_buffer," ");							// splits the line to different tokens separated by space
		subscriber_db[i].sub_no =(unsigned) atol(token);            // extracting Subscriber number and converting it to long unsigned int
		token = strtok(NULL," ");									// token moves to next line
		subscriber_db[i].technology = atoi(token);			        // extracting Technology and converting it to int
		token = strtok(NULL," ");									
		subscriber_db[i].status = atoi(token);			         	// extracting status and converting it to int
		i++;
	}
	fclose(file_pointer);												// closes the file
}


// subscriber details comparison and match with DB
int check_database(struct subscriber_db subscriber_db[],unsigned int sub_no,uint8_t technology) {

	int result = -1;		// stores result status, -1 implies not found

	// comparing subscriber number and technology using for loop

	for(int j = 0; j < LENGTH;j++) {
		if(subscriber_db[j].sub_no == sub_no && subscriber_db[j].technology == technology) {
			return subscriber_db[j].status;
		}
	}
	return result;
}


int main(int argc, char**argv){
	
    struct request_packet request_packet;			// stores request packet
	struct response_packet response_packet;		// stores response packet
	
    struct subscriber_db subscriber_db[LENGTH];  // subscriber_db array to store subscriber data

	//Arguments are ./server - where server program's compiler output is stored and port number (server will LISTEN)

	if (argc != 2)
    {
        fprintf(stderr, "please give command - ./server port_number in order deploy server\n");
        exit(1);
    }

	read_file(subscriber_db);											
    int sock_fd,b;																// socket file descriptor and bytes number received
	struct sockaddr_in server_address;											// server address information
	struct sockaddr_storage server_storage;										// server storage information
	socklen_t address_size;														// stores the server address structure size
	sock_fd=socket(AF_INET,SOCK_DGRAM,0);										// UDP socket creation
    bzero(&server_address,sizeof(server_address));								// all the bytes in server_address are set to zero
	server_address.sin_family = AF_INET;										// Address family is set to IPV4
	server_address.sin_addr.s_addr=htonl(INADDR_ANY);							// setting server to listen from all network interfaces
	server_address.sin_port=htons(atoi(argv[1]));								// port number to big-endian network byte order conversion
    bind(sock_fd,(struct sockaddr *)&server_address,sizeof(server_address));	// socket binding to port number as mentioned by user 
	address_size = sizeof server_address;										// stores server_address size to use in recvfrom() method
	printf("\nServer deployment is completed and no errors to report\n");			
	
	
    for (;;) {                  
		// infinite loop for receiving and processing incoming packets
        // receive the packet over network and store it into request_packet
		b = recvfrom(sock_fd,&request_packet,sizeof(struct request_packet),0,(struct sockaddr *)&server_storage, &address_size);
		printf(" \n\n ------------Start of a New Packet----------- \n\n");
		print_packet(request_packet);

		// program exits when there are more than 10 segments
		if(request_packet.segment_No == 11) {
			exit(0);
		}

		// checking if the Subscriber has valid access
		if(b > 0 && request_packet.access_per == 0XFFF8) {

			// creating response packet
			response_packet = createresponse_packet(request_packet);

			// checking and comparing Subsciber data and DB and returning and printing result 
			int result = check_database(subscriber_db,request_packet.Source_subscriber_No,request_packet.technology);
			if(result == 0) {
				
				response_packet.response_code = NOT_PAID;
				printf("\n.............................\n");
				printf("Subscriber has not paid\n");
				printf(".............................\n");
			}
			else if(result == 1) {
				printf("\n......................................\n");
				printf("Subscriber is permitted to access network\n");
				printf("......................................\n");
				response_packet.response_code = ACCESS_OK;
			}

			else if(result == -1) {
                printf("\n.........................\n");
				printf("No such Subscriber exists\n");
				printf(".........................\n");
				response_packet.response_code = NOT_EXIST;
			}
                                              
			// sends response packet to client
			sendto(sock_fd,&response_packet,sizeof(struct response_packet),0,(struct sockaddr *)&server_storage,address_size);
		}
		b = 0; // Initializing to zero to recvfrom() from next packet
		
	}
}