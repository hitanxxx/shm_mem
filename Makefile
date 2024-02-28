all:
	gcc -c shm.c -o shm.o -I. -lrt -lpthread -Wall -O2 -ffunction-sections -fdata-sections
	ar rcs shm.a *.o
	cp -rf ./shm.a ./test
	cp -rf ./shm.h ./test
clean:
	rm -rf shm.a
	rm -rf shm.o
