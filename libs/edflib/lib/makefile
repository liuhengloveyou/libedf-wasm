#
#
# Author: Teunis van Beelen
#
# email: teuniz@protonmail.com
#
#
# GNU/Linux only for now.
#
# https://tldp.org/HOWTO/Program-Library-HOWTO/shared-libraries.html
#
# nm -D libedf.so.1.2.5
#
# objdump -T libedf.so.1.2.5
#

CC := gcc
CFLAGS := -O2 -fpic -fvisibility=hidden -std=gnu11 -Wall -Wextra -Wshadow -Wformat-nonliteral -Wformat-security \
          -D_LARGEFILE64_SOURCE -D_LARGEFILE_SOURCE -DEDFLIB_SO_DLL -DEDFLIB_BUILD
LDLIBS := -lc
TARGET := libedf.so.1.2.5

LBITS := $(shell getconf LONG_BIT)

ifeq ($(LBITS),64)
   LIBDIR := /lib64
else
   LIBDIR := /lib
endif

ifeq ($(PREFIX),)
    PREFIX := /usr/local
endif

.PHONY: install clean

objects = edflib.o

all: $(TARGET)

$(TARGET) : $(objects)
	$(CC) -shared -Wl,-soname,libedf.so.1 -o $(TARGET) edflib.o $(LDLIBS)

edflib.o : ../edflib.h ../edflib.c
	$(CC) $(CFLAGS) -c ../edflib.c -o edflib.o

install : all
ifneq ($(shell id -u), 0)
	$(error You must be root to perform this action.)
endif
	install -d $(DESTDIR)$(PREFIX)$(LIBDIR)/
	install -m 755 $(TARGET) $(DESTDIR)$(PREFIX)$(LIBDIR)/
	install -d $(DESTDIR)$(PREFIX)/include/
	install -m 644 ../edflib.h $(DESTDIR)$(PREFIX)/include/
	ldconfig

clean :
	$(RM) $(TARGET) $(objects)






