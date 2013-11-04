#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>


void dump_memory_map(void);

int main(int argc, char **argv) {
    void *m1 = malloc(40); 
	void *m2 = malloc(0);
	void *m3 = malloc(100);
	//void *m4 = malloc(0);
	dump_memory_map();
	xfree(m1);
	xfree(m2);
	//xfree(m3);
	dump_memory_map();
	//xfree(m4);
	//dump_memory_map();
    return 0;
}
