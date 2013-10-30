#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <inttypes.h>
#include <math.h>

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

// function declarations
void *malloc(size_t);
void free(void *);
void dump_memory_map(void);
int closestPower2 (size_t requested);
uint64_t diff (void *a, void *b);
int areBuddies (uint64_t diff);
void setHeader (void *v, int size, int next);
int getHeaderSize (void *v);
int getHeaderNext (void *v);
void *firstFreeBlock(void *free_list_local, int amountToAllocate);
void *splitBlock(int *free_list_temp, int amountToAllocate);


const int HEAPSIZE = (1*128); // 1 MB
const int MINIMUM_ALLOC = sizeof(int) * 2;

// global file-scope variables for keeping track
// of the beginning of the heap.
void *heap_begin = NULL;
void *free_list = NULL;

/* works except for following issue:
when you allocate a block inbetween two blocks in the free list
it does not update the offset for the block to the left of allocated
ex:

|16|32|64|

if you allocate block size 32, the offset for 16 should be adjustsed, 
however it is not
*/
void *malloc(size_t request_size) {
	int *temp = NULL;

	int amountToAllocate = 0;
    // if heap_begin is NULL, then this must be the first
    // time that malloc has been called.  ask for a new
    // heap segment from the OS using mmap and initialize
    // the heap begin pointer.
    if (!heap_begin) {
        heap_begin = mmap(NULL, HEAPSIZE, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
		free_list=heap_begin;
		printf("%s", "start of heap:   ");
		printf("%i\n",heap_begin);
		//set header
		setHeader(heap_begin, HEAPSIZE, 0);
        atexit(dump_memory_map);
   }

	//printf("%s","free list location:    ");
	//printf("%i\n",free_list);
	
		
	amountToAllocate = closestPower2(request_size);
	printf("%s", "ammount to allocate:   ");
	printf("%i\n",amountToAllocate);

	temp = firstFreeBlock(free_list, amountToAllocate);


	printf("%s","first block of free list:    ");
	printf("%i\n",free_list);

	splitBlock(temp, amountToAllocate);

}

void *splitBlock(int *free_list_temp, int amountToAllocate) {
		printf("%s\n","SPLIT BLOCK");
	void* tempNext = free_list_temp;
	void* free_list_temp2 = (free_list_temp+(*(free_list_temp+1)/4));
	int difference = 0;

	while (*(free_list_temp)>amountToAllocate) {

		tempNext = free_list_temp;
		free_list_temp2 = free_list_temp;

		//update header of first
		*(free_list_temp) = *(free_list_temp)/2;
		printf("%s", "new first header size:   ");
		printf("%i\n",*(free_list_temp));	
		*(free_list_temp+1) =  *(free_list_temp); //zero because you are allocating
		printf("%s", "new first header next:   ");
		printf("%i\n",*(free_list_temp+1));
		//free_list = (free_list_temp + (*free_list_temp/4));


		//update header of second
		*(free_list_temp + (*free_list_temp/4)) = *(free_list_temp);
		printf("%s", "new second header size:   ");
		printf("%i\n",*(free_list_temp + (*free_list_temp/4)));
		/*printf("%s", "*(free_list_temp + (*free_list_temp/4))");
		printf("%i\n",*(free_list_temp + (*free_list_temp/4)));
		printf("%s", "*(free_list_temp + (*free_list_temp+1/4))");
		printf("%i\n",*(free_list_temp + (*free_list_temp+1/4)));*/
		free_list_temp2 = (free_list_temp+(*(free_list_temp+1)/4));

		//find difference for offset of second block
		difference = diff(tempNext,free_list_temp2);
		//printf("%s", "difference:   ");
		//printf("%i\n",difference);
		*(free_list_temp + (*free_list_temp/4)+1) = difference;
		printf("%s", "new second header next:   ");
		printf("%i\n",*(free_list_temp + (*free_list_temp/4)+1)); //need to fix this for last eleemtn. should be 0
		printf("%s\n","-----next cut in half-------");

		difference = 0;
	}
	setHeader(free_list_temp,*(free_list_temp),0);//set next to 0 to indicate allocated
	free_list = free_list_temp2; //update free list to exclude recently allocated block
	printf("%i\n",free_list);

}


void *firstFreeBlock(void *free_list_local, int amountToAllocate) {
	int* free_list_temp = (int*)free_list_local;
	while (*(free_list_temp+1)!=0) {
		if (amountToAllocate<=*(free_list_temp)) {
			return free_list_temp;
		}
		free_list_temp = free_list_temp + (*free_list_temp/4);
	}

	//for last element	
	if (amountToAllocate<=*(free_list_temp)) {
		return free_list_temp;

	}
	return NULL;
}


void free(void *memory_block) {

}

void dump_memory_map(void) {
		printf("%s\n","-----------------------");
		int* free_list_local = (int*)(free_list);
		while (*(free_list_local+1)!=0) {
			printf("%s","Block Size:  ");
			printf("%i",*(free_list_local));
			printf("%s",",  offset:  ");
			printf("%i",*(free_list_local+1));
			printf("%s\n",",  free");
			free_list_local = free_list_local + (*(free_list_local+1)/4);

			//need to check current minus previous to see if there are allocated chunks inbetween.


		}
		printf("%s\n","-----------------------");
}


//check the difference between two header locations
uint64_t diff (void *a, void *b) {
	//b-a
	int bitDiff = 0;
	uint64_t aConverted = (uint64_t*)a; //typecast
	//printf("%s","a:  ");
	//printf("%" PRIx64 "\n",aConverted);

	uint64_t bConverted = (uint64_t*)b; //typecast
	//printf("%s","b:  ");
	//printf("%" PRIx64 "\n",bConverted);

	uint64_t diffConverted = bConverted^aConverted; //calc. diff

	/*printf("%s","Difference:  ");
	printf("%" PRIx64 "\n",diffConverted);
	bitDiff = areBuddies(diffConverted);
	printf("%s","bitDiff:  ");
	printf("%i\n",bitDiff);*/

	return diffConverted;
	//return(((uint64_t*)b)^((uint64_t*)a));
}


//check if two blocks differ by one bit
int areBuddies (uint64_t diff){
	int bitDiff = 0;
	int i = 0;
	for (;i<64;i++) {
		if ((diff & 0x1)==0x1) {
			bitDiff++;
		}
		diff >>=1;
	}
	return bitDiff;

}

void setHeader (void *v, int size, int next) {
	int* block = (int*)v;
	*block = size;
	*(block+1) = next;
}


int getHeaderSize (void *v) {
	int size = 0;
	int* header = (int*)v;// typecast
	header = header - 2;
	size = *(header);
	printf("%i\n",size);
	return size;
}


int getHeaderNext (void *v) {
	int next = 0;
	int* header = (int*)v;// typecast
	header = header - 2;
	next = *(header+1);
	printf("%i\n",next);
	return next;
}

int closestPower2 (size_t requested) {
	double requestedTemp = (double)requested;
	double toAllocate = 0;
	requestedTemp += 8;

	toAllocate = (ceil(log2(requestedTemp)));
	toAllocate = pow (2, toAllocate);

	return (int)toAllocate;
}

