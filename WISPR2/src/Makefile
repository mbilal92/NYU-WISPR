# build wispr executable when user executes "make"

CC=gcc
wispr: wisprd.o  
	$(CC) wisprd.o wispr_proto.o wispr_net.o -o wispr -ldaemon
wisprd.o: wisprd.c  
	$(CC) $(CFLAGS) -w -c wisprd.c wispr_proto.c wispr_net.c
# remove object files and executable when user executes "make clean"
clean:
	rm *.o wispr wispr2 wispr3 wispr4 wispr5 wispr6 wispr7
