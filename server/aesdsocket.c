/******************************************
 @file aesdsocket.c

 @author Karthik Baggaon Rajendra
 
 @references:
 		https://beej.us/guide/bgnet/html/
 		https://www.geeksforgeeks.org/socket-programming-cc/


*******************************************/





#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <syslog.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/wait.h>


#define PORT "9000"
#define WRITE_FILE_PATH "/var/tmp/aesdsocketdata"
#define BUFFER 2048
#define BACKLOG 10



static int socket_fd = -1;
static int client_fd = -1;
struct addrinfo hints, *res;\
char buf[BUFFER];


void clean_all()
{

      shutdown(socket_fd, SHUT_RDWR);
      close(socket_fd);
      shutdown(client_fd, SHUT_RDWR);
      close(client_fd);

   if(res != NULL)
   {
       freeaddrinfo(res);
   }
   
   // delete the data file
   remove(WRITE_FILE_PATH);
    
   //close the logging	
   closelog();
}


static void signal_handler(int sig_num)
{
    syslog(LOG_INFO, "Signal Caught %d\n\r", sig_num);
    
    if((sig_num == SIGINT) || (sig_num == SIGTERM))
    {
       clean_all();
    }
    
    exit(0);
}



//entry point
int main(int argc, char **argv) 
{
   openlog("aesdsocket", 0, LOG_USER);
   
   // register signals 
   sig_t ret_val = signal(SIGINT, signal_handler);
    
   if (ret_val == SIG_ERR) 
   {
       syslog(LOG_ERR, "Error while registering SIGINT\n\r");
       clean_all();
   }

   ret_val = signal(SIGTERM, signal_handler);
   if (ret_val == SIG_ERR) 
   {
       syslog(LOG_ERR, "Error while registering SIGTERM\n\r");
       clean_all();
   }

   bool daemon_flag = false;
    
   //check for daemon
   if (argc == 2) 
   {
      if (!strcmp(argv[1], "-d")) 
      {
         daemon_flag = true;
      } 
      else 
      {
         printf("wrong arg: %s\nUse -d option for running as daemon", argv[1]);
         syslog(LOG_ERR, "wrong arg: %s\nUse -d option for running as daemon", argv[1]);
         exit(EXIT_FAILURE);
      }
   }

  

    memset(&hints, 0, sizeof(hints));

    
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    int result = getaddrinfo(NULL, (PORT), &hints, &res);
    if (result != 0) 
    {
       syslog(LOG_ERR, "getaddrinfo() error %s\n", gai_strerror(result));
       clean_all();
       exit(EXIT_FAILURE);
    }

    //create socket connection
    socket_fd = socket(res->ai_family, SOCK_STREAM, 0);
    if (socket_fd < 0) 
    {
       syslog(LOG_ERR, "socket creation failed, error number %d\n", errno);
       clean_all();
       exit(EXIT_FAILURE);
    }

    // Set sockopts for reuse of server socket
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) 
    {
       syslog(LOG_ERR, "set socket options failed with error number%d\n", errno);
       clean_all();
       exit(EXIT_FAILURE);
    }

    // Bind device address to socket
    if (bind(socket_fd, res->ai_addr, res->ai_addrlen) < 0) 
    {
       syslog(LOG_ERR, "binding socket error num %d\n", errno);
       clean_all();
       exit(EXIT_FAILURE);
    }

    // Listen for connection
    if (listen(socket_fd, BACKLOG)) 
    {
       syslog(LOG_ERR, "listening for connection error num %d\n", errno);
       clean_all();
       exit(EXIT_FAILURE);
    }

    printf("Listening for connections\n\r");

    if (daemon_flag == true) 
    {
       int ret_val = daemon(0,0);
       
       if(ret_val == -1)
       {
          syslog(LOG_ERR, "failed to create daemon\n");
          clean_all();
          exit(EXIT_FAILURE);
       }
    }

    while(1) 
    {
       struct sockaddr_in client_addr;
       socklen_t client_addr_size = sizeof(client_addr);

       client_fd = accept(socket_fd, (struct sockaddr*)&client_addr, &client_addr_size);
        
       if(client_fd < 0)
       {
          syslog(LOG_ERR, "accepting new connection error is %s", strerror(errno));
          clean_all();
          exit(EXIT_FAILURE);
       } 
       
       char *client_ip = inet_ntoa(client_addr.sin_addr);


       
       syslog(LOG_INFO, "Accepted connection from %s \n\r",client_ip);
       printf("Accepted connection from %s\n\r", client_ip);


       /*read the data from client and write into /var/tmp/aesdsocketdata file*/
       while(1)
       {
          int read_bytes = read(client_fd, buf, (BUFFER));
            
          if (read_bytes < 0) 
          {
             syslog(LOG_ERR, "Error: reading from socket errno=%d\n", errno);
             continue; 
          }
           
          // no bytes to read 
          if (read_bytes == 0)
              continue;

          printf("read %d bytes\n\r", read_bytes);
          
          //open the file for writing
          int fd = open(WRITE_FILE_PATH,O_RDWR | O_CREAT | O_APPEND, 0766);

          if (fd < 0)
             syslog(LOG_ERR, "error opening file errno is %d\n\r", errno);

          int write_bytes = write(fd, buf, read_bytes);

          if(write_bytes < 0)
          {
             syslog(LOG_ERR, "Error writing to file errno is %d\n\r", errno);
             close(fd);
             continue;
          }

          close(fd);
          printf("wrote %d bytes\n\r", write_bytes);

          if (strchr(buf, '\n'))  // check for new line
          {  
             break;
          } 

       }

       int read_offset = 0;

       /*read the data from /var/tmp/aesdsocketdata file and send to the client*/
       while(1) 
       {
          int fd = open(WRITE_FILE_PATH, O_RDWR | O_CREAT | O_APPEND, 0766);
            
          if(fd < 0)
          {
             syslog(LOG_ERR, "file open error with errno=%d\n", errno);
             printf("error is %d\n\r", errno);
             continue; 
          }

          /*update file offset since it would have been updated by previous
            write operation */
          lseek(fd, read_offset, SEEK_SET);
          int read_bytes = read(fd, buf, (BUFFER));
            
          close(fd);
          if(read_bytes < 0)
          {
             syslog(LOG_ERR, "Error reading from file errno %d\n", errno);
             continue;
          }

          if(read_bytes == 0)
             break;

          int write_bytes = write(client_fd, buf, read_bytes);
          printf("wrote %d bytes to client\n\r", write_bytes);

          if(write_bytes < 0)
          {
             printf("errno is %d", errno);

             syslog(LOG_ERR, "Error writing to client fd %d\n", errno);
             continue;
          }
            
          read_offset = read_offset + write_bytes;
       }

       close(client_fd);
        
       client_fd = -1;
       syslog(LOG_INFO, "closing client socket\n\r");
    } 

    clean_all();

    return 0;

}
