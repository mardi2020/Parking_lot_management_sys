.SUFFIXES : .c .o

CC = g++

INC = 
LIBS = -lmysqlclient -lpthread
CFLAGS = -g $(INC)

OBJS = Car.o main.o Parkinglot.o

SRCS = Car.cpp main.cpp Parkinglot.cpp

TARGET = server

all : $(TARGET)

$(TARGET):$(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS)

dep :
	gccmakedep $(INC) $(SRCS)

clean :
	rm -rf $(OBJS) $(TARGET) core

new : 
	$(MAKE) clean 
	$(MAKE) 
