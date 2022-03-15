#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>


#define ARGUMENTS 3

int main(int argc, char *argv[])
{
 
 openlog(NULL, 0, LOG_USER); // syslog using LOG_USER facility
  
 
  if(argc != ARGUMENTS) //check arguments
  {
  syslog(LOG_ERR, "Invalid number of arguments: %d ", argc);
  printf("Enter correct number of argument:\n");
  printf("\t 1. Path to the file along with file name \n");
  printf("\t 2. String to be written in the file\n");
  return 1;
  }

 int fd;
 fd = open( argv[1], O_CREAT | O_RDWR, 0644);  // Open the file.

 if(fd == -1)
 {	
   syslog(LOG_ERR, "Error opening file %s: %s\n", argv[1], strerror(errno));  //Error message
   printf(" File directory not found \n");
   return 1;
 }

 int len = 0;
 len = write( fd, argv[2], strlen(argv[2]));
	
 if(len == -1)
 {
   syslog(LOG_ERR,"%s: File write failed",argv[1]);	
 }
 else if( len != strlen(argv[2]) )
 {
   syslog(LOG_ERR,"%s: Partial file write", argv[1]);
 }
 else
 {
   syslog(LOG_DEBUG,"Writing %s to %s", argv[2], argv[1]);
 }
					
 // close syslog
 closelog();

  return 0;
}
