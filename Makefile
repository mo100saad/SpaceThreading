# Compiler + flags for any errors
CC = gcc
CFLAGS = -g -Wall -Wextra -pthread

# Executable and source files
TARGET = cuinspace
SRCS = main.c system.c manager.c resource.c event.c
OBJS = $(SRCS:.c=.o)

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Compile each c file into an .o file
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets
.PHONY: all clean
