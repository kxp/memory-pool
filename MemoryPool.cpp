// MemoryPool.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <ctime>
#include <iostream>
#include <windows.h>
#include "memory_pools.h"
#include "lfqueue.h"


typedef struct test_s {
    uint64_t value_1;
    uint32_t value_2;
    uint32_t value_3;
    uint16_t value_4;
    uint16_t value_5;
    uint16_t value_6;
    uint16_t value_7;
    uint8_t reserved_1[8];
    uint16_t reserved_2[2];
    uint32_t reserved_3[300];
} test_t;





void test_pool(uint64_t run_amount) {

    uint32_t number_entries = 50;
    size_t entry_size = sizeof(test_t);

    printf("Size of custom struct:%lld\n", entry_size);
    memory_pool* pool = init_pool(number_entries, entry_size);
    clock_t start = clock();
    for (int i = 0; i < run_amount; i++) {

        test_t* val = (test_t*)_malloc_ref(pool, entry_size);
        val->value_3 = i;
        printf("Value: %d\n", val->value_3);
        //Sleep(50);
        _free_ref(pool, (uintptr_t)val);
    }
    clock_t end = clock();
    float seconds = (float)(end - start) / CLOCKS_PER_SEC;
    printf("Pool execution time:%f\n", seconds);
}

void test_malloc(uint64_t run_amount) {
    uint32_t number_entries = 50;
    size_t entry_size = sizeof(test_t);

    printf("Size of custom struct:%lld\n", entry_size);    
    clock_t start = clock();
    for (int i = 0; i < run_amount; i++) {
        test_t* val = (test_t*)malloc(entry_size);
        val->value_3 = i;        
        free(val);
    }
    clock_t end = clock();
    float seconds = (float)(end - start) / CLOCKS_PER_SEC;
    printf("Malloc execution time:%f\n", seconds);
}


void test_custiom_lfqueue(uint64_t run_amount) {
    uint32_t number_entries = 5000;    
    lfqueue_t queue;
    memory_pool* pool = init_pool(number_entries, 32);
    lfqueue_init_mf(&queue, pool, _malloc_ref, _free_ref);
    clock_t start = clock();
    for (int i = 0; i < run_amount; i++) {

        uint64_t value = i;

        lfqueue_enq(&queue, &value);
        uint64_t poped = (uint64_t) lfqueue_deq(&queue);

    }
    clock_t end = clock();
    float seconds = (float)(end - start) / CLOCKS_PER_SEC;
    printf("Custom allocating Queue execution time:%f\n", seconds);
}



void test_lfqueue(uint64_t run_amount) {
    uint32_t number_entries = 50;
    lfqueue_t queue;
    lfqueue_init(&queue);
    
    clock_t start = clock();
    for (int i = 0; i < run_amount; i++) {

        uint64_t value = i;

        lfqueue_enq(&queue, &value);
        uint64_t poped = (uint64_t)lfqueue_deq(&queue);

    }
    clock_t end = clock();
    float seconds = (float)(end - start) / CLOCKS_PER_SEC;
    printf("Normal allocating Queue execution time:%f\n", seconds);
}




int main()
{
    std::cout << "Hello World!\n";
    uint64_t runs = 10000000;
    printf("Running %lld runs.\n", runs);
    //test_pool(50);

    //test_pool(runs);
    //test_malloc(runs);

    test_lfqueue(runs);
    test_custiom_lfqueue(runs);

    printf("Press any key to exit.\n");
    getchar();
}