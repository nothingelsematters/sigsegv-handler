#include <sys/mman.h>
#include "../handler.hpp"


static const int PAGE_SIGE = 8192;

int main() {
    set_handler(SIGSEGV, handler);
    void *memory = mmap(NULL, PAGE_SIGE, PROT_READ, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    if (memory == MAP_FAILED) {
        perror("failed to map memory");
        std::cout << "test failed, try again later\n";
        return EXIT_FAILURE;
    }

    ++*(static_cast<char*>(memory) + 10);
    return 0;
}
