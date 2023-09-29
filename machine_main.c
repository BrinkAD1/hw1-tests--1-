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
    fprintf(stdout, "   %d %s", i, instruction_assembly_form(bi));
    fprintf(stdout, "\n");
}


int main(int argc, char *argv[]) 
{
    //create register array 
    word_type GPR[NUM_REGISTERS];
   
    // [0] = PC, [1] = HI, [2] = LO
    word_type SPR[3] = {0,0,0};

    //initialize register array to 0
    for(int i =0; i < NUM_REGISTERS; i ++ )
    {
        GPR[i] = 0;
    }

    //default tracing is off
    int trace = 1;                

    if(strcmp(argv[1], "-p") == 0 )
    {

        BOFFILE boffile = bof_read_open(argv[2]);

        BOFHeader boffile_header = bof_read_header(boffile);

        int length = boffile_header.text_length / BYTES_PER_WORD;
        int word_length = boffile_header.data_length / BYTES_PER_WORD;
        int word_address = boffile_header.data_start_address;
               

        fprintf(stdout, "Addr Instruction\n");
        for(int i = 0; i < length; i++)
        {
            InstrPrint( instruction_read(boffile), i*BYTES_PER_WORD);
        }
        for(int i =0; i < word_length; i++)
        {
            if(i % 5 == 0)
                fprintf(stdout, "\n");

            word_type word = bof_read_word(boffile);
            fprintf(stdout, "    %u: %d\t", word_address, word);
            word_address +=  BYTES_PER_WORD;
            
        }
        fprintf(stdout, "%u: 0 ...\n", word_address);        
    }

    if(argc == 2)
    {
        BOFFILE boffile = bof_read_open(argv[1]);
        BOFFILE boffile_mem = bof_read_open(argv[1]);

        BOFHeader boffile_header = bof_read_header(boffile);
        BOFHeader boffile_header_mem = bof_read_header(boffile_mem);

        //set the $gp rgister to the start address of data section
        GPR[28] = boffile_header.data_start_address;

        //set $fp and $sp to stack bottom address
        GPR[30] = boffile_header.stack_bottom_addr;
        GPR[29] = boffile_header.stack_bottom_addr;

        //set PC to text section start address
        SPR[0] = boffile_header.text_start_address;

        //Number of instructions to read
        int instruction_length = boffile_header_mem.text_length / BYTES_PER_WORD;

        //Number of words to Read
        int word_length = boffile_header.data_length / BYTES_PER_WORD;

        //start address for instructions in memory
        int instruction_index =  boffile_header.text_start_address;

        //start address for words in memory
        int word_address =  boffile_header.data_start_address;

        //load memory with the instructions
        //int instruction_index = 0;
        for(int i =0; i < instruction_length ; i++)
        {
            memory.instrs[instruction_index] = instruction_read(boffile_mem);
            instruction_index +=BYTES_PER_WORD;
        }

        //load memory with words
        for(int i =0; i < word_length; i++)
        {
            memory.words[word_address + (i * BYTES_PER_WORD)] = bof_read_word(boffile_mem);
        }
        while(1)
        {               

            bin_instr_t bi = memory.instrs[SPR[0]];
            instr_type instr = instruction_type(bi);

            //increment PC for next instruction            

            if(trace)
            {
                fprintf(stdout, "      PC: %d\t", SPR[0]);
                if( SPR[1] != 0 || SPR[2] !=0)
                {
                    fprintf(stdout, "HI: %u\tLO: %u", SPR[1], SPR[2]);
                }
                for(int i =0; i < 32; i ++ )
                {
                    
                    if(i % 6 ==0)
                        fprintf(stdout, "\n");
                        
                    fprintf(stdout, "GPR[%s]: %d   ", regname_get(i), GPR[i]);
                }
                fprintf(stdout, "\n");
                if(word_length > 0)
                {   
                    int i =0;
                    while(i < word_length)
                    {
                        fprintf(stdout, "\t%d:   %d   ", (word_address + (i * BYTES_PER_WORD)), memory.words[word_address + (i * BYTES_PER_WORD)]);
                        i++;
                    }
                    fprintf(stdout, "%d:   0...   \n", word_address+ (i * BYTES_PER_WORD));
                }
                else
                {
                    fprintf(stdout, "\t%d:   0...   \n", word_address);
                }
                fprintf(stdout, "\t%d: 0 ...\n", GPR[29]);
                fprintf(stdout, "==> addr:   ");
                InstrPrint( bi, SPR[0]);
            }   
            //Increment PC by BYTES_PER_WORD
            SPR[0] += BYTES_PER_WORD;         

            switch(instr)
            {              
                case(reg_instr_type):
                    
                    switch(bi.reg.func)
                    {
                        case(ADD_F):
                            GPR[bi.reg.rd] = GPR[bi.reg.rs] + GPR[bi.reg.rt];
                            break;
                        case(SUB_F):
                            GPR[bi.reg.rd] = GPR[bi.reg.rs] - GPR[bi.reg.rt];
                            break;
                        case(MUL_F):
                            unsigned long int result = GPR[bi.reg.rs] * GPR[bi.reg.rt];
                            // Put LSB into LO
                            SPR[2] = result;

                            //PUT MSB into HI
                            SPR[1] = 0;

                            break;
                        case(DIV_F):
                            //HI register
                            SPR[1] = GPR[bi.reg.rs] % GPR[bi.reg.rt];
                            //LO register
                            SPR[2] = GPR[bi.reg.rs] / GPR[bi.reg.rt];
                            break;
                        case(MFHI_F):
                            GPR[bi.reg.rd] = SPR[1];
                            break;
                        case(MFLO_F):
                            GPR[bi.reg.rd] = SPR[2];
                            break;
                        case(AND_F):
                            GPR[bi.reg.rd] = GPR[bi.reg.rs] & GPR[bi.reg.rt];
                            break;
                        case(BOR_F):
                            GPR[bi.reg.rd] = GPR[bi.reg.rs] | GPR[bi.reg.rt];
                            break;
                        case(NOR_F):
                            GPR[bi.reg.rd] = ~(GPR[bi.reg.rs] | GPR[bi.reg.rt]);
                            break;
                        case(XOR_F):
                            GPR[bi.reg.rd] = GPR[bi.reg.rs] ^ GPR[bi.reg.rt];
                            break;
                        case(SLL_F):
                            GPR[bi.reg.rd] = GPR[bi.reg.rt] << bi.reg.shift;
                            break;
                        case(SRL_F):
                            GPR[bi.reg.rd] = GPR[bi.reg.rt] >> bi.reg.shift;
                            break;
                        case(JR_F):
                            //Jump Register: PC ← GPR[s]
                            SPR[0] = GPR[bi.reg.rs];
                            break;
                        case(SYSCALL_F):
                            break;
                    }
                    break;
                case(syscall_instr_type):
                    //fprintf(stdout, "syscall_instr_type\n");
                    //fprintf(stdout, "code: %u func: %u op: %u\n", bi.syscall.code, bi.syscall.func, bi.syscall.op);
                    switch(bi.syscall.code)
                    {
                        case(exit_sc):
                            exit(0);
                            break;
                        case(print_str_sc):
                            //GPR[$v0] ← fprintf(stdout, "%s",&memory[GPR[$a0]])
                            GPR[2] = fprintf(stdout, "%s",memory.bytes[GPR[4]]); //TODO : IDK IF &memory.instr is correct
                            break;
                        case(print_char_sc):
                            //GPR[$v0] ←fputc(GPR[$a0],stdout)
                            GPR[2] = fputc(GPR[4],stdout);
                            break;
                        case(read_char_sc):
                            //GPR[$v0] ← getc(stdin)
                            GPR[2] = getc(stdin);
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
                    //fprintf(stdout, "immed_inst_type\n");
                    //fprintf(stdout, "op: %u rs: %u rt: %u\n",bi.immed.op, bi.immed.rs, bi.immed.rt);
                    switch(bi.immed.op)
                    {
                        case(ADDI_O):
                            GPR[bi.immed.rt] =  GPR[bi.immed.rs] + machine_types_sgnExt(bi.immed.immed);
                            break;
                        case(ANDI_O):
                            GPR[bi.immed.rt] = GPR[bi.immed.rs] & machine_types_zeroExt(bi.immed.immed);
                            break;
                        case(BORI_O):
                            GPR[bi.immed.rt] = GPR[bi.immed.rs] | machine_types_zeroExt(bi.immed.immed);
                            break;
                        case(XORI_O):
                            GPR[bi.immed.rt] = GPR[bi.immed.rs] ^ machine_types_zeroExt(bi.immed.immed);
                            break;
                        case(BEQ_O):
                            //Branch on Equal: if GPR[s] = GPR[t] then PC ← PC + formOffset(o)
                            if(GPR[bi.immed.rs] == GPR[bi.immed.rt])
                            {
                                SPR[0] = SPR[0] + machine_types_formOffset(bi.immed.immed);
                            }
                            break;
                        case(BGEZ_O):
                            // Branch ≥ 0: if GPR[s] ≥ 0 then PC ← PC + formOffset(o)
                            if(GPR[bi.immed.rs] >= 0)
                            {
                                SPR[0] = SPR[0] + machine_types_formOffset(bi.immed.immed);
                            }
                            break;
                        case(BGTZ_O):
                            //Branch > 0: if GPR[s] > 0 then PC ← PC + formOffset(o)
                            if(GPR[bi.immed.rs] > 0)
                            {
                                SPR[0] = SPR[0] + machine_types_formOffset(bi.immed.immed);
                            }
                            break;
                        case(BLEZ_O):
                            //Branch ≤ 0: if GPR[s] ≤ 0 then PC ← PC + formOffset(o)
                            if(GPR[bi.immed.rs] <= 0)
                            {
                                SPR[0] = SPR[0] + machine_types_formOffset(bi.immed.immed);
                            }
                            break;
                        case(BLTZ_O):
                            //Branch < 0: if GPR[s] < 0 then PC ← PC + formOffset(o)
                            if(GPR[bi.immed.rs] < 0)
                            {
                                SPR[0] = SPR[0] + machine_types_formOffset(bi.immed.immed);
                            }
                            break;
                        case(BNE_O):
                            // Branch Not Equal: if GPR[s] ̸= GPR[t] then PC ← PC + formOffset(o)
                            if(GPR[bi.immed.rs] != GPR[bi.immed.rt])
                            {
                                SPR[0] = SPR[0] + machine_types_formOffset(bi.immed.immed);
                            }
                            break;
                        case(LBU_O):
                            //Load Byte Unsigned: GPR[t] ← zeroExt(memory[GPR[b] + formOffset(o)])
                            //TODO: FIX MEMORY.INSTRC to correct
                            GPR[bi.immed.rt] = machine_types_zeroExt(memory.bytes[bi.immed.rs * 9] + machine_types_formOffset(bi.immed.immed));
                            break;
                        case(LW_O):
                            //GPR[t] ← memory[GPR[b] + formOffset(o)]
                            //TO DO: FIX THIS MEMORY
                            GPR[bi.immed.rt] = memory.words[GPR[bi.immed.rs] + machine_types_formOffset(bi.immed.immed)];
                            break;
                        case(SB_O):
                            //Store Byte (least significant byte of GPR[t]): memory[GPR[b] + formOffset(o)] ← GPR[t]
                            memory.bytes[ GPR[bi.immed.rs] + machine_types_formOffset(bi.immed.immed)] = GPR[bi.immed.rt];
                            break;
                        case(SW_O):
                            //Store Word (4 bytes): memory[GPR[b] + formOffset(o)] ← GPR[t]
                            memory.words[GPR[bi.immed.rs] + machine_types_formOffset(bi.immed.immed)] = GPR[bi.immed.rt]; 
                            break;
                    }
                    break;
                case(jump_instr_type):

                    switch(bi.jump.op)
                    {
                        case(JMP_O):
                            //Jump: PC ← formAddress(P C, a)
                            SPR[0] = machine_types_formAddress(SPR[0],bi.jump.addr);
                            break;
                        case(JAL_O):
                            //Jump and Link: GPR[$ra] ← PC; PC ← formAddress(PC, a)
                            GPR[31] = SPR[0];
                            SPR[0] = machine_types_formAddress(SPR[0], bi.jump.addr);
                            break;
                    }
                    break;
                case(error_instr_type):
                    return 1;
                    break;
            }     
        }
    }  
}
