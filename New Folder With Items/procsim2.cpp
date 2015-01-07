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

#define ZERO 0
#define NOT_INITIALIZED -22

struct _reg
{
    int32_t tag;
    int32_t busy;
}reg;

struct _common_data_bus
{
    int32_t line_number;
    int32_t tag;
    int32_t reg;
    int32_t FU;
    int32_t busy;
    int32_t cycle;
}common_data_bus;


struct _timing
{
    int32_t instruction;
    int32_t fetch;
    int32_t dispatch;
    int32_t schedule;
    int32_t execute;
    int32_t state_update;
    int32_t cycle;
}timing;

_timing timer[100500];

struct _function_unit
{
    int32_t cycle;
    int32_t line_number;
    int32_t dest_tag;
    int32_t dest_reg;
    int32_t valid;
    int32_t tag;
} function_unit;

_function_unit *fu;


struct _scheduling_queue
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
    int32_t fire=0;
    int32_t marked_for_deletion=0;
    int32_t already_executed=0;
    
}scheduling_queue;

_scheduling_queue *schedule_queue;



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
uint64_t schedule_available;
int32_t tag=1;

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
            timer[instruction_count].instruction=instruction_count;
            p_inst.line_number=instruction_count;
            timer[instruction_count].fetch=cycle;
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
    
    
    while(fetch_queue.size()!=0 && counta!=0)
    {
        {   

            dispatch_node=fetch_queue.front();
            timer[dispatch_node.line_number].dispatch=cycle;

            dispatch_queue.push(dispatch_node);
            fetch_queue.pop();
            counta--;
        }
        
    }
    
    
}




void schedule_instructions()
{
    
    
    
    for(int i=0;i<schedule_available;i++)
    {
        
        if(schedule_queue[i].busy==NOT_BUSY && dispatch_queue.size()!=0)
        {
            proc_inst_t scheduling_node1 = dispatch_queue.front();
            timer[scheduling_node1.line_number].schedule=cycle;
            schedule_queue[i].cycle=cycle;
            schedule_queue[i].function_unit=scheduling_node1.op_code;
            if(schedule_queue[i].function_unit==-1)
            {
                schedule_queue[i].function_unit=1;
            }
            schedule_queue[i].busy=BUSY;
            schedule_queue[i].fire=0;
            if(scheduling_node1.src_reg[0]!=-1)
            {
                if(register_file[scheduling_node1.src_reg[0]].busy!=BUSY)
                {
                    schedule_queue[i].src1_tag=register_file[scheduling_node1.src_reg[0]].tag;
                    schedule_queue[i].src1_ready=READY;
                }
                else
                {
                    schedule_queue[i].src1_tag=register_file[scheduling_node1.src_reg[0]].tag;
                    schedule_queue[i].src1_ready=NOT_READY;
                }
                
            }
            else
            {
                schedule_queue[i].src1_ready=READY;
            }
            if(scheduling_node1.src_reg[0]!=-1)
            {
                if(register_file[scheduling_node1.src_reg[0]].busy!=BUSY)
                {
                    schedule_queue[i].src2_tag=register_file[scheduling_node1.src_reg[1]].tag;
                    schedule_queue[i].src2_ready=READY;
                }
                else
                {
                    schedule_queue[i].src2_tag=register_file[scheduling_node1.src_reg[1]].tag;
                    schedule_queue[i].src2_ready=NOT_READY;
                }
                
            }
            else
            {
                schedule_queue[i].src2_ready=READY;
            }
            
            
        if(scheduling_node1.dest_reg!=-1)
        {
            schedule_queue[i].dest_tag=tag;
            schedule_queue[i].dest_reg=scheduling_node1.dest_reg;
            register_file[scheduling_node1.dest_reg].tag=tag;
            tag++;
            register_file[scheduling_node1.dest_reg].busy=BUSY;
            
        }
        else
        {
            schedule_queue[i].dest_reg=schedule_queue[i].dest_tag=-1;
        }
    
            dispatch_queue.pop();

        }
    }
    
    
    for(int i=0;i<schedule_available;i++)
    {
        _scheduling_queue temporary_node;
        {
            for(int j=0;j<schedule_available-1;j++)
            {
               if(schedule_queue[j].busy==BUSY && schedule_queue[j+1].busy==BUSY)
               {
                   if(schedule_queue[j].cycle>schedule_queue[j+1].cycle)
                   temporary_node=schedule_queue[j];
                   schedule_queue[j]=schedule_queue[j+1];
                   schedule_queue[j+1]=temporary_node;
                   
               }
            }
                
        }
        
    }
    

}


void execute_instructions()
{
    
    for(int i=0; i<schedule_available;i++)
    {
        if (count0>0 && schedule_queue[i].src1_ready == READY && schedule_queue[i].src2_tag == READY && schedule_queue[i].function_unit==0 && schedule_queue[i].already_executed!=1 && schedule_queue[i].busy==BUSY)
        {   count0--;
            instructions_fired++;
            
            for(int j=0;j<k0+k1+k2;j++)
            { if(fu[j].valid!=BUSY)
            {
                fu[j].valid=BUSY;
                fu[j].dest_tag = schedule_queue[i].dest_tag;
                fu[j].dest_reg = schedule_queue[i].dest_reg;
                fu[j].line_number = sched[i].line_number;
                fu[j].cycle = cycle;
                timer[schedule_queue[i].line_number].execute =cycle;
                schedule_queue[i].fire = 1;
                schedule_queue[i].already_executed = 1;
                
                break;

            }
                

            
            
            
            
       
}
    }
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
    schedule_queue = new _scheduling_queue[2*(k0+k1+k2)];
    
    printf("INST\tFETCH\tDISP\tSCHED\tEXEC\tSTATE\n");
    
    
   fu = new _function_unit[k0+k1+k2];
    for(int i=0;i<(k0+k1+k2);i++)
    {
        fu[i].valid=NOT_BUSY;
    }
    
    for (int i = 0; i<128; i++)
    {
        register_file[i].busy = NOT_BUSY;
    }
    
    _common_data_bus* cdb = new _common_data_bus[r];
    
    for(int i=0; i<r;i++)
    {
        cdb[i].busy=NOT_BUSY;
    }
    
    
    for(int i=0;i<(2*(k0+k1+k2));i++)
    {
        schedule_queue[i].busy=NOT_BUSY;
    }
    
    
    
}

void run_proc(proc_stats_t* p_stats)
{
    cycle = 0;
    
    instruction_count = 0;
    
    
    while(read_over)
    {   cycle++;
        
        
        //    execute_cycle2(); //push instructions into cdb and free FU
        //  stateupdate_cycle2(); //remove from the scheduling queue and set read flag if all instructions are read and complete
        //     stateupdate_cycle1();//reg file update, remove from sched queue and free cdb
        //   schedule_cycle1(); //schedule instructions into FU
        
        schedule_instructions();
        
        
        
        dispatch_instructions(); //dispatch instructions
        
        
        
        fetch_instructions(); //fetch instructions
        
        
    }
    
    cycle = cycle - 2;
    
}

void complete_proc(proc_stats_t *p_stats)
{
    p_stats->retired_instruction = instruction_count;
    p_stats->cycle_count = cycle;
    p_stats->avg_inst_retired = ((double)instruction_count)/cycle;
    p_stats->avg_inst_fired= ((double)instructions_fired/instruction_count);
    for(int i=1;i<30;i++)
    {
        printf("%d\t%d\t%d\t%d\n",timer[i].instruction,timer[i].fetch,timer[i].dispatch,timer[i].schedule);
    }

    printf("\n");
    
}