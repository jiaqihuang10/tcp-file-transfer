# TCP File transfer

A client/server program for file transfer/processing based on Stream (TCP) sockets. The server program which is called tfs.c (TCP File Server) listens on a given TCP IP address/Port number at host-b. The input arguments of this program are an IP address and Port number.

On the other side (host-a), the client program is called tfc.c (TCP File Client) and has 3 input arguments including the IP address and Port number of the server side as well as the Name of a zipped file. The client program sends the given file to the TCP file server and the server program receives and stores the file, first. Then, it unzips the received file and start sending back the received unzipped file to the TCP client program.
