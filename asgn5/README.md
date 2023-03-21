#Assignment 5 directory

This directory contains source code and other files for Assignment 5.

Use this README document to store notes about design, testing, and
questions you have while developing your assignment.

It looks like the code is implementing a cache data structure with three different replacement policies: FIFO, LRU, and clock. The cache structure contains a size and a count field, as well as a pointer to a linked list data field that stores the actual cache entries.

The cache can be initialized using the new_cache function, which takes a size parameter and returns a pointer to a newly allocated cache structure. The free_cache function is provided to free the memory allocated for the cache.

To insert an item into the cache, one of the three insertion functions (insert_fifo, insert_lru, or insert_clock) can be used. These functions take a cache pointer and an item to insert as parameters, and return a HIT or MISS status code indicating whether the item was already present in the cache or not.

The insert_fifo function implements the first-in, first-out policy by removing the oldest entry from the cache when it is full. The insert_lru function implements the least recently used policy by removing the least recently used entry when the cache is full. The insert_clock function implements the clock policy, which uses a clock hand to keep track of the last access time of each entry and evicts the least recently used entry when the cache is full.

Overall, this code provides a basic implementation of a cache data structure with several common replacement policies. However, there are some issues with the code, such as the use of undefined variables (capacity, compulsory_miss_count) in the insert_fifo function and incomplete code in the insert_clock function.
