# CC = gcc
CFLAGS = -Wall -g   

A3OBJS = diskinfo.o
A3OBJS2 = disklist.o
A3OBJS3 = diskget.o
A3OBJS4 = diskput.o

all: diskinfo disklist diskget diskput

diskinfo: $(A3OBJS)
	$(CXX) $(CFLAGS) -o diskinfo $(A3OBJS)

disklist: $(A3OBJS2)
	$(CXX) $(CFLAGS) -o disklist $(A3OBJS2)

diskget: $(A3OBJS3)
	$(CXX) $(CFLAGS) -o diskget $(A3OBJS3)

diskput: $(A3OBJS4)
	$(CXX) $(CFLAGS) -o diskput $(A3OBJS4)

clean: 
	rm -rf $(A3OBJS) diskinfo 
