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


void InstrPrint(bin_instr_t bi, unsigned int i)
{
    printf("   %d %s", i, instruction_assembly_form(bi));
    printf("\n");
}


int main(int argc, char *argv[]) 
{
    word_type register_array[NUM_REGISTERS];
   
    // [0] = PC, [1] = HI, [2] = LO
    word_type special_register_array[3] = {0,0,0};

    //initialize register array to 0
    for(int i =0; i < NUM_REGISTERS; i ++ )
    {
        register_array[i] = 0;
    }

    //default tracing is off
    int trace = 1;                

    if(strcmp(argv[1], "-p") == 0 )
    {

        BOFFILE boffile = bof_read_open(argv[2]);

        BOFHeader boffile_header = bof_read_header(boffile);

        int length = boffile_header.text_length / BYTES_PER_WORD;
        int data_length = boffile_header.data_length / BYTES_PER_WORD;
        int data_address = boffile_header.data_start_address;
               

        printf("Addr Instruction\n");
        for(int i = 0; i < length; i++)
        {
            InstrPrint( instruction_read(boffile), i*BYTES_PER_WORD);
        }
        for(int i =0; i < data_length; i++)
        {
            word_type word = bof_read_word(boffile);
            printf("    %u: %d\t", data_address, word);
            data_address +=  BYTES_PER_WORD;
            
        }

        printf("%u: 0 ...\n", data_address);

        
    }

    if(argc == 2)
    {
        BOFFILE boffile = bof_read_open(argv[1]);

        BOFHeader boffile_header = bof_read_header(boffile);

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
                if( special_register_array[1] != 0 || special_register_array[2] !=0)
                {
                    printf("HI: %u   LO: %u", special_register_array[1], special_register_array[2]);
                }
                for(int i =0; i < 32; i ++ )
                {
                    
                    if(i % 6 ==0)
                        printf("\n");
                        
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
                    switch(bi.reg.func)
                    {
                        case(ADD_F):
                            register_array[bi.reg.rd] = register_array[bi.reg.rs] + register_array[bi.reg.rt];
                            break;
                        case(SUB_F):
                            register_array[bi.reg.rd] = register_array[bi.reg.rs] - register_array[bi.reg.rt];
                            break;
                        case(MUL_F):
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
                            //HI register
                            special_register_array[1] = register_array[bi.reg.rs] % register_array[bi.reg.rt];
                            //LO register
                            special_register_array[2] = register_array[bi.reg.rs] / register_array[bi.reg.rt];
                            break;
                        case(MFHI_F):
                            register_array[bi.reg.rd] = special_register_array[1];
                            break;
                        case(MFLO_F):
                            register_array[bi.reg.rd] = special_register_array[2];
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
                            //Jump Register: PC ← GPR[s]
                            special_register_array[0] = special_register_array[0] + register_array[bi.reg.rs];
                            break;
                        case(SYSCALL_F):
                            break;
                    }
                    break;
                case(syscall_instr_type):
                    //printf("syscall_instr_type\n");
                    //printf("code: %u func: %u op: %u\n", bi.syscall.code, bi.syscall.func, bi.syscall.op);
                    switch(bi.syscall.code)
                    {
                        case(exit_sc):
                            exit(0);
                            break;
                        case(print_str_sc):
                            //GPR[$v0] ← printf("%s",&memory[GPR[$a0]])
                            //register_array[2] = printf("%s",&memory.instrs[register_array[4]]); //TODO : IDK IF &memory.instr is correct
                            break;
                        case(print_char_sc):
                            //GPR[$v0] ←fputc(GPR[$a0],stdout)
                            register_array[2] = fputc(register_array[4],stdout);
                            break;
                        case(read_char_sc):
                            //GPR[$v0] ← getc(stdin)
                            register_array[4] = getc(stdin);
                            break;
                        case(start_tracing_sc):
                            //start tracing output
                            trace = 1;
                            break;
                        case(stop_tracing_sc):
                            //stop tracing output
                            trace = 0;
                            break;
                    }
                    break;
                case(immed_instr_type):
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
                            break;
                        case(BEQ_O):
                            //Branch on Equal: if GPR[s] = GPR[t] then PC ← PC + formOffset(o)
                            if(register_array[bi.immed.rs] == register_array[bi.immed.rt])
                            {
                                special_register_array[0] = special_register_array[0] + machine_types_formOffset(bi.immed.immed);
                            }
                            break;
                        case(BGEZ_O):
                            // Branch ≥ 0: if GPR[s] ≥ 0 then PC ← PC + formOffset(o)
                            if(register_array[bi.immed.rs] >= 0)
                            {
                                special_register_array[0] = special_register_array[0] + machine_types_formOffset(bi.immed.immed);
                            }
                            break;
                        case(BGTZ_O):
                            //Branch > 0: if GPR[s] > 0 then PC ← PC + formOffset(o)
                            if(register_array[bi.immed.rs] > 0)
                            {
                                special_register_array[0] = special_register_array[0] + machine_types_formOffset(bi.immed.immed);
                            }
                            break;
                        case(BLEZ_O):
                            //Branch ≤ 0: if GPR[s] ≤ 0 then PC ← PC + formOffset(o)
                            if(register_array[bi.immed.rs] <= 0)
                            {
                                special_register_array[0] = special_register_array[0] + machine_types_formOffset(bi.immed.immed);
                            }
                            break;
                        case(BLTZ_O):
                            //Branch < 0: if GPR[s] < 0 then PC ← PC + formOffset(o)
                            if(register_array[bi.immed.rs] < 0)
                            {
                                special_register_array[0] = special_register_array[0] + machine_types_formOffset(bi.immed.immed);
                            }
                            break;
                        case(BNE_O):
                            // Branch Not Equal: if GPR[s] ̸= GPR[t] then PC ← PC + formOffset(o)
                            if(register_array[bi.immed.rs] != register_array[bi.immed.rt])
                            {
                                special_register_array[0] = special_register_array[0] + machine_types_formOffset(bi.immed.immed);
                            }
                            break;
                        case(LBU_O):
                            //Load Byte Unsigned: GPR[t] ← zeroExt(memory[GPR[b] + formOffset(o)])
                            //TODO: FIX MEMORY.INSTRC to correct
                            //register_array[bi.immed.rt] = machine_types_zeroExt(memory.instrs[bi.immed.rs] + machine_types_formOffset(bi.immed.immed));
                            break;
                        case(LW_O):
                            //GPR[t] ← memory[GPR[b] + formOffset(o)]
                            //TO DO: FIX THIS MEMORY
                            //register_array[bi.immed.rt] = memory.words[register_array[bi.immed.rs] + machine_types_formOffset(bi.immed.immed)];
                            break;
                        case(SB_O):
                            //Store Byte (least significant byte of GPR[t]): memory[GPR[b] + formOffset(o)] ← GPR[t]

                            break;
                        case(SW_O):
                            //Store Word (4 bytes): memory[GPR[b] + formOffset(o)] ← GPR[t]
                            break;
                    }
                    break;
                case(jump_instr_type):

                    switch(bi.jump.op)
                    {
                        case(JMP_O):
                            //Jump: PC ← formAddress(P C, a)
                            special_register_array[0] = machine_types_formAddress(special_register_array[0],bi.jump.addr);
                            break;
                        case(JAL_O):
                            //Jump and Link: GPR[$ra] ← PC; PC ← formAddress(PC, a)
                            register_array[31] = special_register_array[0];
                            special_register_array[0] = machine_types_formAddress(special_register_array[0], bi.jump.addr);
                            break;
                    }
                    break;
                case(error_instr_type):
                    return 1;
                    break;
            }

            //increment PC for next instruction
            special_register_array[0] += 4;     
        }
    }  
}