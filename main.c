/*
 *  main.c
 *
 *  Author :
 *  Gaurav Kothari (gkothar1@binghamton.edu)
 *  State University of New York, Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"

char *allocCat(const char* const s1){
    char* str = (char*)malloc(strlen(s1)+1);
    strcpy(str, s1);
    return str;
}

int
main(int argc, char const* argv[])
{
//    printf("%d\n",argc);
//    printf("%s\n",argv[2]);
//    printf("%s\n",argv[3]);
  int cyc;
  char comtype[16];
  if (argc != 4) {
    fprintf(stderr, "APEX_Help : Usage %s <input_file>\n", argv[0]);
    exit(1);
  }
    char type1[10];
    strcpy(type1,argv[2]);
//    printf("%s\n",type1);
    
    if (strcmp(type1, "simulate") == 0) {
        Total_type = 1;
    }else if (strcmp(type1, "display") == 0){
        Total_type = 2;
    }else{
        Total_type =0;
    }
//    printf("%d\n",Total_type);
    
    char cyle[5];
    strcpy(cyle,argv[3]);
//    printf("%s\n",cyle);
    
    Total_cycle=atoi(cyle);
    
//    printf("%d\n",Total_cycle);
    

  APEX_CPU* cpu = APEX_cpu_init(argv[1]);
  if (!cpu) {
    fprintf(stderr, "APEX_Error : Unable to initialize CPU\n");
    exit(1);
  }
  if (Total_type == 2)
  {
    APEX_cpu_run(cpu);
    APEX_cpu_stop(cpu);
  }
  else if (Total_type == 1)
  {
    while (APEX_cpu_run(cpu) == 1)
    {
      Total_type = 1;
      printf("Please input next command:\n");
      scanf("%s %d", comtype, &cyc);
      if (strcmp(comtype,"simulate") == 0)
      {  
        printf("current cpu->clock = %d  pc =%d\n", cpu->clock, cpu->pc);
        printf("%s the next %d cycles\n", comtype, cyc);
        Total_cycle = Total_cycle + cyc;
    //    APEX_cpu_stop(cpu);
    //    cpu = APEX_cpu_init(argv[1]);
      }
      else if (strcmp(comtype,"display") == 0)
      {
        printf("%s the content\n", comtype);
        APEX_cpu_stop(cpu);
        cpu = APEX_cpu_init(argv[1]);
        if (!cpu) {
        fprintf(stderr, "APEX_Error : Unable to initialize CPU\n");
        exit(1);
        }
        cpu->clock = 0;
        Total_type = 2;
      }
    } 
  }
  return 0;
}
