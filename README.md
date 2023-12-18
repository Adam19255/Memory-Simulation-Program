# Memory Simulation Program
Authored by Adam Shay

## Description
This program simulates memory management and page replacement algorithms. 
It is designed to load/store and manage pages from an executable file into main memory, utilizing swap space for overflow.

## Features
1. **Page Table:** The program uses a page table to keep track of the status of each page in the main memory. 
   The page table contains entries for each page, including information such as validity, frame number, dirty bit, swap index, and timer.
2. **LRU Page Replacement:** The program implements the Least Recently Used (LRU) page replacement algorithm. 
   It identifies the page that has not been accessed for the longest time and replaces it when there is a need for a new memory page.
3. **Address Parsing:** The program includes a method to parse the address into its corresponding page, offset, and frame values. 
   It extracts the relevant bits from the address and calculates the appropriate values based on the page size.
4. **Memory Allocation:** The program allocates memory for the main memory, swap space, and page tables. 
   It initializes the memory with zeros marks the page usage and swaps blocks accordingly.
5. **Loading Pages:** The program loads pages from the executable file into main memory when requested. 
   It checks the page table to determine the validity and dirtiness of the page and takes appropriate actions such as page replacement or reading from the executable file.
6. **Store Function:** The `store` function is responsible for storing a value in memory at a given address. 
   It manages memory pages, swap space, frames and updates the status of pages and frames.

## How to Use

To use the memory simulation program, follow these steps:

Include the `sim_mem.h` header file in your code.

Create an instance of the `sim_mem` class, providing the necessary parameters such as the `executable file name` `swap file name` `memory sizes`, and `page size`.

Use the `load` method to load pages from memory using the address as input. The method returns the character stored at the specified address.

Use the `store` method to load pages from memory using the address as input. The method will store the value provided in the given address.

Perform other operations and utilize the memory management features as needed.

Clean up resources by deleting the memory instance using the destructor `~sim_mem()`.
