#ifndef _APEX_CPU_H_
#define _APEX_CPU_H_
/**
 *  cpu.h
 *  Contains various CPU and Pipeline Data structures
 *
 *  Author :
 *  Gaurav Kothari (gkothar1@binghamton.edu)
 *  State University of New York, Binghamton
 */

//int rename_index[16]=
//    {0,0,0,0,
//        0,0,0,0,
//        0,0,0,0,
//        0,0,0,0
//};

int Total_type;
int Total_cycle;


enum
{
  F,
  DRF,
  EX,
  MEM,
  WB,
  NUM_STAGES
};

/* Format of an APEX instruction  */
typedef struct APEX_Instruction
{
  char opcode[128];	// Operation Code
  int rd;		    // Destination Register Address
  int rs1;		    // Source-1 Register Address
  int rs2;		    // Source-2 Register Address
  int imm;		    // Literal Value
//    //rename table
//    int rename_rd;
//    int rename_rs1;
//    int rename_rs2;
} APEX_Instruction;

/* Model of CPU stage latch */
typedef struct CPU_Stage
{
  int pc;		    // Program Counter
  char opcode[128];	// Operation Code
  int rs1;		    // Source-1 Register Address
  int rs2;		    // Source-2 Register Address
  int rd;		    // Destination Register Address
  int imm;		    // Literal Value
  int rs1_value;	// Source-1 Register Value
  int rs2_value;	// Source-2 Register Value
  int buffer;		// Latch to hold some value
  int mem_address;	// Computed Memory Address
  int busy;		    // Flag to indicate, stage is performing some action
  int stalled;		// Flag to indicate, stage is stalled
    
    int URF_index;
    int URF_rs1_index,URF_rs2_index;
    int pre_URF_index;
    
    int IQ_index;
    int LSQ_index;
    int ROB_index;
    int IQ_finish_index;
    int LSQ_finish_index;
    int number_instuction_index;
    int ROB_finish_index;
    
    int z_type;
    int function_unin_type;
    
   
//    //rename table
//    int rename_rd;
//    int rename_rs1;
//    int rename_rs2;
//
} CPU_Stage;

/* Model of APEX CPU */
typedef struct APEX_CPU
{
  /* Clock cycles elasped */
  int clock;

  /* Current program counter */
  int pc;

  /* Integer register file */
  int regs[32];
  int regs_valid[32];
    
    int URF[40];
    int URF_valid[40];
    int URF_finished[40];
    int URF_check_vaild[40];
    
    int RAT[32];
    int R_RAT[32];
    int R_RAT_valid[32];
    
    int z_flag[1];
    
    int z_value[500];
    int z_value_valid[500];
//    int PC_Jump_value[0];

  /* Array of 5 CPU_stage */
  CPU_Stage stage[5];
    
    //Array of IQ, LSQ and ROB
    CPU_Stage IQ[16];
    CPU_Stage LSQ[20];
    CPU_Stage ROB[32];
    
    CPU_Stage EX_MUL[1];
    CPU_Stage EX_INT[1];
    CPU_Stage EX_MEM[1];
    CPU_Stage Write_ROB[1];

    
    int IQ_valid[16];
    int LSQ_valid[20];
    int ROB_valid[32];
    
    int instruction_number[0];
    int IQ_check_index[16];
    

  /* Code Memory where instructions are stored */
  APEX_Instruction* code_memory;
  int code_memory_size;

  /* Data Memory */
  int data_memory[4096];

  /* Some stats */
  int ins_completed;

} APEX_CPU;

APEX_Instruction*
create_code_memory(const char* filename, int* size);

APEX_CPU*
APEX_cpu_init(const char* filename);

int
APEX_cpu_run(APEX_CPU* cpu);

void
APEX_cpu_stop(APEX_CPU* cpu);

int
fetch(APEX_CPU* cpu);

int
decode(APEX_CPU* cpu);

int
execute(APEX_CPU* cpu);

int
memory(APEX_CPU* cpu);

int
writeback(APEX_CPU* cpu);

int write_ROB(APEX_CPU* cpu);

#endif
