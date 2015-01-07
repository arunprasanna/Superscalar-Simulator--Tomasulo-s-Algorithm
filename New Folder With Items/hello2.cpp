using namespace std;
#include "procsim.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <queue>

#define FALSE 	-1
#define TRUE  	1

#define UNINITIALIZED -2
#define READY         -3
#define DONE          -4
#define NOT_READY     -5

#define BUSY    0
#define NOT_BUSY    1

struct _reg
{
    int32_t tag;
    int32_t ready;
}reg;

typedef struct _common_data_bus
{
    int32_t line_number;
    int32_t tag;
    int32_t reg;
    int32_t FU;
    int32_t busy;
    int32_t cycle;
}common_data_bus;


typedef struct _timing
{
    int32_t fetch;
    int32_t dispatch;
    int32_t schedule;
    int32_t execute;
    int32_t state_update;
    int32_t cycle;
}timing;

typedef struct _function_unit
{
    int32_t cycle;
    int32_t valid;
    int32_t tag;
} function_unit;

typedef struct _sheduling_queue
{
    int32_t function_unit;
    int32_t cycle;
    int32_t dest_reg;
    int32_t dest_tag;
    int32_t src1_tag;
    int32_t src2_tag;
    int32_t src1_ready;
    int32_t src2_ready;
    int32_t busy;
    int32_t fire;
    
}scheduling_queue;



uint64_t r = 0;
uint64_t k0 = 0;
uint64_t k1 = 0;
uint64_t k2 = 0;
uint64_t f = 0;


_reg register_file[128];


uint64_t count0;
uint64_t count1;
uint64_t count2;
uint64_t cdb_size = 0;

uint32_t instruction_count;
uint32_t read_over  = 1;
uint32_t flag = 1;
uint32_t flag1 = TRUE;
uint32_t cycle = 1;
uint32_t instructions_fired=0;
uint32_t instructions_retired=0;
uint32_t max_dispatch_queue_size=0;
uint32_t schedule_available;

queue <proc_inst_t> fetch_queue;
queue <proc_inst_t> dispatch_queue;


void fetch_instructions()
{
    
    proc_inst_t p_inst;
    
    for (int i = 0; i<f && flag1==TRUE; i++)
    {
        flag1=read_instruction(&p_inst);
        
        if (flag1==TRUE)
        {
            instruction_count++;
            fetch_queue.push(p_inst);
        }
        else
        {
            read_over = 0;
        }
        
        
    }
}





void dispatch_instructions()
{
    
    
    proc_inst_t dispatch_node;
    int counta=4;
    
    
    while(fetch_queue.size() && counta!=0)
    {
        {
            
            dispatch_node=fetch_queue.front();
            dispatch_queue.push(dispatch_node);
            fetch_queue.pop();
            counta--;
        }
        
    }
    
    
}




void schedule_instructions()
{
    
    
    
    for(int i=0;i<(k0+k1+k2);i++)
    {
        proc_inst_t scheduling_node1 = dispatch_queue.front();
        
        if(schedule_queue[i].valid==NOT_BUSY)
        {
            
            schedule_queue[i].function_unit=scheduling_node1.op_code;
            if(scheduling_node1.src_reg[0]!=-1)
            {
                if(register_file[scheduling_node1.src_reg[0]].busy!=BUSY)
                {
                    schedule_queue[i].src1_tag=register_file[scheduling_node1.src_reg[0]];
                    schedule_queue[i].src1_ready=READY;
                }
                else
                {
                    schedule_queue[i].src1_tag=register_file[scheduling_node1.src_reg[0]];
                    schedule_queue[i].src1_ready=NOT_READY;
                }
                
            }
            else
            {
                schedule_queue[i].src1_ready=READY;
            }
            if(scheduling_node1.src_reg[0]!=-1)
            {
                f(register_file[schedule_queue[i].src2_reg[0]].busy!=BUSY)
                {
                    schedule_queue[i]src2_tag==register_file[schedule_queue[i].src_reg[1]];
                    schedule_queue[i].src2_ready=READY;
                }
                else
                {
                    schedule_queue[i]src2_tag==register_file[schedule_queue[i].src_reg[1]];
                    schedule_queue[i].src2_ready=NOT_READY;
                }
                
            }
            else
            {
                schedule_queue[i].src2_ready=READY;
            }
            
            schedule_queue[i].dest_tag=scheduling_node1.dest_tag;
            schedule_queue[i].busy=BUSY;
            
            
        }
        dispatch_queue.pop();
    }
    
}


void execute_instructions()
{
    for(int i=0; i<(2*(k0+k1+k2));i++)
    {
        
    }
    
}





void setup_proc(uint64_t rIn, uint64_t k0In, uint64_t k1In, uint64_t k2In, uint64_t fIn)
{
    
    r = rIn;
    k0 = k0In;
    count0=k0;
    k1 = k1In;
    count1=k1;
    k2 = k2In;
    count2=k2;
    f = fIn;
    cdb_size =r;
    schedule_available= (2*(k0+k1+k2));
    scheduling_queue* schedule_queue = new scheduling_queue[(2*(k0+k1+k2))];
    
    printf("INST\tFETCH\tDISP\tSCHED\tEXEC\tSTATE\tRETIRE\n");
    
    
    function_unit* FU=new function_unit[k0+k1+k2];
    for(int i=0;i<(k0+k1+k2);i++)
    {
        FU[i].valid=NOT_BUSY;
    }
    
    for (int i = 0; i<128; i++)
    {
        register_file[i].valid = NOT_BUSY;
    }
    
    common_data_bus* cdb = new common_data_bus[r];
    
    for(int i=0; i<r;i++)
    {
        cdb[i].busy=NOT_BUSY;
    }
    
    
    for(int i=0;i<(2*(k0+k1+k2));i++)
    {
        schedule_queue[i].valid=NOT_BUSY;
    }
    
    
    
}

void run_proc(proc_stats_t* p_stats)
{
    cycle = 0;
    
    //  instruction_count = 0;
    cycle++;
    // fetch_instructions();
    cycle++;
    // fetch_instructions();
    cycle++;
    // dispatch_instructions();
    fetch_instructions();
    
    /*
     while(flag)
     {
     execute_cycle2(); //push instructions into cdb and free FU
     stateupdate_cycle2(); //remove from the scheduling queue and set read flag if all instructions are read and complete
     stateupdate_cycle1();//reg file update, remove from sched queue and free cdb
     schedule_cycle1(); //schedule instructions into FU
     
     schedule_cycle2(); //check cdb to see if source regs waiting are to be set READY
     dispatch_instructions(); //dispatch instructions
     cycle++;
     
     
     
     instruction_fetch(); //fetch instructions
     
     }
     
     cycle = cycle - 2;
     */
}

void complete_proc(proc_stats_t *p_stats)
{
    p_stats->retired_instruction = instruction_count;
    p_stats->cycle_count = cycle;
    p_stats->avg_inst_retired = ((double)instruction_count)/cycle;
    p_stats->avg_inst_fired= ((double)instructions_fired/instruction_count);
    
    printf("\n");
    
    free(cdb);
}