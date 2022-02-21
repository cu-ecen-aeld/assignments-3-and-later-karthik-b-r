CROSS_COMPILE=

CC = $(CROSS_COMPILE)gcc
CFLAGS = -Wall -Werror -g 
TARGET = aesdsocket
	
all: $(TARGET) 
default: $(TARGET)
	
writer: aesdsocket.c
	$(CC) $(CFLAGS) -c -o aesdsocket.o aesdsocket.c
	$(CC) $(CFLAGS) -o aesdsocket aesdsocket.o
	
        

.PHONY: clean

clean:
	rm -f *.o $(TARGET) 
