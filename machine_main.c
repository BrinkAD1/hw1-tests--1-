#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "bof.h"
#include "instruction.h"
#include "regname.h"
#include "utilities.h"
#include "machine_types.h"

// a size for the memory (2^16 bytes = 64K)
#define MEMORY_SIZE_IN_BYTES (65536 - BYTES_PER_WORD)
#define MEMORY_SIZE_IN_WORDS (MEMORY_SIZE_IN_BYTES / BYTES_PER_WORD)

static union mem_u {
     byte_type bytes[MEMORY_SIZE_IN_BYTES];
     word_type words[MEMORY_SIZE_IN_WORDS];
     bin_instr_t instrs[MEMORY_SIZE_IN_WORDS];
} memory;

void disasmTextSection(BOFFILE bf, BOFHeader bh)
{
    printf(".text %u\n", bh.text_start_address);
    printf(".text %u\n", bh.text_length); 
}


void InstrPrint(bin_instr_t bi, unsigned int i)
{
    printf("%d:\t%s", i, instruction_assembly_form(bi));
    printf("\n");
}


int main(int argc, char *argv[]) 
{
    word_type register_array[32];

    //initialize register array to 0
    for(int i =0; i < 32; i ++ )
    {
        register_array[i] = 0;
    }

    //initialize PC to 0
    int PC = 0;
    //default tracing is off
    int trace = 1;                

    if(strcmp(argv[1], "-p") == 0 )
    {

        BOFFILE boffile = bof_read_open(argv[2]);

        BOFHeader boffile_header = bof_read_header(boffile);

        int length = boffile_header.text_length / BYTES_PER_WORD;

        printf("Addr Instruction\n");
        for(int i = 0; i < length; i++)
        {
            InstrPrint( instruction_read(boffile), i*BYTES_PER_WORD);
        }

        printf("\t%u\n", boffile_header.data_start_address);
    }

    if(argc == 2)
    {
        BOFFILE boffile = bof_read_open(argv[1]);

        BOFHeader boffile_header = bof_read_header(boffile);

        int length = boffile_header.text_length / BYTES_PER_WORD;
        for(int i =0; i < length ; i++)
        {
            if(trace)
            {
                printf("PC: %d", PC);
                for(int i =0; i < 32; i ++ )
                {
                    
                    if(i % 6 ==0)
                        printf("\n");
                        
                    printf("%s: %d\t", regname_get(i), register_array[i]);
                }
            }

            bin_instr_t bi = instruction_read(boffile);

            instr_type instr = instruction_type(bi);

            switch(instr)
            {
                //increment PC on each call
                

                case(reg_instr_type):
                    printf("reg_instr_type\n");
                    printf("reg.func: %u reg.rd: %u reg.rs %u reg.rt: %u reg.shift: %u\n",bi.reg.func, bi.reg.rd, bi.reg.rs, bi.reg.rt, bi.reg.shift);
                    switch(bi.reg.func)
                    {
                        case(ADD_F):
                            register_array[bi.reg.rd] = register_array[bi.reg.rs] + register_array[bi.reg.rt];
                            break;
                        case(SUB_F):
                            register_array[bi.reg.rd] = register_array[bi.reg.rs] - register_array[bi.reg.rt];
                            break;
                        case(MUL_F):
                            break;
                        case(DIV_F):
                            break;
                        case(MFHI_F):
                            break;
                        case(MFLO_F):
                            break;
                        case(AND_F):
                            register_array[bi.reg.rd] = register_array[bi.reg.rs] & register_array[bi.reg.rt];
                            break;
                        case(BOR_F):
                            register_array[bi.reg.rd] = register_array[bi.reg.rs] | register_array[bi.reg.rt];
                            break;
                        case(NOR_F):
                            register_array[bi.reg.rd] = ~(register_array[bi.reg.rs] | register_array[bi.reg.rt]);
                            break;
                        case(XOR_F):
                            register_array[bi.reg.rd] = register_array[bi.reg.rs] ^ register_array[bi.reg.rt];
                            break;
                        case(SLL_F):
                            register_array[bi.reg.rd] = register_array[bi.reg.rt] << bi.reg.shift;
                            break;
                        case(SRL_F):
                            register_array[bi.reg.rd] = register_array[bi.reg.rt] >> bi.reg.shift;
                            break;
                        case(JR_F):
                            break;
                        case(SYSCALL_F):
                            break;
                    }
                    break;
                case(syscall_instr_type):
                    printf("syscall_instr_type\n");
                    printf("code: %u func: %u op: %u\n", bi.syscall.code, bi.syscall.func, bi.syscall.op);
                    switch(bi.syscall.code)
                    {
                        case(exit_sc):
                            exit(0);
                            break;
                        case(print_str_sc):
                            printf("print_str_sc\n");
                            break;
                        case(print_char_sc):
                            printf("print char sc\n");
                            break;
                        case(read_char_sc):
                            printf("read char sc\n");
                            break;
                        case(start_tracing_sc):
                            printf("start tracing sc\n");
                            trace = 1;
                            break;
                        case(stop_tracing_sc):
                            printf("stop char sc\n");
                            trace = 0;
                            break;
                    }
                    break;
                case(immed_instr_type):
                    printf("immed_inst_type\n");
                    printf("op: %u rs: %u rt: %u\n",bi.immed.op, bi.immed.rs, bi.immed.rt);
                    switch(bi.immed.op)
                    {
                        case(REG_O):
                            break;
                        case(ADDI_O):
                            register_array[bi.immed.rt] = bi.immed.immed + register_array[bi.immed.rs];
                            break;
                        case(ANDI_O):
                            break;
                        case(BORI_O):
                            break;
                        case(XORI_O):
                            break;
                        case(BEQ_O):
                            break;
                        case(BGEZ_O):
                            break;
                        case(BGTZ_O):
                            break;
                        case(BLEZ_O):
                            break;
                        case(BLTZ_O):
                            break;
                        case(BNE_O):
                            break;
                        case(LBU_O):
                            break;
                        case(LW_O):
                            break;
                        case(SB_O):
                            break;
                        case(SW_O):
                            break;
                        case(JMP_O):
                            break;
                        case(JAL_O):
                            break;
                    }
                    break;
                case(jump_instr_type):
                    printf("jump_inst_type\n");
                    break;
                case(error_instr_type):
                    return 1;
                    break;
            }
            
            
            //increment PC for next instruction
            PC = PC + 4;     
        }
    }  
}
