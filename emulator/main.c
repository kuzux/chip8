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
uint32_t idx; // a specific idx register

uint8_t kb[16];
uint8_t display[SCR_SIZE];

int debugmode;
int mute;

pthread_t delay_tid, snd_tid;

SDL_Window* win;
SDL_Renderer *render;

// pointer to memory area that stores the character sprites
uint16_t* chars;

uint16_t read_be(uint32_t addr) {
    uint16_t byte1 = mem[addr];
    uint16_t byte2 = mem[addr+1];

    return (byte2 << 8) | byte1;
}

void error(const char* msg) {
    fprintf(stderr, "error: %s\n", msg);
    exit(1);
}

void* delay_fun(void* arg) {
    while(!done){
        if(delay_timer > 0){
            delay_timer--;
        }
    }

    return NULL;
}

void* snd_fun(void* arg) {
    while(!done){
        if(snd_timer > 0){
            snd_timer--;
        }
    }

    return NULL;
}

void write_chars() {

}

void init_state() {
    delay_timer = -1;
    snd_timer = -1;
    done = 1;
    pc = 0x200;
    sp = 0;
    chars = (uint16_t*)(mem + 0x100);
    write_chars();

    srand(time(NULL));
}

void init_threads() {
    pthread_create(&delay_tid, NULL, delay_fun, NULL);
    if(!mute) {
        pthread_create(&snd_tid, NULL, snd_fun, NULL);
    }
}

void init_sdl() {
    if(SDL_Init(SDL_INIT_VIDEO)) {
        error(SDL_GetError());
    }

    win = SDL_CreateWindow(WINDOW_TITLE, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);

    if(!win) {
        SDL_Quit();
        error(SDL_GetError());
    }

    render = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
}

void draw_pixel(SDL_Renderer* rend, int i, int j, uint8_t pixel) {
    SDL_Rect rect;
    rect.x = PIXEL_WIDTH*j;
    rect.y = PIXEL_HEIGHT*i;
    rect.w = PIXEL_WIDTH;
    rect.h = PIXEL_HEIGHT;

    if (pixel) {
        SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);
    } else {
        SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
    }

    SDL_RenderFillRect(rend, &rect);
}

void draw_screen() {
    uint32_t i, j;
    for (i=0; i < SCR_HEIGHT; i++) {
        for (j=0; j < SCR_WIDTH; j++) {
            draw_pixel(render, i, j, display[i*SCR_WIDTH+j]);
        }
    }
}

void load_file(FILE* f) {
    int addr = 0x200;

    while(!feof(f)){
        mem[addr] = fgetc(f);
        addr++;   
    }
}

void do_cls() {
    int i;
    for(i=0;i<SCR_SIZE;i++) {
        display[i] = 0;
    }
}

void do_ret() {
    pc = stack[sp];
    sp--;
}

void do_call(uint32_t addr) {
    stack[sp++] = pc;
    pc = addr;
}

void do_skip_next(uint32_t reg, uint32_t val) {
    if(regs[reg]==val) {
        pc += 2;
    }
}

void do_skip_next_neq(uint32_t reg, uint32_t val) {
    if(regs[reg]!=val) {
        pc += 2;
    }
}

void do_skip_next_eq_reg(uint32_t reg1, uint32_t reg2) {
    if(regs[reg1]==regs[reg2]) {
        pc += 2;
    }
}

void do_load(uint32_t reg, uint32_t val) {
    regs[reg] = val;
}

void do_add(uint32_t reg, uint32_t val) {
    regs[reg] += val;
}

void do_arith(uint32_t reg1, uint32_t reg2, uint32_t op) {
    switch(op) {
        case 0x0:
            regs[reg1] = regs[reg2];
            break;
        case 0x1:
            regs[reg1] |= regs[reg2];
            break;
        case 0x2:
            regs[reg1] &= regs[reg2];
            break;
        case 0x3:
            regs[reg1] ^= regs[reg2];
            break;
        case 0x4:
            if((int)regs[reg1] + (int)regs[reg2] > 256) {
                regs[0xF] = 1;
            }
            regs[reg1] += regs[reg2];
            break;
        case 0x5:
            if((int)regs[reg1] - (int)regs[reg2] < 0) {
                regs[0xF] = 1;
            }
            regs[reg1] -= regs[reg2];
            break;
        case 0x6:
            if(regs[reg1] & 0x0001) {
                regs[0xF] = 1;
            } else {
                regs[0xF] = 0;
            }

            regs[reg1] >>= regs[reg2];
            break;
        case 0x7:
            if((int)regs[reg2] - (int)regs[reg1] < 0) {
                regs[0xF] = 1;
            }
            regs[reg1] = regs[reg2] - regs[reg1];
            break;
        case 0xE:
            if(regs[reg1] & 0x8000) {
                regs[0xF] = 1;
            } else {
                regs[0xF] = 0;
            }

            regs[reg1] <<= regs[reg2];
            break;
        default:
            fprintf(stderr, "unexpected arithmetic operator %d\n", op);
    }
}

void do_skip_next_neq_reg(uint32_t reg1, uint32_t reg2) {
    if(regs[reg1]!=regs[reg2]) {
        pc += 2;
    }
}

void do_load_idx(uint32_t addr) {
    idx = addr;
}

void do_jmp_off(uint32_t addr) {
    pc = addr + regs[0x0];
}

void do_rnd(uint32_t reg, uint32_t mask) {
    regs[reg] = (rand() % 256) & mask;
}

void do_draw(uint32_t vx, uint32_t vy, uint32_t len) {

}

void do_skip_press(uint32_t key) {
    if(kb[key]) {
        pc += 2;
    }
}

void do_skip_nopress(uint32_t key) {
    if(!kb[key]) {
        pc += 2;
    }
}

void do_kb(uint32_t key, uint32_t op) {
    switch(op) {
        case 0x9E:
        do_skip_press(key);
        break;
        case 0xA1:
        do_skip_nopress(key);
        break;
        default:
        fprintf(stderr, "wrong keyboard op %d\n", op);
    }
}

void do_misc(uint32_t reg, uint32_t op) {
    switch(op) {
        default:
        fprintf(stderr, "wrong op %d\n", op);
    }
}

void handle(uint16_t instr) {
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
        pc = instr & 0x0FFF;
        break;
        case 0x2:
        do_call(instr & 0x0FFF);
        break;
        case 0x3:
        do_skip_next(instr & 0x0F00, instr & 0x00FF);
        break;
        case 0x4:
        do_skip_next_neq(instr & 0x0F00, instr & 0x00FF);
        break;
        case 0x5:
        do_skip_next_eq_reg(instr & 0x0F00, instr & 0x00F0);
        break;
        case 0x6:
        do_load(instr & 0x0F00, instr & 0x00FF);
        break;
        case 0x7:
        do_add(instr & 0x0F00, instr & 0x00FF);
        break;
        case 0x8:
        do_arith(instr & 0x0F00, instr & 0x00F0, instr & 0x000F);
        break;
        case 0x9:
        do_skip_next_neq_reg(instr & 0x0F00, instr & 0x00F0);
        break;
        case 0xA:
        do_load_idx(instr & 0x0FFF);
        break;
        case 0xB:
        do_jmp_off(instr & 0x0FFF);
        break;
        case 0xC:
        do_rnd(instr & 0x0F00, instr & 0x00FF);
        break;
        case 0xD:
        do_draw(instr & 0x0F00, instr & 0x00F0, instr & 0x000F);
        break;
        case 0xE:
        do_kb(instr & 0x0F00, instr & 0x00FF);
        break;
        case 0xF:
        do_misc(instr & 0x0F00, instr & 0x00FF);
        break;
    }
    
    printf("%x\n", instr);
}

void run_program() {
    done = 1;
    uint8_t quit = 0;

    while(!quit) {
        uint16_t instr = read_be(pc);
        handle(instr);
        pc += 2;
        if(pc >= 0x220) {
            break;
        }
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }
        //Render the scene
        SDL_RenderClear(render);
        SDL_RenderPresent(render);
    }
}

void cleanup() {
    pthread_join(delay_tid, NULL);
    
    if(!mute) {
        pthread_join(snd_tid, NULL);
    }

    SDL_DestroyRenderer(render);
    SDL_DestroyWindow(win);
    SDL_Quit();
}

void print_version() {
    printf("Chip 8 emulator version %s\n", VERSION);
}

void print_help() {
    print_version();

    printf("Chip8 emulator, just supply a ROM file\n\n");
    printf("USAGE:\n");
    printf("chip8 -[gmhv] file\n\n");
    printf("OPTIONS:\n");
    printf("-g: enable debug mode. implies -m\n");
    printf("-m: mute. no sound. \n");
    printf("-h: print this help message.\n");
    printf("-v: show version.\n");
}

int main(int argc, char** argv){
    int c;

    while((c = getopt(argc, argv, "gmhv")) != -1) {
        switch(c) {
            case 'g':
            debugmode = 1;
            mute = 1;
            break;
            case 'm':
            mute = 1;
            break;
            case 'h':
            print_help();
            return 0;
            case 'v':
            print_version();
            return 0;

            default:
            abort();
        }
    }

    int num_args = argc - optind;

    if(num_args < 1) {
        error("No input file");
    }

    FILE* f = fopen(argv[optind], "rb");

    if(!f) {
        error("No such file or directory");
    }

    load_file(f);

    fclose(f);

    if(debugmode) {
        printf("debug mode\n");
    }

    init_state();
    init_threads();
    init_sdl();

    run_program();

    cleanup();

    return 0;
}
