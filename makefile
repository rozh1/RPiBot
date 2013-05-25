CC		:= gcc
CFLAGS		:= -I/usr/local/include/opencv -L/usr/local/lib
OBJECTS		:= 
LIBRARIES	:= -lopencv_core -lopencv_imgproc -lopencv_highgui -lwiringPi -lpthread

.PHONY: all clean

all: test

test: 
	$(CC) $(CFLAGS) -o tcp_serv tcp_serv.c $(LIBRARIES)
        
clean:
	rm -f *.o
