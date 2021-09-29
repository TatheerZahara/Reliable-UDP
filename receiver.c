
//including libraries to employ their in-built functions and services
#include <stdio.h> //standard input/output
#include <stdlib.h> //standard library
#include <sys/socket.h> //defines the type socklen_t of 32-bits
#include <sys/time.h> //time function for socket creation
#include <sys/uio.h> //defines the iovec structure
#include <sys/stat.h> //structures for fetching information regarding files
#include <fcntl.h> //structures and functions for file control
#include <unistd.h> //access to POSIX operating system API
#include <strings.h> //for using strings and relevant non-standard functions
#include <arpa/inet.h> //makes available in_port_t and in_addr_t
#include <sys/types.h> //basic derived types

//defining macros for sizes
#define segments 5
#define buffer 496

//driver function
int main (int argc, char *argv[])
{

//variable declarations
int i; //bytesReceived to be used in loops
int rwnd = 0; //receiver window initialised to zero at the beginning
int fileDescriptor, socketFileDescriptor; //descriptors for file and socket 
int structureSize; //size of the structure holding IPv4 address
int acknowledgement; //acknowledgement for packets
int totalPackets; //the number of total packets transferred
int packetSize[segments]; //packet size for each segment
int socketBinding; //return value of bind function
int duplicateFound; //to be used with duplicates[] array
long bytesReceived = 0; //64-bit bytesReceived for number of packets
long message; //length of incoming message from the sender
long ACK; //corresponding acknowledgment numbers
char file[256]; //array for the name of the file to be received

//structure for packet
struct packet
{
int sequenceNo; //sequence number of a packet
char store[buffer]; //content of a packet
};

//structures for server and client socket  
struct sockaddr_in server, client;
structureSize = sizeof(struct sockaddr_in);

//dynamically allocating memory to the packets
struct packet * starter = malloc(sizeof(struct packet)); //first packet
struct packet * packet_seq[segments];

//array for all the packets received by the receiver
struct packet allPackets[segments];
   
//condition to check if port number is provided
if (argc != 3)
{
printf("Invalid Arguments!\nUse this format: ./programName hostAddress portNumber\n");
return -1;
}

//socket creation begins here
//socket() requires IP address family, type TCP/ICP, and protocol
socketFileDescriptor = socket(AF_INET, SOCK_DGRAM, 0);

if (socketFileDescriptor == -1)
{
perror("Error! Socket creation failed.\n");
return -1;
}

//setting all server socket structures with null value
bzero(&server, structureSize);

//converting hostlong from host byte order to network byte order for ip address
server.sin_addr.s_addr = htonl(INADDR_ANY);
//converting hostshort from host byte order to network byte order for port no.
server.sin_port = htons(atoi(argv[2])); 
server.sin_family = AF_INET; //assigning socket family
   
//binding the socket to the given address
socketBinding = bind(socketFileDescriptor, (struct sockaddr*)&server, structureSize);

//if socket fails to bind to the server
if (socketBinding == -1)
{
perror("Error! Socket binding failed.\n");
return -1;
}

//socket creationg ends here
   
//file creation
sprintf(file, "ReceivedFile");
printf("Copying content sent by sender to %s.mp4\n", file);

//opening file in create, read/write, or truncate mode
fileDescriptor = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0600);

//if opening fails in all three of the mentioned modes
if (fileDescriptor == -1)
{
perror("Error! File opening failed.\n");
return -1;
}
   
//receiving the total number of packets from the sender
//from segment 0 to 4 (5 in all)
//recvfrom arguments: socket descriptor, buffer, buffer length, flags, socket address structure to record sender address, sender's address length
message = recvfrom(socketFileDescriptor, &totalPackets, segments-1, 0, (struct sockaddr*)&client, &structureSize);
   
printf("Receiving packet 0 to packet %d.\n", totalPackets);
//since packet numbering starts from 0, increment totalPackets to maintain the correct count
totalPackets = totalPackets + 1;

int duplicates[totalPackets]; //array to keep a check of duplicates
//at the start, no duplicates received, so all are set to 0
i = 0;
while ( i < totalPackets )
{   
duplicates[i] = 0;
i++;
}

//first byte of data received from the sender   
message = 1;
   
//receiving all data until end of file reached
while(message)
{
//length of message received from sender
message = recvfrom(socketFileDescriptor, starter, sizeof(*starter), 0, (struct sockaddr*)&client, &structureSize);

//first acknowledgement number is the sequence number of the received packet
acknowledgement = starter->sequenceNo;

//checking for a duplicate
duplicateFound = duplicates[acknowledgement];
//proceed if message length is 1 byte and 
if (message != 0 && duplicateFound == 0)
{
//ACK takes in the number of bytes sent
ACK = sendto(socketFileDescriptor, &acknowledgement, segments - 1, 0, (struct sockaddr*)&client, structureSize);

//moving to the next packet
allPackets[acknowledgement % segments] =* starter;

//acknowledgement % segment determined 0 - 4 segment number, and determines the size of the particular packet
packetSize[acknowledgement % segments] = message - segments - 1;

bytesReceived = bytesReceived + message; //incrementing the number of bytes received
duplicates[acknowledgement] = 1; //replacing 0 with 1 to indicate the particular sequence number has been received
rwnd = rwnd + 1; //moving the receiver window one byte ahead

printf("Received ACK number %d.\n", starter->sequenceNo);
}

//if duplicate ack has been received
else
{
//sending the ACK
ACK = sendto(socketFileDescriptor, &acknowledgement, segments - 1, 0, (struct sockaddr*)&client, structureSize);

printf("Received duplicate acknowledgement for packet %d.\n", acknowledgement);
}

//upon receiving all 5 segments
if(rwnd == segments)
{

i = 0;
while (i < segments)
{
//provided the packet is not empty
if(packetSize[i] !=0)
{
//write the content of the packet to ReceivedFile.mp4
write(fileDescriptor, allPackets[i].store, packetSize[i]);
}
i++;
} //entire file copied

//after succesfully writing the entire file

//set the receiver window size to 0 again
rwnd = 0;

i = 0;
//set the packet size for every packet to 0
while( i < segments )
{
packetSize[i] = 0;
i++;
} //all packet sizes set to 0
       
}
         
} //while ends - end of file

printf("\n---END OF FILE --\n\n");

printf("Total bytes received from the sender: %ld\n\n", bytesReceived);
   
//closing the file
close(fileDescriptor);
//closing the socket
close(socketFileDescriptor);

return 0;
}