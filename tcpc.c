#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/stat.h>

#define BUFF_SIZE 1024

int main(int args, char* argv[])
{
    char* filename;
    char* serv_ip_addr;
    char* serv_port_no;
    filename = (char*)malloc(sizeof(char)*20);
    serv_ip_addr = (char*)malloc(sizeof(char)*20);
    serv_port_no = (char*)malloc(sizeof(char)*7);
    if (args == 4) {
        serv_ip_addr = argv[1];
        serv_port_no = argv[2];
        filename = argv[3];

    } else {
        perror("In the form: ./tcpc [server ip address] [server port] [filename]\n");
        exit(EXIT_FAILURE);
    }

    int bytesReceived = 0;
    int bytesRead = 0;
    struct stat file_stat;
    int filesize;

    char buffer[BUFF_SIZE];
    //memset(recvBuff, '0', sizeof(recvBuff));
    struct sockaddr_in sa;
    socklen_t length;

    /* Create a socket first */
    int socketFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socketFD == -1) {
      perror("cannot create socket");
      exit(EXIT_FAILURE);
    }

    /* Zero out socket address */
    memset(&sa, 0, sizeof sa);
    /* Initialize sockaddr_in data structure */
    sa.sin_family = AF_INET;
    sa.sin_port = htons(9090); // port
    sa.sin_addr.s_addr = inet_addr((serv_ip_addr));
    length = sizeof sa;


    if (connect(socketFD, (struct sockaddr *)&sa, length) == -1) {
      perror("connect failed");
      close(socketFD);
      exit(EXIT_FAILURE);
    }

    //open and read the required file
    FILE *fp = fopen("sample_file.zip","r");
    if(fp == NULL)
    {
        perror("Error opening the file");
        exit(EXIT_FAILURE);
    }

    /*Determine the requested file size*/
    fseek (fp, 0L, SEEK_END);
    filesize = ftell(fp);
    fseek (fp, 0L, SEEK_SET);


    printf("filesize: %d\n", filesize);

    struct timeval startTime, endTime;
    gettimeofday(&startTime, NULL);

    //send the file size to server
    int zipfile_dataLeft = filesize;
    write(socketFD, &filesize, sizeof filesize);

    while(zipfile_dataLeft > 0) {
        memset(buffer,0,sizeof (buffer));
        bytesRead = fread(buffer,1,BUFF_SIZE,fp);
        zipfile_dataLeft -= bytesRead;
        if (bytesRead > 0)
        {
            write(socketFD,buffer,bytesRead);
        }

        if (feof(fp)) {
            printf("End of file\n");
            break;
        }
    }

    fclose(fp);

    fp = fopen("received.txt","w");

    if(fp == NULL)
    {
        perror("Error opening the file: received.txt");
        exit(EXIT_FAILURE);
    }

    int unzipped_filesize;

    //receive the size of the unzippd file
    read(socketFD, &unzipped_filesize, sizeof unzipped_filesize);
    printf("unzipped file size: %d\n", unzipped_filesize);

    int unzipped_dataLeft = unzipped_filesize;

    printf("unzipped file data left size: %d\n", unzipped_dataLeft);
    
    while(unzipped_dataLeft > 0) {
      bytesReceived = read(socketFD,buffer, BUFF_SIZE);
      unzipped_dataLeft -= bytesReceived;
      if (bytesReceived > 0)
      {
          if(fwrite(buffer,1,bytesReceived,fp)<0)
          {
              perror("Error writting the file\n");
              exit(EXIT_FAILURE);
          }
      }
    }
    gettimeofday(&endTime,NULL);
    int transferTime = (endTime.tv_sec - startTime.tv_sec)*1000 + (endTime.tv_usec - startTime.tv_usec)/1000;
    printf("Total duration of file transfer: %d ms.\n", transferTime);

    fclose(fp);
    shutdown(socketFD, SHUT_RDWR);

    close(socketFD);
    return EXIT_SUCCESS;
}
