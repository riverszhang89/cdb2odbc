PREFIX?=/opt/bb
OBJECTS=attr.o connect.o convert.o error.o execute.o handle.o meta.o result.o util.o
LIBRARY=libcdb2odbc.so

arch:=$(shell uname)
export $(arch)

ifeq ($(arch),Linux)
  CC=gcc -g
  CFLAGS+=-Werror -Wall
  CFLAGS_64+=-m64 -std=c99
  CFLAGS_PIC+=-fPIC
  CFLAGS_DEF+=-D_GNU_SOURCE -D_XOPEN_SOURCE=600 -D__UNIXODBC__
  LDFLAGS_64+=-m64
  SHARED=-shared
endif

ifeq ($(arch),SunOS)
  CC=cc
  CFLAGS_64+=-m64 -xc99=all
  CFLAGS_PIC+=-KPIC
  CFLAGS_DEF+=-D__UNIXODBC__
  LDFLAGS_64+=-m64
  LDFLAGS_OSLIBS=-lc -lnsl -lsocket
  SHARED=-G
endif

ifeq ($(arch),AIX)
  CC=xlc_r
  CFLAGS_64+=-q64 -qlanglvl=extc99
  CFLAGS_PIC+=-qpic
  CFLAGS_DEF+=-D__UNIXODBC__
  LDFLAGS_64+=-q64
  SHARED=-G
endif

CFLAGS_DEF+=-D__LOG__ -D__COLORFUL__ #-D__DEBUG__

CFLAGS+=-I$(PREFIX)/include $(CFLAGS_64) $(CFLAGS_PIC) $(CFLAGS_DEF)
LDFLAGS+=-lodbcinst -L$(PREFIX)/lib -lcdb2api -lcdb2protobuf -lprotobuf-c \
         -lssl -lcrypto -lpthread -lrt -lm $(LDFLAGS_OSLIBS) $(LDFLAGS_64)

$(LIBRARY): $(OBJECTS)
	$(CC) $(SHARED) $(LDFLAGS) $^ -o $@

install: $(LIBRARY)
	install -m 644 -D $^ $(PREFIX)/lib

.PHONY: clean
clean:
	rm -f $(OBJECTS) $(LIBRARY)
