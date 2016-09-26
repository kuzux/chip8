#include "main.h"

// internal memory state
uint8_t mem[4096];
uint8_t regs[16];
uint32_t stack[16];

int32_t delay_timer;
int32_t snd_timer;
bool done;
uint32_t pc;
uint32_t sp;

uint16_t read_be(uint32_t addr){
    uint16_t byte1 = mem[addr];
    uint16_t byte2 = mem[addr+1];

    return (byte2 << 8) | byte1;
}

void error(char* msg){
    printf("error: %s\n", msg);
    exit(1);
}

void* delay_fun(void* arg){
    while(!done){
        if(delay_timer > 0){
            delay_timer--;
        }
    }

    return NULL;
}

void* snd_fun(void* arg){
    while(!done){
        if(snd_timer > 0){
            snd_timer--;
        }
    }

    return NULL;
}

void init_state(){
    delay_timer = -1;
    snd_timer = -1;
    done = 1;
    pc = 0x200;
    sp = 0;
}

void load_file(FILE* f){
    int addr = 0x200;

    while(!feof(f)){
        mem[addr] = fgetc(f);
        addr++;   
    }
}

void do_cls(){

}

void do_ret(){
    pc = stack[sp];
    sp--;
}

void handle(uint16_t instr){
    uint16_t op = instr & 0xF000;
    switch(op){
        case 0x0:
        if(instr==0x00e0) {
            do_cls();
        } else if(instr==0x00ee) {
            do_ret();
        } else {
            // ignore this
        }
        break;
        case 0x1:
        break;
        case 0x2:
        break;
        case 0x3:
        break;
        case 0x4:
        break;
        case 0x5:
        break;
        case 0x6:
        break;
        case 0x7:
        break;
        case 0x8:
        break;
        case 0x9:
        break;
        case 0xA:
        break;
        case 0xB:
        break;
        case 0xC:
        break;
        case 0xD:
        break;
        case 0xE:
        break;
        case 0xF:
        break;
    }
    
    printf("%x\n", instr);
}

void run_program(){
    done = 1;

    for(;;){
        uint16_t instr = read_be(pc);
        handle(instr);
        pc += 2;
        if(pc >= 0x220){
            break;
        }
    }
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

    load_file(f);

    fclose(f);

    pthread_t tid, tid2;

    init_state();
    pthread_create(&tid, NULL, delay_fun, NULL);
    pthread_create(&tid2, NULL, snd_fun, NULL);

    run_program();
    
    pthread_join(tid, NULL);
    pthread_join(tid2, NULL);
    cleanup();

    return 0;
}
