#
#  Makefile
#  plotPlanes
#
#  Daine L. Danielson
#  CAPTAIN
#  Los Alamos National Laboratory
#  07/02/2014
#

OBJS = main.o MDatRunReader.o TTPCDataHandler.o

#CC = /opt/local/bin/g++
CC = g++
CFLAGS = -fPIC -std=c++0x -std=c++11 -Wall #-g
INCLUDES = -I/usr/local/include/root
LIBS = -L/usr/local/lib/root -lCore -lRIO -lHist -lGpad


all : plotPlanes
plotPlanes : $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o plotPlanes

main.o : main.cpp TTPCDataHandler.h
	$(CC) $(CFLAGS) $(INCLUDES) -c main.cpp

TTPCDataHandler.o : TTPCDataHandler.cpp TTPCDataHandler.h MDatRunReader.h Configuration.h
	$(CC) $(CFLAGS) $(INCLUDES) -c TTPCDataHandler.cpp

MDatRunReader.o : MDatRunReader.cpp MDatRunReader.h Configuration.h
	$(CC) $(CFLAGS) $(INCLUDES) -c MDatRunReader.cpp

clean :
	rm -f $(OBJS)