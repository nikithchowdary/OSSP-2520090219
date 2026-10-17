
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TOTAL_MEMORY 1024

typedef struct Block {
    int start;
    int size;
    int pid;                 /* 0 means free */
    struct Block *next;
} Block;

Block *head = NULL;
int next_pid = 1;
int allocations = 0, failures = 0, searches = 0;
int strategy = 1;            /* 1 First Fit, 2 Best Fit, 3 Worst Fit */

Block *new_block(int start, int size, int pid) {
    Block *b = malloc(sizeof(Block));
    if (!b) {
        printf("Error: memory allocation failed!\n");
        exit(EXIT_FAILURE);
    }
    b->start = start;
    b->size = size;
    b->pid = pid;
    b->next = NULL;
    return b;
}

void initialize(void) {
    head = new_block(0, TOTAL_MEMORY, 0);
}

const char *strategy_name(void) {
    if (strategy == 1) return "First Fit";
    if (strategy == 2) return "Best Fit";
    return "Worst Fit";
}

void choose_strategy(void) {
    int choice;
    printf("\n1. First Fit\n2. Best Fit\n3. Worst Fit\n");
    printf("Choose strategy: ");
    if (scanf("%d", &choice) != 1) {
        printf("Invalid input.\n");
        exit(EXIT_FAILURE);
    }
    if (choice >= 1 && choice <= 3) {
        strategy = choice;
        printf("Selected: %s\n", strategy_name());
    } else {
        printf("Invalid strategy.\n");
    }
}

void allocate_memory(void) {
    int size;
    printf("Enter process memory required (KB): ");
    if (scanf("%d", &size) != 1) {
        printf("Invalid input.\n");
        exit(EXIT_FAILURE);
    }
    if (size <= 0 || size > TOTAL_MEMORY) {
        printf("Request must be between 1 and %d KB.\n",
               TOTAL_MEMORY);
        failures++;
        return;
    }

    Block *chosen = NULL;
    for (Block *b = head; b; b = b->next) {
        if (b->pid == 0 && b->size >= size) {
            searches++;
            if (strategy == 1) {
                chosen = b;
                break;
            }
            if (!chosen ||
                (strategy == 2 && b->size < chosen->size) ||
                (strategy == 3 && b->size > chosen->size)) {
                chosen = b;
            }
        }
    }

    if (!chosen) {
        failures++;
        printf("Allocation failed! No suitable free block.\n");
        return;
    }

    int pid = next_pid++;
    if (chosen->size == size) {
        chosen->pid = pid;
    } else {
        Block *remainder = new_block(
            chosen->start + size, chosen->size - size, 0);
        remainder->next = chosen->next;
        chosen->next = remainder;
        chosen->size = size;
        chosen->pid = pid;
    }

    allocations++;
    printf("Success! Process P%d allocated %d KB at address %d.\n",
           pid, size, chosen->start);
}

void merge_free_blocks(void) {
    Block *b = head;
    while (b && b->next) {
        if (b->pid == 0 && b->next->pid == 0) {
            Block *temp = b->next;
            b->size += temp->size;
            b->next = temp->next;
            free(temp);
        } else {
            b = b->next;
        }
    }
}

void free_memory(void) {
    int pid, found = 0;
    printf("Enter process ID to release (e.g. 1 for P1): ");
    if (scanf("%d", &pid) != 1) {
        printf("Invalid input.\n");
        exit(EXIT_FAILURE);
    }

    for (Block *b = head; b; b = b->next) {
        if (b->pid == pid && pid > 0) {
            b->pid = 0;
            found = 1;
            printf("Process P%d released successfully.\n", pid);
            break;
        }
    }

    if (!found) {
        printf("Process not found.\n");
        return;
    }
    merge_free_blocks();
}

void display_memory(void) {
    printf("\n========== MEMORY MAP ==========\n");
    printf("Strategy: %s | Total: %d KB\n", strategy_name(),
           TOTAL_MEMORY);
    for (Block *b = head; b; b = b->next) {
        printf("[%4d-%4d KB] ", b->start, b->start + b->size - 1);
        if (b->pid == 0)
            printf("FREE (%d KB)\n", b->size);
        else
            printf("P%d (%d KB)\n", b->pid, b->size);
    }
    printf("================================\n");
}

void show_statistics(void) {
    int used = 0, free_mem = 0, largest_free = 0;
    int free_blocks = 0;

    for (Block *b = head; b; b = b->next) {
        if (b->pid == 0) {
            free_mem += b->size;
            free_blocks++;
            if (b->size > largest_free)
                largest_free = b->size;
        } else {
            used += b->size;
        }
    }

    printf("\n======= MEMORY STATISTICS =======\n");
    printf("Total memory          : %d KB\n", TOTAL_MEMORY);
    printf("Used memory           : %d KB\n", used);
    printf("Free memory           : %d KB\n", free_mem);
    printf("Memory utilization    : %.2f%%\n",
           100.0 * used / TOTAL_MEMORY);
    printf("Successful allocations: %d\n", allocations);
    printf("Failed allocations    : %d\n", failures);
    printf("Search operations     : %d\n", searches);
    printf("Free blocks           : %d\n", free_blocks);
    printf("Largest free block    : %d KB\n", largest_free);

    if (free_mem > 0 && free_blocks > 1) {
        double fragmentation =
            100.0 * (free_mem - largest_free) / free_mem;
        printf("External fragmentation: %.2f%%\n", fragmentation);
    } else {
        printf("External fragmentation: 0.00%%\n");
    }
    printf("=================================\n");
}

void cleanup(void) {
    while (head) {
        Block *temp = head;
        head = head->next;
        free(temp);
    }
}

int main(void) {
    int choice;
    initialize();

    printf("========================================\n");
    printf("  MemScope: Dynamic Memory Simulator\n");
    printf("  OSSP Project | C Programming | Ubuntu\n");
    printf("========================================\n");

    do {
        printf("\n--- MAIN MENU ---\n");
        printf("1. Choose allocation strategy\n");
        printf("2. Allocate memory\n");
        printf("3. Deallocate memory\n");
        printf("4. Display memory map\n");
        printf("5. Show statistics\n");
        printf("6. Exit\n");
        printf("Enter choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Exiting safely.\n");
            break;
        }

        switch (choice) {
            case 1: choose_strategy(); break;
            case 2: allocate_memory(); break;
            case 3: free_memory(); break;
            case 4: display_memory(); break;
            case 5: show_statistics(); break;
            case 6: printf("Thank you for using MemScope!\n"); break;
            default: printf("Invalid choice. Try again.\n");
        }
    } while (choice != 6);

    cleanup();
    return 0;
}
