#ifndef HANDLER_HPP
#define HANDLER_HPP


#include <signal.h>
#include <string.h>
#include <setjmp.h>
#include <unistd.h>
#include <cstdio>
#include <stdlib.h>
#include <utility>
#include <cstring>
#include <limits>

void writer(const char* buffer) {
    write(1, buffer, strlen(buffer));
}

void writer(long long num) {
    if (num) {
        writer(num >> 4);
        long long digit = num & ((1ll << 4) - 1);
        char symbol = ((digit > 9 ? 'a' - 10 : '0') + digit);
        write(1, &symbol, 1);
    }
}

template <typename Arg, typename ArgS, typename... Args>
void writer(Arg arg, ArgS arg2, Args... args) {
    writer(arg);
    writer(arg2, args...);
}

void set_handler(int signo, void (*handler) (int, siginfo_t*, void*)) {
    struct sigaction act{};

    act.sa_sigaction = handler;
    act.sa_flags = SA_SIGINFO;
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, signo);
    act.sa_mask = set;

    if (sigaction(signo, &act, nullptr) == -1) {
        std::perror("setting signal error occurred");
        exit(EXIT_FAILURE);
    }
}


static constexpr size_t DUMP_MEMORY_RANGE = 50;
static jmp_buf jmpbuf;

void help_handler(int sig, siginfo_t* info, void* context) {
    if (info->si_signo == SIGSEGV) {
        siglongjmp(jmpbuf, 1);
    }
}

void handler(int sig, siginfo_t *info, void *context) {
    static const std::pair<const char*, int> regs[] = {
        {"R8", REG_R8},
        {"R9", REG_R9},
        {"R10", REG_R10},
        {"R11", REG_R11},
        {"R12", REG_R12},
        {"R13", REG_R13},
        {"R14", REG_R14},
        {"R15", REG_R15},
        {"CR2", REG_CR2},
        {"RIP", REG_RIP},
        {"RBP", REG_RBP},
        {"RBX", REG_RBX},
        {"RAX", REG_RAX},
        {"RCX", REG_RCX},
        {"RDX", REG_RDX},
        {"RSI", REG_RSI},
        {"RDI", REG_RDI},
        {"RSP", REG_RSP},
        {"EFL", REG_EFL},
        {"CSGSFS", REG_CSGSFS},
        {"ERR", REG_ERR},
        {"TRAPNO", REG_TRAPNO},
        {"OLDMASK", REG_OLDMASK},
    };

    // exit(EXIT_FAILURE);
    writer("Aborted: ", static_cast<const char*>(strsignal(sig)), "\n");
    writer("- reason: ");
    switch (info->si_code) {
        case SEGV_MAPERR:
            writer("address not mapped");
            break;
        case SEGV_ACCERR:
            writer("invalid permissions");
            break;
        default:
            writer("¯\\_(ツ)_/¯");
    }

    intptr_t address = reinterpret_cast<intptr_t>(info->si_addr);
    writer("\n- fault address: ", address, "\n");

    greg_t* gregs = reinterpret_cast<ucontext_t *>(context)->uc_mcontext.gregs;
    writer("- general purpose registers:\n");
    for (auto [name, index] : regs) {
        writer("  + ", name, ": ", gregs[index], "\n");
    }

    writer("- memory nearby: ");
    if (address == 0) {
        writer("nullptr, apparently\n");
        exit(EXIT_FAILURE);
    }

    const size_t SIZE = sizeof(char);
    const long long LLM = std::numeric_limits<long long>::max();
    const long long from = address > DUMP_MEMORY_RANGE * SIZE ? address - DUMP_MEMORY_RANGE * SIZE : address;
    const long long to = LLM - DUMP_MEMORY_RANGE * SIZE > address ? (address + DUMP_MEMORY_RANGE * SIZE) : LLM;
    sigset_t sset;
    sigemptyset(&sset);
    sigaddset(&sset, SIGSEGV);

    for (long long addr = from; addr < to; addr += SIZE) {
        sigprocmask(SIG_UNBLOCK, &sset, nullptr);
        set_handler(SIGSEGV, help_handler);

        if (addr == address) {
            writer("!!");
        } else if (setjmp(jmpbuf) != 0) {
            writer("??");
        } else {
            writer(+*reinterpret_cast<char*>(addr));
        }
        writer(" ");
    }

    exit(EXIT_FAILURE);
}


#endif /* end of include guard: HANDLER_HPP */
