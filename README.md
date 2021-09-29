# Reliable UDP

Classic TCPs ensure reliablity when it comes to successfully sending packets. However, when it comes to UDP, we barely know if our packets were successfully sent or not. This project is just an attempt to make classic UDPs slightly more secure. 

Following are some guidelines that our project follow: 

-A sender and a receiver is implemented for video file transfer over UDP protocol using Linux/GNU C sockets. 
-Sender is required to open a video file, read data chunks from file, and write UDP segments, send these segments on UDP. Receiver must be able to receive, reorder and write
data to a file at the receiving end. 

-->Sender's side: 

The sender will read the file specified by the filename and transfer it using UDP sockets. On completing the transfer, the sender should terminate and exit. The sender should bind to listenport to receive acknowledgments and other signaling from the receiver. A single UDP socket is used for both
sending and receiving data. Packets are restricted to 500 bytes (in payload). 

-->Receiver's side:

The receiver will need to bind to the UDP port specified on the command line and receive a file from the sender sent over the port. 

--> Reliable UDP: 

We will add the following to make UDP better

1-Sequence numbers
2-Retransmission (selective repeat)
3-Window size of 5-10 UDP segments (stop n wait)
4-Re-ordering on receiver's side