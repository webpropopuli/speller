# compiler to use
CC = gcc

# flags to pass compiler
#CFLAGS = -fsanitize=signed-integer-overflow -fsanitize=undefined -ggdb3 -O0 -Wunused-arguments -std=c11 -Wall -Werror -Wextra -Wno-sign-compare -Wshadow
CFLAGS = -ggdb3 -std=c99 -Wall

# name for executable
EXEC = speller.exe

# space-separated list of header files
HDRS = dictionary.h

# space-separated list of libraries, if any,
# each of which should be prefixed with -l
LIBS =

# space-separated list of source files
SRCS = speller.c dictionary.c

# automatically generated list of object files
OBJS = speller.o dictionary.o


# default target
$(EXEC): $(OBJS) $(HDRS) makefile
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

# dependencies
$(OBJS): $(HDRS) Makefile

# housekeeping
clean:
	rm -f core $(EXE) *.o
