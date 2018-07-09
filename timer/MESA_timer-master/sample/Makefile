CC=gcc -g -O0
LIB_PATH=../lib
INC=-I../include
LIB=../lib/lib_MESA_timer.a

TARGET=sample

all:$(TARGET)

sample:sample.c
	$(CC)  -o $@ $(INC) $^ -L$(LIB_PATH) $(LIB)
clean:
	rm -f $(TARGET)
