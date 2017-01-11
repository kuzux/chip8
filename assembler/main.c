#include "main.h"

void print_header() {
    printf("chip8-as v%s\n", VERSION);
}

void error(char* msg) {
    fprintf(stderr, "%s\n", msg);
    exit(1);
}

int main(int argc, char** argv) {
    if(argc < 2){
        error("No input file");
    }

    FILE* f = fopen(argv[1], "rb");

    if(!f){
        error("No such file or directory");
    }

    print_header();

    return 0;
}