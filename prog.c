#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#include <assert.h>

#define BUFMB 10 //Maximum memory, in Mb that we can use. Largely affects the monitoring accuracy (higher = worse).
#define MIN(X,Y) X < Y ? X : Y;

typedef struct {
	char *filename;
	int size;
} args_t;

enum {
	PROGNAME,
	SRCFILE,
	DESTFILE,
	NARGS
};

int file_size(char *filename){
	int ret;
	FILE *fp = fopen(filename, "r");
	assert(fp);
	fseek(fp, 0, SEEK_END);
	ret = ftell(fp);
	fclose(fp);
	return ret;
}

unsigned long int file_monitor(args_t *args){
	unsigned long int base, t;
	int prev, curr;

	prev = file_size(args->filename);
	base = t = time(NULL);
	do {
		while(time(NULL) == t) usleep(10000); //Waits 10ms. For minimizing cpu usage of this loop.
		t = time(NULL);
		curr = file_size(args->filename);
		if(curr != prev)
			printf("Time remaining: %.2lf seconds\n", (args->size-curr)/(double) (curr-prev));
		//else printf("Stalled\n");	
		prev = curr;
	} while(curr != args->size);

	return (unsigned long int) t - base;;
}

void copy(char *dest, char *src){
	void *buf;
	int max, aux, bufsize = BUFMB*1000*1000;
	FILE *d, *s;

	buf = malloc(bufsize);
	max = file_size(src);
	d = fopen(dest, "w");
	s = fopen(src, "r");

	assert(d);
	assert(s);

	while(ftell(d) != max){
		aux = MIN(bufsize, max-ftell(d));
		assert(fread(buf, 1, aux, s) != 0);
		fwrite(buf, 1, aux, d);
		fflush(d);
	}

	free(buf);
	fclose(d);
	fclose(s);
}

int main(int argc, char *argv[]){
	if(argc != NARGS){
		printf("Usage: ./prog [source file] [dest file]\n");
	}

	args_t args;
	pthread_t thread;
	unsigned long int total_time;

	args.size = file_size(argv[SRCFILE]);
	args.filename = argv[DESTFILE];
	pthread_create(&thread, NULL, (void *(*) (void *)) file_monitor, &args);

	copy(argv[DESTFILE], argv[SRCFILE]);

	pthread_join(thread, (void **) &total_time);
	printf("Total time: %lu seconds\n", total_time);

	return 0;
}
