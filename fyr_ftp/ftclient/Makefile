CC := gcc
includepath=/opt/dmdbms/include
libpath=/opt/dmdbms/bin
vpath=./

CFLAGS := -I$(includepath) -Wall -g -Os
LINKFLAGS=-L$(libpath) -ldodbc -Wall -Wl,-rpath $(libpath)
SHDIR := ../common

OBJS = ftclient.o $(SHDIR)/common.o

all: ftclient

ftclient: $(OBJS)
	@$(CC) -o ftclient $(CFLAGS) $(OBJS) -g $(LINKFLAGS)

$(OBJS) : %.o: %.c 
	@$(CC) -c $(CFLAGS) $< -o $@
	@echo make ok.

.PHONY:
clean:
	@rm -f *.o ftclient
	@rm -f ../common/*.o
	@echo Done cleaning