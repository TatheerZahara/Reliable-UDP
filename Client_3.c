#include <stdio.h>
#include <stdlib.h>

// Time function, sockets, htons... file stat
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <math.h>
#include <sys/stat.h>

// File function and bzero
#include <fcntl.h>
#include <unistd.h>
#include <strings.h>

/* Size of the buffer used to send the file
 * in several blocks
 */
#define BUFFERT 471

/* Command to generate a test file
 * dd if=/dev/random of=file count=8
 */

/* Declaration of functions*/
int duration (struct timeval *start,struct timeval *stop, struct timeval *delta);
int create_client_socket (int port, char* ipaddr);
struct sockaddr_in sock_serv;
struct timeval timeout;
struct packets{
int sq_no;
char data[BUFFERT];
};
int main (int argc, char**argv){
	
    struct timeval start, stop, delta,timeout;
    int sfd,fd;
   
    long int size;
    int no_packets;
    int total_acks;
    struct packets packet_s0,packet_s1,packet_s2,packet_s3,packet_s4;
    char buf[BUFFERT];
    off_t count=0, m, sz;
    
    long int n0,n1,n2,n3,n4;
   
    int l=sizeof(struct sockaddr_in);
	struct stat buffer;
    
	if (argc != 4){
		printf("Error usage : %s <ip_serv> <port_serv> <filename>\n",argv[0]);
		return EXIT_FAILURE;
	}
    //creating the socket
    sfd=create_client_socket(atoi(argv[2]), argv[1]);
    
	if ((fd = open(argv[3],O_RDONLY))==-1){
		perror("open fail");
		return EXIT_FAILURE;
	}
    
	//file size
	if (stat(argv[3],&buffer)==-1){
		perror("stat fail");
		return EXIT_FAILURE;
	}
	else
		sz=buffer.st_size;
    
	//preparation of the send
	//determining the number of packets	
	no_packets=ceil((float)sz/471);
	//declare array of the same size 
	struct packets packets_array[no_packets];
	//initializing ack array	
	int acks_recv[no_packets];
	for(int a = 0; a < no_packets; a = a+1 ){
      		acks_recv[a]=-1;
   	}

    gettimeofday(&start,NULL);
    //adding packets into the array
    for(int var=0;var<no_packets;var++){
    n0=read(fd,packets_array[var].data,BUFFERT);}
    //setting sq numbers
    for(int var_1=0;var_1<no_packets;var_1++){
    	packets_array[var_1].sq_no=var_1;
	}

    total_acks = 5;
  
	
    
    
    int pack_window=0;
    int ack_window=0;
    int var_x=0;
    int count_1=0;

//the sending of packets    
    while(pack_window!=no_packets){
	total_acks=0;
	for(int u=0;u<5;u++){
    	if(sendto(sfd,&packets_array[pack_window],sizeof(packets_array[pack_window]),0,(struct sockaddr*)&sock_serv,l)==-1){
			perror("send error");
			return EXIT_FAILURE;}
		pack_window++;}
	while(1){
		recvfrom(sfd,&acks_recv[ack_window],4,0,(struct sockaddr*)&sock_serv,&l);
		if(acks_recv[ack_window]==ack_window){
			printf("Ack Recieved %d\n",acks_recv[ack_window]);
			total_acks++;
			ack_window++;}
		
		else{if(sendto(sfd,&packets_array[ack_window],sizeof(packets_array[ack_window]),0,(struct sockaddr*)&sock_serv,l)==-1){
			perror("send error1");
			return EXIT_FAILURE;}}
		if(total_acks==5){
			break;}

	}}
		

	
	gettimeofday(&stop,NULL);
	duration(&start,&stop,&delta);
    
	//printf("Number of bytes transferred: %lld\n",count);
	printf("On a total size of: %lld \n",sz);
	printf("For a total duration of: %ld.%d \n",delta.tv_sec,delta.tv_usec);
    
    close(sfd);
    close(fd);
    	return EXIT_SUCCESS;}

/*Function allowing the calculation of the duration of the sending*/
int duration (struct timeval *start,struct timeval *stop,struct timeval *delta)
{
    suseconds_t microstart, microstop, microdelta;
    
    microstart = (suseconds_t) (100000*(start->tv_sec))+ start->tv_usec;
    microstop = (suseconds_t) (100000*(stop->tv_sec))+ stop->tv_usec;
    microdelta = microstop - microstart;
    
    delta->tv_usec = microdelta%100000;
    delta->tv_sec = (time_t)(microdelta/100000);
    
    if((*delta).tv_sec < 0 || (*delta).tv_usec < 0)
        return -1;
    else
        return 0;
}

/* Function allowing the creation of a socket
 * Returns a file descriptor
 */
int create_client_socket (int port, char* ipaddr){
    int l;
	int sfd;
    
	sfd = socket(AF_INET,SOCK_DGRAM,0);
	if (sfd == -1){
        perror("socket fail");
        return EXIT_FAILURE;
	}
    
    //preparation of the address of the destination socket
	l=sizeof(struct sockaddr_in);
	bzero(&sock_serv,l);
	
	sock_serv.sin_family=AF_INET;
	sock_serv.sin_port=htons(port);
	timeout.tv_sec = 5;
    	timeout.tv_usec = 0;
    if (inet_pton(AF_INET,ipaddr,&sock_serv.sin_addr)==0){
		printf("Invalid IP adress\n");
		return EXIT_FAILURE;
	}
	
	if (setsockopt(sfd, SOL_SOCKET, SO_RCVTIMEO,&timeout,sizeof(timeout)) < 0) {
    		perror("Error");
	}
    
    return sfd;
}
