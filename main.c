#include "co.h"
#include <stdio.h>

void task1() {
    printf("task1 before\n");
    co_yield();
    printf("task1 after \n");
}

void task2() {
    printf("task2 before\n");
    co_yield();
    printf("task2 after \n");
}

void task3() {
    printf("task3 before\n");
    co_yield();
    printf("task3 after \n");
}

int main() {
    co_create("task1", task1);
    co_create("task2", task2);
    co_create("task3", task3);
    co_run();
}

