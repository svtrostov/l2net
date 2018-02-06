LIBS=-lnet -lpcap -lpthread -lm
CFLAGS=-ggdb -O0 -Wall
OBJS=main.o sys_memory.o sys_utils.o sys_socket.o mod_vars.o mod_config.o mod_arp.o mod_list.o
PROGS=l2net


most: l2net

all: $(PROGS)

l2net: $(OBJS)
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS) $(OPENCL_LIBS)
	rm -f $(OBJS)

clean:
	rm -f $(OBJS) $(PROGS) $(TESTS)
