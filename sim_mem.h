#ifndef EX4_SIM_MEM_H
#define EX4_SIM_MEM_H

#include <iostream>
#include <csignal>
#include <cmath>
#include <string>
#include <bitset>
#include <fcntl.h>
#include <cstring>
using namespace std;

#define MEMORY_SIZE 16
extern char main_memory[MEMORY_SIZE];

typedef struct page_descriptor{
    bool valid;
    int frame;
    bool dirty;
    int swap_index;
    int timer; // Timer to track page usage
} page_descriptor;

class sim_mem {
    int swapfile_fd; //swap file fd
    int program_fd; //executable file fd
    int text_size;
    int data_size;
    int bss_size;
    int heap_stack_size;
    int num_of_pages;
    int page_size;
    page_descriptor **page_table; //pointer to page table
private:
    bool* page_usage; // Array to track page usage
    int* num_of_pages_array;
    int* swapBlocks; // Array to track the availability of blocks in the swap file
public:
    sim_mem(char exe_file_name[], char swap_file_name[], int text_size, int data_size, int bss_size, int heap_stack_size, int page_size);
    char load(int address);
    void store(int address, char value);
    void print_memory();
    void print_swap();
    void print_page_table();
    ~sim_mem();
    void LRU_page_replacement(int *oldestPageOut, int *oldestPageIn);
    void statusUpdate(int out, int in, bool valid, int frame, bool dirty, int swap_index, int time);
    void addTime(); // Adding time to all the valid pages for the LRU algorithm
    void deleteMemory(); // In case we need to delete the memory because of a fail we go here
};
#endif //EX4_SIM_MEM_H
