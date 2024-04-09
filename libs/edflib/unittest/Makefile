#
#
# Author: Teunis van Beelen
#
# email: teuniz@gmail.com
#
#

CC = gcc
CFLAGS = -Wall -Wextra -Wshadow -Wformat-nonliteral -Wformat-security -Wtype-limits -Wfatal-errors -g -D_LARGEFILE64_SOURCE -D_LARGEFILE_SOURCE

all: edflib_test

edflib_test : edflib.o unittest.o
	$(CC) unittest.o edflib.o -o edflib_test

unittest.o : unittest.c ../edflib.h ../edflib.c
	$(CC) $(CFLAGS) -c unittest.c -o unittest.o

edflib.o : ../edflib.h ../edflib.c
	$(CC) $(CFLAGS) -c ../edflib.c -o edflib.o

clean :
	$(RM) *.o edflib_test *.[be]df

#
#
#
#





