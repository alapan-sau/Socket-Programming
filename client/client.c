#include<stdio.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<string.h>
#include<fcntl.h>
#include<errno.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<arpa/inet.h>
#define PORT 8000
#define N 100000
#define BUFFER_LIMIT 10000

// prints the status dynamically
void printStatus(int j, int size){
    double percentage;
    if(size==0){
        percentage = 100.00;
    }
    else{
        percentage = ((double)(size-j)/(double)size) * 100;
    }
    char buffer[100];
    sprintf(buffer,"\rDownloaded %0.2f%%",percentage);
    write(1,buffer,strlen(buffer));
    fflush(stdout);
}

// reads an integer from socket
int readint(int sock){
    int n;
    read(sock, (char *)&n, sizeof(n));
    n = ntohl(n);
    return n;
}

// reads a string from socket
void readstr(int sock, char* buffer){
    int len = readint(sock);
    read(sock, buffer, len);
    buffer[len]='\0';
    return;
}

// sends an integer to socket
void sendint(int sock, int n){
    n = htonl(n);
    send(sock, (char *)&n, sizeof(n), 0);
}

// sends a string to socket
void sendstr(int sock, char* buffer){
    sendint(sock,strlen(buffer));
    send(sock,buffer,strlen(buffer),0);
}



void  downloadFile(int sock, char filename[]){

    int wd = open(filename, O_WRONLY | O_CREAT, 0600);
    int size = readint(sock);

    int j=size;
    printf("Downloading %d bytes\n",size);
    char data[N];

    // Default socket buffer limit is 43689 for LINUX LOW-MEM SYSTEMS!!
    // Using 10,000 to overcome the issue!!
    // If still throws bus error/ seg fault, reduce it!!
    while(j>=BUFFER_LIMIT){

        // send the server a ack to send next!
        sendint(sock,1);

        // read from socket after ack
        // readstr(sock, data);
        read(sock,data, BUFFER_LIMIT);  // check

        // write to the file
        write(wd,data,BUFFER_LIMIT);
        j-=BUFFER_LIMIT;

        printStatus(j,size);
    }

    // send the server a ack to send next!
    sendint(sock,1);

    // read from socket after ack
    // readstr(sock, data);
    read(sock,data, j); // check

    // write to the file
    write(wd,data,j);
    j=0;

    printStatus(j,size);
    printf("\n"); // get rid of the status shit
    if (close(wd) < 0)
    {
        perror("Error ");
        return;
    }
}


int main(int argc, char const *argv[])
{
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[N] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr)); // to make sure the struct is empty. Essentiainty sets sin_zero as 0
                                                // which is meant to be, and rest is defined below

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Converts an IP address in numbers-and-dots notation into either a
    // struct in_addr or a struct in6_addr depending on whether you specify AF_INET or AF_INET6.
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)  // connect to the server address
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    sendint(sock, argc-1);

    // error on no arguments!
    if(argc==1) printf("No files to download!\n");

    for(int i=0;i<argc-1;i++){
        char data[N];
        // donot fuck up with pointers!!
        strcpy(buffer,argv[i+1]);

        // senf the file name to be taken;
        sendstr(sock, buffer);

        // check if the code is success / error
        printf("\n%s\n",buffer);
        if(readint(sock)){
            printf("Success!\n");
            downloadFile(sock,buffer);
        }
        else{
            printf("Error: File not Available!\n");
        }
    }
}
