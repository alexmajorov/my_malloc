#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

typedef enum
{
    HEAP_SUCCESS,
    HEAP_FAILURE,
} heap_e;

struct heapchunk_t 
{
    size_t size;
    bool inuse;
    struct heapchunk_t *next;
} __attribute__((aligned(16)));

struct heapinfo_t
{
    struct heapchunk_t *start;
    size_t avail;
};

// todo: make thread safe with mutex
struct heapinfo_t heap = {0, 0};

heap_e init_heap(struct heapinfo_t *heap)
{
    //alloc memory from kernel for heap
    void *start = mmap(
        NULL
        , getpagesize()
        , PROT_READ | PROT_WRITE
        , MAP_ANONYMOUS | MAP_PRIVATE
        , -1
        , 0
        );

    //create wilderness chunk for heap
    if (start == (void *)(-1))
    {
        perror("mmap");
        return HEAP_FAILURE;
    }

    printf("     Mmaped at %p\n", start);

    //wilderness chunk is the whole size, minus metadata length
    struct heapchunk_t *first = (struct heapchunk_t *)(start);
    first->size = getpagesize() - sizeof(struct heapchunk_t);
    first->inuse = false;
    //first->next = first;
    first->next = NULL;

    //track the chuck in heap metadata
    heap->start = first;
    heap->avail = first->size;
    
    return HEAP_SUCCESS;
}

void *heap_alloc(size_t size)
{
    //round up the size
    size = (size + 15) & ~0xf;


    // do we have enough memory in the heap?
    if (size > heap.avail)
    {
        printf("oops...\n");
        return (void *)(-1);
    }

    struct heapchunk_t *chunk = heap.start;
    //find the (largest?) chunk available
    while (chunk != NULL && size > chunk->size)
    {
        chunk = chunk->next;
        return (void *)(-1);
    }

    if (chunk == NULL)
    {
        printf("We shouldnt be here by any means..... ((\n");
    }

    printf("Found chunk at %p\n", (void *)chunk);

    chunk->size = size;
    chunk->inuse = true;

    void *next = ((char *)chunk) + size;
    heap.start = (struct heapchunk_t *)next;

    //fix metadata
    return (void *)((char *)chunk + sizeof(struct heapchunk_t));
}

heap_e heap_free(void *data)
{
    struct heapchunk_t *chunk = &((struct heapchunk_t *)(data))[-1];
    printf("chunk->size: %zu\n", chunk->size);


    //add freed chunk to the top of the linked list
    struct heapchunk_t *oldfirst = heap.start;

    heap.start = chunk;
    chunk->next = oldfirst;

    //set inuse to false
    chunk->inuse = false;

    //todo: logic for coalescing 

    return HEAP_SUCCESS;
}

int main(int argc, char *argv[])
{
    if (init_heap(&heap) == HEAP_FAILURE)
    {
        printf("Failed to init heap\n");
        return -1;
    }

    char *mystr1 = (char *)heap_alloc(31);
    strncpy(mystr1, "asdfghjkl", 10);
    printf("mystr1 : %s\n", mystr1);    
    heap_free(mystr1);

    char *mystr2 = (char *)heap_alloc(31);
    strncpy(mystr2, "lkjhgfdsa", 10);
    printf("mystr2 : %s\n", mystr2);  
    
    heap_free(mystr2);

    return 0;
}