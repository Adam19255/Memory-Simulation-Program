#include <iostream>
#include "sim_mem.h"

char main_memory[MEMORY_SIZE];

int main() {
    char* exec_file = new char[10];
    char* swap_file = new char[10];
    strcpy(exec_file, "exec_file");
    strcpy(swap_file, "swap_file");

    sim_mem mem_sm(exec_file, swap_file , 16, 32,32, 32, 8);

    mem_sm.store(1025,'$');
    mem_sm.print_memory();
    mem_sm.print_page_table();

    mem_sm.load(3079);

    mem_sm.store(15, '3');

    mem_sm.store(3079,'%');
    mem_sm.print_memory();
    mem_sm.print_page_table();

    mem_sm.store(1048,'(');
    mem_sm.print_memory();
    mem_sm.print_page_table();

    cout << mem_sm.load(3079) << endl;
    mem_sm.print_memory();
    mem_sm.print_page_table();
    mem_sm.print_swap();

    cout << mem_sm.load (1035) << endl;
    mem_sm.print_memory();
    mem_sm.print_page_table();
    mem_sm.print_swap();

    cout << mem_sm.load(15) << endl;
    mem_sm.print_memory();
    mem_sm.print_page_table();
    mem_sm.print_swap();


    cout << mem_sm.load (1025) << endl;
    mem_sm.print_memory();
    mem_sm.print_page_table();
    mem_sm.print_swap();

    cout << mem_sm.load(1031) << endl;
    mem_sm.print_memory();
    mem_sm.print_page_table();
    mem_sm.print_swap();

    cout << mem_sm.load(7) << endl;
    mem_sm.print_memory();
    mem_sm.print_page_table();
    mem_sm.print_swap();

    cout << mem_sm.load(15) << endl;
    mem_sm.print_memory();
    mem_sm.print_page_table();
    mem_sm.print_swap();

    cout << mem_sm.load(1031) << endl;
    mem_sm.print_memory();
    mem_sm.print_page_table();
    mem_sm.print_swap();

    delete[] exec_file;
    delete[] swap_file;
}
