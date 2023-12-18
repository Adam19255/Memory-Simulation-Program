#include "sim_mem.h"

// Adding time to the pages
void sim_mem::addTime() {
    for (int i = 0; i < 4; ++i) {
        num_of_pages = num_of_pages_array[i];
        for (int j = 0; j < num_of_pages; ++j) {
            if (this->page_table[i][j].valid)
                this->page_table[i][j].timer += 1;
        }
    }
}

// Implementation of LRU page replacement
void sim_mem::LRU_page_replacement(int *oldestPageOut, int *oldestPageIn) {
    int max = 0, outIndex, inIndex;
    // Iterating over the outer layer of the page table
    for (int i = 0; i < 4; i++) {
        num_of_pages = num_of_pages_array[i];
        // Iterating over the inner layer of the page table
        for (int j = 0; j < num_of_pages; j++) {
            // Checking to see if the current page hasn't been updated for the longest time
            if (page_table[i][j].valid && page_table[i][j].timer > max) {
                max = page_table[i][j].timer;
                outIndex = i;
                inIndex = j;
            }
        }
    }
    *oldestPageOut = outIndex;
    *oldestPageIn = inIndex;
}

// Method to find an available frame in the main memory
int findEmptyMemory(int page_size, bool page_usage[]){
    int frame = -1;
    for (int i = 0; i < MEMORY_SIZE / page_size; i++) {
        if (!page_usage[i]) {
            frame = i * page_size;
            page_usage[i] = true;
            break;
        }
    }
    return frame;
}

// Method to find an available block in swap
int findEmptySwap(int data_size, int bss_size, int heap_stack_size, int page_size, const int swapBlocks[]){
    int swapIndex = -1;
    for (int i = 0; i < (data_size + bss_size + heap_stack_size) / page_size; ++i) {
        if (swapBlocks[i] == 0) {
            swapIndex = i;
            break;
        }
    }
    return swapIndex;
}

void sim_mem::statusUpdate(int out, int in, bool valid, int frame, bool dirty, int swap_index, int time) {
    page_table[out][in].valid = valid;
    if (frame != -1)
        page_table[out][in].frame = frame / page_size;
    else
        page_table[out][in].frame = frame;
    page_table[out][in].dirty = dirty;
    if (swap_index != -1)
        page_table[out][in].swap_index = swap_index / page_size;
    else
        page_table[out][in].swap_index = swap_index;
    page_table[out][in].timer = time;
}

// Method to change the address to binary
void parse_address(int address, int *page, int *offset, int *in, int *out, int page_size, const int num_of_pages_array[]) {
    int offsetSize = (int)log2(page_size);  // Calculate the size of the offset
    int inSize = 12 - offsetSize; // Where to inSize the offset
    string binaryAddress = bitset<12>(address).to_string();  // Convert address to binary string
    // Extract the 'out' value (last two digits)
    *out = stoi(binaryAddress.substr(0, 2), nullptr, 2);
    // Extract the 'offset' value (first digits with size of 'offsetSize')
    *offset = stoi(binaryAddress.substr(inSize, offsetSize), nullptr, 2);
    // Extract the 'in' value (the remaining digits)
    *in = stoi(binaryAddress.substr(2, inSize - 2), nullptr, 2);
    if (*out != 0) {
        *page = (num_of_pages_array[*out - 1] + *in) * page_size;
    }
    else{
        *page = *in * page_size;
    }
}

sim_mem::sim_mem(char *exe_file_name, char *swap_file_name, int text_size, int data_size, int bss_size,
                 int heap_stack_size, int page_size) {
    // Open the files exe_file_name and swap_file_name
    program_fd = open(exe_file_name, O_RDONLY);
    if (program_fd == -1) {
        perror("Failed to open program file\n");
        exit(EXIT_FAILURE);
    }
    swapfile_fd = open(swap_file_name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (swapfile_fd == -1)
        perror("Failed to open swap file\n");

    // Initialize variables
    this->text_size = text_size;
    this->data_size = data_size;
    this->bss_size = bss_size;
    this->heap_stack_size = heap_stack_size;
    this->page_size = page_size;

    // Initialize main memory with zeros
    for (int i = 0; i < MEMORY_SIZE; ++i) {
        main_memory[i] = '0';
    }
    // Calculate the number of pages needed for each section
    num_of_pages_array = new int[4];
    num_of_pages_array[0] = text_size / page_size;
    num_of_pages_array[1] = data_size / page_size;
    num_of_pages_array[2] = bss_size / page_size;
    num_of_pages_array[3] = heap_stack_size / page_size;

    // Allocate memory for the first layer of the page table
    page_table = new page_descriptor*[4];
    for (int i = 0; i < 4; ++i) {
        num_of_pages = num_of_pages_array[i];
        // Allocate memory for the second layer page table
        page_table[i] = new page_descriptor[num_of_pages];

        // Initialize the page table entries
        for (int j = 0; j < num_of_pages; ++j) {
            statusUpdate(i, j, false, -1, false, -1, 0);
        }
    }
    // Initialize swap_file with zeros
    int buffer_size = data_size + bss_size + heap_stack_size;
    char zeros_buffer[buffer_size];
    for (int i = 0; i < buffer_size; ++i) {
        zeros_buffer[i] = '0';
    }
    if (write(swapfile_fd,zeros_buffer, buffer_size) == -1) {
        perror("Error writing to swap file");
        delete[] num_of_pages_array;
        for (int i = 0; i < 4; ++i) {
            delete[] page_table[i];
        }
        delete[] page_table;
        close(program_fd);
        close(swapfile_fd);
        exit(-1);
    }

    // Initialize the page_usage array
    page_usage = new bool[MEMORY_SIZE / page_size];
    for (int i = 0; i < MEMORY_SIZE / page_size; ++i) {
        page_usage[i] = false;
    }

    int numSwapBlocks = buffer_size / page_size;
    // Allocating memory for the swap block array
    swapBlocks = new int[numSwapBlocks];
    for (int i = 0; i < numSwapBlocks; ++i) {
        swapBlocks[i] = 0;  // 0 indicates that the block is free
    }
}

sim_mem::~sim_mem() {
    delete[] page_usage;
    delete[] swapBlocks;
    delete[] num_of_pages_array;
    for (int i = 0; i < 4; ++i) {
        delete[] page_table[i];
    }
    delete[] page_table;
    close(program_fd);
    close(swapfile_fd);
}

char sim_mem::load(int address) {
    int page, offset, in, out, frame = -1;
    parse_address(address, &page, &offset, &in, &out, page_size, num_of_pages_array);
    if (in < num_of_pages_array[out]) { // Checking first if the page exist
        if ((this->page_table[out][in].valid && !this->page_table[out][in].dirty) ||
            (this->page_table[out][in].valid && this->page_table[out][in].dirty)) {
            frame = this->page_table[out][in].frame * page_size; // getting the frame number
            this->page_table[out][in].timer = 0;
        } else if (!this->page_table[out][in].valid && !this->page_table[out][in].dirty) {
            // If the page is heap_stack it's an error
            if (out == 3) {
                perror("ERR");
                return 0;
            }
            // Setting the courser to the correct page in the file
            lseek(program_fd, page, SEEK_SET);
            frame = findEmptyMemory(page_size, page_usage);
            if (frame == -1) { // No available frame found
                int oldestPageOut, oldestPageIn;
                LRU_page_replacement(&oldestPageOut, &oldestPageIn);
                frame = this->page_table[oldestPageOut][oldestPageIn].frame * page_size;
                // If the page isn't dirty = we can just replace the page
                if (!this->page_table[oldestPageOut][oldestPageIn].dirty) {
                    // Changing the status of the page we replace
                    statusUpdate(oldestPageOut, oldestPageIn, false, -1, false, -1, 0);
                    // Read 'page_size' bytes into 'main_memory'
                    if (read(program_fd, &main_memory[frame], page_size) == -1) {
                        perror("Error reading from exe file");
                        deleteMemory();
                    }
                } else { // The page that will be replaced will move into swap
                    int swapIndex = findEmptySwap(data_size, bss_size, heap_stack_size, page_size, swapBlocks);
                    if (swapIndex != -1) { // Free block was found
                        swapBlocks[swapIndex] = 1;  // Mark the block as in use
                        // Calculate the offset in the swap file using the swap index
                        int swapOffset = swapIndex * page_size;
                        // Write the page content to the swap file at the calculated offset
                        lseek(swapfile_fd, swapOffset, SEEK_SET);
                        if (write(swapfile_fd, &main_memory[page_table[oldestPageOut][oldestPageIn].frame * page_size], page_size) == -1){
                            perror("Error writing to swap file");
                            deleteMemory();
                        }
                        statusUpdate(oldestPageOut, oldestPageIn, false, -1, true, swapOffset, 0);
                        // Read 'page_size' bytes into 'main_memory'
                        if (read(program_fd, &main_memory[frame], page_size) == -1) {
                            perror("Error reading from exe file");
                            deleteMemory();
                        }
                    }
                }
            } else {
                if (read(program_fd, &main_memory[frame], page_size) == -1) {
                    perror("Error reading from exe file");
                    deleteMemory();
                }
            }
            statusUpdate(out, in, true, frame, false, -1, 0);
        } else if (!this->page_table[out][in].valid && this->page_table[out][in].dirty) {
            char zero_buffer[page_size]; // A buffer to zero out the swap
            for (int i = 0; i < page_size; ++i) {
                zero_buffer[i] = '0';
            }
            int swapReplace = this->page_table[out][in].swap_index;
            frame = findEmptyMemory(page_size, page_usage);
            if (frame == -1) { // No available frame found
                int oldestPageOut, oldestPageIn;
                LRU_page_replacement(&oldestPageOut, &oldestPageIn);
                // If the page isn't dirty = we can just replace the page
                if (!this->page_table[oldestPageOut][oldestPageIn].dirty) {
                    frame = this->page_table[oldestPageOut][oldestPageIn].frame * page_size;
                    // Changing the status of the page we replace
                    statusUpdate(oldestPageOut, oldestPageIn, false, -1, false, -1, 0);
                    lseek(swapfile_fd, this->page_table[out][in].swap_index * page_size, SEEK_SET);
                    if (read(swapfile_fd, &main_memory[frame], page_size) == -1) {
                        perror("Error reading from swap file");
                        deleteMemory();
                    }
                    // Zero out the index of where the page was in the swap
                    lseek(swapfile_fd, this->page_table[out][in].swap_index * page_size, SEEK_SET);
                    write(swapfile_fd, zero_buffer, page_size);
                } else { // The page that will be replaced will move into swap
                    int swapIndex = findEmptySwap(data_size, bss_size, heap_stack_size, page_size, swapBlocks);
                    if (swapIndex != -1) { // Free block was found
                        swapBlocks[swapIndex] = 1;  // Mark the block as in use
                        // Calculate the offset in the swap file using the swap index
                        int swapOffset = swapIndex * page_size;
                        // Write the page content to the swap file at the calculated offset
                        lseek(swapfile_fd, swapOffset, SEEK_SET);
                        if (write(swapfile_fd, &main_memory[page_table[oldestPageOut][oldestPageIn].frame * page_size], page_size) == -1){
                            perror("Error writing to swap file");
                            deleteMemory();
                        }
                        statusUpdate(oldestPageOut, oldestPageIn, false, -1, true, swapOffset, 0);
                        // Read 'page_size' bytes into 'main_memory'
                        lseek(swapfile_fd, this->page_table[out][in].swap_index * page_size, SEEK_SET);
                        if (read(swapfile_fd, &main_memory[frame], page_size) == -1) {
                            perror("Error reading from swap file");
                            deleteMemory();
                        }
                        // Zero out the index of where the page was in the swap
                        lseek(swapfile_fd, this->page_table[out][in].swap_index * page_size, SEEK_SET);
                        if (write(swapfile_fd, zero_buffer, page_size) == -1){
                            perror("Error writing to swap file");
                            deleteMemory();
                        }
                    }
                }
            } else {
                lseek(swapfile_fd, this->page_table[out][in].swap_index * page_size, SEEK_SET);
                if (read(swapfile_fd, &main_memory[frame], page_size) == -1) {
                    perror("Error reading from swap file");
                    deleteMemory();
                }
                // Zero out the index of where the page was in the swap
                lseek(swapfile_fd, this->page_table[out][in].swap_index * page_size, SEEK_SET);
                if (write(swapfile_fd, zero_buffer, page_size) == -1){
                    perror("Error writing to swap file");
                    deleteMemory();
                }
            }
            statusUpdate(out, in, true, frame, true, -1, 0);
            swapBlocks[swapReplace] = 0;
        }
        addTime();
        return main_memory[frame+ offset];
    }
    else{
        printf("No such page\n");
        return 0;
    }
}
void sim_mem::store(int address, char value) {
    int offset, page, in, out, frame = -1;
    parse_address(address, &page, &offset, &in, &out, page_size, num_of_pages_array);
    if (in < num_of_pages_array[out]) { // Checking first if the page exist
        if ((this->page_table[out][in].valid && !this->page_table[out][in].dirty) ||
            (this->page_table[out][in].valid && this->page_table[out][in].dirty)) {
            frame = this->page_table[out][in].frame * page_size; // getting the frame number
            statusUpdate(out, in, true, frame, true, -1, 0);
        } else if (!this->page_table[out][in].valid && !this->page_table[out][in].dirty) {
            // If the page is text it's an error
            if (out == 0) {
                perror("Can't write to this page");
            } else {
                lseek(program_fd, page, SEEK_SET);
                frame = findEmptyMemory(page_size, page_usage);
                if (frame == -1) { // No available frame found
                    int oldestPageOut, oldestPageIn;
                    LRU_page_replacement(&oldestPageOut, &oldestPageIn);
                    frame = this->page_table[oldestPageOut][oldestPageIn].frame * page_size;
                    if (!this->page_table[oldestPageOut][oldestPageIn].dirty) {
                        statusUpdate(oldestPageOut, oldestPageIn, false, -1, false, -1, 0);
                        if (out == 1) {
                            if (read(program_fd, &main_memory[frame], page_size) == -1) {
                                perror("Error reading from exe file");
                                deleteMemory();
                            }
                        } else {
                            for (int i = 0; i < page_size; ++i) {
                                main_memory[frame + i] = '0';
                            }
                        }
                    } else {
                        int swapIndex = findEmptySwap(data_size, bss_size, heap_stack_size, page_size, swapBlocks);
                        if (swapIndex != -1) { // Free block was found
                            swapBlocks[swapIndex] = 1;  // Mark the block as in use
                            // Calculate the offset in the swap file using the swap index
                            int swapOffset = swapIndex * page_size;
                            // Write the page content to the swap file at the calculated offset
                            lseek(swapfile_fd, swapOffset, SEEK_SET);
                            if (write(swapfile_fd, &main_memory[page_table[oldestPageOut][oldestPageIn].frame * page_size], page_size) == -1){
                                perror("Error writing to swap file");
                                deleteMemory();
                            }
                            statusUpdate(oldestPageOut, oldestPageIn, false, -1, true, swapOffset, 0);
                            if (out == 1) {
                                if (read(program_fd, &main_memory[frame], page_size) == -1) {
                                    perror("Error reading from exe file");
                                    deleteMemory();
                                }
                            } else {
                                for (int i = 0; i < page_size; ++i) {
                                    main_memory[frame + i] = '0';
                                }
                            }
                        }
                    }
                } else {
                    if (out == 1) {
                        if (read(program_fd, &main_memory[frame], page_size) == -1) {
                            perror("Error reading from exe file");
                            deleteMemory();
                        }
                    } else {
                        for (int i = 0; i < page_size; ++i) {
                            main_memory[frame + i] = '0';
                        }
                    }
                }
            }
        } else if (!this->page_table[out][in].valid && this->page_table[out][in].dirty) {
            char zero_buffer[page_size]; // A buffer to zero out the swap
            for (int i = 0; i < page_size; ++i) {
                zero_buffer[i] = '0';
            }
            int swapReplace = this->page_table[out][in].swap_index;
            frame = findEmptyMemory(page_size, page_usage);
            if (frame == -1) { // No available frame found
                int oldestPageOut, oldestPageIn;
                LRU_page_replacement(&oldestPageOut, &oldestPageIn);
                frame = this->page_table[oldestPageOut][oldestPageIn].frame * page_size;
                if (!this->page_table[oldestPageOut][oldestPageIn].dirty) {
                    statusUpdate(oldestPageOut, oldestPageIn, false, -1, false, -1, 0);

                    lseek(swapfile_fd, this->page_table[out][in].swap_index * page_size, SEEK_SET);
                    if (read(swapfile_fd, &main_memory[frame], page_size) == -1) {
                        perror("Error reading from swap file");
                        deleteMemory();
                    }

                    lseek(swapfile_fd, this->page_table[out][in].swap_index * page_size, SEEK_SET);
                    if (write(swapfile_fd, zero_buffer, page_size) == -1){
                        perror("Error writing to swap file");
                        deleteMemory();
                    }
                } else {
                    int swapIndex = findEmptySwap(data_size, bss_size, heap_stack_size, page_size, swapBlocks);
                    if (swapIndex != -1) { // Free block was found
                        swapBlocks[swapIndex] = 1;  // Mark the block as in use
                        int swapOffset = swapIndex * page_size;

                        lseek(swapfile_fd, swapOffset, SEEK_SET);
                        if (write(swapfile_fd, &main_memory[page_table[oldestPageOut][oldestPageIn].frame * page_size], page_size) == -1){
                            perror("Error writing to swap file");
                            deleteMemory();
                        }
                        statusUpdate(oldestPageOut, oldestPageIn, false, -1, true, swapOffset, 0);

                        lseek(swapfile_fd, this->page_table[out][in].swap_index * page_size, SEEK_SET);
                        if (read(swapfile_fd, &main_memory[frame], page_size) == -1) {
                            perror("Error reading from swap file");
                            deleteMemory();
                        }

                        lseek(swapfile_fd, this->page_table[out][in].swap_index * page_size, SEEK_SET);
                        if (write(swapfile_fd, zero_buffer, page_size) == -1){
                            perror("Error writing to swap file");
                            deleteMemory();
                        }
                    }
                }
            } else {
                lseek(swapfile_fd, this->page_table[out][in].swap_index * page_size, SEEK_SET);
                if (read(swapfile_fd, &main_memory[frame], page_size) == -1) {
                    perror("Error reading from swap file");
                    deleteMemory();
                }

                lseek(swapfile_fd, this->page_table[out][in].swap_index * page_size, SEEK_SET);
                if (write(swapfile_fd, zero_buffer, page_size) == -1){
                    perror("Error writing to swap file");
                    deleteMemory();
                }
            }
            swapBlocks[swapReplace] = 0;
        }
        if (out != 0) {
            main_memory[frame + offset] = value;
            statusUpdate(out, in, true, frame, true, -1, 0);
            addTime();
        }
    }
    else{
        printf("No such page\n");
    }
}

void sim_mem::print_memory() {
    int i;
    printf("\n Physical memory\n");
    for(i = 0; i < MEMORY_SIZE; i++) {
        printf("[%c]\n", main_memory[i]);
    }
}

void sim_mem::print_swap() {
    char* str = new char[page_size];
    int i;
    printf("\n Swap memory\n");
    lseek(swapfile_fd, 0, SEEK_SET); // go to the start of the file, lseek is what control the location of the courser of the exe file
    while(read(swapfile_fd, str, this->page_size) == this->page_size) {
        for(i = 0; i < page_size; i++) {
            printf("%d - [%c]\t", i, str[i]);
        }
        printf("\n");
    }
    delete[] str;
}

void sim_mem::print_page_table() {
    int i,
            num_of_txt_pages = text_size / page_size,
            num_of_data_pages = data_size / page_size,
            num_of_bss_pages = bss_size / page_size,
            num_of_stack_heap_pages = heap_stack_size / page_size;
    printf("Valid\t Dirty\t Frame\t Swap index\n");
    for(i = 0; i < num_of_txt_pages; i++) {
        printf("[%d]\t[%d]\t[%d]\t[%d]\n",
               page_table[0][i].valid,
               page_table[0][i].dirty,
               page_table[0][i].frame ,
               page_table[0][i].swap_index);
    }
    printf("Valid\t Dirty\t Frame\t Swap index\n");
    for(i = 0; i < num_of_data_pages; i++) {
        printf("[%d]\t[%d]\t[%d]\t[%d]\n",
               page_table[1][i].valid,
               page_table[1][i].dirty,
               page_table[1][i].frame ,
               page_table[1][i].swap_index);
    }
    printf("Valid\t Dirty\t Frame\t Swap index\n");
    for(i = 0; i < num_of_bss_pages; i++) {
        printf("[%d]\t[%d]\t[%d]\t[%d]\n",
               page_table[2][i].valid,
               page_table[2][i].dirty,
               page_table[2][i].frame ,
               page_table[2][i].swap_index);
    }
    printf("Valid\t Dirty\t Frame\t Swap index\n");
    for(i = 0; i < num_of_stack_heap_pages; i++) {
        printf("[%d]\t[%d]\t[%d]\t[%d]\n",
               page_table[3][i].valid,
               page_table[3][i].dirty,
               page_table[3][i].frame ,
               page_table[3][i].swap_index);
    }
}

void sim_mem::deleteMemory() {
    delete[] page_usage;
    delete[] swapBlocks;
    delete[] num_of_pages_array;
    for (int i = 0; i < 4; ++i) {
        delete[] page_table[i];
    }
    delete[] page_table;
    close(program_fd);
    close(swapfile_fd);
    exit(-1);
}
