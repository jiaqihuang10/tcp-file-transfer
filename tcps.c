  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <unistd.h>

  #define BUFF_SIZE 1024

  int main(int args, char* argv[])
  {
    char* filename;
    char* serv_ip_addr;
    char* serv_port_no;
    int connectFD;

    filename = (char*)malloc(sizeof(char)*20);
    serv_ip_addr = (char*)malloc(sizeof(char)*20);
    serv_port_no = (char*)malloc(sizeof(char)*7);

    if (args == 3) {
        serv_ip_addr = argv[1];
        serv_port_no = argv[2];
    } else {
        perror("In the form: ./tcpc [server ip address] [server port]\n");
        exit(EXIT_FAILURE);
    }
    char buffer[BUFF_SIZE];
    int bytesReceived,bytesRead;
    memset(&buffer, 0, BUFF_SIZE);

    struct sockaddr_in sa;
    int socketFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socketFD == -1) {
      perror("cannot create socket");
      exit(EXIT_FAILURE);
    }

    memset(&sa, 0, sizeof sa);

    sa.sin_family = AF_INET;
    sa.sin_port = htons(9090);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(socketFD,(struct sockaddr *)&sa, sizeof sa) == -1) {
      perror("bind failed");
      close(socketFD);
      exit(EXIT_FAILURE);
    }

    if (listen(socketFD, 10) == -1) {
      perror("listen failed");
      close(socketFD);
      exit(EXIT_FAILURE);
    }

    printf("Listening.\n");

    while(1) {
      connectFD = accept(socketFD, (struct sockaddr*)NULL, NULL);
      /*
      if (connectFD) {
        perror("accept failed");
        close(socketFD);
        exit(EXIT_FAILURE);
      } */

      int fsize = 0;

      read(connectFD, &fsize, sizeof fsize);
    //  int filesize = ntohl(fsize);
      printf("filesize: %d\n", fsize );

      //open a file for writing
      FILE *fp;
      fp = fopen("received.zip", "wr");

      int dataLeft = fsize;
      while(dataLeft > 0) {
          bytesReceived = recv(connectFD, buffer, BUFF_SIZE,0);
          dataLeft -= bytesReceived;
          if (bytesReceived > 0)
          {
              //printf("%d bytes received, %d left.\n", bytesReceived, dataLeft);
              if(fwrite(buffer,1,bytesReceived,fp)<0)
              {
                  perror("Error writting the file\n");
                  exit(EXIT_FAILURE);
              }
          }
      }

      fclose(fp);

      system("unzip received.zip");

      fp = fopen("sample_file.txt","r");

      /*Determine the requested file size*/
      int unzipped_filesize;
      fseek (fp, 0L, SEEK_END);
      unzipped_filesize = ftell(fp);
      fseek (fp, 0L, SEEK_SET);

      int unzippedLeft = unzipped_filesize;
      //send the filesize to client
      write(connectFD, &unzipped_filesize, sizeof unzipped_filesize);

      while(unzippedLeft > 0) {
        bytesRead = fread(buffer,1,BUFF_SIZE,fp);
        unzippedLeft -= bytesRead;
        if (bytesRead > 0)
        {
            write(connectFD,buffer,bytesRead);
        }

        if (feof(fp)) {
            printf("End of file sent\n");
            break;
        }
      }

      fclose(fp);
      if (shutdown(connectFD, SHUT_RDWR) == -1) {
        perror("shutdown failed");
        close(connectFD);
        close(socketFD);
        exit(EXIT_FAILURE);
      }
      close(connectFD);
    }

    close(socketFD);
    return EXIT_SUCCESS;
}
