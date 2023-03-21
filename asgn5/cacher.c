#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

// Define a struct to hold the cache items
typedef struct CacheItem {
    char* data;                 // data stored in the cache item
    int ref_bit;                // for clock eviction policy
    struct CacheItem* next;     // pointer to the next cache item
} CacheItem;

// Define a struct to hold the cache
typedef struct Cache {
    int size;                   // maximum size of the cache
    int num_items;              // current number of items in the cache
    int compulsory_misses;      // count of compulsory misses
    int capacity_misses;        // count of capacity misses
    CacheItem* head;            // pointer to the first item in the cache
    CacheItem* tail;            // pointer to the last item in the cache
    int clock_hand;             // for clock eviction policy
} Cache;

// Function declarations
Cache* create_cache(int size);
void destroy_cache(Cache* cache);
void print_summary(Cache* cache);
char* read_line();
int contains(CacheItem* item, char* data);
CacheItem* remove_front(CacheItem* head);
CacheItem* remove_first(CacheItem* head, char* data);
CacheItem* append(CacheItem* head, char* data);
int insert_fifo(Cache* cache, char* item_data);
int insert_lru(Cache* cache, char* item_data);
int insert_clock(Cache* cache, char* item_data);

int main(int argc, char* argv[]) {
    // Default values for options
    int size = 0;
    char* policy = "-F";

    // Process command line arguments
    int opt;
    while ((opt = getopt(argc, argv, "-N:LFC")) != -1) {
        switch (opt) {
            case 'N':
                size = atoi(optarg);
                break;
            case 'L': policy = "-L"; break;
            case 'F': policy = "-F"; break;
            case 'C': policy = "-C"; break;
            default:
                fprintf(stderr, "Error: Invalid command line arguments\n");
                return 1;
        }
    }
	
    // Create the cache
    Cache* cache = create_cache(size);

    // Read input and process each line
    char* line = read_line();
    if (line != NULL) {
    	char* item = strtok(line, "\n");
    	while (item != NULL) {
		    int result;
		    if (strcmp(policy, "-L") == 0) {
		        result = insert_lru(cache, item);
		    } else if (strcmp(policy, "-C") == 0) {
		        result = insert_clock(cache, item);
		    } else {
		        result = insert_fifo(cache, item);
		    }
		    if (result == 0) {
		        printf("HIT\n");
		    } else {
		        printf("MISS\n");
		    }
		    item = strtok(NULL, "\n");
		}
    }

    // Print summary and clean up
    print_summary(cache);
    destroy_cache(cache);
    return 0;
}

// Function to create a new cache
Cache* create_cache(int size) {
    Cache* cache = malloc(sizeof(Cache));
    cache->size = size;
    cache->num_items = 0;
    cache->compulsory_misses = 0;
    cache->capacity_misses = 0;
    cache->head = NULL;
    cache->tail = NULL;
    cache->clock_hand = 0;
    return cache;
}

// Function to free the memory used by a cache
void destroy_cache(Cache* cache) {
    CacheItem* current_item = cache->head;
    while (current_item != NULL) {
        CacheItem* next_item = current_item->next;
        free(current_item);
        current_item = next_item;
    }
    free(cache);
}

// Function to print a summary of the cache's performance
void print_summary(Cache* cache) {
    printf("Compulsory misses: %d\n", cache->compulsory_misses);
    printf("Capacity misses: %d\n", cache->capacity_misses);
}

// Function to read a line of input from stdin
char* read_line() {
    // Read a line from stdin and return it as a string
    int buffer_size = 1024;
    char *buffer = malloc(buffer_size);
    int c;
    int i = 0;

    while ((c = getchar()) != '\n' && c != EOF) {
        if (i >= buffer_size - 1) {
            buffer_size *= 2;
            buffer = realloc(buffer, buffer_size);
        }
        buffer[i++] = c;
    }
    if (i == 0 && c == EOF) {
        free(buffer);
        return NULL;
    }

    buffer[i] = '\0';
    return buffer;
}

// Function to check if a cache item contains a given string
int contains(CacheItem* item, char* data) {
    return (strcmp(item->data, data) == 0);
}

// Function to remove the front item from a linked list
CacheItem* remove_front(CacheItem* head) {
    if (head == NULL) {
        return NULL;
    }
    CacheItem* new_head = head->next;
    free(head);
    return new_head;
}

// Function to remove the first occurrence of a given string from a linked list
CacheItem* remove_first(CacheItem* head, char* data) {
    if (head == NULL) {
        return NULL;
    }
    if (contains(head, data)) {
        return remove_front(head);
    }
    CacheItem* prev_item = head;
    CacheItem* current_item = head->next;
    while (current_item != NULL) {
        if (contains(current_item, data)) {
            prev_item->next = current_item->next;
            free(current_item);
            return head;
        }
        prev_item = current_item;
        current_item = current_item->next;
    }
    return head;
}

// Function to append a new item to the end of a linked list
CacheItem* append(CacheItem* head, char* data) {
    CacheItem* new_item = malloc(sizeof(CacheItem));
    new_item->data = strdup(data);
    new_item->ref_bit = 0;
    new_item->next = NULL;
    if (head == NULL) {
        return new_item;
    }
    CacheItem* current_item = head;
    while (current_item->next != NULL) {
        current_item = current_item->next;
    }
    current_item->next = new_item;
    return head;
}

// Function to insert an item using the FIFO policy
int insert_fifo(Cache* cache, char* item_data) {
    CacheItem* current_item = cache->head;
    while (current_item != NULL) {
        if (contains(current_item, item_data)) {
            return 0; //Hit
        }
        current_item = current_item->next;
    }
    cache->num_items++;
    if (cache->num_items > cache->size) {
        cache->capacity_misses++;
        cache->head = remove_front(cache->head);
        cache->num_items--;
    }
    cache->head = append(cache->head, item_data);
    if (cache->num_items == 1) {
        cache->tail = cache->head;
    }
    return 1;
}

// Function to insert an item using the LRU policy
int insert_lru(Cache* cache, char* item_data) {
    CacheItem *current_item = cache->head;
    CacheItem *prev_item = NULL;
    while (current_item != NULL) {
        if (contains(current_item, item_data)) {
// Item is already in cache, move it to the front
            if (prev_item != NULL) {
                prev_item->next = current_item->next;
                current_item->next = cache->head;
                cache->head = current_item;
            }
            return 0; // Hit
        }
        prev_item = current_item;
        current_item = current_item->next;
    }
    // Item is not in cache, add it to the front
    CacheItem *new_item = malloc(sizeof(CacheItem));
    new_item->data = item_data;
    new_item->ref_bit = cache->num_items;
    new_item->next = cache->head;
    cache->head = new_item;
    cache->num_items++;

// If cache is full, remove the last item
    if (cache->num_items > cache->size) {
        CacheItem *last_item = cache->head;
        while (last_item->next != NULL) {
            last_item = last_item->next;
        }
        if (last_item == cache->tail) {
            cache->tail = prev_item;
        }
        prev_item->next = NULL;
        free(last_item);
        cache->num_items--;
        cache->capacity_misses++;
    } else {
        cache->compulsory_misses++;
    }

    return 1;  // Miss
}

// Function to insert an item using the clock policy
int insert_clock(Cache* cache, char* item_data) {
    CacheItem* current_item = cache->head;
    CacheItem* prev_item = NULL;
    while (current_item != NULL) {
        if (contains(current_item, item_data)) {
// Item is already in cache, set its reference bit to 1
            current_item->ref_bit = 1;
            return 0; // Hit
        }
        if (current_item->ref_bit == 0) {
// Item has not been referenced recently, replace it
            current_item->data = item_data;
            current_item->ref_bit = 1;
            cache->clock_hand = (cache->clock_hand + 1) % cache->size;
            return 1; // Miss
        }
        prev_item = current_item;
        current_item = current_item->next;
// If the clock hand makes a full cycle, reset all reference bits to 0
        if (current_item == NULL) {
            current_item = cache->head;
            while (current_item != NULL) {
                current_item->ref_bit = 0;
                current_item = current_item->next;
            }
        }
    }
    // Item is not in cache and there is an empty slot, add it to the current position of the clock hand
    if (cache->num_items < cache->size) {
        CacheItem* new_item = malloc(sizeof(CacheItem));
        new_item->data = item_data;
        new_item->ref_bit = 1;
        if (prev_item == NULL) {
            cache->head = new_item;
        } else {
            prev_item->next = new_item;
        }
        new_item->next = current_item;
        cache->num_items++;
        return 1;  // Miss
    }

// Item is not in cache and all slots are full, replace the first item with ref_bit = 0 after the clock hand
    current_item = cache->head;
    prev_item = NULL;
    while (current_item != NULL) {
        if (current_item->ref_bit == 0 && contains(current_item, item_data) == 0) {
            CacheItem* new_item = malloc(sizeof(CacheItem));
            new_item->data = item_data;
            new_item->ref_bit = 1;
            if (prev_item == NULL) { // If the item to be replaced is the head
                new_item->next = cache->head->next;
                cache->head = new_item;
            } else { // If the item to be replaced is not the head
                new_item->next = current_item->next;
                prev_item->next = new_item;
            }
            free(current_item->data);
            free(current_item);
            return 1;
        }
        current_item->ref_bit = 0;
        prev_item = current_item;
        current_item = current_item->next;
    }
// If no item can be replaced, replace the first item with ref_bit = 0 after the clock hand
    cache->clock_hand = (cache->clock_hand + 1) % cache->size;
    current_item = cache->head;
    prev_item = NULL;
    while (current_item != NULL) {
        if (current_item->ref_bit == 0 && contains(current_item, item_data) == 0) {
            CacheItem* new_item = malloc(sizeof(CacheItem));
            new_item->data = item_data;
            new_item->ref_bit = 1;
            if (prev_item == NULL) { // If the item to be replaced is the head
                new_item->next = cache->head->next;
                cache->head = new_item;
            } else { // If the item to be replaced is not the head
                new_item->next = current_item->next;
                prev_item->next = new_item;
            }
            free(current_item->data);
            free(current_item);
            return 1;
        }
        current_item->ref_bit = 0;
        prev_item = current_item;
        current_item = current_item->next;
    }
// If no item can be replaced, return 0 indicating a capacity miss
    cache->capacity_misses++;
    return 0;
}
