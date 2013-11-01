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
void xfree(void *memory_block);
void dump_memory_map(void);
int closestPower2 (size_t requested);
uint64_t diff (void *a, void *b);
int areBuddies (uint64_t diff);
void setHeader (void *v, int size, int next);
int needCoalesce(int *free_temp);
int getHeaderSize (void *v);
int getHeaderNext (void *v);
void printFreeBlock(int size,int next);
void printAllocatedBlock(int size);
void coal(int *free_temp);
void *firstFreeBlock(void *free_list_local, int amountToAllocate);
void *splitBlock(int *free_list_temp, int amountToAllocate);


const int HEAPSIZE = (1*128); // 1 MB
const int MINIMUM_ALLOC = sizeof(int) * 2;
int sizeOfLastFreeBlock = 0;

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
	void *returnPointer = NULL;
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


	
		
	amountToAllocate = closestPower2(request_size);
	printf("%s", "ammount to allocate:   ");
	printf("%i\n",amountToAllocate);

	temp = firstFreeBlock(free_list, amountToAllocate);


	printf("%s","first block of free list:    ");
	printf("%i\n",free_list);

	returnPointer = splitBlock(temp, amountToAllocate);
	return returnPointer;

}

/*
NEED TO UPDATE THE FREE LIST.
CURRENTLY ONLY UPDATING THE HEADER


NEED TO ADD COALESCING
*/
void xfree(void *memory_block) {
	int* memory_block_temp = (int*)(memory_block);
	void* startOfMem = memory_block_temp-2;
	int differ = memory_block-free_list;
	int* free_list_temp = (int*) free_list;	

	if (free_list>=memory_block) //Memory block is before head of free list
	{	
		differ=differ*(-1);		
		differ+=8;				//Because memory_block starts at user bytes
		*(memory_block_temp-1) = differ;  //Next of memory block first element of old free list
		free_list = startOfMem;  //Set newly freed block to front of free list
	}
	else{	//Memory block is after first element of free list
		differ-=8;
		while (differ-*(free_list_temp+1)>0){	//while loop to find the closest free block
			differ -= *(free_list_temp+1);
			free_list_temp= free_list_temp+(*(free_list_temp+1)/4); //iterate to next free block
		}	
		*(memory_block_temp-1) = *(free_list_temp+1)-differ;	//Change info of free block
		*(free_list_temp+1)=differ;		//Set last free block's next to current freed block
	}

	free_list_temp = free_list;
	coalesce(free_list_temp);
	coalesce(free_list_temp);
	//while(needCoalesce(free_list_temp)){
		//coalesce(free_list_temp);
		//free_list_temp = free_list;
	//
	//}
}


void *splitBlock(int *free_list_temp, int amountToAllocate) {
	printf("%s\n","SPLIT BLOCK");
	int flag = 0;
	void* tempNext = free_list_temp;
	void* free_list_temp2 = (free_list_temp+(*(free_list_temp+1)/4));
	void* free_list_temp3 = (free_list_temp-(sizeOfLastFreeBlock/4)+2);
	void* free_list_temp4 = (free_list_temp-(sizeOfLastFreeBlock/4));
	int difference = 0;

	while (*(free_list_temp)>amountToAllocate) {

		tempNext = free_list_temp;
		free_list_temp2 = free_list_temp;
		if (*(free_list_temp+1) == 0) { //if at end of list
			flag = 1;
		}
		//update header of first
		*(free_list_temp) = *(free_list_temp)/2;
		printf("%s", "new first header size:   ");
		printf("%i\n",*(free_list_temp));	
		*(free_list_temp+1) =  *(free_list_temp); //zero because you are allocating
		printf("%s", "new first header next:   ");
		printf("%i\n",*(free_list_temp+1));


		//update header of second
		*(free_list_temp + (*free_list_temp/4)) = *(free_list_temp);
		printf("%s", "new second header size:   ");
		printf("%i\n",*(free_list_temp + (*free_list_temp/4)));

		free_list_temp2 = (free_list_temp+(*(free_list_temp+1)/4));
		if (flag) {
			difference = 0;
		}
		else {
			//find difference for offset of second block
			difference = diff(tempNext,free_list_temp2);
		}

		*(free_list_temp + (*free_list_temp/4)+1) = difference;
		printf("%s", "new second header next:   ");
		printf("%i\n",*(free_list_temp + (*free_list_temp/4)+1));
		printf("%s\n","-----next cut in half-------");

		difference = 0;
		flag = 0;
	}
	setHeader(free_list_temp4,getHeaderSize(free_list_temp3),getHeaderNext(free_list_temp3)+*(free_list_temp+1));//Go back to last free block and update its next to account for the newly allocated block
	setHeader(free_list_temp,*(free_list_temp),0);//set next to 0 to indicate allocated
	if (sizeOfLastFreeBlock==0){	
		free_list = free_list_temp2; //update first block of free list if it was allocate, else, leave it
	}
	return free_list_temp+2;
}


void *firstFreeBlock(void *free_list_local, int amountToAllocate) {
	int* free_list_temp = (int*)free_list_local;
	sizeOfLastFreeBlock = 0;
	while (*(free_list_temp+1)!=0) {
		if (amountToAllocate<=*(free_list_temp)) {
			printf("%s","Size of last free block:  ");
			printf("%i\n",sizeOfLastFreeBlock);
			return free_list_temp;
		}
		sizeOfLastFreeBlock = *(free_list_temp+1);	//Takes the size of the LAST FREE BLOCK'S NEXT to know how far to go back when updating
		free_list_temp = free_list_temp + (*(free_list_temp+1)/4);
	}

	//for last element	
	if (amountToAllocate<=*(free_list_temp)) {
		printf("%s","Size of last free block:  ");
		printf("%i\n",sizeOfLastFreeBlock);
		return free_list_temp;
	}
	return NULL;
}


void dump_memory_map(void) {
		printf("%s\n","-----------------------");
		int* free_list_local = (int*)(free_list);
		void* end_of_free_list;
		int last = 0;
		if(free_list>heap_begin){ //If first block is allocated, calculates how much of first chunk
			printAllocatedBlock(free_list-heap_begin);
		}
		while (*(free_list_local+1)!=0) {
			printFreeBlock(*(free_list_local),*(free_list_local+1));
			last = *(free_list_local+1)-*(free_list_local);
			if (last!=0){ //In case you have two free blocks next to each who aren't buddies
				printAllocatedBlock(last);
			}
			free_list_local = free_list_local + (*(free_list_local+1)/4);
		}
	
		//print last free block
		printFreeBlock(*(free_list_local),*(free_list_local+1));
		end_of_free_list = free_list_local+(*(free_list_local)/4);
		if(heap_begin+HEAPSIZE>end_of_free_list)
			printAllocatedBlock((heap_begin+HEAPSIZE)-end_of_free_list);
		printf("%s\n","-----------------------");

}


//check the difference between two header locations
uint64_t diff (void *a, void *b) {
	//b-a
	int bitDiff = 0;
	uint64_t aConverted = (uint64_t*)a; //typecast
	uint64_t bConverted = (uint64_t*)b; //typecast
	uint64_t diffConverted = bConverted^aConverted; //calc. diff
	return diffConverted;
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

//Given Left Block of two buddies, merges the two
void mergeBlocks(int *x){
	int oldNext = *(x-1+(*(x-2)/4));
	if (oldNext==0)
		*(x-1) = 0;
	else
		*(x-1)+= oldNext;
	*(x-2) = *(x-2)*2;
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
	//printf("%i\n",size);
	return size;
}

void printFreeBlock(int size,int next){
	printf("%s","Block Size:  ");
	printf("%i",size);
	printf("%s",",  offset:  ");
	printf("%i",next);
	printf("%s\n",",  free");
}

void printAllocatedBlock(int size){
	printf("%s","Block Size:  ");
	printf("%i",size);
	printf("%s",",  offset:  0");
	printf("%s\n",",  allocated");
}

/*
int needCoalesce(int *free_temp){
	void* first_buddy;
	void* second_buddy;
	while(*(free_temp+1)!=0){
		printf("
		if(*(free_temp)==*(free_temp+1)){
			first_buddy = free_temp+2;
			second_buddy = free_temp+(*(free_temp)/4)+2;
			if(areBuddies(diff(first_buddy,second_buddy)))
				return 1;
		}
		free_temp = free_temp+(*(free_temp+1)/4);
	}
	return 0;	
}
*/
void coalesce(int *free_temp){
	void* first_buddy;			//used for diff function later
	void* second_buddy;
	while(*(free_temp+1)!=0){	//loop to coalesce any bud to right
		while(*(free_temp)==*(free_temp+1) && *(free_temp)==*(free_temp+*(free_temp)/4)){
			first_buddy = free_temp+2;	//conditions in place in case areBuddies doesn't work
			second_buddy = free_temp+((*(free_temp)/4)+2);
			if(areBuddies(diff(first_buddy,second_buddy))){
				mergeBlocks(free_temp+2);
			}
		}
		free_temp = free_temp+(*(free_temp+1)/4);
	}
}

int getHeaderNext (void *v) {
	int next = 0;
	int* header = (int*)v;// typecast
	header = header - 2;
	next = *(header+1);
	//printf("%i\n",next);
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

