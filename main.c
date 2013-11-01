#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>


void dump_memory_map(void);

int main(int argc, char **argv) {
	printf("%s\n","main allocating 32");
    void *m1 = malloc(0);  // should allocate 64 bytes
	void *m2 = malloc(0);
	void *m3 = malloc(0);
	void *m4 = malloc(0);
	//printf("%s\n","main allocating 8");		
    //void *m2 = malloc(24); // should allocate 128 bytes
	//diff(m1, m2);
	dump_memory_map();
	xfree(m1);
	xfree(m2);
	xfree(m3);
	dump_memory_map();
	xfree(m4);
	//xfree(m2);
	//dump_memory_map();
	//dump_memory_map();
    //void *m3 = malloc(21);  // should allocate 64 bytes
    /*void *m4 = malloc(11);  // should allocate 32 bytes
	diff(m3, m4);
    free(m3);
    void *m5 = malloc(30);  // should allocate 64 bytes
    void *m6 = malloc(120); // should allocate 128 bytes
	diff(m5, m6);
    free(m2);*/
    return 0;
}
