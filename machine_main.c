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

<<<<<<< HEAD
void InstrPrint(bin_instr_t bi, unsigned int i)
{
    printf("   %d %s", i, instruction_assembly_form(bi));
=======
void disasmTextSection(BOFFILE bf, BOFHeader bh)
{
    printf(".text %u\n", bh.text_start_address);
    printf(".text %u\n", bh.text_length); 
}


void InstrPrint(bin_instr_t bi, unsigned int i)
{
    printf("%d:\t%s", i, instruction_assembly_form(bi));
>>>>>>> 23d0c8d9ae6ea5fe37facffad6145337eed6cb0d
    printf("\n");
}


int main(int argc, char *argv[]) 
{
<<<<<<< HEAD
    word_type register_array[NUM_REGISTERS];
   
    // [0] = PC, [1] = HI, [2] = LO
    word_type special_register_array[3] = {0,0,0};

    //initialize register array to 0
    for(int i =0; i < NUM_REGISTERS; i ++ )
=======
    word_type register_array[32];

    //initialize register array to 0
    for(int i =0; i < 32; i ++ )
>>>>>>> 23d0c8d9ae6ea5fe37facffad6145337eed6cb0d
    {
        register_array[i] = 0;
    }

<<<<<<< HEAD
=======
    //initialize PC to 0
    int PC = 0;
>>>>>>> 23d0c8d9ae6ea5fe37facffad6145337eed6cb0d
    //default tracing is off
    int trace = 1;                

    if(strcmp(argv[1], "-p") == 0 )
    {

        BOFFILE boffile = bof_read_open(argv[2]);

        BOFHeader boffile_header = bof_read_header(boffile);

<<<<<<< HEAD
        int length = boffile_header.text_length / BYTES_PER_WORD; 
=======
        int length = boffile_header.text_length / BYTES_PER_WORD;
>>>>>>> 23d0c8d9ae6ea5fe37facffad6145337eed6cb0d

        printf("Addr Instruction\n");
        for(int i = 0; i < length; i++)
        {
            InstrPrint( instruction_read(boffile), i*BYTES_PER_WORD);
        }

<<<<<<< HEAD
        printf("    %u:\n", boffile_header.data_start_address);
=======
        printf("\t%u\n", boffile_header.data_start_address);
>>>>>>> 23d0c8d9ae6ea5fe37facffad6145337eed6cb0d
    }

    if(argc == 2)
    {
        BOFFILE boffile = bof_read_open(argv[1]);

        BOFHeader boffile_header = bof_read_header(boffile);

<<<<<<< HEAD
        //set the $gp rgister to the start address of data section
        register_array[28] = boffile_header.data_start_address;

        //set $fp and $sp to stack bottom address
        register_array[30] = boffile_header.stack_bottom_addr;
        register_array[29] = boffile_header.stack_bottom_addr;

        //set PC to text section start address
        special_register_array[0] = boffile_header.text_start_address;

        int length = boffile_header.text_length / BYTES_PER_WORD;
        for(int i =0; i < length ; i++)
        {

            bin_instr_t bi = instruction_read(boffile);

            instr_type instr = instruction_type(bi);

            if(trace)
            {
                printf("PC: %d", special_register_array[0]);
=======
        int length = boffile_header.text_length / BYTES_PER_WORD;
        for(int i =0; i < length ; i++)
        {
            if(trace)
            {
                printf("PC: %d", PC);
>>>>>>> 23d0c8d9ae6ea5fe37facffad6145337eed6cb0d
                for(int i =0; i < 32; i ++ )
                {
                    
                    if(i % 6 ==0)
                        printf("\n");
                        
<<<<<<< HEAD
                    printf("GPR[%s]: %d   ", regname_get(i), register_array[i]);
                }
                printf("\n");
                printf("==> addr:   ");
                InstrPrint( bi, i*BYTES_PER_WORD);
            }
            
            

            switch(instr)
            {              
                case(reg_instr_type):
                    //printf("reg_instr_type\n");
                    //printf("reg.func: %u reg.rd: %u reg.rs %u reg.rt: %u reg.shift: %u\n",bi.reg.func, bi.reg.rd, bi.reg.rs, bi.reg.rt, bi.reg.shift);
=======
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
>>>>>>> 23d0c8d9ae6ea5fe37facffad6145337eed6cb0d
                    switch(bi.reg.func)
                    {
                        case(ADD_F):
                            register_array[bi.reg.rd] = register_array[bi.reg.rs] + register_array[bi.reg.rt];
                            break;
                        case(SUB_F):
                            register_array[bi.reg.rd] = register_array[bi.reg.rs] - register_array[bi.reg.rt];
                            break;
                        case(MUL_F):
<<<<<<< HEAD
                            unsigned int result = register_array[bi.reg.rs] * register_array[bi.reg.rt];
                            // Put LSB into LO
                            unsigned int lsb_mask = (1u << 16) - 1; // Create a mask with 16 LSBs
                            unsigned int lsbs = result & lsb_mask; // Use bitwise AND to extract the LSBs
                            special_register_array[2] = lsbs;

                            //PUT MSB into HI
                            unsigned int msb_mask = result & (~0u << 16); // Create a mask with 16 MSBs
                            unsigned int msbs = result & msb_mask; // Use bitwise AND to extract the MSBs
                            msbs >>= 16; // Right-shift to remove the lower 16 bits
                            special_register_array[1] = msbs;

                            break;
                        case(DIV_F):
                            special_register_array[1] = register_array[bi.reg.rs] % register_array[bi.reg.rt];
                            special_register_array[2] = register_array[bi.reg.rs] / register_array[bi.reg.rt];
                            break;
                        case(MFHI_F):
                            register_array[bi.reg.rd] = special_register_array[1];
                            break;
                        case(MFLO_F):
                            register_array[bi.reg.rd] = special_register_array[2];
=======
                            break;
                        case(DIV_F):
                            break;
                        case(MFHI_F):
                            break;
                        case(MFLO_F):
>>>>>>> 23d0c8d9ae6ea5fe37facffad6145337eed6cb0d
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
<<<<<<< HEAD
                    //printf("syscall_instr_type\n");
                    //printf("code: %u func: %u op: %u\n", bi.syscall.code, bi.syscall.func, bi.syscall.op);
=======
                    printf("syscall_instr_type\n");
                    printf("code: %u func: %u op: %u\n", bi.syscall.code, bi.syscall.func, bi.syscall.op);
>>>>>>> 23d0c8d9ae6ea5fe37facffad6145337eed6cb0d
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
<<<<<<< HEAD
                            //printf("start tracing sc\n");
                            trace = 1;
                            break;
                        case(stop_tracing_sc):
                            //printf("stop char sc\n");
=======
                            printf("start tracing sc\n");
                            trace = 1;
                            break;
                        case(stop_tracing_sc):
                            printf("stop char sc\n");
>>>>>>> 23d0c8d9ae6ea5fe37facffad6145337eed6cb0d
                            trace = 0;
                            break;
                    }
                    break;
                case(immed_instr_type):
<<<<<<< HEAD
                    //printf("immed_inst_type\n");
                    //printf("op: %u rs: %u rt: %u\n",bi.immed.op, bi.immed.rs, bi.immed.rt);
                    switch(bi.immed.op)
                    {
                        case(ADDI_O):
                            register_array[bi.immed.rt] =  register_array[bi.immed.rs] + machine_types_sgnExt(bi.immed.immed);
                            break;
                        case(ANDI_O):
                            register_array[bi.immed.rt] = register_array[bi.immed.rs] & machine_types_zeroExt(bi.immed.immed);
                            break;
                        case(BORI_O):
                            register_array[bi.immed.rt] = register_array[bi.immed.rs] | machine_types_zeroExt(bi.immed.immed);
                            break;
                        case(XORI_O):
                            register_array[bi.immed.rt] = register_array[bi.immed.rs] ^ machine_types_zeroExt(bi.immed.immed);
=======
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
>>>>>>> 23d0c8d9ae6ea5fe37facffad6145337eed6cb0d
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
<<<<<<< HEAD
                            
=======
>>>>>>> 23d0c8d9ae6ea5fe37facffad6145337eed6cb0d
                            break;
                        case(LW_O):
                            break;
                        case(SB_O):
                            break;
                        case(SW_O):
                            break;
<<<<<<< HEAD
                    }
                    break;
                case(jump_instr_type):

                    switch(bi.jump.op)
                    {
=======
>>>>>>> 23d0c8d9ae6ea5fe37facffad6145337eed6cb0d
                        case(JMP_O):
                            break;
                        case(JAL_O):
                            break;
                    }
                    break;
<<<<<<< HEAD
=======
                case(jump_instr_type):
                    printf("jump_inst_type\n");
                    break;
>>>>>>> 23d0c8d9ae6ea5fe37facffad6145337eed6cb0d
                case(error_instr_type):
                    return 1;
                    break;
            }
<<<<<<< HEAD

            //increment PC for next instruction
            special_register_array[0] += 4;     
        }
    }  
}
=======
            
            
            //increment PC for next instruction
            PC = PC + 4;     
        }
    }  
}
>>>>>>> 23d0c8d9ae6ea5fe37facffad6145337eed6cb0d
