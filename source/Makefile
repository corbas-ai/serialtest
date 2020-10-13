P=server
N=client
BAUDS=
WAIT_SLEEPNS=
CFLAGS=-std=gnu11 $(BAUDS) $(WAIT_SLEEPNS)

all: $P $N

$P: server.c
$N: client.c

clean:
	rm -rf $P $N

remake:
	make clean
	make
