CC		:= gcc
CFLAGS		:= -c -Wall
LIBRARIES	:= -lopencv_core -lopencv_imgproc -lopencv_highgui -lwiringPi -lpthread
LDFLAGS		:= -I/usr/local/include/opencv -L/usr/local/lib  -I./
SOURCES=tpc_serv.c motion.c image_processing.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=tpc_serv

.PHONY: clean all 

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(LIBRARIES) $(OBJECTS) -o $@ 

.c.o:
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@
        
clean:
	rm -f *.o
