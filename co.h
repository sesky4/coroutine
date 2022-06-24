#include <stdlib.h>
#include <stdint.h>

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

extern int co_get_ctx(Context *);

extern void co_set_ctx(Context *);

typedef struct Task
{
    char *name;
    Context ctx;

    void (*entrypoint)();

    struct Task *next;
} Task;


static Task *task_cur = NULL;
static Task *task_ready = NULL;
static Context ctx_main;

Task *co_get_last_ready() {
    Task *last = task_ready;
    while (last->next != NULL)
        last = last->next;
    return last;
}

void co_set_ready(Task *t) {
    if (task_ready == NULL)
    {
        task_ready = t;
    }
    else
    {
        co_get_last_ready()->next = t;
    }
}

void co_swap(Context *from, Context *to) {
    if (co_get_ctx(from) == 0)
    {
        co_set_ctx(to);
    }
}

void co_exit() {
    Context _;
    co_swap(&_, &ctx_main);
}

void co_create(char *name, void(*fn)()) {
    Task *t = (Task *) malloc(sizeof(Task));
    t->name = name;
    t->entrypoint = fn;
    t->next = NULL;
    t->ctx.rip = (uint64_t) fn;

    // create stack
    uint64_t stack_size = 4096;
    uint64_t sp = (uint64_t) malloc(stack_size);
    sp += stack_size - 8;
    *((uint64_t *) sp) = (uint64_t) co_exit;
    t->ctx.rsp = sp;

    co_set_ready(t);
}


void co_yield() {
    co_set_ready(task_cur);
    co_swap(&task_cur->ctx, &ctx_main);
}

void co_run() {
    while (1)
    {
        if (task_ready == NULL)
        {
            printf("no task ready\n");
            return;
        }

        task_cur = task_ready;
        task_ready = task_ready->next;
        task_cur->next = NULL;

        co_swap(&ctx_main, &task_cur->ctx);
    }
}
