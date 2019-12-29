#include <stdio.h>
#include <string>

#include <emscripten.h>

extern "C" int main(int argc, char **argv) {
    printf("Hello,world!\n");
    return 0;
}