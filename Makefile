all: server client

server: header.h
client: header.h

clean: 
	rm -f server client
