CC = gcc
CFLAGS = -Wall -Wextra -pedantic -O0 -ggdb -fsanitize=address

# Targets
SERVER_TARGET = server.out
CLIENT_TARGET = client.out

# Object files
SERVER_OBJS = server.o utils.o
CLIENT_OBJS = client.o utils.o

# Default target
all: $(SERVER_TARGET) $(CLIENT_TARGET)

# Server build
$(SERVER_TARGET): $(SERVER_OBJS)
	$(CC) $(CFLAGS) $(SERVER_OBJS) -o $(SERVER_TARGET)

server.o: server.c utils.h
	$(CC) $(CFLAGS) -c server.c

utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c utils.c

# Client build
$(CLIENT_TARGET): $(CLIENT_OBJS)
	$(CC) $(CFLAGS) $(CLIENT_OBJS) -o $(CLIENT_TARGET)

client.o: client.c
	$(CC) $(CFLAGS) -c client.c

# Clean build artifacts
clean:
	rm -f *.o $(SERVER_TARGET) $(CLIENT_TARGET)

