CC=gcc
# DM_HOME=/opt/dmdbms
includepath=/opt/dmdbms/include
libpath=/opt/dmdbms/bin
vpath=./

CFLAGS=-I$(includepath) -DDM64 -Wall
LINKFLAGS=-L$(libpath) -ldodbc -Wall -Wl,-rpath $(libpath)

%.o:%.c
	$(CC) -g -c $(CFLAGS) $< -o $@

object_file1=odbc_conn.o
object_file2=odbc_dml.o
object_file3=odbc_bind.o
object_file4=odbc_lob.o

object_files=odbc_conn.o odbc_dml.o odbc_bind.o odbc_lob.o

final_objects=odbc_conn odbc_dml odbc_bind odbc_lob

all : $(final_objects)

.PHONY : all clean rebuild

odbc_conn : $(object_file1)
	$(CC) -o $@ $(object_file1) -g $(LINKFLAGS)
	@echo make ok.

odbc_dml : $(object_file2)
	$(CC) -o $@ $(object_file2) -g $(LINKFLAGS)
	@echo make ok.

odbc_bind : $(object_file3)
	$(CC) -o $@ $(object_file3) -g $(LINKFLAGS)
	@echo make ok.

odbc_lob : $(object_file4)
	$(CC) -o $@ $(object_file4) -g $(LINKFLAGS)
	@echo make ok.

clean :
	@rm -rf $(object_files)
	@rm -rf $(final_objects)

rebuild : clean all

