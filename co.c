#include "co.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>

#ifdef CO_DEBUG
#define co_debug(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define co_debug(fmt, ...)
#endif

Task *task_cur = NULL;
Task *task_ready = NULL;
Context ctx_main;

Task *co_get_last_ready() {
    Task *last = task_ready;
    while (last->next != NULL)
        last = last->next;
    return last;
}

void co_set_ready(Task *t) {
    co_debug("co_set_ready: %s\n", t->name);
    if (task_ready == NULL)
        task_ready = t;
    else
        co_get_last_ready()->next = t;
}

void co_swap(Context *from, Context *to) {
    co_debug("co_swap\n");
    if (co_get_ctx(from) == 0)
        co_set_ctx(to);
}

void co_exit() {
    co_set_ctx(&ctx_main);
}

void _co_create(char *name, void(*fn)(void *), void *arg) {
    co_debug("co_create %s\n", name);

    Task *t = (Task *) malloc(sizeof(Task));
    t->name = name;
    t->entrypoint = fn;
    t->next = NULL;

    co_get_ctx(&t->ctx);

    t->ctx.rip = (uint64_t) fn;

    t->ctx.rdi = (uint64_t) arg;

    // create stack
    uint64_t stack_size = 4096;
    uint64_t leave_some_room = 128;
    uint64_t sp = (uint64_t) malloc(stack_size);
    sp += stack_size - leave_some_room - sizeof(uint64_t);
    *((uint64_t *) sp) = (uint64_t) co_exit;
    t->ctx.rsp = sp;

    co_set_ready(t);
}

int co_yield() {
    co_debug("co_yield current: %s\n", task_cur->name);
    co_set_ready(task_cur);
    co_swap(&task_cur->ctx, &ctx_main);

    int ready_count = 0;
    Task *next = task_ready;
    while (next != NULL)
    {
        ready_count++;
        next = next->next;
    }
    co_debug("co_yield ready: %d\n", ready_count);
    return ready_count;
}

void co_run() {
    while (1)
    {
        if (task_ready == NULL)
        {
            printf("no task ready, exit now.\n");
            return;
        }

        task_cur = task_ready;
        task_ready = task_ready->next;
        task_cur->next = NULL;

        co_debug("co_run swap to %s\n", task_cur->name);
        co_swap(&ctx_main, &task_cur->ctx);
    }
}

#define CO_EPOLL_MAX_EVENTS 100

int co_epoll_fd;

void co_io_scheduler(void *v) {
    co_epoll_fd = epoll_create(1);
    struct epoll_event events[CO_EPOLL_MAX_EVENTS];

    while (1)
    {
        co_debug("co_io_scheduler before yield\n");
        // yield until no other active task
        while (co_yield() != 0);
        co_debug("co_io_scheduler after yield\n");

        co_debug("co_io_scheduler before wait\n");
        int nfd = epoll_wait(co_epoll_fd, events, CO_EPOLL_MAX_EVENTS, -1);
        co_debug("co_io_scheduler after wait\n");
        for (int i = 0; i < nfd; i++)
        {
            Task *task = (Task *) events[i].data.u64;
            co_set_ready(task);
        }
        co_yield();
    }
}

void co_unmark(int fd, Task *task) {
    co_debug("co_unmark: %s %d\n", task->name, fd);
    int ret = epoll_ctl(co_epoll_fd, EPOLL_CTL_DEL, fd, 0);
    if (ret != 0)
    {
        perror("co_unmark fail:");
        exit(1);
    }
}

void co_mark_read(int fd, Task *task) {
    co_debug("co_mark_read: %s %d\n", task->name, fd);
    struct epoll_event ev;
    ev.events |= EPOLLIN;
    ev.data.u64 = (uint64_t) task;
    int ret = epoll_ctl(co_epoll_fd, EPOLL_CTL_ADD, fd, &ev);
    if (ret != 0)
    {
        if (errno == EEXIST)
            return;
        perror("co_mark_read fail:");
        exit(1);
    }
}

void co_mark_write(int fd, Task *task) {
    co_debug("co_mark_write: %s %d\n", task->name, fd);
    struct epoll_event ev;
    ev.events |= EPOLLOUT;
    ev.data.u64 = (uint64_t) task;
    int ret = epoll_ctl(co_epoll_fd, EPOLL_CTL_ADD, fd, &ev);
    if (ret != 0)
    {
        if (errno == EEXIST)
            return;
        perror("co_mark_write fail:");
        exit(1);
    }
}

ssize_t co_read(int fd, void *buf, size_t count) {
    co_mark_read(fd, task_cur);
    co_swap(&task_cur->ctx, &ctx_main);
    co_debug("co_read: %s\n", task_cur->name);
    ssize_t nr = read(fd, buf, count);
    co_unmark(fd, task_cur);
    return nr;
}

ssize_t co_write(int fd, const void *buf, size_t count) {
    co_mark_write(fd, task_cur);
    co_swap(&task_cur->ctx, &ctx_main);
    co_debug("co_write: %s\n", task_cur->name);
    ssize_t nw = write(fd, buf, count);
    co_unmark(fd, task_cur);
    return nw;
}

ssize_t co_accept(int fd, struct sockaddr *restrict addr, socklen_t *restrict addrlen) {
    co_mark_read(fd, task_cur);
    co_swap(&task_cur->ctx, &ctx_main);
    co_debug("co_accept: %s\n", task_cur->name);
    return accept(fd, addr, addrlen);
}

void co_io_init() {
    co_create(co_io_scheduler, 0);
}
