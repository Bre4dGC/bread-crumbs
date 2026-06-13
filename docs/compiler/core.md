# Core - Documentation

## Data Structures

### Arena Allocator

```c
#define ARENA_DEF_SIZE (1024) // Default size for arena blocks, used when creating new blocks if the current block does not have enough space for an allocation.

#define ARENA_BIG_SIZE (1024 * 1024) // Threshold size for big allocations. If an allocation request exceeds this size, it will be allocated in a separate block to avoid fragmentation of the main arena blocks.

#define ARENA_PERM_SIZE (64 * 1024) // Size for permanent arena blocks, which are used for allocations that are expected to last for the duration of the program. This helps to reduce fragmentation and improve performance for long-lived allocations.

#define ARENA_TEMP_SIZE (32 * 1024) // Size for temporary arena blocks, which are used for short-lived allocations that can be freed after use. This allows for efficient memory management for transient data.

#define ARENA_PHASE_SIZE (16 * 1024) // Size for phase-specific arena blocks, which are used for allocations that are specific to a particular phase of the compilation process. This allows for efficient memory management and cleanup after the phase is complete.

struct arena_block_t {
    unsigned char* data; // Pointer to the memory block used for allocations. This is where the actual data for the arena is stored.

    size_t offset; // Current offset within the block, indicating where the next allocation will occur. This is updated as allocations are made to keep track of used space.

    size_t capacity; // Total capacity of the block, indicating how much memory is available for allocations. This is used to determine when a new block needs to be allocated if the current block does not have enough space.

    struct arena_block* next; // Pointer to the next block in the arena, allowing for a linked list of blocks to manage multiple allocations. This is used when the current block is full and a new block needs to be allocated to continue providing memory for the arena.
}

struct arena_t {
    arena_block_t* head;    // Pointer to the first block in the arena, which is used for allocations. This is the starting point for managing memory in the arena.
    arena_block_t* current; // Pointer to the current block being used for allocations. This is updated as allocations are made and blocks are filled, allowing for efficient management of memory within the arena.
}

arena_t* new_arena(size_t size); // Creates a new arena with an initial block of the specified size. This function initializes the arena structure and allocates the first block for use.

void free_arena(arena_t* arena); // Frees all memory associated with the arena, including all blocks. This function traverses the linked list of blocks and deallocates each one, as well as the arena structure itself.

bool arena_expand(arena_t* arena, size_t capacity); // Expands the arena by allocating a new block with the specified capacity. This function is called when the current block does not have enough space for a new allocation, and it adds a new block to the linked list of blocks in the arena.

bool arena_has_space(arena_t* arena, size_t size, size_t align); // Checks if the current block in the arena has enough space for an allocation of the specified size and alignment. This function calculates the required space based on the size and alignment, and compares it to the available space in the current block to determine if an allocation can be made without needing to expand the arena.

void* arena_alloc(arena_t* arena, size_t size, size_t align); // Allocates memory from the arena with the specified size and alignment. This function first checks if there is enough space in the current block using arena_has_space, and if not, it calls arena_expand to create a new block. Once there is enough space, it calculates the aligned offset for the allocation, updates the offset in the current block, and returns a pointer to the allocated memory.

void* arena_alloc_default(arena_t* arena, size_t size); // Allocates memory from the arena with the specified size and a default alignment (typically the alignment of the largest basic type). This function is a convenience wrapper around arena_alloc that uses a predefined default alignment.

void* arena_alloc_array(arena_t* arena, size_t element_size, size_t count, size_t align); // Allocates an array of elements from the arena with the specified element size, count, and alignment. This function calculates the total size needed for the array based on the element size and count, and then calls arena_alloc to perform the allocation with the appropriate alignment.

void arena_clear(arena_t* arena);   // Clears the arena by resetting the offset of all blocks to zero, effectively marking all allocated memory as free without actually deallocating it. This allows for efficient reuse of memory in the arena without the overhead of deallocating and reallocating blocks.
size_t arena_used(arena_t* arena); // Returns the total amount of memory currently used in the arena by summing the offsets of all blocks. This function provides insight into how much memory has been allocated from the arena, which can be useful for debugging and performance analysis.
size_t arena_capacity(arena_t* arena); // Returns the total capacity of the arena by summing the capacities of all blocks. This function provides insight into how much memory is available for allocation in the arena, which can be useful for debugging and performance analysis.
```

### Hash Map

### String Pool

## Language Utilities

### File System

### Source Management

### Error Reporting

### Debug Utilities

### Resolution

## Platform Specifics

### Unix

### Windows