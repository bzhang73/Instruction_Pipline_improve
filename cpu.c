/*
 *  cpu.c
 *  Contains APEX cpu pipeline implementation
 *
 *  Author : Bo bzhang73@binghamton.edu
 *  Gaurav Kothari (gkothar1@binghamton.edu)
 *  State University of New York, Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"


int MUL_INDEX=0;
int STOP_INDEX=0;
int INSTRUCT_INDEX=0;
int HALT_INDEX=0;
int RS1_INDEX=0;
int RS2_INDEX=0;
int free_register=0;
int clear_index=0;
int EX_stage_bz=0;
int IQ_full_index=0;
int LSQ_full_index=0;
int ROB_full_index=0;
int test_instruction_number=0;
int MUL_FINISHED=1;
int MEM_FINISHED=1;
int print_mem_information=0;
int print_mem_time=1;
int HALT_STALL=0;
int load_pre_URF_index=0;
int load_rd=0;
int load_URF_index=0;
int Terminal_index=0;
int Jump_index=0;
int Jump_instruction_index=-1;
int Jal_index=0;
int Jal_instruction_index=-1;
int BZ_index=0;
int BZ_instruction_index=-1;
int BNZ_index=0;
int BNZ_instruction_index=-1;
int PC_Jump_value[1];
int Load_FuncInt_index=0;

/* Set this flag to 1 to enable debug messages */
#define ENABLE_DEBUG_MESSAGES 1


/*
 * This function creates and initializes APEX cpu.
 *
 * Note : You are free to edit this function according to your
 *                 implementation
 */
APEX_CPU*
APEX_cpu_init(const char* filename)
{
    if (!filename) {
        return NULL;
    }
    
    APEX_CPU* cpu = malloc(sizeof(*cpu));
    if (!cpu) {
        return NULL;
    }
    
    /* Initialize PC, Registers and all pipeline stages */
    cpu->pc = 4000;
    
    //z_flag
    cpu->z_flag[0]=0;
    
    memset(cpu->regs, 0, sizeof(int) * 32);
    memset(cpu->regs_valid, 1, sizeof(int) * 32);
    memset(cpu->stage, 0, sizeof(CPU_Stage) * NUM_STAGES);
    memset(cpu->data_memory, 0, sizeof(int) * 4000);
    
    memset(cpu->URF, 1, sizeof(int) * 40);
    memset(cpu->URF_valid, 1, sizeof(int) * 40);
    memset(cpu->URF_finished, 1, sizeof(int) * 40);
    memset(cpu->URF_check_vaild, 1, sizeof(int) * 40);
    
    memset(cpu->RAT, 0, sizeof(int) * 32);
    memset(cpu->R_RAT, 0, sizeof(int) * 32);
    memset(cpu->R_RAT_valid, 1, sizeof(int) * 32);
    
    memset(cpu->IQ, 0, sizeof(CPU_Stage) * 16);
    memset(cpu->LSQ, 0, sizeof(CPU_Stage) * 20);
    memset(cpu->ROB, 0, sizeof(CPU_Stage) * 32);
    
    memset(cpu->EX_MUL, 0, sizeof(CPU_Stage) * 1);
    memset(cpu->EX_INT, 0, sizeof(CPU_Stage) * 1);
    memset(cpu->EX_MEM, 0, sizeof(CPU_Stage) * 1);
    memset(cpu->Write_ROB, 0, sizeof(CPU_Stage) * 1);
    
    memset(cpu->IQ_valid, 1, sizeof(int) * 16);
    memset(cpu->LSQ_valid, 1, sizeof(int) * 20);
    memset(cpu->ROB_valid, 1, sizeof(int) * 32);
    memset(cpu->instruction_number, 1, sizeof(int) * 1);
    memset(cpu->IQ_check_index, 1, sizeof(int) * 16);
    
    memset(cpu->z_value, 1, sizeof(int) * 500);
    memset(cpu->z_value_valid, 1, sizeof(int) * 500);
//    memset(cpu->PC_Jump_value, 1, sizeof(int) * 1);
    
    /* Parse input file and create code memory */
    cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);
    
    if (!cpu->code_memory) {
        free(cpu);
        return NULL;
    }
    for (int a=0; a<40; a++) {
        cpu->URF_valid[a]=1;
        cpu->URF_finished[a]=0;
        cpu->URF_check_vaild[a]=0;
        cpu->regs_valid[a]=1;
        cpu->URF[a]=-1;
        //        printf("%d \n",cpu->URF[a]);
    }
    
    for (int a=0; a<32; a++) {
        cpu->RAT[a]=-1;
        cpu->R_RAT[a]=-1;
        cpu->R_RAT_valid[a]=0;
        //        printf("%d \n",cpu->RAT[a]);
    }
    
    for (int a=0; a<16; a++) {
        cpu->IQ_valid[a]=0;
    }
    
    for (int a=0; a<20; a++) {
        cpu->LSQ_valid[a]=0;
    }
    
    for (int a=0; a<32; a++) {
        cpu->ROB_valid[a]=0;
    }
    
    for (int a=0; a<500; a++) {
        cpu->z_value[a]=-1;
        cpu->z_value_valid[a]=0;
    }
    cpu->instruction_number[0]=0;
    
    //  if (ENABLE_DEBUG_MESSAGES) {
    //    fprintf(stderr,
    //            "APEX_CPU : Initialized APEX CPU, loaded %d instructions\n",
    //            cpu->code_memory_size);
    //    fprintf(stderr, "APEX_CPU : Printing Code Memory\n");
    //    printf("%-9s %-9s %-9s %-9s %-9s\n", "opcode", "rd", "rs1", "rs2", "imm");
    //
    //    for (in    t i = 0; i < cpu->code_memory_size; ++i) {
    //        if (i==0) {
    //            int a=0;
    //            if (cpu->code_memory[i].rs1>-1) {
    //                cpu->RAT[cpu->code_memory[i].rs1]=a;
    //                a++;
    //            }
    //
    //            if (cpu->code_memory[i].rs2>-1) {
    //                if (cpu->code_memory[i].rs1 == cpu->code_memory[i].rs2) {
    //                    cpu->RAT[cpu->code_memory[i].rs1]=a-1;
    //                }else{
    //                    cpu->RAT[cpu->code_memory[i].rs2]=a;
    //                    a++;
    //                }
    //            }
    //
    //            if (cpu->code_memory[i].rd>-1) {
    //                cpu->RAT[cpu->code_memory[i].rd]=a;
    //                a++;
    //            }
    //        }
    //
    //        for (int a=1; a<cpu->code_memory; <#increment#>) {
    //            <#statements#>
    //        }
    //
    //
    //      printf("%-9s %-9d %-9d %-9d %-9d\n",
    //             cpu->code_memory[i].opcode,
    //             cpu->code_memory[i].rd,
    //             cpu->code_memory[i].rs1,
    //             cpu->code_memory[i].rs2,
    //             cpu->code_memory[i].imm);
    //    }
    //  }
    
    
    /* Make all stages busy except Fetch stage, initally to start the pipeline */
    for (int i = 1; i < NUM_STAGES; ++i) {
        cpu->stage[i].busy = 1;
    }
    
    return cpu;
}

/*
 * This function de-allocates APEX cpu.
 *
 * Note : You are free to edit this function according to your
 *                 implementation
 */
void
APEX_cpu_stop(APEX_CPU* cpu)
{
    free(cpu->code_memory);
    free(cpu);
}

/* Converts the PC(4000 series) into
 * array index for code memory
 *
 * Note : You are not supposed to edit this function
 *
 */
int
get_code_index(int pc)
{
    return (pc - 4000) / 4;
}

static void
print_instruction_WB(CPU_Stage* stage)
{
    if (strcmp(stage->opcode, "STORE") == 0) {
        printf(
               "%s,R%d,R%d,#%d MEM(%d)=%d", stage->opcode, stage->rs1, stage->rs2, stage->imm, stage->mem_address, stage->rs1_value);
    }
    
    if (strcmp(stage->opcode, "MOVC") == 0) {
        printf("%s,R%d,#%d R%d(%d)", stage->opcode, stage->rd, stage->imm, stage->rd, stage->buffer);
    }
    
    if (strcmp(stage->opcode, "NOP") == 0) {
        printf("EMPTY");
    }
    
    if (strcmp(stage->opcode, "LOAD") == 0) {
        printf("%s,R%d,R%d,#%d R(%d)=%d", stage->opcode, stage->rd, stage->rs1, stage->imm, stage->rd, stage->buffer);
    }
    
    if (strcmp(stage->opcode, "ADD") == 0) {
        printf(
               "%s,R%d,R%d,R%d R%d(%d)", stage->opcode, stage->rd, stage->rs1, stage->rs2, stage->rd, stage->buffer);
    }
    
    if (strcmp(stage->opcode, "ADDL") == 0) {
        printf(
               "%s,R%d,R%d,#%d R%d(%d)", stage->opcode, stage->rd, stage->rs1, stage->imm, stage->rd, stage->buffer);
    }
    
    if (strcmp(stage->opcode, "SUB") == 0) {
        printf(
               "%s,R%d,R%d,R%d R%d(%d)", stage->opcode, stage->rd, stage->rs1, stage->rs2, stage->rd, stage->buffer);
    }
    
    if (strcmp(stage->opcode, "SUBL") == 0) {
        printf(
               "%s,R%d,R%d,#%d R%d(%d)", stage->opcode, stage->rd, stage->rs1, stage->imm, stage->rd, stage->buffer);
    }
    
    if (strcmp(stage->opcode, "AND") == 0) {
        printf(
               "%s,R%d,R%d,R%d R%d(%d)", stage->opcode, stage->rd, stage->rs1, stage->rs2, stage->rd, stage->buffer);
    }
    
    if (strcmp(stage->opcode, "OR") == 0) {
        printf(
               "%s,R%d,R%d,R%d R%d(%d)", stage->opcode, stage->rd, stage->rs1, stage->rs2, stage->rd, stage->buffer);
    }
    
    if (strcmp(stage->opcode, "EX-OR") == 0) {
        printf(
               "%s,R%d,R%d,R%d R%d(%d)", stage->opcode, stage->rd, stage->rs1, stage->rs2, stage->rd, stage->buffer);
    }
    
    if (strcmp(stage->opcode, "MUL") == 0) {
        printf(
               "%s,R%d,R%d,R%d R%d(%d)", stage->opcode, stage->rd, stage->rs1, stage->rs2, stage->rd, stage->buffer);
    }
    
    if (strcmp(stage->opcode, "BZ") == 0) {
        printf(
               "%s,#%d", stage->opcode, stage->imm);
    }
    
    if (strcmp(stage->opcode, "BNZ") == 0) {
        printf(
               "%s,#%d", stage->opcode, stage->imm);
    }
    
    if (strcmp(stage->opcode, "JUMP") == 0) {
        printf(
               "%s,R%d,#%d", stage->opcode,stage->rs1, stage->imm);
    }
    
    if (strcmp(stage->opcode, "HALT") == 0) {
        printf(
               "%s", stage->opcode);
    }
    
    if (strcmp(stage->opcode, "JAL") == 0) {
        printf(
               "%s,R%d,R%d,#%d", stage->opcode,stage->rd, stage->rs1, stage->imm);
    }
}

/* Debug function which dumps the cpu stage
 * content
 *
 * Note : You are not supposed to edit this function
 *
 */
static void
print_stage_content_WB(char* name, CPU_Stage* stage)
{
    if (strcmp(stage->opcode, "NOP")==0) {
        printf("%-15s: ", name);
        print_instruction_WB(stage);
        printf("\n");
    }else{
        printf("%-15s: pc(%d) ", name, stage->pc);
        print_instruction_WB(stage);
        printf("\n");
    }
    
}

static void
print_instruction(CPU_Stage* stage)
{
    if (strcmp(stage->opcode, "STORE") == 0) {
        printf(
               "%s,R%d,R%d,#%d ", stage->opcode, stage->rs1, stage->rs2, stage->imm);
    }
    
    if (strcmp(stage->opcode, "MOVC") == 0) {
        printf("%s,R%d,#%d ", stage->opcode, stage->rd, stage->imm);
    }
    
    
    if (strcmp(stage->opcode, "NOP") == 0) {
        printf("EMPTY");
    }
    
    if (strcmp(stage->opcode, "LOAD") == 0) {
        printf("%s,R%d,R%d,#%d ", stage->opcode, stage->rd, stage->rs1, stage->imm);
    }
    
    if (strcmp(stage->opcode, "ADD") == 0) {
        printf(
               "%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
    }
    
    if (strcmp(stage->opcode, "ADDL") == 0) {
        printf(
               "%s,R%d,R%d,#%d ", stage->opcode, stage->rd, stage->rs1, stage->imm);
    }
    
    if (strcmp(stage->opcode, "SUB") == 0) {
        printf(
               "%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
    }
    
    if (strcmp(stage->opcode, "SUBL") == 0) {
        printf(
               "%s,R%d,R%d,#%d ", stage->opcode, stage->rd, stage->rs1, stage->imm);
    }
    
    if (strcmp(stage->opcode, "AND") == 0) {
        printf(
               "%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
    }
    
    if (strcmp(stage->opcode, "OR") == 0) {
        printf(
               "%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
    }
    
    if (strcmp(stage->opcode, "EX-OR") == 0) {
        printf(
               "%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
    }
    
    if (strcmp(stage->opcode, "MUL") == 0) {
        printf(
               "%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
    }
    
    if (strcmp(stage->opcode, "BZ") == 0) {
        printf(
               "%s,#%d", stage->opcode, stage->imm);
    }
    
    if (strcmp(stage->opcode, "BNZ") == 0) {
        printf(
               "%s,#%d", stage->opcode, stage->imm);
    }
    
    if (strcmp(stage->opcode, "JUMP") == 0) {
        printf(
               "%s,R%d,#%d", stage->opcode,stage->rs1, stage->imm);
    }
    
    if (strcmp(stage->opcode, "HALT") == 0) {
        printf(
               "%s", stage->opcode);
    }
    
    if (strcmp(stage->opcode, "JAL") == 0) {
        printf(
               "%s,R%d,R%d,#%d", stage->opcode,stage->rd, stage->rs1, stage->imm);
    }
}

/* Debug function which dumps the cpu stage
 * content
 *
 * Note : You are not supposed to edit this function
 *
 */
static void
print_stage_content(char* name, CPU_Stage* stage)
{
    if (strcmp(stage->opcode, "NOP")==0) {
        printf("%-15s: ", name);
        print_instruction(stage);
        printf("\n");
    }else{
        printf("%-15s: pc(%d) ", name, stage->pc);
        print_instruction(stage);
        printf("\n");
        
    }
}

/*
 *  Fetch Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 *                  implementation
 */
int
fetch(APEX_CPU* cpu)
{
//    printf("%d free_register \n",free_register);
    
    CPU_Stage* stage = &cpu->stage[F];
    //    CPU_Stage* stage_MEM = &cpu->stage[MEM];
    if (!stage->busy && !stage->stalled) {
        /* Store current PC in fetch latch */
        stage->pc = cpu->pc;
        
        if (cpu->pc == 4000) {
            for (int a=0; a<10; a++) {
                cpu->URF_check_vaild[a]=0;
                cpu->URF[a]=-1;
                cpu->URF_finished[a]=0;
                cpu->z_value[a]=-1;
                cpu->z_value_valid[a]=0;
            }
            
            for (int a=0; a<16; a++) {
                cpu->IQ_valid[a]=0;
            }
            
            for (int a=0; a<20; a++) {
                cpu->LSQ_valid[a]=0;
            }
            
            for (int a=0; a<32; a++) {
                cpu->ROB_valid[a]=0;
            }
            cpu->instruction_number[0]=0;
        }
        
        /* Index into code memory using this pc and copy all instruction fields into
         * fetch latch
         */
        APEX_Instruction* current_ins = &cpu->code_memory[get_code_index(cpu->pc)];
        strcpy(stage->opcode, current_ins->opcode);
        stage->rd = current_ins->rd;
        stage->rs1 = current_ins->rs1;
        stage->rs2 = current_ins->rs2;
        stage->imm = current_ins->imm;
        stage->rd = current_ins->rd;
        stage->URF_rs1_index=-1;
        stage->URF_rs2_index=-1;
        
        //      printf("%d \n",cpu->URF_valid[0]);
        
        
        int checkstate;
        if (strcmp(stage->opcode, "MOVC")) {
            checkstate=1;
        } else if (strcmp(stage->opcode, "STORE")){
            checkstate=1;
        } else if (strcmp(stage->opcode, "LOAD")){
            checkstate=1;
        } else if (strcmp(stage->opcode, "ADD")){
            checkstate=1;
        } else if (strcmp(stage->opcode, "ADDL")){
            checkstate=1;
        } else if (strcmp(stage->opcode, "SUB")){
            checkstate=1;
        } else if (strcmp(stage->opcode, "SUBL")){
            checkstate=1;
        } else if (strcmp(stage->opcode, "MUL")){
            checkstate=1;
        } else if (strcmp(stage->opcode, "AND")){
            checkstate=1;
        } else if (strcmp(stage->opcode, "OR")){
            checkstate=1;
        } else if (strcmp(stage->opcode, "EX-OR")){
            checkstate=1;
        } else if (strcmp(stage->opcode, "BZ")){
            checkstate=1;
        } else if (strcmp(stage->opcode, "BNZ")){
            checkstate=1;
        } else if (strcmp(stage->opcode, "JUMP")){
            checkstate=1;
        } else if (strcmp(stage->opcode, "JAL")){
            checkstate=1;
        } else if (strcmp(stage->opcode, "HALT")){
            checkstate=1;
        }else{
            checkstate=0;
        }
        
        INSTRUCT_INDEX++;
        if (cpu->pc > (4000+4*cpu->code_memory_size)) {
            strcpy(stage->opcode, "NOP");
            stage->rd = -2;
            stage->rs2 = -1;
            stage->rs1 = -1;
        }
        
//        printf("SDfadsfadsfdasf PC_Jump_value %d\n",PC_Jump_value[0]);
        
        /* Update PC for next instruction */
        cpu->pc += 4;
        
        
//        printf("SDfadsfadsfdasf PC_Jump_value %d\n",PC_Jump_value[0]);
        
        //      for (int b=0; b<40; b++) {
        //
        //          printf("%d \n",cpu->URF[b]);
        //      }
        
        if (checkstate == 0) {
            strcpy(stage->opcode, "NOP");
            stage->rs2 = -1;
            stage->rs1 = -1;
            stage->rd=-2;
        }else if (stage->rd>19000000){
            strcpy(stage->opcode, "NOP");
            stage->rs2 = -1;
            stage->rs1 = -1;
            stage->rd=-2;
        }else if (stage->rd == -2){
            strcpy(stage->opcode, "NOP");
            stage->rs2 = -1;
            stage->rs1 = -1;
            stage->rd = -2;
        } else{
            //          printf("pass %d \n",stage->rd );
            
            
            
        }
        
        
        
        if (cpu->pc >= (4000+ 4*(cpu->code_memory_size+1)) ){
            strcpy(stage->opcode, "NOP");
            stage->rs2 = -1;
            stage->rs1 = -1;
        }
        
        
        //      printf("%s\n",stage->opcode);
        
        if (HALT_STALL==1) {
            strcpy(stage->opcode, "NOP");
        }
        
        
        if (STOP_INDEX != 0 || MUL_INDEX != 0) {
            cpu->pc -=4;
        } else{
            
            /* Copy data from fetch latch to decode latch*/
            cpu->stage[DRF] = cpu->stage[F];
        }
        
        //        //HALT change to NOP
        //        if (strcmp(stage_MEM->opcode, "HALT") == 0) {
        //            strcpy(stage->opcode, "NOP");
        //            stage->rd=-1;
        //        }
        //
        //      if (MUL_INDEX != 0) {
        //          cpu->pc -=4;
        //      }
        
        if (Total_type==2) {
            print_stage_content("01. Instruction at FETCH______STAGE --->", stage);
        }
        
//        printf("SDfadsfadsfdasf PC_Jump_value %d\n",PC_Jump_value[0]);
        if (BZ_index==1) {
            if (BZ_instruction_index>-1) {
                clear_index=1;
                cpu->pc=PC_Jump_value[0];
                
                
                for (int a=0; a<16; a++) {
                    if (cpu->IQ_valid[a]==1) {
                        if (cpu->IQ[a].number_instuction_index>BZ_instruction_index) {
                            cpu->IQ_valid[a]=0;
                        }
                    }
                }
                
                for (int a=0; a<20; a++) {
                    if (cpu->LSQ_valid[a]==1) {
                        if (cpu->LSQ[a].number_instuction_index>BZ_instruction_index) {
                            cpu->LSQ_valid[a]=0;
                        }
                    }
                }
                
                for (int a=0; a<32; a++) {
                    if (cpu->ROB_valid[a]==1) {
                        if (cpu->ROB[a].number_instuction_index>BZ_instruction_index) {
                            cpu->ROB_valid[a]=0;
                        }
                    }
                }
                
                for (int a=0; a<40; a++) {
                    cpu->URF_valid[a]=1;
                }
                
                for (int a=0; a<16; a++) {
                    cpu->RAT[a]=cpu->R_RAT[a];
                    cpu->URF_valid[cpu->RAT[a]]=0;
                    
                    
                }
                
                if (cpu->EX_INT[0].number_instuction_index>BZ_instruction_index) {
                    strcpy(cpu->EX_INT[0].opcode, "NOP");
                }
                
                if (cpu->EX_MUL[0].number_instuction_index>BZ_instruction_index) {
                    strcpy(cpu->EX_MUL[0].opcode, "NOP");
                }
                
                if (cpu->EX_MEM[0].number_instuction_index>BZ_instruction_index) {
                    strcpy(cpu->EX_MEM[0].opcode, "NOP");
                }
            }
            BZ_index=0;
        }
        
//        printf("SDfadsfadsfdasf PC_Jump_value %d\n",PC_Jump_value[0]);
        if (BNZ_index==1) {
//            printf("SDLFADFASDFADSF\n");
            if (BNZ_instruction_index>-1) {
//                printf("SDLFADFASDFADSF  %d\n",BNZ_instruction_index);
                clear_index=1;
                cpu->pc=PC_Jump_value[0];
                
                
                for (int a=0; a<16; a++) {
                    if (cpu->IQ_valid[a]==1) {
                        if (cpu->IQ[a].number_instuction_index>BNZ_instruction_index) {
                            cpu->IQ_valid[a]=0;
                        }
                    }
                }
                
                for (int a=0; a<20; a++) {
                    if (cpu->LSQ_valid[a]==1) {
                        if (cpu->LSQ[a].number_instuction_index>BNZ_instruction_index) {
                            cpu->LSQ_valid[a]=0;
                        }
                    }
                }
                
                for (int a=0; a<32; a++) {
                    if (cpu->ROB_valid[a]==1) {
                        if (cpu->ROB[a].number_instuction_index>BNZ_instruction_index) {
                            cpu->ROB_valid[a]=0;
                        }
                    }
                }
                
                for (int a=0; a<40; a++) {
                    cpu->URF_valid[a]=1;
                }
                
                for (int a=0; a<16; a++) {
                    cpu->RAT[a]=cpu->R_RAT[a];
                    cpu->URF_valid[cpu->RAT[a]]=0;
                    
                    
                }
                
                if (cpu->EX_INT[0].number_instuction_index>BNZ_instruction_index) {
                    strcpy(cpu->EX_INT[0].opcode, "NOP");
                }
                
                if (cpu->EX_MUL[0].number_instuction_index>BNZ_instruction_index) {
                    strcpy(cpu->EX_MUL[0].opcode, "NOP");
                }
                
                if (cpu->EX_MEM[0].number_instuction_index>BNZ_instruction_index) {
                    strcpy(cpu->EX_MEM[0].opcode, "NOP");
                }
            }
            BNZ_index=0;
        }
        
//        printf("SDfadsfadsfdasf PC_Jump_value %d\n",PC_Jump_value[0]);
        if (Jal_index==1) {
            if (Jal_instruction_index>-1) {
                clear_index=1;
                cpu->pc=PC_Jump_value[0];
                
                
                for (int a=0; a<16; a++) {
                    if (cpu->IQ_valid[a]==1) {
                        if (cpu->IQ[a].number_instuction_index>Jal_instruction_index) {
                            cpu->IQ_valid[a]=0;
                        }
                    }
                }
                
                for (int a=0; a<20; a++) {
                    if (cpu->LSQ_valid[a]==1) {
                        if (cpu->LSQ[a].number_instuction_index>Jal_instruction_index) {
                            cpu->LSQ_valid[a]=0;
                        }
                    }
                }
                
                for (int a=0; a<32; a++) {
                    if (cpu->ROB_valid[a]==1) {
                        if (cpu->ROB[a].number_instuction_index>Jal_instruction_index) {
                            cpu->ROB_valid[a]=0;
                        }
                    }
                }
                
                for (int a=0; a<40; a++) {
                    cpu->URF_valid[a]=1;
                }
                
                for (int a=0; a<16; a++) {
                    cpu->RAT[a]=cpu->R_RAT[a];
                    cpu->URF_valid[cpu->RAT[a]]=0;
                    
                    
                }
                
                if (cpu->EX_INT[0].number_instuction_index>Jal_instruction_index) {
                    strcpy(cpu->EX_INT[0].opcode, "NOP");
                }
                
                if (cpu->EX_MUL[0].number_instuction_index>Jal_instruction_index) {
                    strcpy(cpu->EX_MUL[0].opcode, "NOP");
                }
                
                if (cpu->EX_MEM[0].number_instuction_index>Jal_instruction_index) {
                    strcpy(cpu->EX_MEM[0].opcode, "NOP");
                }
            }
            
            Jal_index=0;
        }
        
        
//        printf("SDfadsfadsfdasf PC_Jump_value %d\n",PC_Jump_value[0]);
        if (Jump_index==1) {
            if (Jump_instruction_index>-1) {
                clear_index=1;
//                printf("SDfadsfadsfdasf PC_Jump_value %d\n",PC_Jump_value[0]);
                cpu->pc=PC_Jump_value[0];
                
//                printf("ADfasdfadsfasdfas %d\n",cpu->pc);
                
                for (int a=0; a<16; a++) {
                    if (cpu->IQ_valid[a]==1) {
                        if (cpu->IQ[a].number_instuction_index>Jump_instruction_index) {
                            cpu->IQ_valid[a]=0;
                        }
                    }
                }
                
                for (int a=0; a<20; a++) {
                    if (cpu->LSQ_valid[a]==1) {
                        if (cpu->LSQ[a].number_instuction_index>Jump_instruction_index) {
                            cpu->LSQ_valid[a]=0;
                        }
                    }
                }
                
                for (int a=0; a<32; a++) {
                    if (cpu->ROB_valid[a]==1) {
                        if (cpu->ROB[a].number_instuction_index>Jump_instruction_index) {
                            cpu->ROB_valid[a]=0;
                        }
                    }
                }
                
                for (int a=0; a<40; a++) {
                    cpu->URF_valid[a]=1;
                }
                
                for (int a=0; a<16; a++) {
                    cpu->RAT[a]=cpu->R_RAT[a];
                    cpu->URF_valid[cpu->RAT[a]]=0;
                    
                    
                }
                
                if (cpu->EX_INT[0].number_instuction_index>Jump_instruction_index) {
                    strcpy(cpu->EX_INT[0].opcode, "NOP");
                }
                
                if (cpu->EX_MUL[0].number_instuction_index>Jump_instruction_index) {
                    strcpy(cpu->EX_MUL[0].opcode, "NOP");
                }
                
                if (cpu->EX_MEM[0].number_instuction_index>Jump_instruction_index) {
                    strcpy(cpu->EX_MEM[0].opcode, "NOP");
                }
            }
            
            Jump_index=0;
        }
        
    }
    return 0;
}

/*
 *  Decode Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 *                  implementation
 */
int
decode(APEX_CPU* cpu)
{
    
    
//    printf("SDfadsfadsfdasf PC_Jump_value %d\n",PC_Jump_value[0]);
    //    printf("%d free_regis",free_register);
    CPU_Stage* stage = &cpu->stage[DRF];
    
    //    CPU_Stage* stage_F = &cpu->stage[F];
    CPU_Stage* stage_EX = &cpu->stage[EX];
    
    if (clear_index==1) {
        strcpy(stage->opcode, "NOP");
        clear_index=0;
    }
    
    
    
    //      if (strcmp(stage_EX->opcode, "LOAD") == 0) {
    
    //        //BZ change to NOP
    //        if (strcmp(stage_WB->opcode, "BZ") == 0) {
    //            if (cpu->z_flag[0] != 0) {
    //                strcpy(stage->opcode, "NOP");
    //                stage->rd=-1;
    //                stage->URF_rs1_index=-1;
    //                stage->URF_rs2_index=-2;
    //            }
    //        }
    //
    //        //BNZ change to NOP
    //        if (strcmp(stage_WB->opcode, "BNZ") == 0) {
    //            if (cpu->z_flag[0] == 0) {
    //                strcpy(stage->opcode, "NOP");
    //                stage->rd=-1;
    //                stage->URF_rs1_index=-1;
    //                stage->URF_rs2_index=-1;
    //            }
    //        }
    //
    //        //JUMP change to NOP
    //        if (strcmp(stage_WB->opcode, "JUMP") == 0) {
    //            strcpy(stage->opcode, "NOP");
    //            stage->rd=-1;
    //            stage->URF_rs1_index=-1;
    //            stage->URF_rs2_index=-1;
    //        }
    //
    //        //HALT change to NOP
    //        if (strcmp(stage_WB->opcode, "HALT") == 0) {
    //            strcpy(stage->opcode, "NOP");
    //            stage->URF_rs1_index=-1;
    //            stage->URF_rs2_index=-1;
    //            stage->rd=-1;
    //        }
    //
    //        //HALT change to NOP
    //        if (strcmp(stage_MEM->opcode, "HALT") == 0) {
    //            strcpy(stage->opcode, "NOP");
    //            stage->URF_rs1_index=-1;
    //            stage->URF_rs2_index=-1;
    //            stage->rd=-1;
    //        }
    //
    
    /* Read data from register file for store */
    if (strcmp(stage->opcode, "STORE") == 0) {
        if (stage->rs1>-1) {
            if (cpu->RAT[stage->rs1] != -1) {
                stage->URF_rs1_index=cpu->RAT[stage->rs1];
//                printf("%d ,RAT %d ,  value %d\n",stage->rs1, stage->URF_rs1_index,cpu->URF[stage->URF_rs1_index]);
                
                cpu->URF_check_vaild[stage->URF_rs1_index]++;
            }
        }
        
        
        if (stage->rs2>-1) {
            if (cpu->RAT[stage->rs2] != -1) {
                stage->URF_rs2_index=cpu->RAT[stage->rs2];
//                printf("%d ,RAT %d , value %d\n",stage->rs2, stage->URF_rs2_index,cpu->URF[stage->URF_rs2_index]);
                
                cpu->URF_check_vaild[stage->URF_rs2_index]++;
            }
        }
        
        
        if (cpu->URF_valid[stage->URF_rs1_index] <= 0) {
            if (strcmp(stage_EX->opcode, "LOAD") == 0) {
                RS1_INDEX=1;
                
            }else{
                
                stage->rs1_value = cpu->URF[stage->URF_rs1_index];
//                printf("urf rs1 %d \n",stage->rs1_value );
                RS1_INDEX=0;
            }
            
        }else{
            stage->rs1_value = cpu->regs[stage->rs1];
            
//            printf("regs rs1 %d \n",stage->rs1_value );
            RS1_INDEX=0;
            
        }
        
        if (cpu->URF_valid[stage->URF_rs2_index] <= 0) {
            if (strcmp(stage_EX->opcode, "LOAD") == 0) {
                
                RS2_INDEX=1;
            }else{
                
                stage->rs2_value = cpu->URF[stage->URF_rs2_index];
                
//                printf("urf rs2 %d \n",stage->rs2_value );
                
                RS2_INDEX=0;
                
            }
            
        } else{
            stage->rs2_value = cpu->regs[stage->rs2];
            
//            printf("regs rs2 %d \n",stage->rs2_value );
            
            RS2_INDEX=0;
        }
        
        if (RS1_INDEX == 0 && RS2_INDEX == 0) {
            STOP_INDEX=0;
        }else{
            STOP_INDEX=1;
        }
        
        
        
        int temp;
        for (temp=0 ; temp<16; temp++) {
            if (cpu->IQ_valid[temp]==0 ) {
                stage->IQ_index=temp;
                IQ_full_index=0;
                break;
            }
        }
        if (temp>15) {
            IQ_full_index=1;
        }
        
        for (temp=0 ; temp<20; temp++) {
            if (cpu->LSQ_valid[temp]==0 ) {
                stage->LSQ_index=temp;
                LSQ_full_index=0;
                break;
            }
        }
        if (temp>19) {
            LSQ_full_index=1;
        }
        
        for (temp=0 ; temp<32; temp++) {
            if (cpu->ROB_valid[temp]==0 ) {
                stage->ROB_index=temp;
                ROB_full_index=0;
                break;
            }
        }
        if (temp>31) {
            ROB_full_index=1;
        }
        
        if (IQ_full_index==0 && LSQ_full_index==0 && ROB_full_index==0) {
            cpu->instruction_number[0]=cpu->instruction_number[0]+1;
            test_instruction_number++;
            CPU_Stage* IQ = &cpu->IQ[stage->IQ_index];
            IQ->number_instuction_index=test_instruction_number;
            IQ->pc=stage->pc;
            strcpy(IQ->opcode, stage->opcode);
            IQ->rd=stage->rd;
            IQ->rs1=stage->rs1;
            IQ->rs2=stage->rs2;
            IQ->imm=stage->imm;
            IQ->URF_index=stage->URF_index;
            IQ->pre_URF_index=stage->pre_URF_index;
                IQ->URF_rs1_index=stage->URF_rs1_index;
            IQ->URF_rs2_index=stage->URF_rs2_index;
            IQ->IQ_index=stage->IQ_index;
            IQ->LSQ_index=stage->LSQ_index;
            IQ->ROB_index=stage->ROB_index;
            IQ->IQ_finish_index=0;
            cpu->IQ_valid[stage->IQ_index]=1;
            cpu->z_value_valid[test_instruction_number]=0;
            
            
            CPU_Stage* LSQ = &cpu->LSQ[stage->LSQ_index];
            LSQ->pc=stage->pc;
            strcpy(LSQ->opcode, stage->opcode);
            LSQ->rd=stage->rd;
            LSQ->rs1=stage->rs1;
            LSQ->rs2=stage->rs2;
            LSQ->imm=stage->imm;
            LSQ->pre_URF_index=stage->pre_URF_index;
                LSQ->URF_index=stage->URF_index;
            LSQ->URF_rs1_index=stage->URF_rs1_index;
            LSQ->URF_rs2_index=stage->URF_rs2_index;
            LSQ->IQ_index=stage->IQ_index;
            LSQ->LSQ_index=stage->LSQ_index;
            LSQ->ROB_index=stage->ROB_index;
            LSQ->number_instuction_index=test_instruction_number;
            cpu->LSQ_valid[stage->LSQ_index]=1;
            
            CPU_Stage* ROB = &cpu->ROB[stage->ROB_index];
            ROB->pc=stage->pc;
            ROB->number_instuction_index=test_instruction_number;
            strcpy(ROB->opcode, stage->opcode);
            ROB->rd=stage->rd;
            ROB->rs1=stage->rs1;
            ROB->rs2=stage->rs2;
            ROB->imm=stage->imm;
            ROB->pre_URF_index=stage->pre_URF_index;
                ROB->URF_index=stage->URF_index;
            ROB->URF_rs1_index=stage->URF_rs1_index;
            ROB->URF_rs2_index=stage->URF_rs2_index;
            ROB->IQ_index=stage->IQ_index;
            ROB->LSQ_index=stage->LSQ_index;
            ROB->ROB_index=stage->ROB_index;
            ROB->ROB_finish_index=0;
            cpu->ROB_valid[stage->ROB_index]=1;
        }
        
        /* Copy data from decode latch to execute latch*/
        //                cpu->stage[EX] = cpu->stage[DRF];
        
    }
    
    /* Read data from register file for load */
    if (strcmp(stage->opcode, "LOAD") == 0) {
        if (stage->rs1>-1) {
            if (cpu->RAT[stage->URF_rs1_index] != -1) {
                stage->URF_rs1_index=cpu->RAT[stage->rs1];
//                printf("%d ,RAT %d ,  value %d \n",stage->rs1, stage->URF_rs1_index,cpu->URF[stage->URF_rs1_index]);
                
                stage->rs1_value = cpu->URF[stage->URF_rs1_index];
                RS1_INDEX=0;
                cpu->URF_check_vaild[stage->URF_rs1_index]++;
            }
        }
        
        if (cpu->URF_valid[stage->URF_rs1_index] <= 0) {
            if (strcmp(stage_EX->opcode, "LOAD") == 0) {
                RS1_INDEX=1;
                
            }else{
                
                stage->rs1_value = cpu->URF[stage->URF_rs1_index];
//                printf("urf rs1 %d \n",stage->rs1_value );
                RS1_INDEX=0;
            }
            
        }else{
            stage->rs1_value = cpu->regs[stage->rs1];
            
//            printf("regs rs1 %d \n",stage->rs1_value );
            RS1_INDEX=0;
            
        }
        
        if (RS1_INDEX == 0) {
            STOP_INDEX=0;
        }else{
            STOP_INDEX=1;
        }
        
        
        
        
        
        
        if (stage->rd>-1) {
            
            free_register=0;
            for (; free_register<40; free_register++) {
                if (cpu->URF_valid[free_register] == 1) {
                    
                    
                    int temp=cpu->RAT[stage->rd];
                    
                    cpu->RAT[stage->rd]=free_register;
                    
                    stage->URF_index=cpu->RAT[stage->rd];
                    
                    cpu->URF_valid[stage->URF_index]=0;
                    
                    //                  printf("rat %d rat=%d urfvalid=%d \n",stage->rd,cpu->RAT[stage->rd],cpu->URF_valid[cpu->RAT[stage->rd]]);
                    
                    cpu->URF_valid[stage->URF_index]=0;
                    cpu->URF_finished[stage->URF_index]=0;
                    
                    
                    cpu->URF_check_vaild[stage->URF_index]=1;
                    
                    //                                      if (cpu->URF_check_vaild[temp] == 0) {
                    //                                          stage->pre_URF_index=temp;
                    //                                      }
                    stage->pre_URF_index=temp;
                    
//                    printf("---------pre_URF_index");
                    break;
                }
            }
            
            int temp;
            for (temp=0 ; temp<16; temp++) {
                if (cpu->IQ_valid[temp]==0 ) {
                    stage->IQ_index=temp;
                    IQ_full_index=0;
                    break;
                }
            }
            if (temp>15) {
                IQ_full_index=1;
            }
            
            for (temp=0 ; temp<20; temp++) {
                if (cpu->LSQ_valid[temp]==0 ) {
                    stage->LSQ_index=temp;
                    LSQ_full_index=0;
                    break;
                }
            }
            if (temp>19) {
                LSQ_full_index=1;
            }
            
            for (temp=0 ; temp<32; temp++) {
                if (cpu->ROB_valid[temp]==0 ) {
                    stage->ROB_index=temp;
                    ROB_full_index=0;
                    break;
                }
            }
            if (temp>31) {
                ROB_full_index=1;
            }
            
            if (IQ_full_index==0 && LSQ_full_index==0 && ROB_full_index==0) {
                cpu->instruction_number[0]=cpu->instruction_number[0]+1;
                test_instruction_number++;
                CPU_Stage* IQ = &cpu->IQ[stage->IQ_index];
                IQ->number_instuction_index=test_instruction_number;
                IQ->pc=stage->pc;
                strcpy(IQ->opcode, stage->opcode);
                IQ->rd=stage->rd;
                IQ->rs1=stage->rs1;
                IQ->rs2=stage->rs2;
                IQ->imm=stage->imm;
                IQ->URF_index=stage->URF_index;
                IQ->pre_URF_index=stage->pre_URF_index;
                IQ->URF_rs1_index=stage->URF_rs1_index;
                IQ->URF_rs2_index=stage->URF_rs2_index;
                IQ->IQ_index=stage->IQ_index;
                IQ->LSQ_index=stage->LSQ_index;
                IQ->ROB_index=stage->ROB_index;
                IQ->IQ_finish_index=0;
                cpu->IQ_valid[stage->IQ_index]=1;
                cpu->z_value_valid[test_instruction_number]=0;
                
                CPU_Stage* LSQ = &cpu->LSQ[stage->LSQ_index];
                LSQ->pc=stage->pc;
                strcpy(LSQ->opcode, stage->opcode);
                LSQ->rd=stage->rd;
                LSQ->rs1=stage->rs1;
                LSQ->rs2=stage->rs2;
                LSQ->imm=stage->imm;
                LSQ->pre_URF_index=stage->pre_URF_index;
                LSQ->URF_index=stage->URF_index;
                LSQ->URF_rs1_index=stage->URF_rs1_index;
                LSQ->URF_rs2_index=stage->URF_rs2_index;
                LSQ->IQ_index=stage->IQ_index;
                LSQ->LSQ_index=stage->LSQ_index;
                LSQ->ROB_index=stage->ROB_index;
                LSQ->number_instuction_index=test_instruction_number;
                cpu->LSQ_valid[stage->LSQ_index]=1;
                
                CPU_Stage* ROB = &cpu->ROB[stage->ROB_index];
                ROB->pc=stage->pc;
                ROB->number_instuction_index=test_instruction_number;
                strcpy(ROB->opcode, stage->opcode);
                ROB->rd=stage->rd;
                ROB->rs1=stage->rs1;
                ROB->rs2=stage->rs2;
                ROB->imm=stage->imm;
                ROB->pre_URF_index=stage->pre_URF_index;
                ROB->URF_index=stage->URF_index;
                ROB->URF_rs1_index=stage->URF_rs1_index;
                ROB->URF_rs2_index=stage->URF_rs2_index;
                ROB->IQ_index=stage->IQ_index;
                ROB->LSQ_index=stage->LSQ_index;
                ROB->ROB_index=stage->ROB_index;
                ROB->ROB_finish_index=0;
                ROB->pre_URF_index=stage->pre_URF_index;
                cpu->ROB_valid[stage->ROB_index]=1;
            }
            
            
        }
        
        
        
        
        /* Copy data from decode latch to execute latch*/
        //                cpu->stage[EX] = cpu->stage[DRF];
        
        
    }
    
    /* No Register file read needed for MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {
        //no instruction needed for movc
//        printf("%d, zflag %d \n",clear_index, cpu->z_flag[0]);
        
        
        
        
        if (stage->rd>-1) {
            
            for (free_register=0; free_register<40; free_register++) {
                if (cpu->URF_valid[free_register] == 1) {
                    
                    
                    stage->pre_URF_index=cpu->RAT[stage->rd];
                    
                    cpu->RAT[stage->rd]=free_register;
                    
                    stage->URF_index=cpu->RAT[stage->rd];
                    
                    cpu->URF_valid[stage->URF_index]=0;
                    cpu->URF_finished[stage->URF_index]=0;
                    
                    //                  printf("rat %d rat=%d urfvalid=%d \n",stage->rd,cpu->RAT[stage->rd],cpu->URF_valid[cpu->RAT[stage->rd]]);
                    
                    cpu->URF_valid[stage->URF_index]=0;
                    break;
                }
            }
            
            int temp;
            for (temp=0 ; temp<16; temp++) {
                if (cpu->IQ_valid[temp]==0 ) {
                    stage->IQ_index=temp;
                    IQ_full_index=0;
                    break;
                }
            }
            if (temp>15) {
                IQ_full_index=1;
            }
            
            for (temp=0 ; temp<32; temp++) {
                if (cpu->ROB_valid[temp]==0 ) {
                    stage->ROB_index=temp;
                    ROB_full_index=0;
                    break;
                }
            }
            if (temp>31) {
                ROB_full_index=1;
            }
            
            if (IQ_full_index==0 && LSQ_full_index==0 && ROB_full_index==0) {
                cpu->instruction_number[0]=cpu->instruction_number[0]+1;
                test_instruction_number++;
//                printf("%d instuction unmber   test_instuction %d\n",cpu->instruction_number[0],test_instruction_number);
                CPU_Stage* IQ = &cpu->IQ[stage->IQ_index];
                IQ->number_instuction_index=test_instruction_number;
                IQ->pc=stage->pc;
                strcpy(IQ->opcode, stage->opcode);
                IQ->rd=stage->rd;
                IQ->rs1=stage->rs1;
                IQ->rs2=stage->rs2;
                IQ->imm=stage->imm;
                IQ->URF_index=stage->URF_index;
                IQ->pre_URF_index=stage->pre_URF_index;
                IQ->URF_rs1_index=stage->URF_rs1_index;
                IQ->URF_rs2_index=stage->URF_rs2_index;
                IQ->IQ_index=stage->IQ_index;
                IQ->LSQ_index=stage->LSQ_index;
                IQ->ROB_index=stage->ROB_index;
                IQ->IQ_finish_index=0;
                cpu->IQ_valid[stage->IQ_index]=1;
                cpu->z_value_valid[test_instruction_number]=0;
                
                CPU_Stage* ROB = &cpu->ROB[stage->ROB_index];
                ROB->pc=stage->pc;
                ROB->number_instuction_index=test_instruction_number;
                strcpy(ROB->opcode, stage->opcode);
                ROB->rd=stage->rd;
                ROB->rs1=stage->rs1;
                ROB->rs2=stage->rs2;
                ROB->imm=stage->imm;
                ROB->pre_URF_index=stage->pre_URF_index;
                ROB->URF_index=stage->URF_index;
                ROB->URF_rs1_index=stage->URF_rs1_index;
                ROB->URF_rs2_index=stage->URF_rs2_index;
                ROB->IQ_index=stage->IQ_index;
                ROB->LSQ_index=stage->LSQ_index;
                ROB->ROB_index=stage->ROB_index;
                ROB->ROB_finish_index=0;
                ROB->pre_URF_index=stage->pre_URF_index;
                cpu->ROB_valid[stage->ROB_index]=1;
            }
            
        }
        
//        printf("%d RAT %d, value %d\n",stage->rd, stage->URF_index,cpu->URF[stage->URF_index]);
        
        /* Copy data from decode latch to execute latch*/
        //                cpu->stage[EX] = cpu->stage[DRF];
        
    }
    
    /* No Register file read needed for ADD */
    if (strcmp(stage->opcode, "ADD") == 0) {
        if (stage->rs1>-1) {
            if (cpu->RAT[stage->rs1] != -1) {
                stage->URF_rs1_index=cpu->RAT[stage->rs1];
//                printf("%d ,RAT %d ,  value %d    %d\n",stage->rs1, stage->URF_rs1_index,cpu->URF[stage->URF_rs1_index],cpu->URF_valid[stage->URF_rs1_index]);
                
                cpu->URF_check_vaild[stage->URF_rs1_index]++;
            }
        }
        
        
        if (stage->rs2>-1) {
            if (cpu->RAT[stage->rs2] != -1) {
                stage->URF_rs2_index=cpu->RAT[stage->rs2];
//                printf("%d ,RAT %d , value %d      %d\n",stage->rs2, stage->URF_rs2_index,cpu->URF[stage->URF_rs2_index],cpu->URF_valid[stage->URF_rs2_index]);
                
                cpu->URF_check_vaild[stage->URF_rs2_index]++;
            }
        }
        
        
        
        if (cpu->URF_valid[stage->URF_rs1_index] <= 0) {
            if (strcmp(stage_EX->opcode, "LOAD") == 0) {
                RS1_INDEX=1;
                
            }else{
                
                stage->rs1_value = cpu->URF[stage->URF_rs1_index];
//                printf("urf rs1 %d , %d\n",stage->rs1_value ,cpu->URF[stage->URF_rs1_index]);
                RS1_INDEX=0;
            }
            
        }else{
            stage->rs1_value = cpu->regs[stage->rs1];
            
//            printf("regs rs1 %d \n",stage->rs1_value );
            RS1_INDEX=0;
            
        }
        
        if (cpu->URF_valid[stage->URF_rs2_index] <= 0) {
            if (strcmp(stage_EX->opcode, "LOAD") == 0) {
                
                RS2_INDEX=1;
            }else{
                
                stage->rs2_value = cpu->URF[stage->URF_rs2_index];
                
//                printf("urf rs2 %d \n",stage->rs2_value );
                
                RS2_INDEX=0;
                
            }
            
        } else{
            stage->rs2_value = cpu->regs[stage->rs2];
            
//            printf("regs rs2 %d \n",stage->rs2_value );
            
            RS2_INDEX=0;
        }
        
        if (RS1_INDEX == 0 && RS2_INDEX == 0) {
            STOP_INDEX=0;
        }else{
            STOP_INDEX=1;
        }
        
        
        
        
        if (stage->rd>-1) {
            
            free_register=0;
            
            for (; free_register<40; free_register++) {
                
//                printf("%d free_register \n",free_register);
                
                if (cpu->URF_valid[free_register] == 1) {
                    
                    
                    int temp=cpu->RAT[stage->rd];
                    
                    cpu->RAT[stage->rd]=free_register;
                    
                    stage->URF_index=cpu->RAT[stage->rd];
                    
                    cpu->URF_valid[stage->URF_index]=0;
                    
                    cpu->RAT[stage->rd]=stage->URF_index;
//                                      printf("rat %d rat=%d urfvalid=%d \n",stage->rd,cpu->RAT[stage->rd],cpu->URF_valid[cpu->RAT[stage->rd]]);
                    
                    cpu->URF_valid[stage->URF_index]=0;
                    cpu->URF_finished[stage->URF_index]=0;
                    
                    
                    cpu->URF_check_vaild[stage->URF_index]=1;
                    
                    if (cpu->URF_check_vaild[temp] == 0) {
                        stage->pre_URF_index=temp;
                    }
                    stage->pre_URF_index=temp;
                    
                    
//                                                          printf("RAT 2 %d \n", cpu->RAT[2]);
                    break;
                }
            }
            
            
            int temp;
            for (temp=0 ; temp<16; temp++) {
                if (cpu->IQ_valid[temp]==0 ) {
                    stage->IQ_index=temp;
                    IQ_full_index=0;
                    break;
                }
            }
            if (temp>15) {
                IQ_full_index=1;
            }
            
            for (temp=0 ; temp<32; temp++) {
                if (cpu->ROB_valid[temp]==0 ) {
                    stage->ROB_index=temp;
                    ROB_full_index=0;
                    break;
                }
            }
            if (temp>31) {
                ROB_full_index=1;
            }
            
            if (IQ_full_index==0 && LSQ_full_index==0 && ROB_full_index==0) {
                cpu->instruction_number[0]=cpu->instruction_number[0]+1;
                test_instruction_number++;
                CPU_Stage* IQ = &cpu->IQ[stage->IQ_index];
                IQ->number_instuction_index=test_instruction_number;
                IQ->pc=stage->pc;
                strcpy(IQ->opcode, stage->opcode);
                IQ->rd=stage->rd;
                IQ->rs1=stage->rs1;
                IQ->rs2=stage->rs2;
                IQ->imm=stage->imm;
                IQ->URF_index=stage->URF_index;
                IQ->pre_URF_index=stage->pre_URF_index;
                IQ->URF_rs1_index=stage->URF_rs1_index;
                IQ->URF_rs2_index=stage->URF_rs2_index;
                IQ->IQ_index=stage->IQ_index;
                IQ->LSQ_index=stage->LSQ_index;
                IQ->ROB_index=stage->ROB_index;
                IQ->IQ_finish_index=0;
                cpu->IQ_valid[stage->IQ_index]=1;
                cpu->z_value_valid[test_instruction_number]=1;
                
                CPU_Stage* ROB = &cpu->ROB[stage->ROB_index];
                ROB->pc=stage->pc;
                ROB->number_instuction_index=test_instruction_number;
                strcpy(ROB->opcode, stage->opcode);
                ROB->rd=stage->rd;
                ROB->rs1=stage->rs1;
                ROB->rs2=stage->rs2;
                ROB->imm=stage->imm;
                ROB->pre_URF_index=stage->pre_URF_index;
                ROB->URF_index=stage->URF_index;
                ROB->URF_rs1_index=stage->URF_rs1_index;
                ROB->URF_rs2_index=stage->URF_rs2_index;
                ROB->IQ_index=stage->IQ_index;
                ROB->LSQ_index=stage->LSQ_index;
                ROB->ROB_index=stage->ROB_index;
                ROB->ROB_finish_index=0;
                ROB->pre_URF_index=stage->pre_URF_index;
                cpu->ROB_valid[stage->ROB_index]=1;
            }
            
            
        }
        
//        printf("%d RAT %d, value %d\n",stage->rd, stage->URF_index,cpu->URF[stage->URF_index]);
        
        
        
        /* Copy data from decode latch to execute latch*/
        //                cpu->stage[EX] = cpu->stage[DRF];
        
    }
    
    /* No Register file read needed for ADDL */
    if (strcmp(stage->opcode, "ADDL") == 0) {
        if (stage->rs1>-1) {
            if (cpu->RAT[stage->rs1] != -1) {
                stage->URF_rs1_index=cpu->RAT[stage->rs1];
//                printf("%d ,RAT %d ,  value %d    %d\n",stage->rs1, stage->URF_rs1_index,cpu->URF[stage->URF_rs1_index],cpu->URF_valid[stage->URF_rs1_index]);
                
                cpu->URF_check_vaild[stage->URF_rs1_index]++;
            }
        }
        
        
        if (cpu->URF_valid[stage->URF_rs1_index] <= 0) {
            if (strcmp(stage_EX->opcode, "LOAD") == 0) {
                RS1_INDEX=1;
                
            }else{
                
                stage->rs1_value = cpu->URF[stage->URF_rs1_index];
//                printf("urf rs1 %d , %d\n",stage->rs1_value ,cpu->URF[stage->URF_rs1_index]);
                RS1_INDEX=0;
            }
            
        }else{
            stage->rs1_value = cpu->regs[stage->rs1];
            
//            printf("regs rs1 %d \n",stage->rs1_value );
            RS1_INDEX=0;
            
        }
        
        if (RS1_INDEX == 0 && RS2_INDEX == 0) {
            STOP_INDEX=0;
        }else{
            STOP_INDEX=1;
        }
        
        
        
        
        if (stage->rd>-1) {
            
            free_register=0;
            
            for (; free_register<40; free_register++) {
                
//                printf("%d free_register \n",free_register);
                
                if (cpu->URF_valid[free_register] == 1) {
                    
                    
                    int temp=cpu->RAT[stage->rd];
                    
                    cpu->RAT[stage->rd]=free_register;
                    
                    stage->URF_index=cpu->RAT[stage->rd];
                    
                    cpu->URF_valid[stage->URF_index]=0;
                    
                    cpu->RAT[stage->rd]=stage->URF_index;
                    //                  printf("rat %d rat=%d urfvalid=%d \n",stage->rd,cpu->RAT[stage->rd],cpu->URF_valid[cpu->RAT[stage->rd]]);
                    
                    cpu->URF_valid[stage->URF_index]=0;
                    cpu->URF_finished[stage->URF_index]=0;
                    
                    
                    cpu->URF_check_vaild[stage->URF_index]=1;
                    
                    if (cpu->URF_check_vaild[temp] == 0) {
                        stage->pre_URF_index=temp;
                    }
                    stage->pre_URF_index=temp;
                    
                    
                    //                                      printf("RAT 2 %d \n", cpu->RAT[2]);
                    break;
                }
            }
            
            
            int temp;
            for (temp=0 ; temp<16; temp++) {
                if (cpu->IQ_valid[temp]==0 ) {
                    stage->IQ_index=temp;
                    IQ_full_index=0;
                    break;
                }
            }
            if (temp>15) {
                IQ_full_index=1;
            }
            
            for (temp=0 ; temp<32; temp++) {
                if (cpu->ROB_valid[temp]==0 ) {
                    stage->ROB_index=temp;
                    ROB_full_index=0;
                    break;
                }
            }
            if (temp>31) {
                ROB_full_index=1;
            }
            
            if (IQ_full_index==0 && LSQ_full_index==0 && ROB_full_index==0) {
                cpu->instruction_number[0]=cpu->instruction_number[0]+1;
                test_instruction_number++;
                CPU_Stage* IQ = &cpu->IQ[stage->IQ_index];
                IQ->number_instuction_index=test_instruction_number;
                IQ->pc=stage->pc;
                strcpy(IQ->opcode, stage->opcode);
                IQ->rd=stage->rd;
                IQ->rs1=stage->rs1;
                IQ->rs2=stage->rs2;
                IQ->imm=stage->imm;
                IQ->URF_index=stage->URF_index;
                IQ->pre_URF_index=stage->pre_URF_index;
                IQ->URF_rs1_index=stage->URF_rs1_index;
                IQ->URF_rs2_index=stage->URF_rs2_index;
                IQ->IQ_index=stage->IQ_index;
                IQ->LSQ_index=stage->LSQ_index;
                IQ->ROB_index=stage->ROB_index;
                IQ->IQ_finish_index=0;
                cpu->IQ_valid[stage->IQ_index]=1;
                cpu->z_value_valid[test_instruction_number]=1;
                
                CPU_Stage* ROB = &cpu->ROB[stage->ROB_index];
                ROB->pc=stage->pc;
                ROB->number_instuction_index=test_instruction_number;
                strcpy(ROB->opcode, stage->opcode);
                ROB->rd=stage->rd;
                ROB->rs1=stage->rs1;
                ROB->rs2=stage->rs2;
                ROB->imm=stage->imm;
                ROB->pre_URF_index=stage->pre_URF_index;
                ROB->URF_index=stage->URF_index;
                ROB->URF_rs1_index=stage->URF_rs1_index;
                ROB->URF_rs2_index=stage->URF_rs2_index;
                ROB->IQ_index=stage->IQ_index;
                ROB->LSQ_index=stage->LSQ_index;
                ROB->ROB_index=stage->ROB_index;
                ROB->ROB_finish_index=0;
                ROB->pre_URF_index=stage->pre_URF_index;
                cpu->ROB_valid[stage->ROB_index]=1;
            }
            
            
        }
        
//        printf("%d RAT %d, value %d\n",stage->rd, stage->URF_index,cpu->URF[stage->URF_index]);
        
        
        
        /* Copy data from decode latch to execute latch*/
        //                cpu->stage[EX] = cpu->stage[DRF];
        
    }
    
    /* No Register file read needed for SUBL */
    if (strcmp(stage->opcode, "SUBL") == 0) {
        if (stage->rs1>-1) {
            if (cpu->RAT[stage->rs1] != -1) {
                stage->URF_rs1_index=cpu->RAT[stage->rs1];
//                printf("%d ,RAT %d ,  value %d    %d\n",stage->rs1, stage->URF_rs1_index,cpu->URF[stage->URF_rs1_index],cpu->URF_valid[stage->URF_rs1_index]);
                
                cpu->URF_check_vaild[stage->URF_rs1_index]++;
            }
        }
        
        
        if (cpu->URF_valid[stage->URF_rs1_index] <= 0) {
            if (strcmp(stage_EX->opcode, "LOAD") == 0) {
                RS1_INDEX=1;
                
            }else{
                
                stage->rs1_value = cpu->URF[stage->URF_rs1_index];
//                printf("urf rs1 %d , %d\n",stage->rs1_value ,cpu->URF[stage->URF_rs1_index]);
                RS1_INDEX=0;
            }
            
        }else{
            stage->rs1_value = cpu->regs[stage->rs1];
            
//            printf("regs rs1 %d \n",stage->rs1_value );
            RS1_INDEX=0;
            
        }
        
        if (RS1_INDEX == 0 && RS2_INDEX == 0) {
            STOP_INDEX=0;
        }else{
            STOP_INDEX=1;
        }
        
        
        
        
        if (stage->rd>-1) {
            
            free_register=0;
            
            for (; free_register<40; free_register++) {
                
//                printf("%d free_register \n",free_register);
                
                if (cpu->URF_valid[free_register] == 1) {
                    
                    
                    int temp=cpu->RAT[stage->rd];
                    
                    cpu->RAT[stage->rd]=free_register;
                    
                    stage->URF_index=cpu->RAT[stage->rd];
                    
                    cpu->URF_valid[stage->URF_index]=0;
                    
                    cpu->RAT[stage->rd]=stage->URF_index;
//                                      printf("rat %d rat=%d urfvalid=%d \n",stage->rd,cpu->RAT[stage->rd],cpu->URF_valid[cpu->RAT[stage->rd]]);
                    
                    cpu->URF_valid[stage->URF_index]=0;
                    cpu->URF_finished[stage->URF_index]=0;
                    
                    
                    cpu->URF_check_vaild[stage->URF_index]=1;
                    
                    if (cpu->URF_check_vaild[temp] == 0) {
                        stage->pre_URF_index=temp;
                    }
                    stage->pre_URF_index=temp;
                    
                    
//                                                          printf("RAT 2 %d \n", cpu->RAT[2]);
                    break;
                }
            }
            
            
            int temp;
            for (temp=0 ; temp<16; temp++) {
                if (cpu->IQ_valid[temp]==0 ) {
                    stage->IQ_index=temp;
                    IQ_full_index=0;
                    break;
                }
            }
            if (temp>15) {
                IQ_full_index=1;
            }
            
            for (temp=0 ; temp<32; temp++) {
                if (cpu->ROB_valid[temp]==0 ) {
                    stage->ROB_index=temp;
                    ROB_full_index=0;
                    break;
                }
            }
            if (temp>31) {
                ROB_full_index=1;
            }
            
            if (IQ_full_index==0 && LSQ_full_index==0 && ROB_full_index==0) {
                cpu->instruction_number[0]=cpu->instruction_number[0]+1;
                test_instruction_number++;
                CPU_Stage* IQ = &cpu->IQ[stage->IQ_index];
                IQ->number_instuction_index=test_instruction_number;
                IQ->pc=stage->pc;
                strcpy(IQ->opcode, stage->opcode);
                IQ->rd=stage->rd;
                IQ->rs1=stage->rs1;
                IQ->rs2=stage->rs2;
                IQ->imm=stage->imm;
                IQ->URF_index=stage->URF_index;
                IQ->pre_URF_index=stage->pre_URF_index;
                IQ->URF_rs1_index=stage->URF_rs1_index;
                IQ->URF_rs2_index=stage->URF_rs2_index;
                IQ->IQ_index=stage->IQ_index;
                IQ->LSQ_index=stage->LSQ_index;
                IQ->ROB_index=stage->ROB_index;
                IQ->IQ_finish_index=0;
                cpu->IQ_valid[stage->IQ_index]=1;
                cpu->z_value_valid[test_instruction_number]=1;
                
                CPU_Stage* ROB = &cpu->ROB[stage->ROB_index];
                ROB->pc=stage->pc;
                ROB->number_instuction_index=test_instruction_number;
                strcpy(ROB->opcode, stage->opcode);
                ROB->rd=stage->rd;
                ROB->rs1=stage->rs1;
                ROB->rs2=stage->rs2;
                ROB->imm=stage->imm;
                ROB->pre_URF_index=stage->pre_URF_index;
                ROB->URF_index=stage->URF_index;
                ROB->URF_rs1_index=stage->URF_rs1_index;
                ROB->URF_rs2_index=stage->URF_rs2_index;
                ROB->IQ_index=stage->IQ_index;
                ROB->LSQ_index=stage->LSQ_index;
                ROB->ROB_index=stage->ROB_index;
                ROB->ROB_finish_index=0;
                ROB->pre_URF_index=stage->pre_URF_index;
                cpu->ROB_valid[stage->ROB_index]=1;
            }
            
            
        }
        
//        printf("%d RAT %d, value %d\n",stage->rd, stage->URF_index,cpu->URF[stage->URF_index]);
        
        
        
        /* Copy data from decode latch to execute latch*/
        //                cpu->stage[EX] = cpu->stage[DRF];
        
    }
    
    
    /* No Register file read needed for SUB */
    if (strcmp(stage->opcode, "SUB") == 0) {
        if (stage->rs1>-1) {
            if (cpu->RAT[stage->rs1] != -1) {
                stage->URF_rs1_index=cpu->RAT[stage->rs1];
//                printf("%d ,RAT %d ,  value %d\n",stage->rs1, stage->URF_rs1_index,cpu->URF[stage->URF_rs1_index]);
                
                cpu->URF_check_vaild[stage->URF_rs1_index]++;
            }
        }
        
        
        if (stage->rs2>-1) {
            if (cpu->RAT[stage->rs2] != -1) {
                stage->URF_rs2_index=cpu->RAT[stage->rs2];
//                printf("%d ,RAT %d , value %d\n",stage->rs2, stage->URF_rs2_index,cpu->URF[stage->URF_rs2_index]);
                
                cpu->URF_check_vaild[stage->URF_rs2_index]++;
            }
        }
        
        if (cpu->URF_valid[stage->URF_rs1_index] <= 0) {
            if (strcmp(stage_EX->opcode, "LOAD") == 0) {
                RS1_INDEX=1;
                
            }else{
                stage->rs1_value = cpu->URF[stage->URF_rs1_index];
                RS1_INDEX=0;
            }
        }else{
            stage->rs1_value = cpu->regs[stage->rs1];
            RS1_INDEX=0;
            
        }
        
        if (cpu->URF_valid[stage->URF_rs2_index] <= 0) {
            if (strcmp(stage_EX->opcode, "LOAD") == 0) {
                
                RS2_INDEX=1;
            }else{
                stage->rs2_value = cpu->URF[stage->URF_rs2_index];
                
                RS2_INDEX=0;
            }
        } else{
            stage->rs2_value = cpu->regs[stage->rs2];
            
            RS2_INDEX=0;
        }
        
        
        
        
        
        if (stage->rd>-1) {
            
            free_register=0;
            for (; free_register<40; free_register++) {
                if (cpu->URF_valid[free_register] == 1) {
                    
                    
                    int temp=cpu->RAT[stage->rd];
                    
                    cpu->RAT[stage->rd]=free_register;
                    
                    stage->URF_index=cpu->RAT[stage->rd];
                    
                    cpu->URF_valid[stage->URF_index]=0;
                    cpu->URF_finished[stage->URF_index]=0;
                    
//                                      printf("rat %d rat=%d urfvalid=%d \n",stage->rd,cpu->RAT[stage->rd],cpu->URF_valid[cpu->RAT[stage->rd]]);
                    
                    cpu->URF_valid[stage->URF_index]=0;
                    
                    
                    cpu->URF_check_vaild[stage->URF_index]=1;
                    
                    if (cpu->URF_check_vaild[temp] == 0) {
                        stage->pre_URF_index=temp;
                    }
                    stage->pre_URF_index=temp;
                    break;
                }
            }
            
            
            int temp;
            for (temp=0 ; temp<16; temp++) {
                if (cpu->IQ_valid[temp]==0 ) {
                    stage->IQ_index=temp;
                    IQ_full_index=0;
                    break;
                }
            }
            if (temp>15) {
                IQ_full_index=1;
            }
            
            for (temp=0 ; temp<32; temp++) {
                if (cpu->ROB_valid[temp]==0 ) {
                    stage->ROB_index=temp;
                    ROB_full_index=0;
                    break;
                }
            }
            if (temp>31) {
                ROB_full_index=1;
            }
            
            if (IQ_full_index==0 && LSQ_full_index==0 && ROB_full_index==0) {
                cpu->instruction_number[0]=cpu->instruction_number[0]+1;
                test_instruction_number++;
                CPU_Stage* IQ = &cpu->IQ[stage->IQ_index];
                IQ->number_instuction_index=test_instruction_number;
                IQ->pc=stage->pc;
                strcpy(IQ->opcode, stage->opcode);
                IQ->rd=stage->rd;
                IQ->rs1=stage->rs1;
                IQ->rs2=stage->rs2;
                IQ->imm=stage->imm;
                IQ->URF_index=stage->URF_index;
                IQ->pre_URF_index=stage->pre_URF_index;
                IQ->URF_rs1_index=stage->URF_rs1_index;
                IQ->URF_rs2_index=stage->URF_rs2_index;
                IQ->IQ_index=stage->IQ_index;
                IQ->LSQ_index=stage->LSQ_index;
                IQ->ROB_index=stage->ROB_index;
                IQ->IQ_finish_index=0;
                cpu->IQ_valid[stage->IQ_index]=1;
                cpu->z_value_valid[test_instruction_number]=1;
                
                CPU_Stage* ROB = &cpu->ROB[stage->ROB_index];
                ROB->pc=stage->pc;
                ROB->number_instuction_index=test_instruction_number;
                strcpy(ROB->opcode, stage->opcode);
                ROB->rd=stage->rd;
                ROB->rs1=stage->rs1;
                ROB->rs2=stage->rs2;
                ROB->imm=stage->imm;
                ROB->pre_URF_index=stage->pre_URF_index;
                ROB->URF_index=stage->URF_index;
                ROB->URF_rs1_index=stage->URF_rs1_index;
                ROB->URF_rs2_index=stage->URF_rs2_index;
                ROB->IQ_index=stage->IQ_index;
                ROB->LSQ_index=stage->LSQ_index;
                ROB->ROB_index=stage->ROB_index;
                ROB->ROB_finish_index=0;
                ROB->pre_URF_index=stage->pre_URF_index;
                cpu->ROB_valid[stage->ROB_index]=1;
            }
            
            
        }
        
//        printf("%d RAT %d, value %d\n",stage->rd, stage->URF_index,cpu->URF[stage->URF_index]);
        
        
        
        
        /* Copy data from decode latch to execute latch*/
        //                cpu->stage[EX] = cpu->stage[DRF];
        
    }
    
    /* Read data from register file for AND */
    if (strcmp(stage->opcode, "AND") == 0) {
        if (stage->rs1>-1) {
            if (cpu->RAT[stage->rs1] != -1) {
                stage->URF_rs1_index=cpu->RAT[stage->rs1];
//                printf("%d ,RAT %d ,  value %d\n",stage->rs1, stage->URF_rs1_index,cpu->URF[stage->URF_rs1_index]);
                
                cpu->URF_check_vaild[stage->URF_rs1_index]++;
            }
        }
        
        
        if (stage->rs2>-1) {
            if (cpu->RAT[stage->rs2] != -1) {
                stage->URF_rs2_index=cpu->RAT[stage->rs2];
//                printf("%d ,RAT %d , value %d\n",stage->rs2, stage->URF_rs2_index,cpu->URF[stage->URF_rs2_index]);
                
                cpu->URF_check_vaild[stage->URF_rs2_index]++;
            }
        }
        
        if (cpu->URF_valid[stage->URF_rs1_index] <= 0) {
            if (strcmp(stage_EX->opcode, "LOAD") == 0) {
                RS1_INDEX=1;
                
            }else{
                stage->rs1_value = cpu->URF[stage->URF_rs1_index];
                RS1_INDEX=0;
            }
        }else{
            stage->rs1_value = cpu->regs[stage->rs1];
            RS1_INDEX=0;
            
        }
        
        if (cpu->URF_valid[stage->URF_rs2_index] <= 0) {
            if (strcmp(stage_EX->opcode, "LOAD") == 0) {
                
                RS2_INDEX=1;
            }else{
                stage->rs2_value = cpu->URF[stage->URF_rs2_index];
                
                RS2_INDEX=0;
            }
        } else{
            stage->rs2_value = cpu->regs[stage->rs2];
            
            RS2_INDEX=0;
        }
        
        if (RS1_INDEX == 0 && RS2_INDEX == 0) {
            STOP_INDEX=0;
        }else{
            STOP_INDEX=1;
        }
        
        
        
        
        
        if (stage->rd>-1) {
            
            free_register=0;
            for (; free_register<40; free_register++) {
                if (cpu->URF_valid[free_register] == 1) {
                    
                    
                    int temp=cpu->RAT[stage->rd];
                    
                    cpu->RAT[stage->rd]=free_register;
                    
                    stage->URF_index=cpu->RAT[stage->rd];
                    
                    cpu->URF_valid[stage->URF_index]=0;
                    cpu->URF_finished[stage->URF_index]=0;
                    
//                                      printf("rat %d rat=%d urfvalid=%d \n",stage->rd,cpu->RAT[stage->rd],cpu->URF_valid[cpu->RAT[stage->rd]]);
                    
                    cpu->URF_valid[stage->URF_index]=0;
                    
                    
                    cpu->URF_check_vaild[stage->URF_index]=1;
                    
                    
                    stage->pre_URF_index=temp;
                    break;
                }
            }
            
            
            
            int temp;
            for (temp=0 ; temp<16; temp++) {
                if (cpu->IQ_valid[temp]==0 ) {
                    stage->IQ_index=temp;
                    IQ_full_index=0;
                    break;
                }
            }
            if (temp>15) {
                IQ_full_index=1;
            }
            
            for (temp=0 ; temp<32; temp++) {
                if (cpu->ROB_valid[temp]==0 ) {
                    stage->ROB_index=temp;
                    ROB_full_index=0;
                    break;
                }
            }
            if (temp>31) {
                ROB_full_index=1;
            }
            
            if (IQ_full_index==0 && LSQ_full_index==0 && ROB_full_index==0) {
                cpu->instruction_number[0]=cpu->instruction_number[0]+1;
                test_instruction_number++;
                CPU_Stage* IQ = &cpu->IQ[stage->IQ_index];
                IQ->number_instuction_index=test_instruction_number;
                IQ->pc=stage->pc;
                strcpy(IQ->opcode, stage->opcode);
                IQ->rd=stage->rd;
                IQ->rs1=stage->rs1;
                IQ->rs2=stage->rs2;
                IQ->imm=stage->imm;
                IQ->URF_index=stage->URF_index;
                IQ->pre_URF_index=stage->pre_URF_index;
                IQ->URF_rs1_index=stage->URF_rs1_index;
                IQ->URF_rs2_index=stage->URF_rs2_index;
                IQ->IQ_index=stage->IQ_index;
                IQ->LSQ_index=stage->LSQ_index;
                IQ->ROB_index=stage->ROB_index;
                IQ->IQ_finish_index=0;
                cpu->IQ_valid[stage->IQ_index]=1;
                cpu->z_value_valid[test_instruction_number]=0;
                
                CPU_Stage* ROB = &cpu->ROB[stage->ROB_index];
                ROB->pc=stage->pc;
                ROB->number_instuction_index=test_instruction_number;
                strcpy(ROB->opcode, stage->opcode);
                ROB->rd=stage->rd;
                ROB->rs1=stage->rs1;
                ROB->rs2=stage->rs2;
                ROB->imm=stage->imm;
                ROB->pre_URF_index=stage->pre_URF_index;
                ROB->URF_index=stage->URF_index;
                ROB->URF_rs1_index=stage->URF_rs1_index;
                ROB->URF_rs2_index=stage->URF_rs2_index;
                ROB->IQ_index=stage->IQ_index;
                ROB->LSQ_index=stage->LSQ_index;
                ROB->ROB_index=stage->ROB_index;
                ROB->ROB_finish_index=0;
                ROB->pre_URF_index=stage->pre_URF_index;
                cpu->ROB_valid[stage->ROB_index]=1;
            }
            
        }
        
//        printf("%d RAT %d, value %d\n",stage->rd, stage->URF_index,cpu->URF[stage->URF_index]);
        
        
        
        /* Copy data from decode latch to execute latch*/
        //                cpu->stage[EX] = cpu->stage[DRF];
        
    }
    
    /* Read data from register file for OR */
    if (strcmp(stage->opcode, "OR") == 0) {
        if (stage->rs1>-1) {
            if (cpu->RAT[stage->rs1] != -1) {
                stage->URF_rs1_index=cpu->RAT[stage->rs1];
//                printf("%d ,RAT %d ,  value %d\n",stage->rs1, stage->URF_rs1_index,cpu->URF[stage->URF_rs1_index]);
                
                cpu->URF_check_vaild[stage->URF_rs1_index]++;
            }
        }
        
        
        if (stage->rs2>-1) {
            if (cpu->RAT[stage->rs2] != -1) {
                stage->URF_rs2_index=cpu->RAT[stage->rs2];
//                printf("%d ,RAT %d , value %d\n",stage->rs2, stage->URF_rs2_index,cpu->URF[stage->URF_rs2_index]);
                
                cpu->URF_check_vaild[stage->URF_rs2_index]++;
            }
        }
        
        if (cpu->URF_valid[stage->URF_rs1_index] <= 0) {
            if (strcmp(stage_EX->opcode, "LOAD") == 0) {
                RS1_INDEX=1;
                
            }else{
                stage->rs1_value = cpu->URF[stage->URF_rs1_index];
                RS1_INDEX=0;
            }
        }else{
            stage->rs1_value = cpu->regs[stage->rs1];
            RS1_INDEX=0;
            
        }
        
        if (cpu->URF_valid[stage->URF_rs2_index] <= 0) {
            if (strcmp(stage_EX->opcode, "LOAD") == 0) {
                
                RS2_INDEX=1;
            }else{
                stage->rs2_value = cpu->URF[stage->URF_rs2_index];
                
                RS2_INDEX=0;
            }
        } else{
            stage->rs2_value = cpu->regs[stage->rs2];
            
            RS2_INDEX=0;
        }
        
        if (RS1_INDEX == 0 && RS2_INDEX == 0) {
            STOP_INDEX=0;
        }else{
            STOP_INDEX=1;
        }
        
        
        
        
        if (stage->rd>-1) {
            
            free_register=0;
            for (; free_register<40; free_register++) {
                if (cpu->URF_valid[free_register] == 1) {
                    
                    
                    int temp=cpu->RAT[stage->rd];
                    
                    cpu->RAT[stage->rd]=free_register;
                    
                    stage->URF_index=cpu->RAT[stage->rd];
                    
                    cpu->URF_valid[stage->URF_index]=0;
                    cpu->URF_finished[stage->URF_index]=0;
                    
//                                      printf("rat %d rat=%d urfvalid=%d \n",stage->rd,cpu->RAT[stage->rd],cpu->URF_valid[cpu->RAT[stage->rd]]);
                    
                    cpu->URF_valid[stage->URF_index]=0;
                    
                    
                    cpu->URF_check_vaild[stage->URF_index]=1;
                    
                    
                    stage->pre_URF_index=temp;
                    break;
                }
            }
            
            
            
            int temp;
            for (temp=0 ; temp<16; temp++) {
                if (cpu->IQ_valid[temp]==0 ) {
                    stage->IQ_index=temp;
                    IQ_full_index=0;
                    break;
                }
            }
            if (temp>15) {
                IQ_full_index=1;
            }
            
            for (temp=0 ; temp<32; temp++) {
                if (cpu->ROB_valid[temp]==0 ) {
                    stage->ROB_index=temp;
                    ROB_full_index=0;
                    break;
                }
            }
            if (temp>31) {
                ROB_full_index=1;
            }
            
            if (IQ_full_index==0 && LSQ_full_index==0 && ROB_full_index==0) {
                cpu->instruction_number[0]=cpu->instruction_number[0]+1;
                test_instruction_number++;
                CPU_Stage* IQ = &cpu->IQ[stage->IQ_index];
                IQ->number_instuction_index=test_instruction_number;
                IQ->pc=stage->pc;
                strcpy(IQ->opcode, stage->opcode);
                IQ->rd=stage->rd;
                IQ->rs1=stage->rs1;
                IQ->rs2=stage->rs2;
                IQ->imm=stage->imm;
                IQ->URF_index=stage->URF_index;
                IQ->pre_URF_index=stage->pre_URF_index;
                IQ->URF_rs1_index=stage->URF_rs1_index;
                IQ->URF_rs2_index=stage->URF_rs2_index;
                IQ->IQ_index=stage->IQ_index;
                IQ->LSQ_index=stage->LSQ_index;
                IQ->ROB_index=stage->ROB_index;
                IQ->IQ_finish_index=0;
                cpu->IQ_valid[stage->IQ_index]=1;
                cpu->z_value_valid[test_instruction_number]=0;
                
                CPU_Stage* ROB = &cpu->ROB[stage->ROB_index];
                ROB->pc=stage->pc;
                ROB->number_instuction_index=test_instruction_number;
                strcpy(ROB->opcode, stage->opcode);
                ROB->rd=stage->rd;
                ROB->rs1=stage->rs1;
                ROB->rs2=stage->rs2;
                ROB->imm=stage->imm;
                ROB->pre_URF_index=stage->pre_URF_index;
                ROB->URF_index=stage->URF_index;
                ROB->URF_rs1_index=stage->URF_rs1_index;
                ROB->URF_rs2_index=stage->URF_rs2_index;
                ROB->IQ_index=stage->IQ_index;
                ROB->LSQ_index=stage->LSQ_index;
                ROB->ROB_index=stage->ROB_index;
                ROB->ROB_finish_index=0;
                ROB->pre_URF_index=stage->pre_URF_index;
                cpu->ROB_valid[stage->ROB_index]=1;
            }
            
        }
        
//        printf("%d RAT %d, value %d\n",stage->rd, stage->URF_index,cpu->URF[stage->URF_index]);
        
        
        
        
        /* Copy data from decode latch to execute latch*/
        //                cpu->stage[EX] = cpu->stage[DRF];
        
    }
    
    /* Read data from register file for EX-OR */
    if (strcmp(stage->opcode, "EX-OR") == 0) {
        if (stage->rs1>-1) {
            if (cpu->RAT[stage->rs1] != -1) {
                stage->URF_rs1_index=cpu->RAT[stage->rs1];
//                printf("%d ,RAT %d ,  value %d\n",stage->rs1, stage->URF_rs1_index,cpu->URF[stage->URF_rs1_index]);
                
                cpu->URF_check_vaild[stage->URF_rs1_index]++;
            }
        }
        
        
        if (stage->rs2>-1) {
            if (cpu->RAT[stage->rs2] != -1) {
                stage->URF_rs2_index=cpu->RAT[stage->rs2];
//                printf("%d ,RAT %d , value %d\n",stage->rs2, stage->URF_rs2_index,cpu->URF[stage->URF_rs2_index]);
                
                cpu->URF_check_vaild[stage->URF_rs2_index]++;
            }
        }
        
        if (cpu->URF_valid[stage->URF_rs1_index] <= 0) {
            if (strcmp(stage_EX->opcode, "LOAD") == 0) {
                RS1_INDEX=1;
                
            }else{
                stage->rs1_value = cpu->URF[stage->URF_rs1_index];
                RS1_INDEX=0;
            }
        }else{
            stage->rs1_value = cpu->regs[stage->rs1];
            RS1_INDEX=0;
            
        }
        
        if (cpu->URF_valid[stage->URF_rs2_index] <= 0) {
            if (strcmp(stage_EX->opcode, "LOAD") == 0) {
                
                RS2_INDEX=1;
            }else{
                stage->rs2_value = cpu->URF[stage->URF_rs2_index];
                
                RS2_INDEX=0;
            }
        } else{
            stage->rs2_value = cpu->regs[stage->rs2];
            
            RS2_INDEX=0;
        }
        
        if (RS1_INDEX == 0 && RS2_INDEX == 0) {
            STOP_INDEX=0;
        }else{
            STOP_INDEX=1;
        }
        
        
        
        if (stage->rd>-1) {
            
            free_register=0;
            for (; free_register<40; free_register++) {
                if (cpu->URF_valid[free_register] == 1) {
                    
                    
                    int temp=cpu->RAT[stage->rd];
                    
                    cpu->RAT[stage->rd]=free_register;
                    
                    stage->URF_index=cpu->RAT[stage->rd];
                    
                    cpu->URF_valid[stage->URF_index]=0;
                    cpu->URF_finished[stage->URF_index]=0;
                    
//                                      printf("rat %d rat=%d urfvalid=%d \n",stage->rd,cpu->RAT[stage->rd],cpu->URF_valid[cpu->RAT[stage->rd]]);
                    
                    cpu->URF_valid[stage->URF_index]=0;
                    
                    
                    cpu->URF_check_vaild[stage->URF_index]=1;
                    
                    
                    stage->pre_URF_index=temp;
                    break;
                }
            }
            
            
            
            int temp;
            for (temp=0 ; temp<16; temp++) {
                if (cpu->IQ_valid[temp]==0 ) {
                    stage->IQ_index=temp;
                    IQ_full_index=0;
                    break;
                }
            }
            if (temp>15) {
                IQ_full_index=1;
            }
            
            for (temp=0 ; temp<32; temp++) {
                if (cpu->ROB_valid[temp]==0 ) {
                    stage->ROB_index=temp;
                    ROB_full_index=0;
                    break;
                }
            }
            if (temp>31) {
                ROB_full_index=1;
            }
            
            if (IQ_full_index==0 && LSQ_full_index==0 && ROB_full_index==0) {
                cpu->instruction_number[0]=cpu->instruction_number[0]+1;
                test_instruction_number++;
                CPU_Stage* IQ = &cpu->IQ[stage->IQ_index];
                IQ->number_instuction_index=test_instruction_number;
                IQ->pc=stage->pc;
                strcpy(IQ->opcode, stage->opcode);
                IQ->rd=stage->rd;
                IQ->rs1=stage->rs1;
                IQ->rs2=stage->rs2;
                IQ->imm=stage->imm;
                IQ->URF_index=stage->URF_index;
                IQ->pre_URF_index=stage->pre_URF_index;
                IQ->URF_rs1_index=stage->URF_rs1_index;
                IQ->URF_rs2_index=stage->URF_rs2_index;
                IQ->IQ_index=stage->IQ_index;
                IQ->LSQ_index=stage->LSQ_index;
                IQ->ROB_index=stage->ROB_index;
                IQ->IQ_finish_index=0;
                cpu->IQ_valid[stage->IQ_index]=1;
                cpu->z_value_valid[test_instruction_number]=0;
                
                CPU_Stage* ROB = &cpu->ROB[stage->ROB_index];
                ROB->pc=stage->pc;
                ROB->number_instuction_index=test_instruction_number;
                strcpy(ROB->opcode, stage->opcode);
                ROB->rd=stage->rd;
                ROB->rs1=stage->rs1;
                ROB->rs2=stage->rs2;
                ROB->imm=stage->imm;
                ROB->pre_URF_index=stage->pre_URF_index;
                ROB->URF_index=stage->URF_index;
                ROB->URF_rs1_index=stage->URF_rs1_index;
                ROB->URF_rs2_index=stage->URF_rs2_index;
                ROB->IQ_index=stage->IQ_index;
                ROB->LSQ_index=stage->LSQ_index;
                ROB->ROB_index=stage->ROB_index;
                ROB->ROB_finish_index=0;
                ROB->pre_URF_index=stage->pre_URF_index;
                cpu->ROB_valid[stage->ROB_index]=1;
            }
            
        }
        
//        printf("%d RAT %d, value %d\n",stage->rd, stage->URF_index,cpu->URF[stage->URF_index]);
        
        
        
        /* Copy data from decode latch to execute latch*/
        //                cpu->stage[EX] = cpu->stage[DRF];
        
    }
    
    /* No Register file read needed for MUL */
    if (strcmp(stage->opcode, "MUL") == 0) {
        if (stage->rs1>-1) {
            if (cpu->RAT[stage->rs1] != -1) {
                stage->URF_rs1_index=cpu->RAT[stage->rs1];
//                printf("%d ,RAT %d ,  value %d\n",stage->rs1, stage->URF_rs1_index,cpu->URF[stage->URF_rs1_index]);
                
                cpu->URF_check_vaild[stage->URF_rs1_index]++;
            }
        }
        
        
        if (stage->rs2>-1) {
            if (cpu->RAT[stage->rs2] != -1) {
                stage->URF_rs2_index=cpu->RAT[stage->rs2];
//                printf("%d ,RAT %d , value %d\n",stage->rs2, stage->URF_rs2_index,cpu->URF[stage->URF_rs2_index]);
                
                cpu->URF_check_vaild[stage->URF_rs2_index]++;
            }
        }
        
        if (cpu->URF_valid[stage->URF_rs1_index] <= 0) {
            if (strcmp(stage_EX->opcode, "LOAD") == 0) {
                RS1_INDEX=1;
                
            }else{
                stage->rs1_value = cpu->URF[stage->URF_rs1_index];
                RS1_INDEX=0;
            }
        }else{
            stage->rs1_value = cpu->regs[stage->rs1];
            RS1_INDEX=0;
            
        }
        
        if (cpu->URF_valid[stage->URF_rs2_index] <= 0) {
            if (strcmp(stage_EX->opcode, "LOAD") == 0) {
                
                RS2_INDEX=1;
            }else{
                stage->rs2_value = cpu->URF[stage->URF_rs2_index];
                
                RS2_INDEX=0;
            }
        } else{
            stage->rs2_value = cpu->regs[stage->rs2];
            
            RS2_INDEX=0;
        }
        
        if (RS1_INDEX == 0 && RS2_INDEX == 0) {
            STOP_INDEX=0;
        }else{
            STOP_INDEX=1;
        }
        
        
        
        
        
        if (stage->rd>-1) {
            
            free_register=0;
            for (; free_register<40; free_register++) {
                if (cpu->URF_valid[free_register] == 1) {
                    
                    int temp=cpu->RAT[stage->rd];
                    
                    cpu->RAT[stage->rd]=free_register;
                    
                    stage->URF_index=cpu->RAT[stage->rd];
                    
                    cpu->URF_valid[stage->URF_index]=0;
                    
//                                      printf("rat %d rat=%d urfvalid=%d \n",stage->rd,cpu->RAT[stage->rd],cpu->URF_valid[cpu->RAT[stage->rd]]);
                    
                    cpu->URF_valid[stage->URF_index]=0;
                    cpu->URF_finished[stage->URF_index]=0;
                    
                    
                    cpu->URF_check_vaild[stage->URF_index]=1;
                    
                    
                    stage->pre_URF_index=temp;
                    break;
                }
            }
            
            
            
            int temp;
            for (temp=0 ; temp<16; temp++) {
                if (cpu->IQ_valid[temp]==0 ) {
                    stage->IQ_index=temp;
                    IQ_full_index=0;
                    break;
                }
            }
            if (temp>15) {
                IQ_full_index=1;
            }
            
            for (temp=0 ; temp<32; temp++) {
                if (cpu->ROB_valid[temp]==0 ) {
                    stage->ROB_index=temp;
                    ROB_full_index=0;
                    break;
                }
            }
            if (temp>31) {
                ROB_full_index=1;
            }
            
            if (IQ_full_index==0 && LSQ_full_index==0 && ROB_full_index==0) {
                cpu->instruction_number[0]=cpu->instruction_number[0]+1;
                test_instruction_number++;
                CPU_Stage* IQ = &cpu->IQ[stage->IQ_index];
                IQ->number_instuction_index=test_instruction_number;
                IQ->pc=stage->pc;
                strcpy(IQ->opcode, stage->opcode);
                IQ->rd=stage->rd;
                IQ->rs1=stage->rs1;
                IQ->rs2=stage->rs2;
                IQ->imm=stage->imm;
                IQ->URF_index=stage->URF_index;
                IQ->pre_URF_index=stage->pre_URF_index;
                IQ->URF_rs1_index=stage->URF_rs1_index;
                IQ->URF_rs2_index=stage->URF_rs2_index;
                IQ->IQ_index=stage->IQ_index;
                IQ->LSQ_index=stage->LSQ_index;
                IQ->ROB_index=stage->ROB_index;
                IQ->IQ_finish_index=0;
                cpu->IQ_valid[stage->IQ_index]=1;
                cpu->z_value_valid[test_instruction_number]=1;
                
                CPU_Stage* ROB = &cpu->ROB[stage->ROB_index];
                ROB->pc=stage->pc;
                ROB->number_instuction_index=test_instruction_number;
                strcpy(ROB->opcode, stage->opcode);
                ROB->rd=stage->rd;
                ROB->rs1=stage->rs1;
                ROB->rs2=stage->rs2;
                ROB->imm=stage->imm;
                ROB->pre_URF_index=stage->pre_URF_index;
                ROB->URF_index=stage->URF_index;
                ROB->URF_rs1_index=stage->URF_rs1_index;
                ROB->URF_rs2_index=stage->URF_rs2_index;
                ROB->IQ_index=stage->IQ_index;
                ROB->LSQ_index=stage->LSQ_index;
                ROB->ROB_index=stage->ROB_index;
                ROB->ROB_finish_index=0;
                ROB->pre_URF_index=stage->pre_URF_index;
                cpu->ROB_valid[stage->ROB_index]=1;
            }
            
        }
        
//        printf("%d RAT %d, value %d\n",stage->rd, stage->URF_index,cpu->URF[stage->URF_index]);
        
        
        
        /* Copy data from decode latch to execute latch*/
        //                cpu->stage[EX] = cpu->stage[DRF];
        
    }
    
    /* Read data from register file for BZ */
    if (strcmp(stage->opcode, "BZ") == 0) {
        
        
        int temp;
        for (temp=0 ; temp<16; temp++) {
            if (cpu->IQ_valid[temp]==0 ) {
                stage->IQ_index=temp;
                IQ_full_index=0;
                break;
            }
        }
        if (temp>15) {
            IQ_full_index=1;
        }
        
        for (temp=0 ; temp<32; temp++) {
            if (cpu->ROB_valid[temp]==0 ) {
                stage->ROB_index=temp;
                ROB_full_index=0;
                break;
            }
        }
        if (temp>31) {
            ROB_full_index=1;
        }
        
        if (IQ_full_index==0 && LSQ_full_index==0 && ROB_full_index==0) {
            cpu->instruction_number[0]=cpu->instruction_number[0]+1;
            test_instruction_number++;
            CPU_Stage* IQ = &cpu->IQ[stage->IQ_index];
            IQ->number_instuction_index=test_instruction_number;
            IQ->pc=stage->pc;
            strcpy(IQ->opcode, stage->opcode);
            IQ->rd=stage->rd;
            IQ->rs1=stage->rs1;
            IQ->rs2=stage->rs2;
            IQ->imm=stage->imm;
            IQ->URF_index=stage->URF_index;
            IQ->pre_URF_index=stage->pre_URF_index;
                IQ->URF_rs1_index=stage->URF_rs1_index;
            IQ->URF_rs2_index=stage->URF_rs2_index;
            IQ->IQ_index=stage->IQ_index;
            IQ->LSQ_index=stage->LSQ_index;
            IQ->ROB_index=stage->ROB_index;
            IQ->IQ_finish_index=0;
            cpu->IQ_valid[stage->IQ_index]=1;
            cpu->z_value_valid[test_instruction_number]=2;
            
            CPU_Stage* ROB = &cpu->ROB[stage->ROB_index];
            ROB->pc=stage->pc;
            ROB->number_instuction_index=test_instruction_number;
            strcpy(ROB->opcode, stage->opcode);
            ROB->rd=stage->rd;
            ROB->rs1=stage->rs1;
            ROB->rs2=stage->rs2;
            ROB->imm=stage->imm;
            ROB->pre_URF_index=stage->pre_URF_index;
                ROB->URF_index=stage->URF_index;
            ROB->URF_rs1_index=stage->URF_rs1_index;
            ROB->URF_rs2_index=stage->URF_rs2_index;
            ROB->IQ_index=stage->IQ_index;
            ROB->LSQ_index=stage->LSQ_index;
            ROB->ROB_index=stage->ROB_index;
            ROB->ROB_finish_index=0;
            cpu->ROB_valid[stage->ROB_index]=1;
        }
        
        /* Copy data from decode latch to execute latch*/
        //            cpu->stage[EX] = cpu->stage[DRF];
        
    }
    
    /* Read data from register file for BNZ */
    if (strcmp(stage->opcode, "BNZ") == 0) {
        
        int temp;
        for (temp=0 ; temp<16; temp++) {
            if (cpu->IQ_valid[temp]==0 ) {
                stage->IQ_index=temp;
                IQ_full_index=0;
                break;
            }
        }
        if (temp>15) {
            IQ_full_index=1;
        }
        
        for (temp=0 ; temp<32; temp++) {
            if (cpu->ROB_valid[temp]==0 ) {
                stage->ROB_index=temp;
                ROB_full_index=0;
                break;
            }
        }
        if (temp>31) {
            ROB_full_index=1;
        }
        
        if (IQ_full_index==0 && LSQ_full_index==0 && ROB_full_index==0) {
            cpu->instruction_number[0]=cpu->instruction_number[0]+1;
            test_instruction_number++;
            CPU_Stage* IQ = &cpu->IQ[stage->IQ_index];
            IQ->number_instuction_index=test_instruction_number;
            IQ->pc=stage->pc;
            strcpy(IQ->opcode, stage->opcode);
            IQ->rd=stage->rd;
            IQ->rs1=stage->rs1;
            IQ->rs2=stage->rs2;
            IQ->imm=stage->imm;
            IQ->URF_index=stage->URF_index;
            IQ->pre_URF_index=stage->pre_URF_index;
                IQ->URF_rs1_index=stage->URF_rs1_index;
            IQ->URF_rs2_index=stage->URF_rs2_index;
            IQ->IQ_index=stage->IQ_index;
            IQ->LSQ_index=stage->LSQ_index;
            IQ->ROB_index=stage->ROB_index;
            IQ->IQ_finish_index=0;
            cpu->IQ_valid[stage->IQ_index]=1;
            cpu->z_value_valid[test_instruction_number]=2;
            
            CPU_Stage* ROB = &cpu->ROB[stage->ROB_index];
            ROB->pc=stage->pc;
            ROB->number_instuction_index=test_instruction_number;
            strcpy(ROB->opcode, stage->opcode);
            ROB->rd=stage->rd;
            ROB->rs1=stage->rs1;
            ROB->rs2=stage->rs2;
            ROB->imm=stage->imm;
            ROB->pre_URF_index=stage->pre_URF_index;
                ROB->URF_index=stage->URF_index;
            ROB->URF_rs1_index=stage->URF_rs1_index;
            ROB->URF_rs2_index=stage->URF_rs2_index;
            ROB->IQ_index=stage->IQ_index;
            ROB->LSQ_index=stage->LSQ_index;
            ROB->ROB_index=stage->ROB_index;
            ROB->ROB_finish_index=0;
            cpu->ROB_valid[stage->ROB_index]=1;
        }
        /* Copy data from decode latch to execute latch*/
        //            cpu->stage[EX] = cpu->stage[DRF];
        
    }
    
    /* Read data from register file for JUMP */
    if (strcmp(stage->opcode, "JUMP") == 0) {
        if (stage->rs1>-1) {
            if (cpu->RAT[stage->rs1] != -1) {
                stage->URF_rs1_index=cpu->RAT[stage->rs1];
//                printf("%d ,RAT %d ,  value %d\n",stage->rs1, stage->URF_rs1_index,cpu->URF[stage->URF_rs1_index]);
                
                cpu->URF_check_vaild[stage->URF_rs1_index]++;
            }
        }
        
        
        if (cpu->URF_valid[stage->URF_rs1_index] <= 0) {
            if (strcmp(stage_EX->opcode, "LOAD") == 0) {
                RS1_INDEX=1;
                
            }else{
                stage->rs1_value = cpu->URF[stage->URF_rs1_index];
                RS1_INDEX=0;
            }
        }else{
            stage->rs1_value = cpu->regs[stage->rs1];
            RS1_INDEX=0;
            
        }
        
        if (RS1_INDEX == 0) {
            STOP_INDEX=0;
        }else{
            STOP_INDEX=1;
        }
        
        
        
        
        int temp;
        for (temp=0 ; temp<16; temp++) {
            if (cpu->IQ_valid[temp]==0 ) {
                stage->IQ_index=temp;
                IQ_full_index=0;
                break;
            }
        }
        if (temp>15) {
            IQ_full_index=1;
        }
        
        for (temp=0 ; temp<32; temp++) {
            if (cpu->ROB_valid[temp]==0 ) {
                stage->ROB_index=temp;
                ROB_full_index=0;
                break;
            }
        }
        if (temp>31) {
            ROB_full_index=1;
        }
        
        if (IQ_full_index==0 && LSQ_full_index==0 && ROB_full_index==0) {
            cpu->instruction_number[0]=cpu->instruction_number[0]+1;
            test_instruction_number++;
            CPU_Stage* IQ = &cpu->IQ[stage->IQ_index];
            IQ->number_instuction_index=test_instruction_number;
            IQ->pc=stage->pc;
            strcpy(IQ->opcode, stage->opcode);
            IQ->rd=stage->rd;
            IQ->rs1=stage->rs1;
            IQ->rs2=stage->rs2;
            IQ->imm=stage->imm;
            IQ->URF_index=stage->URF_index;
            IQ->pre_URF_index=stage->pre_URF_index;
                IQ->URF_rs1_index=stage->URF_rs1_index;
            IQ->URF_rs2_index=stage->URF_rs2_index;
            IQ->IQ_index=stage->IQ_index;
            IQ->LSQ_index=stage->LSQ_index;
            IQ->ROB_index=stage->ROB_index;
            IQ->IQ_finish_index=0;
            cpu->IQ_valid[stage->IQ_index]=1;
            cpu->z_value_valid[test_instruction_number]=3;
            
            CPU_Stage* ROB = &cpu->ROB[stage->ROB_index];
            ROB->pc=stage->pc;
            ROB->number_instuction_index=test_instruction_number;
            strcpy(ROB->opcode, stage->opcode);
            ROB->rd=stage->rd;
            ROB->rs1=stage->rs1;
            ROB->rs2=stage->rs2;
            ROB->imm=stage->imm;
            ROB->pre_URF_index=stage->pre_URF_index;
                ROB->URF_index=stage->URF_index;
            ROB->URF_rs1_index=stage->URF_rs1_index;
            ROB->URF_rs2_index=stage->URF_rs2_index;
            ROB->IQ_index=stage->IQ_index;
            ROB->LSQ_index=stage->LSQ_index;
            ROB->ROB_index=stage->ROB_index;
            ROB->ROB_finish_index=0;
            cpu->ROB_valid[stage->ROB_index]=1;
            
        }
        
        //              clear_index=1;
        
        /* Copy data from decode latch to execute latch*/
        //                cpu->stage[EX] = cpu->stage[DRF];
        
        /* Copy data from decode latch to execute latch*/
        //            cpu->stage[EX] = cpu->stage[DRF];
        
    }
    
    /* Read data from register file for JAL */
    if (strcmp(stage->opcode, "JAL") == 0) {
        if (stage->rs1>-1) {
            if (cpu->RAT[stage->rs1] != -1) {
                stage->URF_rs1_index=cpu->RAT[stage->rs1];
//                printf("%d ,RAT %d ,  value %d\n",stage->rs1, stage->URF_rs1_index,cpu->URF[stage->URF_rs1_index]);
                
                cpu->URF_check_vaild[stage->URF_rs1_index]++;
            }
        }
        
        
        if (cpu->URF_valid[stage->URF_rs1_index] <= 0) {
            if (strcmp(stage_EX->opcode, "LOAD") == 0) {
                RS1_INDEX=1;
                
            }else{
                stage->rs1_value = cpu->URF[stage->URF_rs1_index];
                RS1_INDEX=0;
            }
        }else{
            stage->rs1_value = cpu->regs[stage->rs1];
            RS1_INDEX=0;
            
        }
        
        if (RS1_INDEX == 0) {
            STOP_INDEX=0;
        }else{
            STOP_INDEX=1;
        }
        
        
        
        
        if (stage->rd>-1) {
            
            free_register=0;
            for (; free_register<40; free_register++) {
                if (cpu->URF_valid[free_register] == 1) {
                    
                    
                    int temp=cpu->RAT[stage->rd];
                    
                    cpu->RAT[stage->rd]=free_register;
                    
                    stage->URF_index=cpu->RAT[stage->rd];
                    
                    cpu->URF_valid[stage->URF_index]=0;
                    cpu->URF_finished[stage->URF_index]=0;
                    
//                                      printf("rat %d rat=%d urfvalid=%d \n",stage->rd,cpu->RAT[stage->rd],cpu->URF_valid[cpu->RAT[stage->rd]]);
                    
                    cpu->URF_valid[stage->URF_index]=0;
                    cpu->URF[stage->URF_index]=stage->pc+4;
                    
                    
                    cpu->URF_check_vaild[stage->URF_index]=1;
                    
                    
                    stage->pre_URF_index=temp;
                    break;
                }
            }
            
            
            int temp;
            for (temp=0 ; temp<16; temp++) {
                if (cpu->IQ_valid[temp]==0 ) {
                    stage->IQ_index=temp;
                    IQ_full_index=0;
                    break;
                }
            }
            if (temp>15) {
                IQ_full_index=1;
            }
            
            for (temp=0 ; temp<32; temp++) {
                if (cpu->ROB_valid[temp]==0 ) {
                    stage->ROB_index=temp;
                    ROB_full_index=0;
                    break;
                }
            }
            if (temp>31) {
                ROB_full_index=1;
            }
            
            if (IQ_full_index==0 && LSQ_full_index==0 && ROB_full_index==0) {
                cpu->instruction_number[0]=cpu->instruction_number[0]+1;
                test_instruction_number++;
                CPU_Stage* IQ = &cpu->IQ[stage->IQ_index];
                IQ->number_instuction_index=test_instruction_number;
                IQ->pc=stage->pc;
                strcpy(IQ->opcode, stage->opcode);
                IQ->rd=stage->rd;
                IQ->rs1=stage->rs1;
                IQ->rs2=stage->rs2;
                IQ->imm=stage->imm;
                IQ->URF_index=stage->URF_index;
                IQ->pre_URF_index=stage->pre_URF_index;
                IQ->URF_rs1_index=stage->URF_rs1_index;
                IQ->URF_rs2_index=stage->URF_rs2_index;
                IQ->IQ_index=stage->IQ_index;
                IQ->LSQ_index=stage->LSQ_index;
                IQ->ROB_index=stage->ROB_index;
                IQ->IQ_finish_index=0;
                cpu->IQ_valid[stage->IQ_index]=1;
                cpu->z_value_valid[test_instruction_number]=3;
                
                CPU_Stage* ROB = &cpu->ROB[stage->ROB_index];
                ROB->pc=stage->pc;
                ROB->number_instuction_index=test_instruction_number;
                strcpy(ROB->opcode, stage->opcode);
                ROB->rd=stage->rd;
                ROB->rs1=stage->rs1;
                ROB->rs2=stage->rs2;
                ROB->imm=stage->imm;
                ROB->pre_URF_index=stage->pre_URF_index;
                ROB->URF_index=stage->URF_index;
                ROB->URF_rs1_index=stage->URF_rs1_index;
                ROB->URF_rs2_index=stage->URF_rs2_index;
                ROB->IQ_index=stage->IQ_index;
                ROB->LSQ_index=stage->LSQ_index;
                ROB->ROB_index=stage->ROB_index;
                ROB->ROB_finish_index=0;
                cpu->ROB_valid[stage->ROB_index]=1;
                
            }
        }
        
        //              clear_index=1;
        
        /* Copy data from decode latch to execute latch*/
        //                cpu->stage[EX] = cpu->stage[DRF];
        
        /* Copy data from decode latch to execute latch*/
        //            cpu->stage[EX] = cpu->stage[DRF];
        
    }
    
    /* No Register file read needed for HALT */
    if (strcmp(stage->opcode, "HALT") == 0) {
        //          clear_index=1;
        //no instruction needed for nop
        
        
        int temp;
        for (temp=0 ; temp<16; temp++) {
            if (cpu->IQ_valid[temp]==0 ) {
                stage->IQ_index=temp;
                IQ_full_index=0;
                break;
            }
        }
        if (temp>15) {
            IQ_full_index=1;
        }
        
        for (temp=0 ; temp<32; temp++) {
            if (cpu->ROB_valid[temp]==0 ) {
                stage->ROB_index=temp;
                ROB_full_index=0;
                break;
            }
        }
        if (temp>31) {
            ROB_full_index=1;
        }
        
        if (IQ_full_index==0 && LSQ_full_index==0 && ROB_full_index==0) {
            cpu->instruction_number[0]=cpu->instruction_number[0]+1;
            test_instruction_number++;
            
            HALT_STALL=1;
            
            CPU_Stage* ROB = &cpu->ROB[stage->ROB_index];
            ROB->pc=stage->pc;
            ROB->number_instuction_index=test_instruction_number;
            strcpy(ROB->opcode, stage->opcode);
            ROB->rd=stage->rd;
            ROB->rs1=stage->rs1;
            ROB->rs2=stage->rs2;
            ROB->imm=stage->imm;
            ROB->pre_URF_index=stage->pre_URF_index;
                ROB->URF_index=stage->URF_index;
            ROB->URF_rs1_index=stage->URF_rs1_index;
            ROB->URF_rs2_index=stage->URF_rs2_index;
            ROB->IQ_index=stage->IQ_index;
            ROB->LSQ_index=stage->LSQ_index;
            ROB->ROB_index=stage->ROB_index;
            ROB->ROB_finish_index=1;
            
            cpu->ROB_valid[stage->ROB_index]=1;
        }
        /* Copy data from decode latch to execute latch*/
        //            cpu->stage[EX] = cpu->stage[DRF];
    }
    
    /* No Register file read needed for NOP */
    if (strcmp(stage->opcode, "NOP") == 0) {
        //no instruction needed for nop
        
        stage->URF_rs1_index=-1;
        stage->URF_rs2_index=-1;
        
//                  printf("%d \n",MUL_INDEX);
        if (MUL_INDEX == 0){
            /* Copy data from decode latch to execute latch*/
            //                cpu->stage[EX] = cpu->stage[DRF];
        }
        
    }
    
    if (Total_type==2) {
        print_stage_content("02. Instruction at DECODE_RF__STAGE --->", stage);
        
        
        
        
        printf("--------------------------------\n");
        printf("Details of RENAME TABLE (RAT) State --\n");
        for (int a=0; a<16; a++) {
            if (cpu->RAT[a]>-1) {
                printf("RAT[%2d]  -->  U%d\n",a,cpu->RAT[a]);
            }
        }
        
        printf("--------------------------------\n");
        printf("Details of RENAME TABLE (R-RAT) State --\n");
        for (int a=0; a<16; a++) {
            if (cpu->R_RAT_valid[a] == 1) {
                printf("R-RAT[%2d]  -->  U%d\n",a,cpu->R_RAT[a]);
            }
        }
        
        printf("--------------------------------\n");
    }
   
    
    return 0;
}

/*
 *  Writeback Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 *                  implementation
 */
int
IQ_LSQ(APEX_CPU* cpu,CPU_Stage* stage,int Instruction_indx)
{
    /* Read data from register file for store */
    if (strcmp(stage->opcode, "STORE") == 0) {
        
        
//        stage->rs1_value = cpu->URF[stage->URF_rs1_index];
//        printf("urf rs1 %d   urf_rs1 %d  urf_rs1_value %d \n",stage->rs1_value ,stage->URF_rs1_index,cpu->URF[stage->URF_rs1_index]);
        
        
        
        stage->rs2_value = cpu->URF[stage->URF_rs2_index];
//        printf("urf rs2 %d   urf_rs2 %d  urf_rs2_value %d \n",stage->rs1_value ,stage->URF_rs1_index,cpu->URF[stage->URF_rs1_index]);
        
        
        
        /* Copy data from decode latch to execute latch*/
        cpu->EX_INT[0] = cpu->IQ[Instruction_indx];
        
        cpu->IQ_valid[Instruction_indx]=0;
        
        
    }
    
    /* Read data from register file for load */
    if (strcmp(stage->opcode, "LOAD") == 0) {
        
        stage->rs1_value = cpu->URF[stage->URF_rs1_index];
        
        
        /* Copy data from decode latch to execute latch*/
        cpu->EX_INT[0] = cpu->IQ[Instruction_indx];
        cpu->IQ_valid[Instruction_indx]=0;
        cpu->LSQ[stage->LSQ_index].LSQ_finish_index=0;
        
//        printf("%s %d %d #%d  LSQ_FINISHED %d\n",stage->opcode,stage->rd,stage->rs1,stage->imm,cpu->LSQ[stage->LSQ_index].LSQ_finish_index);
        
    }
    
    /* No Register file read needed for MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {
        //no instruction needed for movc
//        printf("%d, zflag %d \n",clear_index, cpu->z_flag[0]);
        
        
        /* Copy data from decode latch to execute latch*/
        cpu->EX_INT[0] = cpu->IQ[Instruction_indx];
        cpu->IQ_valid[Instruction_indx]=0;
    }
    
    /* No Register file read needed for ADD */
    if (strcmp(stage->opcode, "ADD") == 0) {
        
        
        
        stage->rs1_value = cpu->URF[stage->URF_rs1_index];
        
        stage->rs2_value = cpu->URF[stage->URF_rs2_index];
        
        
        /* Copy data from decode latch to execute latch*/
        cpu->EX_INT[0] = cpu->IQ[Instruction_indx];
        cpu->IQ_valid[Instruction_indx]=0;
    }
    
    /* No Register file read needed for ADD */
    if (strcmp(stage->opcode, "ADDL") == 0) {
        
        
        
        stage->rs1_value = cpu->URF[stage->URF_rs1_index];
        
        
        /* Copy data from decode latch to execute latch*/
        cpu->EX_INT[0] = cpu->IQ[Instruction_indx];
        cpu->IQ_valid[Instruction_indx]=0;
    }
    
    /* No Register file read needed for SUB */
    if (strcmp(stage->opcode, "SUB") == 0) {
        
        stage->rs1_value = cpu->URF[stage->URF_rs1_index];
        
        stage->rs2_value = cpu->URF[stage->URF_rs2_index];
        
        
        /* Copy data from decode latch to execute latch*/
        cpu->EX_INT[0] = cpu->IQ[Instruction_indx];
        cpu->IQ_valid[Instruction_indx]=0;
        
    }
    
    /* No Register file read needed for SUB */
    if (strcmp(stage->opcode, "SUBL") == 0) {
        
        stage->rs1_value = cpu->URF[stage->URF_rs1_index];
        
        
        /* Copy data from decode latch to execute latch*/
        cpu->EX_INT[0] = cpu->IQ[Instruction_indx];
        cpu->IQ_valid[Instruction_indx]=0;
        
    }
    
    /* Read data from register file for AND */
    if (strcmp(stage->opcode, "AND") == 0) {
        
        stage->rs1_value = cpu->URF[stage->URF_rs1_index];
        
        stage->rs2_value = cpu->URF[stage->URF_rs2_index];
        
        
        /* Copy data from decode latch to execute latch*/
        cpu->EX_INT[0] = cpu->IQ[Instruction_indx];
        cpu->IQ_valid[Instruction_indx]=0;
        
    }
    
    /* Read data from register file for OR */
    if (strcmp(stage->opcode, "OR") == 0) {
        
        stage->rs1_value = cpu->URF[stage->URF_rs1_index];
        
        stage->rs2_value = cpu->URF[stage->URF_rs2_index];
        
        
        
        /* Copy data from decode latch to execute latch*/
        cpu->EX_INT[0] = cpu->IQ[Instruction_indx];
        cpu->IQ_valid[Instruction_indx]=0;
        
    }
    
    /* Read data from register file for EX-OR */
    if (strcmp(stage->opcode, "EX-OR") == 0) {
        
        stage->rs1_value = cpu->URF[stage->URF_rs1_index];
        
        stage->rs2_value = cpu->URF[stage->URF_rs2_index];
        
        
        
        /* Copy data from decode latch to execute latch*/
        cpu->EX_INT[0] = cpu->IQ[Instruction_indx];
        cpu->IQ_valid[Instruction_indx]=0;
        
    }
    
    /* No Register file read needed for MUL */
    if (strcmp(stage->opcode, "MUL") == 0) {
        
        stage->rs1_value = cpu->URF[stage->URF_rs1_index];
        
        stage->rs2_value = cpu->URF[stage->URF_rs2_index];
        
//                printf("The tings in the mul function %d %s %d %d %d\n", Instruction_indx,cpu->IQ[Instruction_indx].opcode,cpu->IQ[Instruction_indx].rd, cpu->IQ[Instruction_indx].rs1, cpu->IQ[Instruction_indx].rs2);
        
        
        /* Copy data from decode latch to execute latch*/
        cpu->EX_MUL[0] = cpu->IQ[Instruction_indx];
        cpu->IQ_valid[Instruction_indx]=0;
        
    }
    
    /* Read data from register file for BZ */
    if (strcmp(stage->opcode, "BZ") == 0) {
//        if (cpu->z_flag[0] != 0) {
//            clear_index=1;
//        }
        
        
        /* Copy data from decode latch to execute latch*/
//        cpu->stage[EX] = cpu->IQ[Instruction_indx];
//        cpu->IQ_valid[Instruction_indx]=0;

        cpu->EX_INT[0] = cpu->IQ[Instruction_indx];
        cpu->IQ_valid[Instruction_indx]=0;
        
    }
    
    /* Read data from register file for BNZ */
    if (strcmp(stage->opcode, "BNZ") == 0) {
//        if (cpu->z_flag[0] == 0) {
//            clear_index=1;
//        }
        
        /* Copy data from decode latch to execute latch*/
//        cpu->stage[EX] = cpu->IQ[Instruction_indx];
//        cpu->IQ_valid[Instruction_indx]=0;

        cpu->EX_INT[0] = cpu->IQ[Instruction_indx];
        cpu->IQ_valid[Instruction_indx]=0;
    }
    
    /* Read data from register file for JUMP */
    if (strcmp(stage->opcode, "JUMP") == 0) {
        
        stage->rs1_value = cpu->URF[stage->URF_rs1_index];
        
        
        /* Copy data from decode latch to execute latch*/
        cpu->EX_INT[0] = cpu->IQ[Instruction_indx];
        cpu->IQ_valid[Instruction_indx]=0;
        
        
        
    }
    
    /* Read data from register file for JAL */
    if (strcmp(stage->opcode, "JAL") == 0) {
        
        stage->rs1_value = cpu->URF[stage->URF_rs1_index];
        
        
        /* Copy data from decode latch to execute latch*/
        cpu->EX_INT[0] = cpu->IQ[Instruction_indx];
        cpu->IQ_valid[Instruction_indx]=0;
        
        
        
    }
    
    /* No Register file read needed for HALT */
    if (strcmp(stage->opcode, "HALT") == 0) {
        //          clear_index=1;
        //no instruction needed for nop
        
        
        /* Copy data from decode latch to execute latch*/
        
        cpu->EX_INT[0] = cpu->IQ[Instruction_indx];
        cpu->IQ_valid[Instruction_indx]=0;
    }
    
    return 0;
}


/*
 *  Execute Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 *                  implementation
 */
int
execute(APEX_CPU* cpu)
{
    //    printf("%d free_regis",free_register);
    
    CPU_Stage* stage = &cpu->stage[EX];
    //    CPU_Stage* stage_F = &cpu->stage[F];
    //    CPU_Stage* stage_DRF = &cpu->stage[DRF];
    CPU_Stage* stage_WB = &cpu->stage[WB];
    
    if (!stage->busy && !stage->stalled) {
        
//        //BZ change to NOP
//        if (strcmp(stage_WB->opcode, "BZ") == 0) {
//            if (cpu->z_flag[0] != 0) {
//                strcpy(stage->opcode, "NOP");
//                stage->rd=-1;
//
//            }
//        }
//
//        //BNZ change to NOP
//        if (strcmp(stage_WB->opcode, "BNZ") == 0) {
//            if (cpu->z_flag[0] == 0) {
//                strcpy(stage->opcode, "NOP");
//                stage->rd=-1;
//
//            }
//        }
        
        //JUMP change to NOP
        if (strcmp(stage_WB->opcode, "JUMP") == 0) {
            strcpy(stage->opcode, "NOP");
            stage->rd=-1;
//            clear_index=1;
        }
        
        //HALT change to NOP
        if (strcmp(stage_WB->opcode, "HALT") == 0) {
            strcpy(stage->opcode, "NOP");
            stage->rd=-1;
//            clear_index=1;
        }
        
        
        
        //        /* MUL  */
        //        if (strcmp(stage->opcode, "MUL") == 0) {
        //            //        printf("%d \n",MUL_INDEX);
        //            if (MUL_INDEX == 0) {
        //
        //
        //                cpu->stage[DRF] = cpu->stage[F];
        //                //              STOP_INDEX=1;
        //                strcpy(stage_MEM->opcode, "NOP");
        //                stage_MEM->rd=0;
        //                MUL_INDEX = 1;
        //            } else {
        //                //              STOP_INDEX=0;
        //                printf("%d %d\n",stage->rs1_value,stage->rs2_value);
        //                stage->buffer = stage->rs1_value * stage->rs2_value;
        //                MUL_INDEX = 0;
        //
        //                cpu->URF[stage->URF_index]=stage->buffer;
        //                cpu->URF_finished[stage->URF_index]=1;
        //                printf("%d rat%d urf%d\n",stage->rd,stage->URF_index,cpu->URF[stage->URF_index]);
        //
        //                //z_flag
        //                if (stage->buffer == 0) {
        //                    cpu->z_flag[0] = 1;
        //                } else{
        //                    cpu->z_flag[0] = 0;
        //                }
        //
        //                /* Copy data from Execute latch to Memory latch*/
        //                cpu->stage[MEM] = cpu->stage[EX];
        //
        //                if (stage->URF_rs1_index>-1) {
        //                    cpu->URF_check_vaild[stage->URF_rs1_index]--;
        //                }
        //
        //                if (stage->URF_rs2_index>-1) {
        //                    cpu->URF_check_vaild[stage->URF_rs2_index]--;
        //                }
        //            }
        //
        //
        //            //          stage->buffer = stage->rs1_value * stage->rs2_value;
        //        }
        
        /* BZ  */
        if (strcmp(stage->opcode, "BZ") == 0) {
            stage->buffer = stage->pc + stage->imm;
            
            /* Copy data from Execute latch to Memory latch*/
            cpu->stage[MEM] = cpu->stage[EX];
        }
        
        /* BNZ  */
        if (strcmp(stage->opcode, "BNZ") == 0) {
            stage->buffer = stage->pc + stage->imm;
            
            
            
            /* Copy data from Execute latch to Memory latch*/
            cpu->stage[MEM] = cpu->stage[EX];
        }
        
        /* JUMP  */
        if (strcmp(stage->opcode, "JUMP") == 0) {
            stage->buffer = stage->rs1_value + stage->imm;
            
            /* Copy data from Execute latch to Memory latch*/
            cpu->stage[MEM] = cpu->stage[EX];
            
            if (stage->URF_rs1_index>-1) {
                cpu->URF_check_vaild[stage->URF_rs1_index]--;
            }
        }
        
        
        /* No Register file read needed for HALT */
        if (strcmp(stage->opcode, "HALT") == 0) {
            //no instruction needed for nop
            
            /* Copy data from Execute latch to Memory latch*/
            cpu->stage[MEM] = cpu->stage[EX];
        }
        
        /* No Register file read needed for NOP */
        if (strcmp(stage->opcode, "NOP") == 0) {
            //no instruction needed for nop
            
            
            stage->URF_rs1_index=-1;
            stage->URF_rs2_index=-2;
            
            /* Copy data from Execute latch to Memory latch*/
            cpu->stage[MEM] = cpu->stage[EX];
        }
        
        //      if (cpu->ins_completed == (cpu->code_memory_size-2)) {
        //          strcpy(stage->opcode,"NOP");
        //      }
        
        
        
        print_stage_content("Execute", stage);
        
    }
    return 0;
}

/*
 *  Memory Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 *                  implementation
 */
int
memory(APEX_CPU* cpu)
{
    CPU_Stage* stage = &cpu->stage[MEM];
    
    CPU_Stage* stage_DRF = &cpu->stage[DRF];
    CPU_Stage* stage_EX = &cpu->stage[EX];
    
    if (!stage->busy && !stage->stalled) {
        
        //        /* Store */
        //        if (strcmp(stage->opcode, "STORE") == 0) {
        //            stage->mem_address = stage->buffer;
        //            cpu->data_memory[stage->mem_address] = stage->rs1_value;
        //
        //            cpu->URF[stage->rd]=stage->buffer;
        //
        //            if (stage->URF_rs1_index>-1) {
        //                cpu->URF_check_vaild[stage->URF_rs1_index]--;
        //            }
        //
        //            cpu->LSQ_valid[stage->LSQ_index]=0;
        //
        //
        //            CPU_Stage* ROB=&cpu->ROB[stage->ROB_index];
        //            ROB->ROB_finish_index=1;
        //            ROB->mem_address=stage->mem_address;
        //            ROB->buffer=stage->buffer;
        //            ROB->rs1_value=stage->rs1_value;
        //        }
        //
        //        /* LOAD */
        //        if (strcmp(stage->opcode, "LOAD") == 0) {
        //            stage->mem_address = stage->buffer;
        //            stage->rs2_value = cpu->data_memory[stage->mem_address];
        //
        //            printf("buffer %d  value %d  \n",stage->buffer, stage->rs2_value);
        //
        //
        //            stage->buffer=stage->rs2_value;
        //
        //            cpu->URF[stage->URF_index]=stage->buffer;
        //            cpu->URF_finished[stage->URF_index]=1;
        //
        //            cpu->LSQ_valid[stage->LSQ_index]=0;
        //
        //            CPU_Stage* ROB=&cpu->ROB[stage->ROB_index];
        //            ROB->ROB_finish_index=1;
        //            ROB->mem_address=stage->mem_address;
        //            ROB->buffer=stage->buffer;
        //            ROB->rs1_value=stage->rs1_value;
        //        }
        
        
        
        
        /* BZ */
        if (strcmp(stage->opcode, "BZ") == 0) {
            //no instructions in memory
            if (cpu->z_flag[0] == 1) {
                MUL_INDEX =0;
                STOP_INDEX=0;
                
                strcpy(stage_EX->opcode, "NOP");
                //          stage_EX->pc = 0;
                
                stage_EX->rd=-1;
                
                strcpy(stage_DRF->opcode, "NOP");
                //          stage_DRF->pc = 0;
                stage_DRF->rd=-1;
                
                cpu->pc = stage->buffer;
            }
            
            
            
            CPU_Stage* ROB=&cpu->ROB[stage->ROB_index];
            ROB->ROB_finish_index=1;
            ROB->mem_address=stage->mem_address;
            ROB->buffer=stage->buffer;
            ROB->rs1_value=stage->rs1_value;
        }
        
        /* BNZ */
        if (strcmp(stage->opcode, "BNZ") == 0) {
            //no instructions in memory
            if (cpu->z_flag[0] == 0) {
                MUL_INDEX =0;
                STOP_INDEX=0;
                
                strcpy(stage_EX->opcode, "NOP");
                //          stage_EX->pc = 0;
                
                stage_EX->rd=-1;
                
                stage_EX->URF_rs1_index=-1;
                stage_EX->URF_rs2_index=-2;
                
                strcpy(stage_DRF->opcode, "NOP");
                //          stage_DRF->pc = 0;
                stage_DRF->rd=-1;
                stage_DRF->URF_rs1_index=-1;
                stage_DRF->URF_rs2_index=-2;
                
                cpu->pc = stage->buffer;
            }
            
            
            
            CPU_Stage* ROB=&cpu->ROB[stage->ROB_index];
            ROB->ROB_finish_index=1;
            ROB->mem_address=stage->mem_address;
            ROB->buffer=stage->buffer;
            ROB->rs1_value=stage->rs1_value;
        }
        
        //        /* JUMP */
        //        if (strcmp(stage->opcode, "JUMP") == 0) {
        //            //no instructions in memory
        //            MUL_INDEX =0;
        //            STOP_INDEX=0;
        //
        //            strcpy(stage_EX->opcode, "NOP");
        //            //          stage_EX->pc = 0;
        //
        //            stage_EX->rd=-1;
        //
        //            strcpy(stage_DRF->opcode, "NOP");
        //            //          stage_DRF->pc = 0;
        //            stage_DRF->rd=-1;
        //
        //            cpu->pc = stage->buffer;
        //
        //            CPU_Stage* ROB=&cpu->ROB[stage->ROB_index];
        //            ROB->ROB_finish_index=1;
        //            ROB->mem_address=stage->mem_address;
        //            ROB->buffer=stage->buffer;
        //            ROB->rs1_value=stage->rs1_value;
        //        }
        
        /* HALT */
        if (strcmp(stage->opcode, "HALT") == 0) {
            //no instructions in memory
            
            CPU_Stage* ROB=&cpu->ROB[stage->ROB_index];
            ROB->ROB_finish_index=1;
            ROB->mem_address=stage->mem_address;
            ROB->buffer=stage->buffer;
            ROB->rs1_value=stage->rs1_value;
            
        }
        
        /* Copy data from decode latch to execute latch*/
        //        cpu->stage[WB] = cpu->stage[MEM];
        
        
        print_stage_content("Memory", stage);
        
    }
    return 0;
}

/*
 *  Writeback Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 *                  implementation
 */
int
writeback(APEX_CPU* cpu)
{
    CPU_Stage* stage = &cpu->stage[WB];
    if (!stage->busy && !stage->stalled) {
        
        /* Update register file */
        if (strcmp(stage->opcode, "MOVC") == 0) {
            cpu->regs_valid[stage->rd] = 1;
            cpu->regs[stage->rd] = stage->buffer;
            
            //        cpu->URF_valid[stage->URF_index]=1;
            cpu->URF[stage->URF_index]=stage->buffer;
            cpu->URF_check_vaild[stage->URF_index]--;
            cpu->ins_completed++;
            
            cpu->URF_valid[stage->pre_URF_index]=1;
        }
        /* Store */
        if (strcmp(stage->opcode, "STORE") == 0) {
            cpu->ins_completed++;
        }
        
        /* LOAD */
        if (strcmp(stage->opcode, "LOAD") == 0) {
            cpu->regs_valid[stage->rd] = 1;
            cpu->regs[stage->rd] = stage->rs2_value;
            
            //        cpu->URF_valid[stage->URF_index]=1;
            cpu->URF[stage->URF_index]=stage->buffer;
            cpu->URF_check_vaild[stage->URF_index]--;
            
            cpu->ins_completed++;
            
            cpu->URF_valid[stage->pre_URF_index]=1;
        }
        
        /* ADD */
        if (strcmp(stage->opcode, "ADD") == 0) {
            cpu->regs_valid[stage->rd] = 1;
            cpu->regs[stage->rd] = stage->buffer;
            
            cpu->URF[stage->URF_index]=stage->buffer;
            //          cpu->URF_valid[stage->URF_index]=1;
            cpu->URF_check_vaild[stage->URF_index]--;
            
            cpu->ins_completed++;
            
            cpu->URF_valid[stage->pre_URF_index]=1;
        }
        
        /* SUB */
        if (strcmp(stage->opcode, "SUB") == 0) {
            cpu->regs_valid[stage->rd] = 1;
            cpu->regs[stage->rd] = stage->buffer;
            
            cpu->URF[stage->URF_index]=stage->buffer;
            //          cpu->URF_valid[stage->URF_index]=1;
            cpu->URF_check_vaild[stage->URF_index]--;
            
            cpu->ins_completed++;
            
            cpu->URF_valid[stage->pre_URF_index]=1;
        }
        
        /* AND */
        if (strcmp(stage->opcode, "AND") == 0) {
            cpu->regs_valid[stage->rd] = 1;
            cpu->regs[stage->rd] = stage->buffer;
            
            cpu->URF[stage->URF_index]=stage->buffer;
            //          cpu->URF_valid[stage->URF_index]=1;
            cpu->URF_check_vaild[stage->URF_index]--;
            
            cpu->ins_completed++;
            
            cpu->URF_valid[stage->pre_URF_index]=1;
        }
        
        /* OR */
        if (strcmp(stage->opcode, "OR") == 0) {
            cpu->regs_valid[stage->rd] = 1;
            cpu->regs[stage->rd] = stage->buffer;
            
            cpu->URF[stage->URF_index]=stage->buffer;
            //          cpu->URF_valid[stage->URF_index]=1;
            cpu->URF_check_vaild[stage->URF_index]--;
            
            cpu->ins_completed++;
            
            cpu->URF_valid[stage->pre_URF_index]=1;
        }
        
        /* EX-OR */
        if (strcmp(stage->opcode, "EX-OR") == 0) {
            cpu->regs_valid[stage->rd] = 1;
            cpu->regs[stage->rd] = stage->buffer;
            
            cpu->URF[stage->URF_index]=stage->buffer;
            //          cpu->URF_valid[stage->URF_index]=1;
            cpu->URF_check_vaild[stage->URF_index]--;
            
            cpu->ins_completed++;
            
            cpu->URF_valid[stage->pre_URF_index]=1;
        }
        
        /* MUL */
        if (strcmp(stage->opcode, "MUL") == 0) {
            cpu->regs_valid[stage->rd] = 1;
            cpu->regs[stage->rd] = stage->buffer;
            
            cpu->URF[stage->URF_index]=stage->buffer;
            //          cpu->URF_valid[stage->URF_index]=1;
            cpu->URF_check_vaild[stage->URF_index]--;
            
            cpu->ins_completed++;
            
            cpu->URF_valid[stage->pre_URF_index]=1;
        }
        
        /* BZ */
        if (strcmp(stage->opcode, "BZ") == 0) {
            cpu->ins_completed++;
            clear_index=0;
        }
        
        /* BNZ */
        if (strcmp(stage->opcode, "BNZ") == 0) {
            cpu->ins_completed++;
            clear_index=0;
        }
        
        /* JUMP */
        if (strcmp(stage->opcode, "JUMP") == 0) {
            cpu->ins_completed++;
            clear_index=0;
            
            cpu->URF[stage->URF_index]=stage->buffer;
            //          cpu->URF_valid[stage->URF_index]=1;
        }
        
        /* HALT */
        if (strcmp(stage->opcode, "HALT") == 0) {
            HALT_INDEX = 1;
            clear_index=0;
        }
        
        /* NOP */
        if (strcmp(stage->opcode, "NOP") == 0) {
            
            stage->URF_rs1_index=-1;
            stage->URF_rs2_index=-2;
        }
        
        print_stage_content_WB("Writeback", stage);
        
    }
    return 0;
}

/*
 *  Writeback Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 *                  implementation
 */
int
LSQ_printing_stage(APEX_CPU* cpu)
{
    if (Total_type==2) {
//        printf("0 cpu_urfvalid %d \n",cpu->URF_valid[0]);
        printf("--------------------------------\n");
        printf("Details of LSQ (Load-Store Queue) State --\n");
        for (int a=0; a<20; a++) {
            if (cpu->LSQ_valid[a]==1) {
                CPU_Stage* LSQ = &cpu->LSQ[a];
                //            printf("%2d. ",a);
                print_instruction(LSQ);
                printf("  number of insturction %d",LSQ->number_instuction_index);
                printf("\n");
            }
        }
        
        printf("--------------------------------\n");
        printf("Details of ROB (Reorder Buffer) State --\n");
        for (int a=0; a<32; a++) {
            if (cpu->ROB_valid[a]==1) {
                CPU_Stage* ROB = &cpu->ROB[a];
                //            printf("%2d. ",a);
                print_instruction(ROB);
                printf("  number of instruction %d",ROB->number_instuction_index);
                printf("\n");
            }
        }
        
        printf("--------------------------------\n");
        printf("Details of IQ (Issue Queue) State --\n");
        for (int a=0; a<16; a++) {
            if (cpu->IQ_valid[a]==1) {
                CPU_Stage* IQ = &cpu->IQ[a];
                //            printf("%2d. ",a);
                print_instruction(IQ);
                printf("  number of instruction %d",IQ->number_instuction_index);
                printf("\n");
            }
        }
        
        printf("--------------------------------\n");
    }
    
//    printf("6 URF_value %d  URF_FINISHED %d\n",cpu->URF_valid[6],cpu->URF_finished[6]);
    
    
//    printf("%d  4   FINISHED %d\n",MUL_INDEX,cpu->URF_finished[4]);
    
    
    for (int a=0; a<16; a++) {
        if (cpu->IQ_valid[a]==1) {
            CPU_Stage* test = &cpu->IQ[a];
            
            //            printf("%2d, %s IQ_valid=%d \n",a,test->opcode, cpu->IQ_valid[a]);
            cpu->IQ_check_index[a]=1;
            if (strcmp(test->opcode, "STORE") == 0) {
//                printf("%d STORE URF_index%d CPU_valid %d URF_finished %d \n",a,test->URF_rs1_index,cpu->URF_valid[test->URF_rs1_index],cpu->URF_finished[test->URF_rs1_index]);

                
//                if (cpu->URF_valid[test->URF_rs1_index]==0 && cpu->URF_finished[test->URF_rs1_index]==1) {
//                    //                    cpu->IQ_check_index[a]=1;
////                    printf("sdlfjadslfjasdlf\n");
//                }else{
//                    cpu->IQ_check_index[a]=0;
//                }
//                printf("%d check_id\n",cpu->IQ_check_index[a]);
//                printf("%d STORE URF_index%d CPU_valid %d URF_finished %d \n",a,test->URF_rs2_index,cpu->URF_valid[test->URF_rs2_index],cpu->URF_finished[test->URF_rs2_index]);
                
                if (cpu->URF_valid[test->URF_rs2_index]==0 && cpu->URF_finished[test->URF_rs2_index]==1) {
                    //                    cpu->IQ_check_index[a]=1;
                }else{
                    cpu->IQ_check_index[a]=0;
                }
                
//                printf("%d check_id\n",cpu->IQ_check_index[a]);
                
                if (cpu->IQ_check_index[a]==1) {
                    test->IQ_finish_index=1;
                }
            }else if (strcmp(test->opcode, "LOAD") == 0){
                if (cpu->URF_valid[test->URF_rs1_index]==0 && cpu->URF_finished[test->URF_rs1_index]==1) {
                    //                    cpu->IQ_check_index[a]=1;
                }else{
                    cpu->IQ_check_index[a]=0;
                }
                
                if (cpu->IQ_check_index[a]==1) {
                    test->IQ_finish_index=1;
                }
            }else if (strcmp(test->opcode, "MOVC") == 0){
                //                cpu->IQ_check_index[a]=1;
                
                if (cpu->IQ_check_index[a]==1) {
                    test->IQ_finish_index=1;
                }
            }else if (strcmp(test->opcode, "ADD") == 0){
                if (cpu->URF_valid[test->URF_rs1_index]==0 && cpu->URF_finished[test->URF_rs1_index]==1) {
                    //                    cpu->IQ_check_index[a]=1;
                }else{
                    cpu->IQ_check_index[a]=0;
                }
                
                if (cpu->URF_valid[test->URF_rs2_index]==0 && cpu->URF_finished[test->URF_rs2_index]==1) {
                    //                    cpu->IQ_check_index[a]=1;
                }else{
                    cpu->IQ_check_index[a]=0;
                }
                
                if (cpu->IQ_check_index[a]==1) {
                    test->IQ_finish_index=1;
                }
            }else if (strcmp(test->opcode, "ADDL") == 0){
                if (cpu->URF_valid[test->URF_rs1_index]==0 && cpu->URF_finished[test->URF_rs1_index]==1) {
                    //                    cpu->IQ_check_index[a]=1;
                }else{
                    cpu->IQ_check_index[a]=0;
                }
                
                if (cpu->IQ_check_index[a]==1) {
                    test->IQ_finish_index=1;
                }
            }else if (strcmp(test->opcode, "SUB") == 0){
                if (cpu->URF_valid[test->URF_rs1_index]==0 && cpu->URF_finished[test->URF_rs1_index]==1) {
                    //                    cpu->IQ_check_index[a]=1;
                }else{
                    cpu->IQ_check_index[a]=0;
                }
                
                if (cpu->URF_valid[test->URF_rs2_index]==0 && cpu->URF_finished[test->URF_rs2_index]==1) {
                    //                    cpu->IQ_check_index[a]=1;
                }else{
                    cpu->IQ_check_index[a]=0;
                }
                
                if (cpu->IQ_check_index[a]==1) {
                    test->IQ_finish_index=1;
                }
            }else if (strcmp(test->opcode, "SUBL") == 0){
                if (cpu->URF_valid[test->URF_rs1_index]==0 && cpu->URF_finished[test->URF_rs1_index]==1) {
                    //                    cpu->IQ_check_index[a]=1;
                }else{
                    cpu->IQ_check_index[a]=0;
                }
                
                if (cpu->IQ_check_index[a]==1) {
                    test->IQ_finish_index=1;
                }
            }else if (strcmp(test->opcode, "AND") == 0){
                if (cpu->URF_valid[test->URF_rs1_index]==0 && cpu->URF_finished[test->URF_rs1_index]==1) {
                    //                    cpu->IQ_check_index[a]=1;
                }else{
                    cpu->IQ_check_index[a]=0;
                }
                
                if (cpu->URF_valid[test->URF_rs2_index]==0 && cpu->URF_finished[test->URF_rs2_index]==1) {
                    //                    cpu->IQ_check_index[a]=1;
                }else{
                    cpu->IQ_check_index[a]=0;
                }
                
                if (cpu->IQ_check_index[a]==1) {
                    test->IQ_finish_index=1;
                }
            }else if (strcmp(test->opcode, "OR") == 0){
                if (cpu->URF_valid[test->URF_rs1_index]==0 && cpu->URF_finished[test->URF_rs1_index]==1) {
                    //                    cpu->IQ_check_index[a]=1;
                }else{
                    cpu->IQ_check_index[a]=0;
                }
                
                if (cpu->URF_valid[test->URF_rs2_index]==0 && cpu->URF_finished[test->URF_rs2_index]==1) {
                    //                    cpu->IQ_check_index[a]=1;
                }else{
                    cpu->IQ_check_index[a]=0;
                }
                
                if (cpu->IQ_check_index[a]==1) {
                    test->IQ_finish_index=1;
                }
            }else if (strcmp(test->opcode, "EX-OR") == 0){
                if (cpu->URF_valid[test->URF_rs1_index]==0 && cpu->URF_finished[test->URF_rs1_index]==1) {
                    //                    cpu->IQ_check_index[a]=1;
                }else{
                    cpu->IQ_check_index[a]=0;
                }
                
                if (cpu->URF_valid[test->URF_rs2_index]==0 && cpu->URF_finished[test->URF_rs2_index]==1) {
                    //                    cpu->IQ_check_index[a]=1;
                }else{
                    cpu->IQ_check_index[a]=0;
                }
                
                if (cpu->IQ_check_index[a]==1) {
                    test->IQ_finish_index=1;
                }
            }else if (strcmp(test->opcode, "MUL") == 0){
                if (cpu->URF_valid[test->URF_rs1_index]==0 && cpu->URF_finished[test->URF_rs1_index]==1) {
                    //                    cpu->IQ_check_index[a]=1;
                }else{
                    cpu->IQ_check_index[a]=0;
                }
                
                if (cpu->URF_valid[test->URF_rs2_index]==0 && cpu->URF_finished[test->URF_rs2_index]==1) {
                    //                    cpu->IQ_check_index[a]=1;
                }else{
                    cpu->IQ_check_index[a]=0;
                }
                
                if (cpu->IQ_check_index[a]==1) {
                    test->IQ_finish_index=1;
                }
            }else if (strcmp(test->opcode, "BZ") == 0){
                //                cpu->IQ_check_index[a]=1;
                cpu->IQ_check_index[a]=0;
//                printf("PAss BZ\n");
                for (int b=test->number_instuction_index; b>0; b--) {
                    if (cpu->z_value_valid[b]==1) {
//                        printf("%d z_valid==1   z_value=%d\n",b,cpu->z_value[b]);
                        if (cpu->z_value[b]==1 || cpu->z_value[b]==0) {
                            cpu->IQ_check_index[a]=1;
                        }else{
                            cpu->IQ_check_index[a]=0;
                        }
//                        printf("%d cpu->IQ_Check _index",cpu->IQ_check_index[0]);
                        break;
                    }
                }
                if (cpu->IQ_check_index[a]==1) {
                    test->IQ_finish_index=1;
                }
            }else if (strcmp(test->opcode, "BNZ") == 0){
                //                cpu->IQ_check_index[a]=1;
                cpu->IQ_check_index[a]=0;
                for (int b=test->number_instuction_index; b>0; b--) {
                    if (cpu->z_value_valid[b]==1) {
                        if (cpu->z_value[b]>-1) {
                            cpu->IQ_check_index[a]=1;
                        }else{
                            cpu->IQ_check_index[a]=0;
                        }
                        break;
                    }
                }
                if (cpu->IQ_check_index[a]==1) {
                    test->IQ_finish_index=1;
                }
            }else if (strcmp(test->opcode, "JUMP") == 0){
                //                cpu->IQ_check_index[a]=1;
                
                if (cpu->IQ_check_index[a]==1) {
                    test->IQ_finish_index=1;
                }
            }else if (strcmp(test->opcode, "JAL") == 0){
                //                cpu->IQ_check_index[a]=1;
                
                if (cpu->IQ_check_index[a]==1) {
                    test->IQ_finish_index=1;
                }
            }else if (strcmp(test->opcode, "HALT") == 0){
                //                cpu->IQ_check_index[a]=1;
                
                if (cpu->IQ_check_index[a]==1) {
                    test->IQ_finish_index=1;
                }
            }
            
            if (strcmp(test->opcode, "STORE") == 0) {
                test->function_unin_type=1;
            }else if (strcmp(test->opcode, "LOAD") == 0){
                test->function_unin_type=1;
            }else if (strcmp(test->opcode, "ADD") == 0){
                test->function_unin_type=1;
            }else if (strcmp(test->opcode, "ADDL") == 0){
                test->function_unin_type=1;
            }else if (strcmp(test->opcode, "MOVC") == 0){
                test->function_unin_type=1;
//                                printf("pass1 \n");
            }else if (strcmp(test->opcode, "SUB") == 0){
                test->function_unin_type=1;
            }else if (strcmp(test->opcode, "SUBL") == 0){
                test->function_unin_type=1;
            }else if (strcmp(test->opcode, "AND") == 0){
                test->function_unin_type=1;
            }else if (strcmp(test->opcode, "OR") == 0){
                test->function_unin_type=1;
            }else if (strcmp(test->opcode, "EX-OR") == 0){
                test->function_unin_type=1;
            }else if (strcmp(test->opcode, "BZ") == 0){
                test->function_unin_type=1;
            }else if (strcmp(test->opcode, "BNZ") == 0){
                test->function_unin_type=1;
            }else if (strcmp(test->opcode, "JUMP") == 0){
                test->function_unin_type=1;
            }else if (strcmp(test->opcode, "JAL") == 0){
                test->function_unin_type=1;
            }else if (strcmp(test->opcode, "HALT") == 0){
                test->function_unin_type=1;
            }else if (strcmp(test->opcode, "MUL") == 0){
                test->function_unin_type=2;
            }
        }
    }
    
    int temp=1000;
    int instruction_indx1=-1;
    int temp2=1000;
    int instruction_indx2=-1;
    
    
    
    for (int a=0; a<16; a++) {
        if (cpu->IQ_valid[a]==1) {
            if (cpu->IQ_check_index[a]==1) {
                
                if (cpu->IQ[a].IQ_finish_index==1 && cpu->IQ[a].function_unin_type==1) {
                    if (cpu->IQ[a].number_instuction_index<temp) {
                        temp=cpu->IQ[a].number_instuction_index;
                        instruction_indx1=a;
                    }
                }
            }
            
        }
    }
    
    if (instruction_indx1>-1 && instruction_indx1<16) {
        CPU_Stage* IQ= &cpu->IQ[instruction_indx1];
        
        IQ_LSQ(cpu, IQ, instruction_indx1);
        cpu->IQ_valid[instruction_indx1]=0;
        
    }else{
        CPU_Stage* stage= &cpu->EX_INT[0];
        strcpy(stage->opcode, "NOP");
        stage->rd=-1;
        stage->URF_index=-1;
        stage->URF_rs1_index=-1;
        stage->URF_rs2_index=-1;
        
    }
    
    for (int a=0; a<16; a++) {
        if (cpu->IQ_valid[a]==1) {
            if (cpu->IQ_check_index[a]==1) {
                
                if (cpu->IQ[a].IQ_finish_index==1 && cpu->IQ[a].function_unin_type==2) {
//                    printf("%d  %s %d %d %d  mul_valid %d\n",instruction_indx2,cpu->IQ[instruction_indx2].opcode,cpu->IQ[instruction_indx2].rd,cpu->IQ[instruction_indx2].rs1, cpu->IQ[instruction_indx2].rs2, cpu->IQ_check_index[instruction_indx2]);
                    
                    if (cpu->IQ[a].number_instuction_index<temp2) {
                        temp2=cpu->IQ[a].number_instuction_index;
                        instruction_indx2=a;
                    }
                }
            }
        }
        
    }
    
    
    
    if (MUL_FINISHED==1) {
        
        
        if (instruction_indx2>-1 && instruction_indx2<16) {
            CPU_Stage* IQ = &cpu->IQ[instruction_indx2];
            
            //            printf("The tings in the mul function %d %s %d %d %d\n", instruction_indx2,cpu->IQ[instruction_indx2].opcode,cpu->IQ[instruction_indx2].rd, cpu->IQ[instruction_indx2].rs1, cpu->IQ[instruction_indx2].rs2);
            
            IQ_LSQ(cpu, IQ, instruction_indx2);
            cpu->IQ_valid[instruction_indx2]=0;
            
            
        }else{
            CPU_Stage* fu_mul = &cpu->EX_MUL[0];
            strcpy(fu_mul->opcode, "NOP");
        }
    }else{
        
    }
    
    
    
    return 0;
}


/*
 *  Memory Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 *                  implementation
 */
int
execuse_MEM(APEX_CPU* cpu)
{
    CPU_Stage* stage = &cpu->EX_MEM[0];
    if (Total_type == 2)
    {    
        printf("EXECUSE_MEM the MEM_FINISHED_ID %d\n",MEM_FINISHED);
    }
    
    if (MEM_FINISHED==1) {
        
        /* Store */
        if (strcmp(stage->opcode, "STORE") == 0) {
            
            MEM_FINISHED=2;
        }/* LOAD */
        else if (strcmp(stage->opcode, "LOAD") == 0) {
            
            
            MEM_FINISHED=2;
        }else{
            strcpy(stage->opcode, "NOP");
        }
    }else if (MEM_FINISHED==2){
        /* Store */
        if (strcmp(stage->opcode, "STORE") == 0) {
            
            MEM_FINISHED=3;
        }
        
        /* LOAD */
        if (strcmp(stage->opcode, "LOAD") == 0) {
            
            
            MEM_FINISHED=3;
        }
    }else if (MEM_FINISHED==3){
        MEM_FINISHED=4;
        /* Store */
        if (strcmp(stage->opcode, "STORE") == 0) {
            stage->rs1_value = cpu->URF[stage->URF_rs1_index];
            stage->mem_address = stage->buffer;
            cpu->data_memory[stage->mem_address] = stage->rs1_value;
            
            cpu->URF[stage->rd]=stage->buffer;
            
            if (stage->URF_rs1_index>-1) {
                cpu->URF_check_vaild[stage->URF_rs1_index]--;
            }
            
            //            cpu->LSQ_valid[stage->LSQ_index]=0;
            
            
            CPU_Stage* ROB=&cpu->ROB[stage->ROB_index];
            ROB->ROB_finish_index=1;
            ROB->mem_address=stage->mem_address;
            ROB->buffer=stage->buffer;
            ROB->rs1_value=stage->rs1_value;
            
            ROB->ROB_finish_index=0;
            
            //            print_mem_information=1;
            //            cpu->Write_ROB[0]=cpu->ROB[stage->ROB_index];
            
            //            cpu->ROB_valid[stage->ROB_index]=0;
        }
        
        /* LOAD */
        if (strcmp(stage->opcode, "LOAD") == 0) {
            stage->mem_address = stage->buffer;
            stage->rs2_value = cpu->data_memory[stage->mem_address];
            
//            printf("buffer %d  value %d  LOAD LSQ index %d\n",stage->buffer, stage->rs2_value,stage->LSQ_finish_index);
            
            
            stage->buffer=stage->rs2_value;
            
            cpu->URF[stage->URF_index]=stage->buffer;
            cpu->URF_finished[stage->URF_index]=1;
            
//            printf("URF_index %d, URF_value %d, URF_FINISHED %d\n",stage->URF_index,cpu->URF[stage->URF_index],cpu->URF_finished[stage->URF_index]);
            
            //            cpu->LSQ_valid[stage->LSQ_index]=0;
            
            
            cpu->regs_valid[stage->rd] = 1;
            cpu->regs[stage->rd] = stage->rs2_value;
            
            //        cpu->URF_valid[stage->URF_index]=1;
            cpu->URF_check_vaild[stage->URF_index]--;
            
            cpu->ins_completed++;
            
            cpu->URF_valid[stage->pre_URF_index]=1;
            cpu->R_RAT[stage->rd]=stage->URF_index;
            cpu->R_RAT_valid[stage->rd]=1;
            
            load_pre_URF_index=stage->pre_URF_index;
            load_rd=stage->rd;
            load_URF_index=stage->URF_index;
            
            
            CPU_Stage* ROB=&cpu->ROB[stage->ROB_index];
            ROB->ROB_finish_index=1;
            ROB->mem_address=stage->mem_address;
            ROB->buffer=stage->buffer;
            ROB->rs1_value=stage->rs1_value;
            
            //            print_mem_information=1;
            //            cpu->Write_ROB[0]=cpu->ROB[stage->ROB_index];
        
            
            Load_FuncInt_index=1;
            
        }
        
    }
    
    if (Total_type==2) {
        print_stage_content("Details of MEM FU --", stage);
        printf("--------------------------------\n");
    }
    
    
    if (MEM_FINISHED==4) {
        MEM_FINISHED=1;
        
        strcpy(stage->opcode, "NOP");
    }
    return 0;
}


/*
 *  Execute_Int Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 *                  implementation
 */
int
execute_Int(APEX_CPU* cpu)
{
    CPU_Stage* stage = &cpu->EX_INT[0];
    
    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0) {
//        printf("rs1value %d  urf %d\n", stage->rs1_value,cpu->URF[stage->URF_rs1_index]);
//        printf("rs2value %d  urf %d\n", stage->rs2_value,cpu->URF[stage->URF_rs2_index]);
//        printf("IMM %d  \n", stage->imm);
        
        if (cpu->URF_valid[stage->URF_rs1_index] <= 0) {
            
            stage->rs1_value = cpu->URF[stage->URF_rs1_index];
            
        }else{
            stage->rs1_value = cpu->regs[stage->rs1];
            
            
        }
        
        stage->buffer = stage->rs2_value + stage->imm;
        
//        printf("BUFFER %d  \n", stage->buffer);
        
        /* Copy data from Execute latch to Memory latch*/
        //        cpu->stage[MEM] = cpu->stage[EX];
        
        if (stage->URF_rs2_index>-1) {
            cpu->URF_check_vaild[stage->URF_rs2_index]--;
        }
        
        CPU_Stage* LSQ = &cpu->LSQ[stage->LSQ_index];
        LSQ->buffer=stage->buffer;
        LSQ->rs1_value=stage->rs1_value;
        LSQ->LSQ_finish_index=1;
        
        CPU_Stage* ROB =&cpu->ROB[stage->ROB_index];
        ROB->ROB_finish_index=1;
        ROB->buffer=stage->buffer;
        ROB->rs1_value=stage->rs1_value;
    }
    
    /* MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {
        stage->buffer = stage->imm + 0;
        
        cpu->URF[stage->URF_index]=stage->buffer;
        cpu->URF_finished[stage->URF_index]=1;
        
        //            /* Copy data from Execute latch to Memory latch*/
        //            cpu->stage[MEM] = cpu->stage[EX];
        
        CPU_Stage* ROB=&cpu->ROB[stage->ROB_index];
        ROB->ROB_finish_index=1;
        ROB->mem_address=stage->mem_address;
        ROB->buffer=stage->buffer;
        ROB->rs1_value=stage->rs1_value;
    }
    
    
    /* LOAD  */
    if (strcmp(stage->opcode, "LOAD") == 0) {
        
        
        
        stage->buffer = stage->rs1_value + stage->imm;
        
//        printf("rs1 %d urf_valid %d   URF %d  result %d\n",stage->rs1_value,cpu->URF_valid[stage->URF_rs1_index], cpu->URF[stage->URF_rs1_index],stage->buffer);
        
        /* Copy data from Execute latch to Memory latch*/
        //        cpu->stage[MEM] = cpu->stage[EX];
        
        if (stage->URF_rs1_index>-1) {
            cpu->URF_check_vaild[stage->URF_rs1_index]--;
        }
        
        CPU_Stage* LSQ = &cpu->LSQ[stage->LSQ_index];
        LSQ->buffer=stage->buffer;
        LSQ->rs1_value=stage->rs1_value;
        LSQ->LSQ_finish_index=1;
        
        CPU_Stage* ROB =&cpu->ROB[stage->ROB_index];
        ROB->ROB_finish_index=1;
        ROB->buffer=stage->buffer;
        ROB->rs1_value=stage->rs1_value;
    }
    
    /* ADD  */
    if (strcmp(stage->opcode, "ADD") == 0) {
        //        printf("%d  4   FINISHED %d\n",MUL_INDEX,cpu->URF_finished[4]);
        stage->buffer = stage->rs1_value + stage->rs2_value;
        
        cpu->URF[stage->URF_index]=stage->buffer;
        cpu->URF_finished[stage->URF_index]=1;
        
        //        printf("%d  4   FINISHED %d\n",MUL_INDEX,cpu->URF_finished[4]);
        
        //z_flag
        if (stage->buffer == 0) {
            cpu->z_value[stage->number_instuction_index] = 1;
        } else{
            cpu->z_value[stage->number_instuction_index] = 0;
        }
        
        //            /* Copy data from Execute latch to Memory latch*/
        //            cpu->stage[MEM] = cpu->stage[EX];
        
        if (stage->URF_rs1_index>-1) {
            cpu->URF_check_vaild[stage->URF_rs1_index]--;
        }
        
        if (stage->URF_rs2_index>-1) {
            cpu->URF_check_vaild[stage->URF_rs2_index]--;
        }
        
        CPU_Stage* ROB=&cpu->ROB[stage->ROB_index];
        ROB->ROB_finish_index=1;
        ROB->mem_address=stage->mem_address;
        ROB->buffer=stage->buffer;
        ROB->rs1_value=stage->rs1_value;
    }
    
    /* ADDL  */
    if (strcmp(stage->opcode, "ADDL") == 0) {
        //        printf("%d  4   FINISHED %d\n",MUL_INDEX,cpu->URF_finished[4]);
        stage->buffer = stage->rs1_value + stage->imm;
        
        cpu->URF[stage->URF_index]=stage->buffer;
        cpu->URF_finished[stage->URF_index]=1;
        
        //        printf("%d  4   FINISHED %d\n",MUL_INDEX,cpu->URF_finished[4]);
        
        //z_flag
        if (stage->buffer == 0) {
            cpu->z_value[stage->number_instuction_index] = 1;
        } else{
            cpu->z_value[stage->number_instuction_index] = 0;
        }
        
        //            /* Copy data from Execute latch to Memory latch*/
        //            cpu->stage[MEM] = cpu->stage[EX];
        
        if (stage->URF_rs1_index>-1) {
            cpu->URF_check_vaild[stage->URF_rs1_index]--;
        }
        
        if (stage->URF_rs2_index>-1) {
            cpu->URF_check_vaild[stage->URF_rs2_index]--;
        }
        
        CPU_Stage* ROB=&cpu->ROB[stage->ROB_index];
        ROB->ROB_finish_index=1;
        ROB->mem_address=stage->mem_address;
        ROB->buffer=stage->buffer;
        ROB->rs1_value=stage->rs1_value;
    }
    
    /* SUB  */
    if (strcmp(stage->opcode, "SUB") == 0) {
        stage->buffer = stage->rs1_value - stage->rs2_value;
        
        cpu->URF[stage->URF_index]=stage->buffer;
        cpu->URF_finished[stage->URF_index]=1;
        
        //z_flag
        if (stage->buffer == 0) {
            cpu->z_value[stage->number_instuction_index] = 1;
        } else{
            cpu->z_value[stage->number_instuction_index] = 0;
        }
        
        //            /* Copy data from Execute latch to Memory latch*/
        //            cpu->stage[MEM] = cpu->stage[EX];
        
        if (stage->URF_rs1_index>-1) {
            cpu->URF_check_vaild[stage->URF_rs1_index]--;
        }
        
        if (stage->URF_rs2_index>-1) {
            cpu->URF_check_vaild[stage->URF_rs2_index]--;
        }
        
        CPU_Stage* ROB=&cpu->ROB[stage->ROB_index];
        ROB->ROB_finish_index=1;
        ROB->mem_address=stage->mem_address;
        ROB->buffer=stage->buffer;
        ROB->rs1_value=stage->rs1_value;
    }
    
    /* SUBL  */
    if (strcmp(stage->opcode, "SUBL") == 0) {
        stage->buffer = stage->rs1_value - stage->imm;
        
        cpu->URF[stage->URF_index]=stage->buffer;
        cpu->URF_finished[stage->URF_index]=1;
        
        //z_flag
        if (stage->buffer == 0) {
            cpu->z_value[stage->number_instuction_index] = 1;
        } else{
            cpu->z_value[stage->number_instuction_index] = 0;
        }
        
        //            /* Copy data from Execute latch to Memory latch*/
        //            cpu->stage[MEM] = cpu->stage[EX];
        
        if (stage->URF_rs1_index>-1) {
            cpu->URF_check_vaild[stage->URF_rs1_index]--;
        }
        
        if (stage->URF_rs2_index>-1) {
            cpu->URF_check_vaild[stage->URF_rs2_index]--;
        }
        
        CPU_Stage* ROB=&cpu->ROB[stage->ROB_index];
        ROB->ROB_finish_index=1;
        ROB->mem_address=stage->mem_address;
        ROB->buffer=stage->buffer;
        ROB->rs1_value=stage->rs1_value;
    }
    
    /* AND  */
    if (strcmp(stage->opcode, "AND") == 0) {
        stage->buffer = stage->rs1_value & stage->rs2_value;
        
        cpu->URF[stage->URF_index]=stage->buffer;
        cpu->URF_finished[stage->URF_index]=1;
        
        //            /* Copy data from Execute latch to Memory latch*/
        //            cpu->stage[MEM] = cpu->stage[EX];
        
        if (stage->URF_rs1_index>-1) {
            cpu->URF_check_vaild[stage->URF_rs1_index]--;
        }
        
        if (stage->URF_rs2_index>-1) {
            cpu->URF_check_vaild[stage->URF_rs2_index]--;
        }
        
        CPU_Stage* ROB=&cpu->ROB[stage->ROB_index];
        ROB->ROB_finish_index=1;
        ROB->mem_address=stage->mem_address;
        ROB->buffer=stage->buffer;
        ROB->rs1_value=stage->rs1_value;
    }
    
    /* OR  */
    if (strcmp(stage->opcode, "OR") == 0) {
        stage->buffer = stage->rs1_value | stage->rs2_value;
        
        cpu->URF[stage->URF_index]=stage->buffer;
        cpu->URF_finished[stage->URF_index]=1;
        
        //            /* Copy data from Execute latch to Memory latch*/
        //            cpu->stage[MEM] = cpu->stage[EX];
        
        if (stage->URF_rs1_index>-1) {
            cpu->URF_check_vaild[stage->URF_rs1_index]--;
        }
        
        if (stage->URF_rs2_index>-1) {
            cpu->URF_check_vaild[stage->URF_rs2_index]--;
        }
        
        CPU_Stage* ROB=&cpu->ROB[stage->ROB_index];
        ROB->ROB_finish_index=1;
        ROB->mem_address=stage->mem_address;
        ROB->buffer=stage->buffer;
        ROB->rs1_value=stage->rs1_value;
    }
    
    /* EX-OR  */
    if (strcmp(stage->opcode, "EX-OR") == 0) {
        stage->buffer = stage->rs1_value ^ stage->rs2_value;
        
        cpu->URF[stage->URF_index]=stage->buffer;
        cpu->URF_finished[stage->URF_index]=1;
        
        //            /* Copy data from Execute latch to Memory latch*/
        //            cpu->stage[MEM] = cpu->stage[EX];
        
        if (stage->URF_rs1_index>-1) {
            cpu->URF_check_vaild[stage->URF_rs1_index]--;
        }
        
        if (stage->URF_rs2_index>-1) {
            cpu->URF_check_vaild[stage->URF_rs2_index]--;
        }
        
        CPU_Stage* ROB=&cpu->ROB[stage->ROB_index];
        ROB->ROB_finish_index=1;
        ROB->mem_address=stage->mem_address;
        ROB->buffer=stage->buffer;
        ROB->rs1_value=stage->rs1_value;
    }
    
    /* No Register file read needed for HALT */
    if (strcmp(stage->opcode, "HALT") == 0) {
        //no instruction needed for nop
        
        CPU_Stage* ROB=&cpu->ROB[stage->ROB_index];
        ROB->ROB_finish_index=1;
        /* Copy data from Execute latch to Memory latch*/
    }
    
    
    /* BZ  */
    if (strcmp(stage->opcode, "BZ") == 0) {
        stage->buffer = stage->pc + stage->imm;
        
        /* Copy data from Execute latch to Memory latch*/
        
        if (stage->URF_rs1_index>-1) {
            cpu->URF_check_vaild[stage->URF_rs1_index]--;
        }
        
        int temp_index=0;
        for (int a=stage->number_instuction_index; a>0; a--) {
            if (cpu->z_value_valid[a]==1) {
                temp_index=a;
                break;
            }
        }
        
//        printf("%d temp index , z_valid %d,   z_Vlaue %d\n",temp_index,cpu->z_value_valid[temp_index],cpu->z_value[temp_index]);
        
        if (cpu->z_value[temp_index]==1) {
            
            BZ_index=1;
            BZ_instruction_index=stage->number_instuction_index;
            PC_Jump_value[0]=stage->buffer;
        }
        
        
        CPU_Stage* ROB=&cpu->ROB[stage->ROB_index];
        ROB->ROB_finish_index=1;
        ROB->mem_address=stage->mem_address;
        ROB->buffer=stage->buffer;
        ROB->rs1_value=stage->rs1_value;
    }
    
    
    /* BNZ  */
    if (strcmp(stage->opcode, "BNZ") == 0) {
        stage->buffer = stage->pc + stage->imm;
        
        /* Copy data from Execute latch to Memory latch*/
        
        if (stage->URF_rs1_index>-1) {
            cpu->URF_check_vaild[stage->URF_rs1_index]--;
        }
        
        int temp_index=0;
        for (int a=stage->number_instuction_index; a>0; a--) {
            if (cpu->z_value_valid[a]==1) {
                temp_index=a;
                break;
            }
        }
        
//        printf("%d temp index , z_valid %d,   z_Vlaue %d\n",temp_index,cpu->z_value_valid[temp_index],cpu->z_value[temp_index]);
        
        if (cpu->z_value[temp_index]==0) {
           
            BNZ_index=1;
            BNZ_instruction_index=stage->number_instuction_index;
            PC_Jump_value[0]=stage->buffer;
//            printf("paaaaaaa\n");
        }
        
        CPU_Stage* ROB=&cpu->ROB[stage->ROB_index];
        ROB->ROB_finish_index=1;
        ROB->mem_address=stage->mem_address;
        ROB->buffer=stage->buffer;
        ROB->rs1_value=stage->rs1_value;
    }
    
    
    /* JUMP  */
    if (strcmp(stage->opcode, "JUMP") == 0) {
        stage->buffer = stage->rs1_value + stage->imm;
        
//        printf("%d PC value aaaaaaa\n",stage->buffer);
        /* Copy data from Execute latch to Memory latch*/
        
        if (stage->URF_rs1_index>-1) {
            cpu->URF_check_vaild[stage->URF_rs1_index]--;
        }
        
        Jump_index=1;
        Jump_instruction_index=stage->number_instuction_index;
        PC_Jump_value[0]=stage->buffer;
        
//        printf("%d PC_Jump_value\n",PC_Jump_value[0]);
        
        
        CPU_Stage* ROB=&cpu->ROB[stage->ROB_index];
        ROB->ROB_finish_index=1;
        ROB->mem_address=stage->mem_address;
        ROB->buffer=stage->buffer;
        ROB->rs1_value=stage->rs1_value;
    }
    
    
    /* JAL  */
    if (strcmp(stage->opcode, "JAL") == 0) {
        stage->buffer = stage->rs1_value + stage->imm;
        cpu->URF_finished[stage->URF_index]=1;
        
        
        /* Copy data from Execute latch to Memory latch*/
        
        if (stage->URF_rs1_index>-1) {
            cpu->URF_check_vaild[stage->URF_rs1_index]--;
        }
       
        
        cpu->URF_valid[stage->pre_URF_index]=1;
        cpu->R_RAT[stage->rd]=stage->URF_index;
        cpu->R_RAT_valid[stage->rd]=1;
        
        
        Jal_index=1;
        Jal_instruction_index=stage->number_instuction_index;
        PC_Jump_value[0]=stage->buffer;
        
        
        
        CPU_Stage* ROB=&cpu->ROB[stage->ROB_index];
        ROB->ROB_finish_index=1;
        ROB->mem_address=stage->mem_address;
        ROB->buffer=stage->buffer;
        ROB->rs1_value=stage->rs1_value;
    }
    
    /* No Register file read needed for NOP */
    if (strcmp(stage->opcode, "NOP") == 0) {
        //no instruction needed for nop
        
        
        stage->URF_rs1_index=-1;
        stage->URF_rs2_index=-2;
        
        /* Copy data from Execute latch to Memory latch*/
        cpu->stage[MEM] = cpu->stage[EX];
    }
    
    
    if (Total_type==2) {
        print_stage_content("03. Instruction at EX_INT_FU__STAGE --->", stage);
    }
    
    
    strcpy(stage->opcode, "NOP");
    
    return 0;
}


/*
 *  Execute_Mul Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 *                  implementation
 */
int
execute_MUL(APEX_CPU* cpu)
{
    CPU_Stage* stage = &cpu->EX_MUL[0];
    
    /* MUL  */
    if (strcmp(stage->opcode, "MUL") == 0) {
        
        if (MUL_FINISHED == 2) {
            //              STOP_INDEX=0;
//                        printf("%d %d\n",stage->rs1_value,stage->rs2_value);
            stage->buffer = stage->rs1_value * stage->rs2_value;
            MUL_INDEX = 0;
            
            
            cpu->URF[stage->URF_index]=stage->buffer;
            cpu->URF_finished[stage->URF_index]=1;
            
            //            printf("%d rat%d urf%d\n",stage->rd,stage->URF_index,cpu->URF[stage->URF_index]);
            
            //z_flag
            if (stage->buffer == 0) {
                cpu->z_value[stage->number_instuction_index] = 1;
            } else{
                cpu->z_value[stage->number_instuction_index] = 0;
            }
            
            /* Copy data from Execute latch to Memory latch*/
            //            cpu->stage[MEM] = cpu->stage[EX];
            
            if (stage->URF_rs1_index>-1) {
                cpu->URF_check_vaild[stage->URF_rs1_index]--;
            }
            
            if (stage->URF_rs2_index>-1) {
                cpu->URF_check_vaild[stage->URF_rs2_index]--;
            }
            
            CPU_Stage* ROB=&cpu->ROB[stage->ROB_index];
            ROB->ROB_finish_index=1;
            ROB->mem_address=stage->mem_address;
            ROB->buffer=stage->buffer;
            ROB->rs1_value=stage->rs1_value;
            
            
            MUL_FINISHED=1;
        }else{
            MUL_FINISHED++;
        }
        
        //        printf("%d  6   FINISHED %d\n",MUL_INDEX,cpu->URF_finished[6]);
        
    }
    
    
    /* No Register file read needed for NOP */
    if (strcmp(stage->opcode, "NOP") == 0) {
        //no instruction needed for nop
        
        
        stage->URF_rs1_index=-1;
        stage->URF_rs2_index=-2;
        
        /* Copy data from Execute latch to Memory latch*/
        cpu->stage[MEM] = cpu->stage[EX];
    }
    
    
    if (Total_type==2) {
        print_stage_content("04. Instruction at EX_MUL_FU_STAGE --->", stage);
    }
    
    
    return 0;
}


/*
 *  ROB_checking Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 *                  implementation
 */
int
ROB_checking(APEX_CPU* cpu)
{
    int first_index=1000;
    int second_index=1000;
    int first_ROB_index=-1;
    int second_ROB_index=-1;
    int first_checking=0;
    int second_checking=0;
    
    
//    printf("%d 0 valid \n",cpu->ROB_valid[0]);
    for (int a=0; a<32; a++) {
        if (cpu->ROB_valid[a]==1) {
//            printf("%d  %s %d,  number %d\n",a,cpu->ROB[a].opcode, cpu->ROB[a].rd,cpu->ROB[a].number_instuction_index);
        }
    }
    
    
    
    for (int a=0; a<32; a++) {
        if (cpu->ROB_valid[a]==1) {
            if (cpu->ROB[a].number_instuction_index<first_index) {
                
                first_index=cpu->ROB[a].number_instuction_index;
                first_ROB_index=a;
                
//                printf("%d number: %d  index:%d  ROB ready: %d  rd %d\n",a,first_index,first_ROB_index,cpu->ROB[a].ROB_finish_index, cpu->ROB[a].rd);
            }
        }
    }
    
    if (first_index<300) {
        
        if (cpu->ROB[first_ROB_index].ROB_finish_index==1) {
            first_checking=1;
        }else{
            first_checking=0;
        }
    }
//    printf("The name of the instruction %s\n",cpu->ROB[first_ROB_index].opcode);
    
//    printf("%d firstchecking\n",first_checking);
    
    if (first_checking==1) {
        if (strcmp(cpu->ROB[first_ROB_index].opcode, "HALT")==0) {
            //        printf("adsfasdfasdfa\n");
            if (strcmp(cpu->EX_MUL[0].opcode, "MUL")==0) {
                
                first_checking=0;
                //            printf("pass\n");
            }else{
                if (strcmp(cpu->EX_MEM[0].opcode, "STORE")==0 || strcmp(cpu->EX_MEM[0].opcode, "LOAD")==0) {
                    //
                    
//                    printf("%d firstchecking\n",first_checking);
                    if (MEM_FINISHED==3||MEM_FINISHED==2) {
                        
                    }else{
                        first_checking=0;
                    }
                }else{
                    
                }
                
                //            printf("pass123\n");
            }
        }
    }
    
//    printf("%d firstchecking\n",first_checking);
    
    
    if (first_checking==1) {
        if (strcmp(cpu->ROB[first_ROB_index].opcode, "STORE")==0 ) {
            if (strcmp(cpu->EX_MEM[0].opcode, "STORE")==0 ||strcmp(cpu->EX_MEM[0].opcode, "LOAD")==0) {
                
                first_checking=0;
            }else{
                cpu->EX_MEM[0]=cpu->LSQ[cpu->ROB[first_ROB_index].LSQ_index];
                cpu->LSQ_valid[cpu->ROB[first_ROB_index].LSQ_index]=0;
                
            }
            
        }
    }
    
    if (first_checking==1) {
        if (strcmp(cpu->ROB[first_ROB_index].opcode, "LOAD")==0) {
            
            if ( strcmp(cpu->EX_MEM[0].opcode, "STORE")==0 || strcmp(cpu->EX_MEM[0].opcode, "LOAD")==0) {
                first_checking=0;
            }else{
                if (Load_FuncInt_index==1) {
                    Load_FuncInt_index=0;
                }else{
                    cpu->EX_MEM[0]=cpu->LSQ[cpu->ROB[first_ROB_index].LSQ_index];
                    cpu->LSQ_valid[cpu->ROB[first_ROB_index].LSQ_index]=0;
                    first_checking=0;
                    
                }
            }
        }
    }

    
//    printf("%d firstchecking\n",first_checking);
    
    if (first_checking==1) {
        
        
//        printf("%d firstchecking\n",first_checking);
        
        CPU_Stage* ROB = &cpu->ROB[first_ROB_index];
        CPU_Stage* stage = &cpu->Write_ROB[0];
        
        int code_size=4000+4*(cpu->code_memory_size-1);
        
//        printf("ROB_PC %d   Codememory_size %d \n",ROB->pc,code_size);
        
        if (ROB->pc == code_size) {
            Terminal_index=1;
        }
        
        //printf("Terminal_index %d pc_value %d  ROB_pc_value %d code_size %d\n",Terminal_index,cpu->pc,ROB->pc,code_size);
        
        stage->pc=ROB->pc;
        stage->rs1=ROB->rs1;
        stage->rs2=ROB->rs2;
        stage->imm=ROB->imm;
        
        stage->buffer=ROB->buffer;
        stage->rs1_value=ROB->rs1_value;
        stage->mem_address=ROB->mem_address;
        
        stage->pre_URF_index=ROB->pre_URF_index;
        
        cpu->Write_ROB[0]=cpu->ROB[first_ROB_index];
        
//        printf("%d firstchecking\n",first_checking);
        write_ROB(cpu);
        
//        printf("%d firstchecking\n",first_checking);
        
        ROB->ROB_finish_index=0;
        cpu->ROB_valid[first_ROB_index]=0;
        
//        printf("%d number: %d   ROB ready: %d  valid: %d\n",first_ROB_index,first_index,cpu->ROB[first_ROB_index].ROB_finish_index, cpu->ROB_valid[first_ROB_index]);
        
        
        
        
//        int mem_instruction_indx=-1;
//        int mem_temp_index=1000;
//
//        for (int a=0; a<20; a++) {
//            if (cpu->LSQ_valid[a]==1) {
//                if (cpu->LSQ[a].number_instuction_index<mem_temp_index) {
//                    mem_temp_index=cpu->LSQ[a].number_instuction_index;
//                    mem_instruction_indx=a;
//                }
//            }
//        }
//
//
//        if (MEM_FINISHED==1) {
//            if (mem_instruction_indx>-1 && mem_instruction_indx<20) {
//                if (cpu->LSQ[mem_instruction_indx].LSQ_finish_index==1) {
//                    cpu->EX_MEM[0]=cpu->LSQ[mem_instruction_indx];
//                    cpu->LSQ_valid[mem_instruction_indx]=0;
//
//
//                    print_mem_information=1;
//                    cpu->Write_ROB[0]=cpu->LSQ[mem_instruction_indx];
//                    cpu->LSQ_valid[mem_instruction_indx]=0;
//                }
//            }
//            //        else{
//            //            CPU_Stage* fu_mem = &cpu->EX_MEM[0];
//            //            strcpy(fu_mem->opcode, "NOP");
//            //        }
//        }else{
//
//        }
        
        
        
//        printf("%d 0 valid \n",cpu->ROB_valid[0]);
        
        
        for (int a=0; a<32; a++) {
            if (cpu->ROB_valid[a]==1) {
                if (cpu->ROB[a].number_instuction_index<second_index) {
                    
                    second_index=cpu->ROB[a].number_instuction_index;
                    second_ROB_index=a;
                    
//                    printf("Second %d number: %d  index:%d  ROB ready: %d  %s rd %d\n",a,second_index,second_ROB_index,cpu->ROB[a].ROB_finish_index,cpu->ROB[a].opcode, cpu->ROB[a].rd);
                }
            }
        }
        
        if (second_index<300) {
            if (cpu->ROB[second_ROB_index].ROB_finish_index==1) {
                second_checking=1;
            }else{
                second_checking=0;
            }
        }
        
//        printf("%d second_checking\n",second_checking);
        
        if (second_checking==1) {
            if (strcmp(cpu->ROB[second_ROB_index].opcode, "HALT")==0) {
                    //    printf("adsfasdfasdfa\n");
                if (strcmp(cpu->EX_MUL[0].opcode, "MUL")==0) {
                    
                    second_checking=0;
                    //            printf("pass\n");
                }else{
                    if (strcmp(cpu->EX_MEM[0].opcode, "STORE")==0 || strcmp(cpu->EX_MEM[0].opcode, "LOAD")==0) {
                        //
                        
//                        printf("%d second\n",first_checking);
                        if (MEM_FINISHED==3||MEM_FINISHED==2) {
                            
                        }else{
                            second_checking=0;
                        }
                    }else{
                        
                    }
                    
                    //            printf("pass123\n");
                }
            }
        }
        
        if (second_checking==1) {
            if (strcmp(cpu->ROB[second_ROB_index].opcode, "STORE")==0 ) {
                if (strcmp(cpu->EX_MEM[0].opcode, "STORE")==0 ||strcmp(cpu->EX_MEM[0].opcode, "LOAD")==0) {
                    
                    second_checking=0;
                }else{
                    cpu->EX_MEM[0]=cpu->LSQ[cpu->ROB[second_ROB_index].LSQ_index];
                    cpu->LSQ_valid[cpu->ROB[second_ROB_index].LSQ_index]=0;
                    
                }
                
            }
        }
        
        if (second_checking==1) {
            if (strcmp(cpu->ROB[second_ROB_index].opcode, "LOAD")==0) {
                
                if ( strcmp(cpu->EX_MEM[0].opcode, "STORE")==0 || strcmp(cpu->EX_MEM[0].opcode, "LOAD")==0) {
                    second_checking=0;
                }else{
                    if (Load_FuncInt_index==1) {
                        Load_FuncInt_index=0;
                    }else{
                        cpu->EX_MEM[0]=cpu->LSQ[cpu->ROB[second_ROB_index].LSQ_index];
                        cpu->LSQ_valid[cpu->ROB[second_ROB_index].LSQ_index]=0;
                        second_checking=0;
                        
                    }
                }
            }
        }
        
        
        
        
        
        
        if (second_checking==1) {
            
        
            CPU_Stage* ROB2 = &cpu->ROB[second_ROB_index];
            CPU_Stage* stage=&cpu->Write_ROB[0];
            
            if (ROB2->pc == code_size) {
                Terminal_index=1;
            }
            
         //   printf("Terminal_index %d pc_value %d  ROB_pc_value %d code_size %d\n",Terminal_index,cpu->pc,ROB2->pc,code_size);
            
            stage->pc=ROB2->pc;
            stage->rs1=ROB2->rs1;
            stage->rs2=ROB2->rs2;
            stage->imm=ROB2->imm;
            
            stage->buffer=ROB2->buffer;
            stage->rs1_value=ROB2->rs1_value;
            stage->mem_address=ROB2->mem_address;
            
            stage->pre_URF_index=ROB->pre_URF_index;
            
            //            cpu->Write_ROB[0]=cpu->ROB[second_ROB_index];
            cpu->Write_ROB[0]=cpu->ROB[second_ROB_index];
            
//            printf("%d firstchecking\n",second_checking);
            write_ROB(cpu);
            
//            printf("%d firstchecking\n",second_checking);
            
            
            ROB2->ROB_finish_index=0;
            cpu->ROB_valid[second_ROB_index]=0;
        }
        
    }else{
        if (print_mem_information==1) {
            //stall
            write_ROB(cpu);
        }else{
            CPU_Stage* stage=&cpu->Write_ROB[0];
            strcpy(stage->opcode, "NOP");
            
            write_ROB(cpu);
            print_mem_information=0;
        }
        
        
    }
    
    
    
    return 0;
}

/*
 *  write_ROB Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 *                  implementation
 */
int
write_ROB(APEX_CPU* cpu)
{
    CPU_Stage* stage = &cpu->Write_ROB[0];
    if (!stage->busy && !stage->stalled) {
        
        /* Update register file */
        if (strcmp(stage->opcode, "MOVC") == 0) {
            cpu->regs_valid[stage->rd] = 1;
            cpu->regs[stage->rd] = stage->buffer;
            
            //        cpu->URF_valid[stage->URF_index]=1;
            cpu->URF[stage->URF_index]=stage->buffer;
            cpu->URF_check_vaild[stage->URF_index]--;
            cpu->ins_completed++;
            
            cpu->URF_valid[stage->pre_URF_index]=1;
            cpu->R_RAT[stage->rd]=stage->URF_index;
            cpu->R_RAT_valid[stage->rd]=1;
        }
        /* Store */
        if (strcmp(stage->opcode, "STORE") == 0) {
            cpu->ins_completed++;
            cpu->ROB_valid[stage->ROB_index]=0;
        }
        
        /* LOAD */
        if (strcmp(stage->opcode, "LOAD") == 0) {
            //            cpu->regs_valid[stage->rd] = 1;
            //            cpu->regs[stage->rd] = stage->rs2_value;
            //
            //            //        cpu->URF_valid[stage->URF_index]=1;
            //            cpu->URF[stage->URF_index]=stage->buffer;
            //            cpu->URF_check_vaild[stage->URF_index]--;
            //
            //            cpu->ins_completed++;
            //
            //            cpu->URF_valid[stage->pre_URF_index]=1;
            //            cpu->R_RAT[stage->rd]=stage->URF_index;
            //            cpu->R_RAT_valid[stage->rd]=1;
            cpu->ROB_valid[stage->ROB_index]=0;
        }
        
        /* ADD */
        if (strcmp(stage->opcode, "ADD") == 0) {
            cpu->regs_valid[stage->rd] = 1;
            cpu->regs[stage->rd] = stage->buffer;
            
            cpu->URF[stage->URF_index]=stage->buffer;
            //          cpu->URF_valid[stage->URF_index]=1;
            cpu->URF_check_vaild[stage->URF_index]--;
            
            cpu->ins_completed++;
            
            cpu->URF_valid[stage->pre_URF_index]=1;
            cpu->R_RAT[stage->rd]=stage->URF_index;
            cpu->R_RAT_valid[stage->rd]=1;
            
            if (stage->buffer==0) {
                cpu->z_flag[0]=1;
            }else{
                cpu->z_flag[0]=0;
            }
            
//            printf("%s  result  %d    z_flag  %d\n",stage->opcode,stage->buffer,cpu->z_flag[0]);
        }
        
        /* ADDL */
        if (strcmp(stage->opcode, "ADDL") == 0) {
            cpu->regs_valid[stage->rd] = 1;
            cpu->regs[stage->rd] = stage->buffer;
            
            cpu->URF[stage->URF_index]=stage->buffer;
            //          cpu->URF_valid[stage->URF_index]=1;
            cpu->URF_check_vaild[stage->URF_index]--;
            
            cpu->ins_completed++;
            
            cpu->URF_valid[stage->pre_URF_index]=1;
            cpu->R_RAT[stage->rd]=stage->URF_index;
            cpu->R_RAT_valid[stage->rd]=1;
            
            if (stage->buffer==0) {
                cpu->z_flag[0]=1;
            }else{
                cpu->z_flag[0]=0;
            }
            
//            printf("%s  result  %d    z_flag  %d\n",stage->opcode,stage->buffer,cpu->z_flag[0]);
        }
        
        /* SUB */
        if (strcmp(stage->opcode, "SUB") == 0) {
            cpu->regs_valid[stage->rd] = 1;
            cpu->regs[stage->rd] = stage->buffer;
            
            cpu->URF[stage->URF_index]=stage->buffer;
            //          cpu->URF_valid[stage->URF_index]=1;
            cpu->URF_check_vaild[stage->URF_index]--;
            
            cpu->ins_completed++;
            
            cpu->URF_valid[stage->pre_URF_index]=1;
            cpu->R_RAT[stage->rd]=stage->URF_index;
            cpu->R_RAT_valid[stage->rd]=1;
            
            if (stage->buffer==0) {
                cpu->z_flag[0]=1;
            }else{
                cpu->z_flag[0]=0;
            }
            
//            printf("%s  result  %d    z_flag  %d\n",stage->opcode,stage->buffer,cpu->z_flag[0]);
        }
        
        /* SUBL */
        if (strcmp(stage->opcode, "SUBL") == 0) {
            cpu->regs_valid[stage->rd] = 1;
            cpu->regs[stage->rd] = stage->buffer;
            
            cpu->URF[stage->URF_index]=stage->buffer;
            //          cpu->URF_valid[stage->URF_index]=1;
            cpu->URF_check_vaild[stage->URF_index]--;
            
            cpu->ins_completed++;
            
            cpu->URF_valid[stage->pre_URF_index]=1;
            cpu->R_RAT[stage->rd]=stage->URF_index;
            cpu->R_RAT_valid[stage->rd]=1;
            
            if (stage->buffer==0) {
                cpu->z_flag[0]=1;
            }else{
                cpu->z_flag[0]=0;
            }
            
//            printf("%s  result  %d    z_flag  %d\n",stage->opcode,stage->buffer,cpu->z_flag[0]);
        }
        
        /* AND */
        if (strcmp(stage->opcode, "AND") == 0) {
            cpu->regs_valid[stage->rd] = 1;
            cpu->regs[stage->rd] = stage->buffer;
            
            cpu->URF[stage->URF_index]=stage->buffer;
            //          cpu->URF_valid[stage->URF_index]=1;
            cpu->URF_check_vaild[stage->URF_index]--;
            
            cpu->ins_completed++;
            
            cpu->URF_valid[stage->pre_URF_index]=1;
            cpu->R_RAT[stage->rd]=stage->URF_index;
            cpu->R_RAT_valid[stage->rd]=1;
        }
        
        /* OR */
        if (strcmp(stage->opcode, "OR") == 0) {
            cpu->regs_valid[stage->rd] = 1;
            cpu->regs[stage->rd] = stage->buffer;
            
            cpu->URF[stage->URF_index]=stage->buffer;
            //          cpu->URF_valid[stage->URF_index]=1;
            cpu->URF_check_vaild[stage->URF_index]--;
            
            cpu->ins_completed++;
            
            cpu->URF_valid[stage->pre_URF_index]=1;
            cpu->R_RAT[stage->rd]=stage->URF_index;
            cpu->R_RAT_valid[stage->rd]=1;
        }
        
        /* EX-OR */
        if (strcmp(stage->opcode, "EX-OR") == 0) {
            cpu->regs_valid[stage->rd] = 1;
            cpu->regs[stage->rd] = stage->buffer;
            
            cpu->URF[stage->URF_index]=stage->buffer;
            //          cpu->URF_valid[stage->URF_index]=1;
            cpu->URF_check_vaild[stage->URF_index]--;
            
            cpu->ins_completed++;
            
            cpu->URF_valid[stage->pre_URF_index]=1;
            cpu->R_RAT[stage->rd]=stage->URF_index;
            cpu->R_RAT_valid[stage->rd]=1;
        }
        
        /* MUL */
        if (strcmp(stage->opcode, "MUL") == 0) {
            cpu->regs_valid[stage->rd] = 1;
            cpu->regs[stage->rd] = stage->buffer;
            
            cpu->URF[stage->URF_index]=stage->buffer;
            //          cpu->URF_valid[stage->URF_index]=1;
            cpu->URF_check_vaild[stage->URF_index]--;
            
            cpu->ins_completed++;
            
            cpu->URF_valid[stage->pre_URF_index]=1;
            cpu->R_RAT[stage->rd]=stage->URF_index;
            cpu->R_RAT_valid[stage->rd]=1;
            
            if (stage->buffer==0) {
                cpu->z_flag[0]=1;
            }else{
                cpu->z_flag[0]=0;
            }
            
//            printf("%s  result  %d    z_flag  %d\n",stage->opcode,stage->buffer,cpu->z_flag[0]);
        }
        
        /* BZ */
        if (strcmp(stage->opcode, "BZ") == 0) {
            cpu->ins_completed++;
            
//            printf("BZ z_flag %d \n",cpu->z_flag[0] );
            
            if (cpu->z_flag[0]==1) {
                clear_index=1;
                
                cpu->pc=stage->buffer;
                
                
                for (int a=0; a<16; a++) {
                    if (cpu->IQ_valid[a]==1) {
                        if (cpu->IQ[a].number_instuction_index>stage->number_instuction_index) {
                            cpu->IQ_valid[a]=0;
                        }
                    }
                }
                
                for (int a=0; a<20; a++) {
                    if (cpu->LSQ_valid[a]==1) {
                        if (cpu->LSQ[a].number_instuction_index>stage->number_instuction_index) {
                            cpu->LSQ_valid[a]=0;
                        }
                    }
                }
                
                for (int a=0; a<32; a++) {
                    if (cpu->ROB_valid[a]==1) {
                        if (cpu->ROB[a].number_instuction_index>stage->number_instuction_index) {
                            cpu->ROB_valid[a]=0;
                        }
                    }
                }
                
                for (int a=0; a<40; a++) {
                    cpu->URF_valid[a]=1;
                }
                
                for (int a=0; a<16; a++) {
                    cpu->RAT[a]=cpu->R_RAT[a];
                    cpu->URF_valid[cpu->RAT[a]]=0;
                    
                    
                }
                
                if (cpu->EX_INT[0].number_instuction_index>stage->number_instuction_index) {
                    strcpy(cpu->EX_INT[0].opcode, "NOP");
                }
                
                if (cpu->EX_MUL[0].number_instuction_index>stage->number_instuction_index) {
                    strcpy(cpu->EX_MUL[0].opcode, "NOP");
                }
                
                if (cpu->EX_MEM[0].number_instuction_index>stage->number_instuction_index) {
                    strcpy(cpu->EX_MEM[0].opcode, "NOP");
                }
            }
        }
        
        /* BNZ */
        if (strcmp(stage->opcode, "BNZ") == 0) {
            cpu->ins_completed++;
            
//            printf("BZ z_flag %d \n",cpu->z_flag[0] );
        }
        
        /* JUMP */
        if (strcmp(stage->opcode, "JUMP") == 0) {
            cpu->ins_completed++;
            
            
//                        printf("pass instruction \n");
            
            
            //          cpu->URF_valid[stage->URF_index]=1;
        }
        
        /* JAL */
        if (strcmp(stage->opcode, "JAL") == 0) {
            cpu->ins_completed++;
            //            printf("pass instruction \n");
            
//            cpu->URF_valid[stage->URF_index]=1;
           
            
            //          cpu->URF_valid[stage->URF_index]=1;
        }
        
        /* HALT */
        if (strcmp(stage->opcode, "HALT") == 0) {
            HALT_INDEX = 1;
            clear_index=0;
            
            
            for (int a=0; a<16; a++) {
                if (cpu->IQ_valid[a]==1) {
                    if (cpu->IQ[a].number_instuction_index>stage->number_instuction_index) {
                        cpu->IQ_valid[a]=0;
                    }
                }
            }
            
            for (int a=0; a<20; a++) {
                if (cpu->LSQ_valid[a]==1) {
                    if (cpu->LSQ[a].number_instuction_index>stage->number_instuction_index) {
                        cpu->LSQ_valid[a]=0;
                    }
                }
            }
            
            for (int a=0; a<32; a++) {
                if (cpu->ROB_valid[a]==1) {
                    if (cpu->ROB[a].number_instuction_index>stage->number_instuction_index) {
                        cpu->ROB_valid[a]=0;
                    }
                }
            }
            
            for (int a=0; a<16; a++) {
                cpu->RAT[a]=cpu->R_RAT[a];
            }
            
            if (cpu->EX_INT[0].number_instuction_index>stage->number_instuction_index) {
                strcpy(cpu->EX_INT[0].opcode, "NOP");
            }
            
            if (cpu->EX_MUL[0].number_instuction_index>stage->number_instuction_index) {
                strcpy(cpu->EX_MUL[0].opcode, "NOP");
            }
            
            if (cpu->EX_MEM[0].number_instuction_index>stage->number_instuction_index) {
                strcpy(cpu->EX_MEM[0].opcode, "NOP");
            }
        }
        
        /* NOP */
        if (strcmp(stage->opcode, "NOP") == 0) {
            
            stage->URF_rs1_index=-1;
            stage->URF_rs2_index=-2;
        }
        
        if (Total_type==2) {
//            print_stage_content_WB("Details of ROB Retired Instructions --", stage);
            print_stage_content("Details of ROB Retired Instructions --", stage);
            printf("--------------------------------\n");
        }
        
        strcpy(stage->opcode, "NOP");
        
    }
    return 0;
}


/*
 *  APEX CPU simulation loop
 *
 *  Note : You are free to edit this function according to your
 *                  implementation
 */
int
APEX_cpu_run(APEX_CPU* cpu)
{
    int i;
    int flag = 0;
    while (1) {
        
        /* All the instructions committed, so exit */
        if ( cpu->clock == Total_cycle ) {
            flag = 1;
            printf("(apex) >> Simulation Complete");
            //        printf("\n%d \n",cpu->code_memory_size);
            //        printf("%d \n",cpu->ins_completed);
            break;
        }
        
        
        
        if (Total_type==2) {
            
            printf("\n");
            printf("\n");
            printf("----------------------------------------------------------------------------\n");
            printf("Clock Cycle #: %d\n", (cpu->clock+1));
            printf("----------------------------------------------------------------------------\n");
        }
        
        
        ROB_checking(cpu);
        
        memory(cpu);
        execuse_MEM(cpu);
        execute_MUL(cpu);
        execute_Int(cpu);
        execute(cpu);
        LSQ_printing_stage(cpu);
        decode(cpu);
        fetch(cpu);
        cpu->clock++;
        
        if (Terminal_index==1) {
            if (strcmp(cpu->EX_MEM[0].opcode, "STORE")==0 || strcmp(cpu->EX_MEM[0].opcode, "LOAD")==0) {
                if (MEM_FINISHED==1) {
                    break;
                }
            }else{
                break;
                
            }
        }
        
        if (HALT_INDEX == 1) {
            if (strcmp(cpu->EX_MEM[0].opcode, "STORE")==0 || strcmp(cpu->EX_MEM[0].opcode, "LOAD")==0) {
                if (MEM_FINISHED==1) {
                    break;
                }
            }else{
                break;
                
            }
        }
    }
    
    if (Total_type != 0) {/*
        printf("\n=============== STATE OF ARCHITECTURAL REGISTER FILE ==========\n");
        
        for (int a=0; a<16; a++) {
            char str[10];
            if (cpu->regs_valid[a] == 1) {
                strcpy(str, "Valid");
            } else{
                strcpy(str, "Invalid");
            }
            
            if (a<10) {
                printf("|   REG[0%d]  |   Value = %4d  |   Status = %10s    |\n",a, cpu->regs[a], str);
            } else{
                printf("|   REG[%d]  |   Value = %4d  |   Status = %10s    |\n",a, cpu->regs[a], str);
            }
        }*/
        printf("\n=============== STATE OF URF REGISTER FILE ==========\n");
        
        for (int a=0; a<16; a++) {
            char str[10];
            if (cpu->URF_valid[a] == 0) {
                strcpy(str, "Valid");
                
                
                if (a<10) {
                    printf("|   REG[0%d]  |   Value = %4d  |   Status = %10s    |\n",a, cpu->URF[a], str);
                } else{
                    printf("|   REG[%d]  |   Value = %4d  |   Status = %10s    |\n",a, cpu->URF[a], str);
                }
            } else{
                //nothing
            }
        }
        
        printf("\n============== STATE OF DATA MEMORY =============\n");
        
//        for (int a=0; a<100; a++) {
//
//            if (a<10) {
//                printf("|   MEM[0%d]  |   Value = %4d  |\n",a, cpu->data_memory[a]);
//            } else{
//                printf("|   REG[%d]  |   Value = %4d  |\n",a, cpu->data_memory[a]);
//            }
//        }
/*
        for (int a=0; a<100; a++) {
            
            if (a<10) {
                printf("|   MEM[0%d]  |   Value = %4d  |\n",a, cpu->data_memory[a]);
            } else{
                printf("|   REG[%d]  |   Value = %4d  |\n",a, cpu->data_memory[a]);
            }
        }*/
        for (i = 0; i < 20; i++)
        {
            printf("memory[%2d] = %-9d memory[%2d] = %-9d memory[%2d] = %-9d memory[%2d] = %-9d memory[%2d] = %-9d\n", 5*i, cpu->data_memory[5*i], 5*i+1, cpu->data_memory[5*i+1], 5*i+2, 
            cpu->data_memory[5*i+2], 5*i+3, cpu->data_memory[5*i+3], 5*i+4, cpu->data_memory[5*i+4]);
        }
        
    }
    
//    printf("\n============== STATE OF Zflagevalid and z_flage_value =============\n");
//    for (int a=0; a<20; a++) {
//
//        printf("%d temp index , z_valid %d,   z_Vlaue %d\n",a,cpu->z_value_valid[a],cpu->z_value[a]);
//    }
    
    if (flag == 1)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}







