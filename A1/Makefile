CFLAGS = -Wall -g  
LDFLAGS =  

OBJS = main.o

all: a1 

a1: $(OBJS)
	$(CC) $(CFLAGS) -o a1 $(OBJS) $(LDFLAGS) -lreadline -lhistory -ltermcap

clean: 
	rm -rf $(OBJS) a1 
