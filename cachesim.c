/* CS 211 PA5
*/

//CS211 - Computer Architecture - Spring 2020.
//Abhay Saxena
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int reads = 0;
int writes = 0;
int hits = 0;
int misses = 0;
int dim_Set = 0;
int dim_Block = 0;
int sizeOfCache = 0;
int blockWidth = 0;

long int offset = 0;
long int bits_S = 0;
long int mask = 0;

char operation;
char check;
char redundant;
long int redundant2;
long int address;

struct cachetype {
	long int tagbits;
	struct cachetype *next;
};

struct cachetype** prefetchCache;
struct cachetype** nonprefetchCache;

void reset();
void operateWithoutPrefetch(char operation, struct cachetype** cache);
void operateWithPrefetch(char operation, struct cachetype** cache);
void prefetching(struct cachetype** cache);

void reset(){
	reads = 0;
	writes = 0;
	hits = 0;
	misses = 0;
}

void operateWithoutPrefetch(char operation, struct cachetype** cache){

	long int target_Tag= (address >> bits_S) >> offset;
	int setNum = (address >> offset) & mask;
	
	struct cachetype *prev = NULL;
	struct cachetype *curr = cache[setNum];
	
	//If set is not empty.
	if (curr != NULL){
	
	//Search for block.
	int iteration = 0, callPrefetch = 1;
	for (int iteration = 0; iteration != dim_Block && curr != NULL && (callPrefetch); iteration++) {
		if (curr->tagbits == target_Tag){
			writes = (operation == 'W') ? writes + 1 : writes;
			hits++;
			callPrefetch = 0;
			break;
		}
		
		if (curr->next != NULL){ // Move forward
			prev = curr;
			curr = curr->next;
		}
		else {
			iteration++;
			break;
		}

	}
	
	if (!callPrefetch) return;
	
	//Cache miss.
	struct cachetype* newBlock = malloc(sizeof(struct cachetype));
	newBlock->tagbits = target_Tag;
	
	//Set not full.
	if (iteration != dim_Block){
		newBlock->next = cache[setNum];
		cache[setNum] = newBlock;
	
		writes = (operation == 'W') ? writes + 1 : writes;
		reads++;
		misses++;
	} else {
		prev->next = NULL;
		newBlock->next = cache[setNum];
		cache[setNum] = newBlock;
		if (operation == 'W'){
			writes++;
		}
		reads++;
		misses++;
			return;
	}

	return;
	}
	
	else {
		//Set is empty.	
		struct cachetype* newBlock = malloc(sizeof(struct cachetype));
		newBlock->tagbits = target_Tag;
		cache[setNum] = newBlock;
		newBlock->next = NULL;
		
		
		writes = (operation == 'W') ? writes + 1 : writes;
		reads++;
		misses++;
		
		return;
		}
}

void operateWithPrefetch(char operation, struct cachetype** cache){

	long int target_Tag= (address >> bits_S) >> offset;
	int setNum = (address >> offset) & mask;
	
	struct cachetype *prev = NULL;
	struct cachetype *curr = cache[setNum];
	
	//If set is not empty.
	if (curr != NULL){
	
	//Search for block.
	int iteration = 0, callPrefetch = 1, breaker = 1;
	for (int iteration = 0; iteration != dim_Block && curr != NULL && (callPrefetch) && (breaker); iteration++) {
		if (curr->tagbits == target_Tag){
			writes = (operation == 'W') ? writes + 1 : writes;
			hits++;
			callPrefetch = 0;
			breaker = 1;
			break;
		}
		
		if (curr->next != NULL){ // Move forward
			prev = curr;
			curr = curr->next;
		}
		else {
			iteration++;
			breaker = 1;
			break;
		}

	}
	
	if (!callPrefetch) return;
	if (!breaker) return;
	
	//Cache miss.
	struct cachetype* newBlock = malloc(sizeof(struct cachetype));
	newBlock->tagbits = target_Tag;
	
	//If set is not full.
	if (iteration != dim_Block){
		newBlock->next = cache[setNum];
		cache[setNum] = newBlock;
		prefetching(cache);
	
		writes = (operation == 'W') ? writes + 1 : writes;
		misses++;
		reads++;
	} else {
		prev->next = NULL;
		newBlock->next = cache[setNum];
		cache[setNum] = newBlock;
		prefetching(cache);
		if (operation == 'W'){
			writes++;
		}
		misses++;
		reads++;
			return;
	}

	return;
	}
	
	else {
		//Set is empty.	
		struct cachetype* newBlock = malloc(sizeof(struct cachetype));
		newBlock->tagbits = target_Tag;
		cache[setNum] = newBlock;
		newBlock->next = NULL;
		prefetching(cache);
		
		writes = (operation == 'W') ? writes + 1 : writes;
		misses++;
		reads++;
		
		return;
		}
}

void prefetching(struct cachetype** cache){

	long int prefetchAddress = address + blockWidth;
	int set = (prefetchAddress >> offset) & mask;
	long int newTag = (prefetchAddress >> offset) >> bits_S;
	
	struct cachetype* curr = cache[set];
	struct cachetype* prev = NULL;
	
	//Search if block is present.
	int iteration = 0, tracker = 0, hit = 0;
	for (int iteration = 0; iteration != dim_Block && curr != NULL; iteration++) {
	if (curr->tagbits == newTag) {
		hit = 1;
		break;
	}
	if (curr->next != NULL){
		prev = curr;
		curr = curr->next;
		tracker++;
	}
	else {
		iteration++;
		break;
	}
	}

	if (hit) return;
      
        struct cachetype* newBlock = malloc(sizeof(struct cachetype));
	newBlock->tagbits = newTag;
	
	if(iteration != dim_Block){ 
		newBlock->next=cache[set];
		cache[set]=newBlock;
		reads++;
		return;
	} else {
		prev->next=NULL;
		newBlock->next=cache[set];
		cache[set]=newBlock;
		reads++;
		return;
	
	}
}

int main(int argc, char **argv)
{
    int n_assoc = 0;
    
    //Argument initialization
    sizeOfCache = atoi(argv[1]);
    //argv[2] = Assosciativity
    //argv[3] = Replacement Policy
    blockWidth = atoi(argv[4]);
    //argv[5] = File Trace 
    
    //Check for Cache Size.
    if (sizeOfCache == 0){
    	return 0;
    }
    
    int test = sizeOfCache;
    while (test != 1){
    	if (test % 2 != 0){
    		return 0;
    	} else {
    		test = test/2;
    	}
    }
    
    //Check for Block Size.
    if (blockWidth == 0){
    	return 0;
    }
    
    test = blockWidth;
    while (test != 1){
    	if (test % 2 != 0){
    		return 0;
    	} else {
    		test = test/2;
    	}
    }
    
    //Check associativity
    if (strcmp(argv[2], "direct") == 0){
    	dim_Set = (sizeOfCache/blockWidth);	//C = SAB where A = 1 (Direct Associativity) -> One Cache Line per set
    	dim_Block = 1;				
    } else if (argv[2][5] != ':'){	//Condition for Fully Assosciative Cache (All cache lines in single set)
    	dim_Set = 1;
    	dim_Block = (sizeOfCache/blockWidth);
    } else { 				//n:Assosciativity -> n cache lines per set
    	sscanf(argv[2], "assoc:%d", &n_assoc);
    	//Check for n:assosciativity.
    	test = n_assoc;
    	while (test != 1){
    		if (test % 2 != 0){
    			return 0;
    		} else {
    			test = test/2;
    		}
   	 }
	dim_Set = (sizeOfCache/(n_assoc*blockWidth));
	dim_Block = n_assoc;
    }
    
    //Initialize two seperate caches
    prefetchCache = malloc(dim_Set*sizeof(struct cachetype*));
    nonprefetchCache = malloc(dim_Set*sizeof(struct cachetype*));
    int i = 0;
    while (i < dim_Set) {
    	prefetchCache[i] = malloc(sizeof(struct cachetype));
    	nonprefetchCache[i] = malloc(sizeof(struct cachetype));
    	prefetchCache[i] = NULL; nonprefetchCache[i] = NULL;
    	i++;
    }
    
    //Initialize required offsetsets and masks
    bits_S = log(dim_Set)/(log(2));
    mask = (1 << bits_S) - 1;
    offset = log(blockWidth)/(log(2));
    
    /*
    Print to check.
    printf("%lu\n", offset);
    printf("%lx\n", bits_S);
    printf("%lx\n", mask);
    */

    
    //File pointer to trace file
    FILE *ptr = fopen(argv[5], "r");
        
    while (fscanf(ptr, "%c", &check) != EOF && check != '#'){
	    if (check == '0'){
	    	fscanf(ptr, "%c %lx : %c %lx\n", &redundant, &redundant2, &operation, &address);
	    	//printf("%c %lx\n", operation, address);
	    	//printf("%c %ld\n", operation, address);
	    	operateWithoutPrefetch(operation, nonprefetchCache);
	    }
    }
    
	printf("Prefetch 0\n");	//Prefetch is offset/not active. This prints the values for the non-prefetch cache.
	printf("Memory reads: %d\n", reads);
	printf("Memory writes: %d\n", writes);
	printf("Cache hits: %d\n", hits);
	printf("Cache misses: %d\n", misses);
    
    fclose(ptr);
    
    reset();
    
    ptr = fopen(argv[5], "r");
    
  while (fscanf(ptr, "%c", &check) != EOF && check != '#'){
	    if (check == '0'){
	    	fscanf(ptr, "%c %lx : %c %lx\n", &redundant, &redundant2, &operation, &address);
	    	//printf("%c %lx\n", operation, address);
	    	operateWithPrefetch(operation, prefetchCache);
	    }
    }
    
	printf("Prefetch 1\n");	//Prefetch is offset
	printf("Memory reads: %d\n", reads);
	printf("Memory writes: %d\n", writes);
	printf("Cache hits: %d\n", hits);
	printf("Cache misses: %d\n", misses);
    
    fclose(ptr);
    
    //Remember to free caches!
    
    return EXIT_SUCCESS;
}
