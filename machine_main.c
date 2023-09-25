#include "instruction.h"
#include "bof.h"

int main(int argc, char *argv[]) 
{
    int register_array[32];

    //default registers to zero
    for(int i =0; i < 32; i ++ )
    {
        register_array[i] = 0;
    }

    if(argc == 2)
    {
        BOFFILE boffile = bof_read_open(argv[1]);

        BOFHeader boffile_header = bof_read_header(boffile);
        
        bin_instr_t bi = instruction_read(boffile);

        instr_type instr = instruction_type(bi);

        switch(instr)
        {   
            case(reg_instr_type):

                reg_instr_t ri = bi.reg;
                switch(ri.func)
                {
                    case ADD_F:

                        break;

                    case SUB_F:

                        break;

                    case MUL_F:

                        break;

                    case DIV_F:

                        break;

                    case MFHI_F:

                        break;

                    case MFLO_F:

                        break;

                    case AND_F:

                        break;

                    case BOR_F:

                        break;

                    case NOR_F:

                        break;

                    case XOR_F:

                        break;

                    case SLL_F:

                        break;
                    
                    case SRL_F:

                        break;

                    case JR_F:

                        break;

                    case SYSCALL_F:


                
            }
        }
        
    }
}