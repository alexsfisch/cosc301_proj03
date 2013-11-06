#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>


void dump_memory_map(void);
void xfree(void *memory_block);

int main(int argc, char **argv) {
    void *m1 = malloc(3000); 
	//void *m2 = malloc(0);
	//void *m3 = malloc(0);
	//void *m4 = malloc(0); 
	//void *m5 = malloc(0);
	//void *m6 = malloc(0);
	//void *m7 = malloc(0); 
	dump_memory_map();
	//void *m8 = malloc(0);
	//dump_memory_map();
	//xfree(m1);
	//dump_memory_map();
	//xfree(m2);
	//xfree(m3);
	//xfree(m4);
	//xfree(m5);
	//xfree(m6);
	//xfree(m7);
	//dump_memory_map();
	//xfree(m8);
    return 0;
}
