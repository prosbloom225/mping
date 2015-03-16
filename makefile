CC=clang
CFLAGS=-I. -Wall  -std=c99 -D_BSD_SOURCE -Wno-unused-variable -fcolor-diagnostics -g
DEPS = 
OBJ = ping.o  
LDFLAGS = 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

mping: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm -rf *.o mping 
install:
	cp mping /usr/bin
