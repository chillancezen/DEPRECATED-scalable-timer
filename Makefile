DLIB_TARGET=libutimer.so
HEADER=./include

#INCLUDE=-I/mnt/projects/vlink-frontend/ -I/mnt/projects/DPDK/dpdk-mapping/include -I/mnt/projects/meowlover/include/
#INCLUDE=$(INCLUDE_DIR)
INCLUDE=-I./include
CCFLAG+= -g3 -Wall -fpic -O0 
LDFLAG+= -lpthread -lrt 
CC=gcc
LD=ld
SRC=$(wildcard src/*.c)
SRC+=$(wildcard client/*.c)

OBJ=$(patsubst %.c,%.o,$(SRC))


%.o:%.c
	$(CC) -I$(HEADER) $(INCLUDE)  $(CCFLAG) -c -o $@ $<
$(DLIB_TARGET):$(OBJ)
	gcc -shared $(LDFLAG) -o $(DLIB_TARGET)  $(OBJ)
install:$(DLIB_TARGET)
	rm -f /lib64/$(DLIB_TARGET)
	cp ./$(DLIB_TARGET) /lib64
uninstall:
	-rm -f /lib64/$(DLIB_TARGET)
clean:
	-rm -f $(DLIB_TARGET)
	find -name "*.o" -exec rm -f {} \;
