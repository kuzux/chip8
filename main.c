#include "main.h"

// internal memory state
uint8_t mem[4096];
uint8_t regs[16];
uint32_t stack[16];

int32_t delay_timer;
int32_t snd_timer;
bool done;
uint32_t pc;

void error(char* msg){
    printf("error: %s\n", msg);
    exit(1);
}

void init_state(){
    delay_timer = -1;
    snd_timer = -1;
    done = 0;
    pc = 0;
}

void run_program(FILE* f){
    while(!feof(f)){
        
        pc++;
    }
    done = 1;
}

void cleanup(){

}

int main(int argc, char** argv){
    if(argc < 2){
        error("No input file");
    }

    FILE* f = fopen(argv[1], "rb");

    if(!f){
        error("No such file or directory");
    }

    init_state();
    run_program(f);
    cleanup();

    fclose(f);

    return 0;
}
