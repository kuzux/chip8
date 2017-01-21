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

int ischar(char* buf, char c) {
    if(*buf == tolower(c) || *buf==toupper(c)) {
        return 1;
    } else {
        return 0;
    }
}

int isstr(char* buf, char* str) {
    int n = strlen(buf);
    int m = strlen(str);

    if(n != m) return 0;

    int i;
    for(i = 0; i<m; i++) {
        if(!ischar(buf, *str)) return 0;
        buf++;
        str++;
    }

    return 1;
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

        if(!strcmp(op, "sys")) {
            int n; 
            readint(buf, &n);

            write_instr(0x0FFF & (n >> 1));
        } else if(!strcmp(op, "cls")) {
            write_instr(0x00E0);
        } else if(!strcmp(op, "ret")) {
            write_instr(0x00EE);
        } else if(!strcmp(op, "call")) {
            int n; 
            readint(buf, &n);

            write_instr(0x2000 | (n & 0x0FFF));
        } else if(!strcmp(op, "jmp")) {
            int n;

            if(!isreg(buf, 0)) {
                // got an address
                // jmp nnn
                readint(buf, &n);

                write_instr(0x2000 | (n & 0x0FFF));
            } else {
                // got a register, read another integer
                // jmp v0 nnn                
                buf = strtok(NULL, " ");

                readint(buf, &n);

                write_instr(0xB000 | (n & 0x0FFF));
            }
        } else if(!strcmp(op, "se")) {
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
        } else if(!strcmp(op, "sne")) {
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
        } else if(!strcmp(op, "ld")) {
            // TODO fill those in
            int n, m;

            if(isstr(buf, "i")) {
                // ld i nnn
                buf = strtok(NULL, " ");

                readint(buf, &n);
                write_instr(0xA000 | (n & 0x0FFF));
            } else if(isstr(buf, "st")) {
                // ld st vx
                buf = strtok(NULL, " ");

                readreg(buf, &n);
                write_instr(0xF018 | (n & 0x0F00));
            } else if(isstr(buf, "dt")) {
                // ld dt vx
                buf = strtok(NULL, " ");

                readreg(buf, &n);
                write_instr(0xF015 | (n & 0x0F00));
            } else if(isstr(buf, "[i]")) {
                // ld [i] vx
                buf = strtok(NULL, " ");

                readreg(buf, &n);
                write_instr(0xF055 | (n & 0x0F00));
            } else if(isstr(buf, "f")) {
                // ld f vx
                buf = strtok(NULL, " ");

                readreg(buf, &n);
                write_instr(0xF029 | (n & 0x0F00));
            } else if(isstr(buf, "b")) {
                // ld b vx
                buf = strtok(NULL, " ");

                readreg(buf, &n);
                write_instr(0xF033 | (n & 0x0F00));
            } else {
                // first operand is a register
                readreg(buf, &n);

                buf = strtok(NULL, " ");

                // switch on the new buf value
                if(isstr(buf, "dt")) {

                } else if(isstr(buf, "k")) {

                } else if(isstr(buf, "[i]")) {
                    
                } else if(!readreg(buf, &m)) {
                    // got a register
                } else {
                    // got a byte value
                }
            }
        } else if(!strcmp(op, "add")) {
            if(ischar(buf, 'i')) {
                // got an add i vx type instruction

                // get the vx bit
                buf = strtok(NULL, " ");

                int n;
                readreg(buf, &n);

                write_instr(0xF01E | (n & 0x0F00));
            } else {
                int n, m;
                readreg(buf, &n);
                
                buf = strtok(NULL, " ");

                if(!readreg(buf, &m)) {
                    // add vx vy

                    write_instr(0x8004 | (n & 0x0F00) | (m & 0x00F0));
                } else {
                    // add vx nnn
                    readint(buf, &m);
                    write_instr(0x7000 | (n & 0x0F00) | (m & 0x00FF));       
                }
            }
        } else if(!strcmp(op, "and")) {
            // and vx vy
            int n, m;

            readreg(buf, &n);
                
            buf = strtok(NULL, " ");

            readreg(buf, &m);

            write_instr(0x8002 | (n & 0x0F00) | (m & 0x00F0));
        } else if(!strcmp(op, "or")) {
            // or vx vy
            int n, m;

            readreg(buf, &n);
                
            buf = strtok(NULL, " ");

            readreg(buf, &m);

            write_instr(0x8001 | (n & 0x0F00) | (m & 0x00F0));
        } else if(!strcmp(op, "xor")) {
            // xor vx vy
            int n, m;

            readreg(buf, &n);
                
            buf = strtok(NULL, " ");

            readreg(buf, &m);

            write_instr(0x8003 | (n & 0x0F00) | (m & 0x00F0));
        } else if(!strcmp(op, "sub")) {
            // sub vx vy
            int n, m;

            readreg(buf, &n);
                
            buf = strtok(NULL, " ");

            readreg(buf, &m);

            write_instr(0x8005 | (n & 0x0F00) | (m & 0x00F0));
        } else if(!strcmp(op, "subn")) {
            // subn vx vy
            int n, m;

            readreg(buf, &n);
                
            buf = strtok(NULL, " ");

            readreg(buf, &m);

            write_instr(0x8007 | (n & 0x0F00) | (m & 0x00F0));
        } else if(!strcmp(op, "shr")) {
            // shr vx
            int n;

            readreg(buf, &n);

            write_instr(0x8006 | (n & 0x0F00));
        } else if(!strcmp(op, "shl")) {
            // shl vx
            int n;

            readreg(buf, &n);

            write_instr(0x800E | (n & 0x0F00));
        } else if(!strcmp(op, "rnd")) {
            // rnd vx nnn
            int n, m;

            readreg(buf, &n);

            buf = strtok(NULL, " ");

            readint(buf, &m);

            write_instr(0xC000 | (n & 0x0F00) | (m & 0x00FF));
        } else if(!strcmp(op, "drw")) {
            // drw vx vy n
            int n, m, k;

            readreg(buf, &n);

            buf = strtok(NULL, " ");

            readreg(buf, &m);

            buf = strtok(NULL, " ");

            readint(buf, &k);

            write_instr(0xD000 | (n & 0x0F00) | (m & 0x00F0) | (k & 0x000F));
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