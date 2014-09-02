#
#  Makefile
#  plotPlanes
#
#  Daine L. Danielson
#  CAPTAIN
#  Los Alamos National Laboratory
#  07/02/2014
#

OBJS = main.o TTPCDataHandler.o MDatRunReader.o MCRunReader.o VMRunReader.o

#CC = /opt/local/bin/g++
CC = g++
CFLAGS = -fPIC -std=c++0x -std=c++11 -Wall #-g
INCLUDES = -I/usr/local/include/root
LIBS = -L/usr/local/lib/root -lCore -lRIO -lHist -lGpad -lGraf


all : plotPlanes
plotPlanes : $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o plotPlanes

main.o : main.cpp TTPCDataHandler.h Configuration.h
	$(CC) $(CFLAGS) $(INCLUDES) -c main.cpp

TTPCDataHandler.o : TTPCDataHandler.cpp TTPCDataHandler.h MDatRunReader.h \
                    MCRunReader.h Configuration.h
	$(CC) $(CFLAGS) $(INCLUDES) -c TTPCDataHandler.cpp

VMRunReader.o : VMRunReader.cpp VMRunReader.h Configuration.h
	$(CC) $(CFLAGS) $(INCLUDES) -c VMRunReader.cpp

MDatRunReader.o : MDatRunReader.cpp MDatRunReader.h VMRunReader.h Configuration.h
	$(CC) $(CFLAGS) $(INCLUDES) -c MDatRunReader.cpp

MCRunReader.o : MCRunReader.cpp MCRunReader.h VMRunReader.h Configuration.h
	$(CC) $(CFLAGS) $(INCLUDES) -c MCRunReader.cpp

clean :
	rm -f $(OBJS)