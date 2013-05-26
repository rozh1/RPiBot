CC			:= g++
CFLAGS		:= -c -Wall -I/usr/local/include/opencv -I./ -L/usr/local/lib
LIBRARIES	:= -lopencv_core -lopencv_imgproc -lopencv_highgui -lwiringPi -lpthread
LDFLAGS		:= 
SOURCES		:= tcp_serv.cpp motion.cpp image_processing.cpp
OBJECTS		:= $(SOURCES:.cpp=.o)
EXECUTABLE	:= serv

.PHONY:	all clean

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(LIBRARIES) $(OBJECTS) -o $@ 

.SUFFIXES: .cpp .o
.cpp.o:
	$(CC) $(LDFLAGS) $(CFLAGS)  $< -o $@
        
clean:
	rm -f *.o
