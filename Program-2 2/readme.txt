Program 2: Client using customized protocol on top of UDP protocol for requesting identification from server for access permission to the cellular network(one client connects to one server).

Steps to compile and execute the program:

i. Open terminal and change directory using command cd to the folder in which the programs are.
ii.Compile server program by using command gcc -o server_2 server_2.c
iii.Compile client program by using command gcc -o client_2 client_2.c
iv. First, run server program by using command ./server_2 port_number (./server_2 8080)
v. Then run client program by using command ./client_2 localhost port_number ((./client_2 localhost 8080))


Notes:

i. Make sure visual studio and gcc compiler are installed in your system.
ii. Be very precise to start with server program.
iii. Server program expects 2 command-line arguments - executable file name and port number.
iv.Client program expects 3 command-line arguments - executable file name, host name and port number.
v.Since I will be demonstrating on my system, both client and server will be on the same system, so hostname will be localhost.
vi.Run both the programs on same port number else there will be no communication established and there will be no response.
vii. To avoid compilation errors, data packet headers and trailers are defined in both client and server as they will be used in both files.
viii. Keep both the input files in same directory and do not change name or extension as segmentation error will arise.
ix. Enter different port numbers in client and server programs to test ack_timer.
x. Do not update name of log file. Using the log file to store Response packet from server as good practice instead of printing on terminal.
