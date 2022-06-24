#pragma once

#include <stdint.h>
#include <sys/socket.h>

typedef struct Context
{
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbp;
    uint64_t rsp;
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t rip;
} Context;

int co_get_ctx(Context *);

void co_set_ctx(Context *);

typedef struct Task
{
    char *name;
    Context ctx;

    void (*entrypoint)(void *);

    void *arg;

    struct Task *next;
} Task;

void _co_create(char *name, void(*fn)(void *), void *arg);

#define co_create(fn, arg) _co_create(#fn, fn, arg)

// return ready task count
int co_yield();

void co_run();

void co_mark_read(int fd, Task *task);

void co_mark_write(int fd, Task *task);

ssize_t co_read(int fd, void *buf, size_t count);

ssize_t co_write(int fd, const void *buf, size_t count);

ssize_t co_accept(int fd, struct sockaddr *restrict addr, socklen_t *restrict addrlen);

void co_io_init();
