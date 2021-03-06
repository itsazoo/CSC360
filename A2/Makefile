# CC = gcc
CFLAGS = -Wall -g   
LDFLAGS = -lpthread 

A2OBJS = assign2.o train.o

all: assign2 

assign2: $(A2OBJS)
	$(CXX) $(CFLAGS) -o assign2 $(A2OBJS) $(LDFLAGS) 

clean: 
	rm -rf $(A2OBJS) assign2 
