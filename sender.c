
//including libraries to employ their in-built functions and services
#include <stdio.h> //standard input/output
#include <stdlib.h> //standard library
#include <stdbool.h> //macros for boolean type data
#include <sys/time.h> //time function for socket creation
#include <stdbool.h> //macros for boolean type data
#include <stdbool.h> //macros for boolean type data
#include <sys/uio.h> //defines the iovec structure
#include <sys/stat.h> //structures for fetching information regarding files
#include <fcntl.h> //structures and functions for file control
#include <unistd.h> //access to POSIX operating system API
#include <strings.h> //for using strings and relevant non-standard functions
#include <string.h> //for using standard strings and relevant functions
#include <arpa/inet.h> //makes available in_port_t and in_addr_t
#include <sys/types.h> //basic derived types

//defining macros for sizes
#define segments 5
#define buffer 496

//driver function
int main (int argc, char**argv){

//variable declarations
int i, j, k, l; //counters to be used in loops
int structureSize; //size of the structure holding IPv4 address
int fileDescriptor; //information regarding the file to be sent
int socketFileDescriptor; //information regarding the socket
int ipConversion; //return value from inet_pton()
int fileInfo; //return value of stat()
int totalPackets; //the number of total packets transferred
int sequence = 0; //initially the sequence number will be zero
int receivedAck[segments]; //array for received acknowledgements
int ackNumber[segments]; //array for acknowledgement for all segments
int expectedAck[segments]; //array for expected acknowledgements
int duplicate; //duplicate Acks
int latestAck; //the last received acknowledgement
int totalAcks; //to keep count of all the acknowledgements received
char file[256]; //array for the name of the file to be sent
long bytesReceived = 0; //64-bit bytesReceived for number of packets
long fileSize; //size of the file  information structure
long message[segments]; //array for messages in each segment
long messageAck; //acknowledgement for each
bool flag = false; //flag to check if all acknowledgements have been received
long int packetNum[segments]; //array for the packets per segment

//structure for packet
struct packet
{
int sequenceNo; //sequence number of a packet
char store[buffer]; //content to be held in a packet
};

//structures for server and client socket  
struct sockaddr_in server;
structureSize = sizeof(struct sockaddr_in);

//structure to store information of mp4 file
struct stat information;

//sender window size of 5
struct packet packet[segments];

//proceed only if host address, port number, and file name are given
if (argc != 4)
{
printf("Invalid Arguments!\nUse this format: ./programName hostAddress portNumber fileName\n");
return -1;
}
   
//socket creation begins here
 
//socket() arguments: address family, type of connection, protocol
socketFileDescriptor = socket(AF_INET, SOCK_DGRAM, 0);

//if socket connection fails
if (socketFileDescriptor == -1)
{
perror("Error! Socket connection failed.\n");
return -1;
}

//setting all socket structures to null value  
bzero(&server, structureSize);

//htons is used for host byte order to network byte order conversion
server.sin_port = htons(atoi(argv[2])); //assigning port number
//converting IP address to numeric binary form
ipConversion = inet_pton(AF_INET, argv[1], &server.sin_addr);
server.sin_family = AF_INET; //assigning the Ip address family

//if IP address is invalid
if (ipConversion == 0)
{
printf("Error! Your given IP Address is not valid.\n");
return -1;
}

//structure to set maximum time for socket connection
struct timeval tv;
tv.tv_sec = 1;
tv.tv_usec = 0;

//socket options manipulation
setsockopt(socketFileDescriptor, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
   
//opening SentFile.mp4 in read only mode
//storing the return value in file descriptor
fileDescriptor = open(argv[3], O_RDONLY);

//if file does not open in read only mode
if (fileDescriptor == -1)
{
perror("Error! File opening failed.\n");
return -1;
}
   
//checking file's information
fileInfo = stat(argv[3], &information);

//if stat() function does not run succesfully
if (fileInfo == -1)
{
perror("Error! Stat() function failed.\n");
return -1;
}

else
//file size of the information stat
fileSize = information.st_size;

//determining the number of packets to be sent
//(total size of the video file / size of each packet) + 1 for remainder data
totalPackets = (fileSize / buffer) + 1;

//loop runs till all acknowledgements are received
while(!flag)
{

//setting the messageAck and duplicate to zero at the beginning
messageAck = 0;
duplicate = 0;

//sending data to the receiver
message[0] = sendto(socketFileDescriptor, &totalPackets, segments-1, 0, (struct sockaddr*)&server, structureSize);

//getting acknowledgement for the message from the receiver
messageAck = recvfrom(socketFileDescriptor, &totalAcks, segments-1, 0, (struct sockaddr *)&server, &structureSize);

//when total acknowledgements received are equal to the total packets sent
if(totalAcks == totalPackets)
//set flag to 1
flag = true;
}

//reading the content of the first packet
packetNum[0] = read(fileDescriptor, packet[0].store, buffer);

while(packetNum[0]) //loop runs till the end of the packet
{

//if unable to read
if(packetNum[0] == -1)
{
perror("Error! Reading failed.\n");
return -1;
}

//the last received acknowledgement is zero at the start
latestAck = 0;

//setting all received acknowledgements to zero initially for each segment
for (i=0; i<segments; i++)
receivedAck[i] = 0;

//for all the segments
for (i=0; i<segments; i++)
{
//the expected acknowledgment and the current packet's sequence number are the sequence number at present
packet[i].sequenceNo = sequence;
expectedAck[i] = sequence;

//the acknowledgement number for the segment is set to -1
ackNumber[i] = -1;

//sequence number is incremented for next ACK
sequence = sequence + 1;
}

//for the remainder of the segments
for (i=1; i<segments; i++)
{
//reading the content of the packet into PacketNum[]
packetNum[i] = read(fileDescriptor, packet[i].store, buffer);
//clearing the message for that packet
message[i] = 0;
}

//sending the first packet
message[0] = sendto(socketFileDescriptor, (struct packet*)&packet[0], (packetNum[0]+segments-1), 0, (struct sockaddr*)&server, structureSize);

for(i=1; i<segments; i++)
{ 
//for every non-empty packet
if(packetNum[i] != 0)
{
//message is sent
message[i] = sendto(socketFileDescriptor, (struct packet*)&packet[i], (packetNum[i]+segments-1), 0, (struct sockaddr*)&server, structureSize);
}

else
{
//latestAck is incremented to indicate another byte has been received
latestAck = latestAck + 1;
//setting receivedAck to one to cater to duplicates later on
receivedAck[i] = 1;
}

} //loop ends

//if empty message received
if(message[0] == -1)
{
perror("Error! Failed to read the contents of the packet.\n");
return -1;
} 

for(i=0; i<segments; i++)
{
//receiving acknowledgement from the receiver
messageAck = recvfrom(socketFileDescriptor,&ackNumber[i],segments-1,0,(struct sockaddr *)&server,&structureSize);
//if the received acknowledgment number is equal to the expected ack of any of the 5 segments
if(ackNumber[i] == expectedAck[0] || ackNumber[i]==expectedAck[1] || ackNumber[i]==expectedAck[2] || ackNumber[i]==expectedAck[3] || ackNumber[i]==expectedAck[4])
{
//the remainder's counter is found
receivedAck[ackNumber[0]%segments] = 1; 

printf("Received Ack Number: %d \n", ackNumber[i]);

//increment the acknowledgement
latestAck = latestAck + 1;
//clearing the messagingAck variable
messageAck = 0;
}

//if the current acknowledgement number doesn't match any of the expected acknolwedgments
//or if the messageAck is not -1
else if((ackNumber[i] != expectedAck[0] || ackNumber[i]!=expectedAck[1] || ackNumber[i]!=expectedAck[2] || ackNumber[i]!=expectedAck[3] || ackNumber[i]!=expectedAck[4]) && messageAck!=-1)
{
//increment the count for duplicate acks
duplicate = duplicate + 1;
printf("Duplicate Ack for packet: %d\n", ackNumber[i]);
}

}

//for duplicate acks, receiving the acks
for( i=0;i<duplicate;i++)
{
//receiving the acks
messageAck=recvfrom(socketFileDescriptor,&ackNumber[1],segments-1,0,(struct sockaddr *)&server,&structureSize);

//if match found in first segment
if(ackNumber[1] == expectedAck[0] || ackNumber[1]==expectedAck[1] || ackNumber[1]==expectedAck[2] || ackNumber[1]==expectedAck[3] || ackNumber[1]==expectedAck[4])
{
//incrementing the last received ACK
latestAck = latestAck + 1;
//the receivedAck of the remainder is updated to 1 
receivedAck[ackNumber[1]%segments] = 1;

printf("Received Ack Number %d.\n", ackNumber[1]);
}

//if match not found in last segment
else if((ackNumber[4] != expectedAck[0] || ackNumber[4]!=expectedAck[1] || ackNumber[4]!=expectedAck[2] || ackNumber[4]!=expectedAck[3] || ackNumber[4]!=expectedAck[4]) && messageAck!=-1)
{
printf("Received duplicated Ack: %d\n", ackNumber[4]);
//incrementing the count of duplicate
duplicate = duplicate + 1;
}
}

//implementation of selective repeat
//resending lost packets
//for all the segments
while(latestAck != segments)
{

for(i=0; i<segments; i++)
{
//if acknowledgement has not been received
if(receivedAck[i]==0)
{
printf("Resending lost packet no. %d\n", expectedAck[i]);

//sending the packet to the receiver 
message[0] = sendto(socketFileDescriptor,(struct packet*)&packet[i],(packetNum[0]+segments-1),0,(struct sockaddr*)&server,structureSize);

}

else
{
//packet received successfully so changing the check back to -1
expectedAck[i] = -1; 
}
}

//handling acknowledgements of resent packets
for(int counter=latestAck; counter<segments; counter+=1)
{
//receiving the ack again from the receiver
messageAck=recvfrom(socketFileDescriptor,&ackNumber[0],segments-1,0,(struct sockaddr *)&server,&structureSize);
//if acknowledgement number of first segment matches any
if(ackNumber[0] == expectedAck[0] || ackNumber[0]==expectedAck[1] || ackNumber[0]==expectedAck[2] || ackNumber[0]==expectedAck[3] || ackNumber[0]==expectedAck[4])
{
receivedAck[ackNumber[0]%segments]=1;
//incrementing the last received Ack
latestAck = latestAck + 1;
printf("Received Ack Number %d.\n", ackNumber[0]);

for (i = 0; i < 5; i++)
{
//if the acknowledgement number of 1 segment matches
if(ackNumber[0] == expectedAck[i])
//set the flag back to -1
expectedAck[i] = -1;
}

} //if ends
} //for ends
} //while ends till all acks received

//bytesReceived the total size sent
long total = 0;
for (i=0; i < 5; i++)
total = total + (message[i]);

//determining the total bytes received
bytesReceived = bytesReceived + total;

//clearing the packet from memory
memset(packet[0].store, 0,buffer);

//reading the packet again
packetNum[0]=read(fileDescriptor,packet[0].store,buffer);

}

printf("\n---END OF FILE---\n\n");
       
//notify sender that all packets have been received
//indicated by the 0 in the fourth argument
message[0] = sendto(socketFileDescriptor,(struct packet*)&packet[0],0,0,(struct sockaddr*)&server,structureSize);

printf("\nSize of file: %ld\n", fileSize);
printf("\nTotal bytes transferred: %ld\n", bytesReceived);
   
//closing the file
close(fileDescriptor);
//closing the socket
close(socketFileDescriptor);

return 0;
}