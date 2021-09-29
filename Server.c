
#include <stdio.h>
#include <stdlib.h>

// Time function, sockets, htons... file stat
#include <sys/time.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/uio.h>
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
 * dd if = / dev / random of = file count = 8
 */

/* Declaration of functions*/
int duration (struct timeval *start,struct timeval *stop, struct timeval *delta);
int create_server_socket (int port);

struct sockaddr_in sock_serv,clt;

struct packets{
int sq_no;
char data[BUFFERT];
};
int main (int argc, char**argv){
    //creating variables which will recieve the packets
	int fd, sfd;
 	struct packets * packet_recv0 = malloc(sizeof(struct packets));
   	struct packets * packet_recv1 = malloc(sizeof(struct packets));
	struct packets * packet_recv2 = malloc(sizeof(struct packets));
	struct packets * packet_recv3 = malloc(sizeof(struct packets));
	struct packets * packet_recv4 = malloc(sizeof(struct packets));
	struct packets * packet_seq[1000];
	off_t count=0, n0,n1,n2,n3,n4; // long type
	char filename[256];
	int ack_send0,ack_send1,ack_send2,ack_send3,ack_send4=-1;
        unsigned int l=sizeof(struct sockaddr_in);		
	int frame_id=0;
	
    // Variable for the date
	time_t intps;
	struct tm* tmi;
    
	if (argc != 2){
		printf("Error usage : %s <port_serv>\n",argv[0]);
		return EXIT_FAILURE;
	}
    //creating the socket
    sfd = create_server_socket(atoi(argv[1]));
    
	intps = time(NULL);
	tmi = localtime(&intps);
	bzero(filename,256);
	sprintf(filename,"clt.%d.%d.%d.%d.%d.%d",tmi->tm_mday,tmi->tm_mon+1,1900+tmi->tm_year,tmi->tm_hour,tmi->tm_min,tmi->tm_sec);
	printf("Creating the output file : %s\n",filename);
    
	//open file
	if((fd=open(filename,O_CREAT|O_WRONLY|O_TRUNC,0600))==-1){
		perror("open fail");
		return EXIT_FAILURE;
	};
    
	//preparation of the send
	
n0=1;  
int write_var=0;
	while(n0){
		n0=recvfrom(sfd,packet_recv0,sizeof(*packet_recv0),0,(struct sockaddr *)&clt,&l);
		ack_send0=packet_recv0->sq_no;
		packet_seq[packet_recv0->sq_no]=packet_recv0;
		sendto(sfd,&ack_send0,4,0,(struct sockaddr*)&clt,l);
		printf("Packet Recieved %d\n",packet_recv0->sq_no);
		if(n0==-1){
			perror("read fails");
			return EXIT_FAILURE;
		};
		n1=recvfrom(sfd,packet_recv1,sizeof(*packet_recv1),0,(struct sockaddr *)&clt,&l);
		ack_send1=packet_recv1->sq_no;
		packet_seq[packet_recv1->sq_no]=packet_recv1;
		sendto(sfd,&ack_send1,4,0,(struct sockaddr*)&clt,l);
		printf("Packet Recieved %d\n",ack_send1);
		if(n1==-1){
			perror("read fails");
			return EXIT_FAILURE;
		};

		n2=recvfrom(sfd,packet_recv2,sizeof(*packet_recv2),0,(struct sockaddr *)&clt,&l);
		ack_send2=packet_recv2->sq_no;
		packet_seq[packet_recv2->sq_no]=packet_recv2;
		sendto(sfd,&ack_send2,4,0,(struct sockaddr*)&clt,l);
		printf("Packet Recieved %d\n",ack_send2);
		if(n2==-1){
			perror("read fails");
			return EXIT_FAILURE;
		};
		n3=recvfrom(sfd,packet_recv3,sizeof(*packet_recv3),0,(struct sockaddr *)&clt,&l);
		ack_send3=packet_recv3->sq_no;
		sendto(sfd,&ack_send3,4,0,(struct sockaddr*)&clt,l);
		packet_seq[packet_recv3->sq_no]=packet_recv3;
		printf("Packet Recieved %d\n",ack_send3);
		if(n3==-1){
			perror("read fails");
			return EXIT_FAILURE;
		};
		n4=recvfrom(sfd,packet_recv4,sizeof(*packet_recv4),0,(struct sockaddr *)&clt,&l);
		ack_send4=packet_recv4->sq_no;
		packet_seq[packet_recv4->sq_no]=packet_recv4;
		sendto(sfd,&ack_send4,4,0,(struct sockaddr*)&clt,l);
		printf("Packet Recieved %d\n",ack_send4);
		if(n4==-1){
			perror("read fails");
			return EXIT_FAILURE;
		};
		for(int x=0;x<5;x++){
			write(fd,packet_seq[write_var]->data,BUFFERT);
			write_var++;}
		
		};
		count=count+n0+n1+n2+n3+n4;
	
    
	printf("Number of bytes transferred: %lld \n",count);
    
    close(sfd);
    close(fd);
	return EXIT_SUCCESS;
}

/* Function allowing the calculation of the duration of the sending */
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

/* Function allowing the creation of a socket and its attachment to the system
 * Returns a file descriptor in the process descriptor table
 * bind allows its definition in the system
 */
int create_server_socket (int port){
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
	sock_serv.sin_addr.s_addr=htonl(INADDR_ANY);
    
	//Assign an identity to the socket
	if(bind(sfd,(struct sockaddr*)&sock_serv,l)==-1){
		perror("bind fail");
		return EXIT_FAILURE;
	}
    
    
    return sfd;
}

