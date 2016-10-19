all:
	gcc -Wall -O3 prog.c -o copy -lpthread

run:
	./prog

clean:
	find -type f -name "*~" -delete
