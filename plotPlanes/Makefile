#
#  Makefile
#  plotPlanes
#
#  Daine L. Danielson
#  CAPTAIN
#  Los Alamos National Laboratory
#  07/02/2014
#

OBJS = main.o TTPCDataHandler.o  VMRunReader.o \
       MSingleEventRunReader.o

#CC = /opt/local/bin/g++
CC = g++
CFLAGS = -fPIC -std=c++0x -std=c++11 -Wall -O3 #-g
INCLUDES = -I/usr/local/include/root
LIBS = -L/usr/local/lib/root -lCore -lRIO -lHist -lGpad


all : plotPlanes
plotPlanes : $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o plotPlanes

main.o : main.cpp TTPCDataHandler.h Configuration.h
	$(CC) $(CFLAGS) $(INCLUDES) -c main.cpp

TTPCDataHandler.o : TTPCDataHandler.cpp TTPCDataHandler.h Configuration.h \
                    MSingleEventRunReader.h
	$(CC) $(CFLAGS) $(INCLUDES) -c TTPCDataHandler.cpp

VMRunReader.o : VMRunReader.cpp VMRunReader.h Configuration.h
	$(CC) $(CFLAGS) $(INCLUDES) -c VMRunReader.cpp

MSingleEventRunReader.o : MSingleEventRunReader.cpp MSingleEventRunReader.h \
                          VMRunReader.h Configuration.h
	$(CC) $(CFLAGS) $(INCLUDES) -c MSingleEventRunReader.cpp

clean :
	rm -f $(OBJS)