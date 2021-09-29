# Reliable UDP

Classic TCPs ensure reliablity when it comes to successfully sending packets. However, when it comes to UDP, we barely know if our packets were successfully sent or not. This project is just an attempt to make classic UDPs slightly more secure. 

Following are some guidelines that our project follow: 

-A sender and a receiver is implemented for video file transfer over UDP protocol using Linux/GNU C sockets. 
-Sender is required to open a video file, read data chunks from file, and write UDP segments, send these segments on UDP. Receiver must be able to receive, reorder and write
data to a file at the receiving end. 

-->Sender's side: 
 
