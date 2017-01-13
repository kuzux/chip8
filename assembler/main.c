#include "main.h"

uint16_t outbuf[65536];
uint32_t outidx;

void write_instr(uint16_t instr) {
    outbuf[outidx] = instr;
    outidx++;
}

void print_header() {
    printf("chip8-as v%s\n", VERSION);
}

void error(char* msg) {
    fprintf(stderr, "error: %s\n", msg);
    exit(1);
}

void chomp(char* str, int n) {
    if(str[n-1] == '\n') {
        str[n-1] = 0;
    }
}

void downcase(char* str, int n) {
    int i;
    for(i=0;i<n;i++) {
        str[i] = tolower(str[i]);
    }
}

void readint(char* str, int* x) {
    str = strtok(NULL, " "); 
    *x = atoi(str);

    if(*x > 65535) {
        fprintf(stderr, "warning: operand too large: %d\n", *x);
    }
}

int readreg(char* str, int* n) {
    if(*str=='v'||*str=='V') {
        str++;
        if(isxdigit(*str)) {
            *n = strtol(str, NULL, 16);
            return 0;
        } else {
            return 1;
        }
    } else {
        return 1;
    }
}

int isreg(char* str, int n) {
    if(*str=='v'||*str=='V') {
        str++;
        if(*str - '0' == n) {
            return 1;
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}

int isidx(char* buf) {
    if(*buf == 'i' || *buf=='I') {
        return 1;
    } else {
        return 0;
    }
}

void handle_file(FILE* f) {
    char* buf;
    buf = malloc(256);
    buf[255] = 0;
    char* op;

    int line = 1;

    while(fgets(buf, 255, f)) {
        op = strtok(buf, " ");

        chomp(op, strlen(op));
        downcase(op, strlen(op));

        buf = strtok(NULL, " ");

        if(!strcmp(buf, "sys")) {
            int n; 
            readint(buf, &n);

            write_instr(0x0FFF & (n >> 1));
        } else if(!strcmp(buf, "cls")) {
            write_instr(0x00E0);
        } else if(!strcmp(buf, "ret")) {
            write_instr(0x00EE);
        } else if(!strcmp(buf, "call")) {
            int n; 
            readint(buf, &n);

            write_instr(0x2000 | (n & 0x0FFF));
        } else if(!strcmp(buf, "jmp")) {
            int n;

            if(!isreg(buf, 0)) {
                // got an address
                readint(buf, &n);

                write_instr(0x2000 | (n & 0x0FFF));
            } else {
                // got a register, read another integer                
                buf = strtok(NULL, " ");

                readint(buf, &n);

                write_instr(0xB000 | (n & 0x0FFF));
            }
        } else if(!strcmp(buf, "se")) {
            int n, m;
            readreg(buf, &n);

            buf = strtok(NULL, " ");

            if(readreg(buf, &m)) {
                // immediate value
                readint(buf, &m);
                write_instr(0x3000 | (n & 0x0F00) | (m &0x00FF));
            } else {
                // 2nd arg is a register
                write_instr(0x5000 | (n & 0x0F00) | (m & 0x00F0));
            }
        } else if(!strcmp(buf, "sne")) {
            int n, m;
            readreg(buf, &n);

            buf = strtok(NULL, " ");

            if(readreg(buf, &m)) {
                // immediate value
                readint(buf, &m);
                write_instr(0x4000 | (n & 0x0F00) | (m &0x00FF));
            } else {
                // 2nd arg is a register
                write_instr(0x9000 | (n & 0x0F00) | (m & 0x00F0));
            }
        } else {
            fprintf(stderr, "invalid op %s at line %d\n", op, line);
        }
        line++;

    }
}

void write_out(char* filename) {
    FILE* outfile = fopen(filename, "wb");
    if(!outfile) {
        fprintf(stderr, "No such file or directory:\n");
        error(filename);
    }

    fwrite(outbuf, outidx, 1, outfile);
}

int main(int argc, char** argv) {
    if(argc < 2){
        error("No input file");
    }

    FILE* f = fopen(argv[1], "rb");

    if(!f){
        fprintf(stderr, "No such file or directory:\n");
        error(argv[1]);
    }

    print_header();
    handle_file(f);
    fclose(f);
    write_out("a.rom");

    return 0;
}