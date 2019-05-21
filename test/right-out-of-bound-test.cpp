#include "../handler.hpp"

int main() {
    set_handler(SIGSEGV, handler);
    char a[1];

    for (auto ptr = a; ; ++ptr) {
        ++*ptr;
    }
    return 0;
}
