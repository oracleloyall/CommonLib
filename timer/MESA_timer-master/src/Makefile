CC=gcc
CCC=g++

CFLAGS=-g -Wall -fPIC -shared
LIBPATH=../lib
H_DIR=-I../include

OBJS=MESA_timer.o

TARGET=lib_MESA_timer.a lib_MESA_timer.so

all:$(TARGET)

.c.o:
	$(CC) -c $(CFLAGS) -I. $(H_DIR) $<
.cpp.o:
	$(CCC) -c $(CFLAGS) -I. $(H_DIR) $<
lib_MESA_timer.a: $(OBJS)
	(rm -f $@ ;ar -r $@ $^; cp $@ $(LIBPATH);)
lib_MESA_timer.so: $(OBJS)
	(rm -f $@; gcc -o $@ $(CFLAGS) $^; cp $@ $(LIBPATH);)
clean:
	rm -f *.o $(TARGET)
