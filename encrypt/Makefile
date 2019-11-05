CC=gcc
SSL_FLAGS= -lcrypto -lssl
CFLAGS=-I.

all:
	$(CC) -o test test.c encrypt.c $(CFLAGS) $(SSL_FLAGS)

clean:
	rm test
