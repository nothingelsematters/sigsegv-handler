#ifndef HANDLER_HPP
#define HANDLER_HPP


#include <signal.h>
#include <string.h>
#include <setjmp.h>
#include <limits>
#include <iostream>


void set_handler(int signo, void (*handler) (int, siginfo_t*, void*)) {
    struct sigaction act{};

    act.sa_sigaction = handler;
    act.sa_flags = SA_SIGINFO;
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, signo);
    act.sa_mask = set;

    if (sigaction(signo, &act, nullptr) == -1) {
        perror("setting signal error occurred");
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
    static const std::pair<std::string, int> regs[] = {
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

    std::cout << "Aborted: " << strsignal(sig) << '\n';
    std::string reason;
    switch (info->si_code) {
        case SEGV_MAPERR:
            reason = "address not mapped";
            break;
        case SEGV_ACCERR:
            reason = "invalid permissions";
            break;
        default:
            reason = "¯\\_(ツ)_/¯";
    }
    std::cout << "- reason: " << reason << '\n';

    intptr_t address = reinterpret_cast<intptr_t>(info->si_addr);
    std::cout << "- fault address: " << std::hex << address << '\n';

    greg_t* gregs = reinterpret_cast<ucontext_t *>(context)->uc_mcontext.gregs;
    std::cout << "- general purpose registers:\n";
    for (auto [name, index] : regs) {
        std::cout << "  + " << name << ": " << gregs[index] << '\n';
    }

    std::cout << "- memory nearby: ";
    if (address == 0) {
        std::cout << "nullptr, apparently\n";
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
            std::cout << "!!";
        } else if (setjmp(jmpbuf) != 0) {
            std::cout << "??";
        } else {
            std::cout << +*reinterpret_cast<char*>(addr);
        }
        std::cout << ' ';
    }

    exit(EXIT_FAILURE);
}


#endif /* end of include guard: HANDLER_HPP */
