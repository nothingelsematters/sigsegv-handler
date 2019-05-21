#include "../handler.hpp"

int main() {
    set_handler(SIGSEGV, handler);
    int *ptr = nullptr;
    ++(*ptr);
    return 0;
}
