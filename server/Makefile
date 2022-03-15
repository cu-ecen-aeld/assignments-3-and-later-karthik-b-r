CROSS_COMPILE=

ifeq ($(CC),)
	CC = $(CROSS_COMPILE)gcc
endif

ifeq ($(CFLAGS),)
	CFLAGS = -Wall -Werror -g 
endif

ifeq ($(LDFLAGS),)
	LDFLAGS = -pthread -lrt
endif

TARGET = aesdsocket
	
all: $(TARGET) 
default: $(TARGET)
	
aesdsocket: aesdsocket.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c -o aesdsocket.o aesdsocket.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o aesdsocket aesdsocket.o
	
        

.PHONY: clean

clean:
	rm -f *.o $(TARGET) 
