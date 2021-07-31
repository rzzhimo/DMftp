CC := gcc
includepath=/opt/dmdbms/include
libpath=/opt/dmdbms/bin
vpath=./

CFLAGS := -I$(includepath) -Wall -g -Os -DDM64
LINKFLAGS=-L$(libpath) -ldodbc -Wall -Wl,-rpath $(libpath)

SHDIR := ../common

OBJS = ftserver.o $(SHDIR)/common.o

all: ftserver

ftserver: $(OBJS)
	@$(CC) -o ftserver $(CFLAGS) $(OBJS) -g $(LINKFLAGS)

$(OBJS) : %.o: %.c 
	@$(CC) -c $(CFLAGS) $< -o $@
	@echo make ok.

.PHONY:
clean:
	@rm -f *.o ftserver
	@rm -f ../common/*.o
	@echo Done cleaning