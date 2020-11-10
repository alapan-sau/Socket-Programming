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


void uploadFile(int sock, char filename[]){
    char data[N];
    int rd = open(filename,O_RDONLY);
    if(rd == -1)
    {
        perror("Error");

        // sends a error code to download
        sendint(sock, 0);
        return;
    }
    else{
        struct stat st;
        if (fstat(rd, &st) < 0){
            perror("Error");

            // sends a error code to download
            sendint(sock, 0);
            return;
        }
        if(! S_ISREG(st.st_mode)){
            printf("Error: Not a regular file\n");
            
            // sends a error code to download
            sendint(sock, 0);
            return;
        }


        // sends a success code to download
        sendint(sock, 1);
    }

    int size = lseek(rd,0,SEEK_END);

    // sends the size of file
    sendint(sock, size);

    // remaining characters
    int j = size;

    lseek(rd,0,SEEK_SET);

    printf("Uploading %d bytes\n",j);

    // Default socket buffer limit is 43689 for LINUX LOW-MEM SYSTEMS!!
    // Using 10,000 to overcome the issue!!
    // If still throws bus error/ seg fault, reduce it!!
    while(j >= BUFFER_LIMIT){
        // read data from file
        read(rd,data,BUFFER_LIMIT);
        data[BUFFER_LIMIT]='\0';

        // send data only after ack from client
        if(!readint(sock)) return;
        // sendstr(sock, data);
        send(sock, data, BUFFER_LIMIT, 0);  // check
        j-=BUFFER_LIMIT;
    }
    // read data from file
    read(rd,data,j);
    data[j]='\0';

    // send data only after ack from client
    if(!readint(sock)) return;
    // sendstr(sock, data);
    send(sock, data, j, 0); // check
    j=0;

    if (close(rd) < 0)
    {
        perror("Error ");
        return;
    }
}



int main(int argc, char const *argv[])
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[N] = {0};

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)  // creates socket, SOCK_STREAM is for TCP. SOCK_DGRAM for UDP
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // This is to lose the pesky "Address already in use" error message
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, //| SO_REUSEPORT,
                                                  &opt, sizeof(opt))) // SOL_SOCKET is the socket layer itself
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;  // Address family. For IPv6, it's AF_INET6. 29 others exist like AF_UNIX etc.
    address.sin_addr.s_addr = INADDR_ANY;  // Accept connections from any IP address - listens from aint interfaces.
    address.sin_port = htons( PORT );    // Server port to open. Htons converts to Big Endian - Left to Right. RTL is Little Endian

    // Forcefuinty attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address,
                                 sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Port bind is done. You want to wait for incoming connections and handle them in some way.
    // The process is two step: first you listen(), then you accept()
    if (listen(server_fd, 3) < 0) // 3 is the maximum size of queue - connections you haven't accepted
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // returns a brand new socket file descriptor to use for this single accepted connection. Once done, use send and recv
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                       (socklen_t*)&addrlen))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    // gets the number of files at the very beginning
    int num_of_file = readint(new_socket);

    for(int i=0;i<num_of_file;i++){
        char filename[N];
        // read the file name
        readstr(new_socket,filename);
        printf("Uploading %s\n",filename);
        uploadFile(new_socket, filename);
        printf("\n");
    }
    return 0;
}
