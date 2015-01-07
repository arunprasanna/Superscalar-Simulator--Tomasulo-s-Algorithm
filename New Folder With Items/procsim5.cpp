using namespace std;
#include "procsim.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <queue>

#define FALSE 	-1
#define TRUE  	1

#define UNINITIALIZED -2
#define READY         1
#define DONE          -4
#define NOT_READY     0

#define BUSY    1
#define NOT_BUSY    0

#define ZERO 0
#define NOT_INITIALIZED -22

struct _reg
{
    int32_t tag;
    int32_t ready;
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
_common_data_bus *cdb;


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
    int32_t busy;
    int32_t tag;
    int32_t f_unit;
    
} function_unit;

_function_unit *fu;


struct _scheduling_queue
{
    int32_t function_unit;
    int32_t line_number;
    int32_t cycle;
    int32_t dest_reg;
    int32_t dest_tag;
    int32_t src1_tag;
    int32_t src2_tag;
    int32_t src1_ready;
    int32_t src2_ready;
    int32_t busy;
    int32_t delay;
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
int32_t tag=0;

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
            dispatch_queue.pop();
            timer[scheduling_node1.line_number].schedule=cycle;
            schedule_queue[i].line_number=scheduling_node1.line_number;
            schedule_queue[i].cycle=cycle;
            schedule_queue[i].function_unit=scheduling_node1.op_code;
            schedule_queue[i].busy=BUSY;
            if(schedule_queue[i].function_unit==-1)
            {
                schedule_queue[i].function_unit=1;
            }
            
            
            
            if(scheduling_node1.src_reg[0]!=-1)
            {
                
                if(register_file[scheduling_node1.src_reg[0]].ready==READY)
                {
                    schedule_queue[i].src1_ready=READY;

                }
                else
                {
                    schedule_queue[i].src1_tag=register_file[scheduling_node1.src_reg[0]].tag;
                    if(schedule_queue[i].dest_tag==register_file[scheduling_node1.src_reg[0]].tag)
                        schedule_queue[i].src1_ready=READY;
                    else
                        schedule_queue[i].src1_ready=NOT_READY;
                    
                    
                }
            }
            else
            {
                schedule_queue[i].src1_ready=READY;


            }
            
            if(scheduling_node1.src_reg[1]!=-1)
            {
                if(register_file[scheduling_node1.src_reg[1]].ready==READY)
                {
                    schedule_queue[i].src2_ready=READY;


                }
                else
                {
                    schedule_queue[i].src2_tag=register_file[scheduling_node1.src_reg[1]].tag;
                    if(schedule_queue[i].dest_tag==register_file[scheduling_node1.src_reg[1]].tag)
                        schedule_queue[i].src2_ready=READY;
                    else
                        schedule_queue[i].src2_ready=NOT_READY;
                    
                    
                }
                
                
            }
            else
            {
                schedule_queue[i].src2_ready=READY;


            }
   
            
            {
                
                schedule_queue[i].dest_reg=scheduling_node1.dest_reg;
                schedule_queue[i].dest_tag=tag;
                register_file[scheduling_node1.dest_reg].tag=tag;
                register_file[scheduling_node1.dest_reg].ready=NOT_READY;
                tag++;
                
                
            }
            else
            {
                schedule_queue[i].dest_reg=-1;
                schedule_queue[i].dest_tag=-1;
            }
            
     
        }
    }
    
  
    
   
     for(int i=0;i<schedule_available;i++)
     {
      _scheduling_queue temporary_node;
      {
      for(int j=0;j<(schedule_available-i-1);j++)
      {
          if(schedule_queue[j].busy==BUSY && schedule_queue[j+1].busy==BUSY)
          {
              if(schedule_queue[j].cycle>schedule_queue[j+1].cycle)
              {
                  temporary_node=schedule_queue[j];
                  schedule_queue[j]=schedule_queue[j+1];
                  schedule_queue[j+1]=temporary_node;
              }
          }
      }
     
     }
     
     }
     
    
}


void execute_instructions()
{
    
    for(int i=0; i<schedule_available;i++)
    {
    //    printf("\n\tCount0 %llu\t",count0);
      //  printf("\tCount1 %llu\t",count1);

        //printf("\tCount2 %llu\t",count2);

        if (count0!=0 && schedule_queue[i].src1_ready == READY && schedule_queue[i].src2_ready == READY && schedule_queue[i].function_unit==0 && schedule_queue[i].already_executed!=1 && schedule_queue[i].busy==BUSY)
        {
            count0--;
            instructions_fired++;
            for(int j=0;j<k0+k1+k2;j++)
            {
                if(fu[j].busy==NOT_BUSY)
            {
                fu[j].busy=BUSY;
                fu[j].dest_tag = schedule_queue[i].dest_tag;
                fu[j].dest_reg = schedule_queue[i].dest_reg;
                fu[j].line_number = schedule_queue[i].line_number;
                fu[j].f_unit=schedule_queue[i].function_unit;
                fu[j].cycle = cycle;
                timer[schedule_queue[i].line_number].execute =cycle;
                schedule_queue[i].already_executed = 1;
                break;
                
            }
            }
        }
        else if (count1!=0 && schedule_queue[i].src1_ready == READY && schedule_queue[i].src2_ready == READY && schedule_queue[i].function_unit==1 && schedule_queue[i].already_executed!=1 && schedule_queue[i].busy==BUSY)
        {   count1--;
            instructions_fired++;
            for(int j=0;j<k0+k1+k2;j++)
            { if(fu[j].busy==NOT_BUSY)
            {
                fu[j].busy=BUSY;
                fu[j].dest_tag = schedule_queue[i].dest_tag;
                fu[j].dest_reg = schedule_queue[i].dest_reg;
                fu[j].line_number = schedule_queue[i].line_number;
                fu[j].f_unit=schedule_queue[i].function_unit;
                fu[j].cycle = cycle;
                timer[schedule_queue[i].line_number].execute =cycle;
                schedule_queue[i].already_executed = 1;
                break;
                
            }
            }
        }
        
        else if (count2!=0 && schedule_queue[i].src1_ready == READY && schedule_queue[i].src2_ready == READY && schedule_queue[i].function_unit==2 && schedule_queue[i].already_executed!=1 && schedule_queue[i].busy==BUSY)
        {   count2--;
            instructions_fired++;
          //  printf("HELLO\n");

            for(int j=0;j<k0+k1+k2;j++)
            { if(fu[j].busy==NOT_BUSY)
            {
                fu[j].busy=BUSY;
                fu[j].dest_tag = schedule_queue[i].dest_tag;
                fu[j].dest_reg = schedule_queue[i].dest_reg;
                fu[j].line_number = schedule_queue[i].line_number;
                fu[j].f_unit=schedule_queue[i].function_unit;
                fu[j].cycle = cycle;
                timer[schedule_queue[i].line_number].execute =cycle;
                schedule_queue[i].already_executed = 1;
                break;
                
            }
            }
        }

    }
    
    //cdb updates Scheduling queue
    for(int i=0;i<schedule_available;i++)
    {
        for(int j = 0; j< r; j++)
        {
            if(cdb[j].busy == BUSY && cdb[j].tag == schedule_queue[i].src1_tag && cdb[j].tag != 0 )
                {
                    
                    schedule_queue[i].src1_ready = 1;
                }
           else if(cdb[j].busy == BUSY && cdb[j].tag == schedule_queue[i].src2_tag && cdb[j].tag != 0 )
            {
                
                schedule_queue[i].src1_ready = 1;
            }


        }
    }

   
    //update register file
    for(int i = 0; i<r; i++)
    {
        for(int reg_count=0;reg_count<schedule_available;reg_count++)
        {
            if(cdb[i].busy==BUSY)
            {
                
                if(schedule_queue[reg_count].dest_tag == cdb[i].tag && cdb[i].tag != 0)
                {
                    schedule_queue[reg_count].busy = NOT_BUSY;
                    
                }
                
            }
            
        }
        
    }
    
    
    for (int i=0;i<(k0+k1+k2);i++)
    {
        _function_unit temporary;
        for(int j=0;j<(k0+k1+k2-i);j++)
        {
            if(fu[j].line_number>fu[j+1].line_number)
            {
                if(fu[j].busy==BUSY && fu[j+1].busy==BUSY)
                {
                    temporary=fu[j];
                    fu[j]=fu[j+1];
                    fu[j+1]=temporary;
                }
            }
        }
        
        
    }
    
    for (int i=0;i<(k0+k1+k2);i++)
    {
        _function_unit temporary;
        for(int j=0;j<(k0+k1+k2-i);j++)
        {
            if(fu[j].cycle>fu[j+1].cycle)
            {
                if(fu[j].busy==BUSY && fu[j+1].busy==BUSY)
                {
                    temporary=fu[j];
                    fu[j]=fu[j+1];
                    fu[j+1]=temporary;
                }
            }
        }
        
        
    }

    for(int i=0;i<r;i++)
    {
        cdb[i].busy=NOT_BUSY;
        
    }
    for(int j=0;j<(k0+k1+k2);j++)
    {
        for(int i=0;i<r;i++)
        {
            if(cdb[i].busy!=BUSY)
            {
                if(fu[j].busy==BUSY)
                {
                    cdb[i].busy=BUSY;
                    if(fu[j].f_unit==0)
                    {
                        count0++;
                        fu[j].busy=NOT_BUSY;
                        cdb[i].tag=fu[j].dest_tag;
                        printf("\t CDB TAG %d \t LiNE %d\t",cdb[i].tag,cdb[i].line_number);

                        cdb[i].reg=fu[j].dest_reg;
                        for(int sched=0;sched<schedule_available;sched++)
                        {
                            if(schedule_queue[sched].line_number==fu[j].line_number && schedule_queue[sched].dest_tag!=0)
                            {
                                schedule_queue[sched].marked_for_deletion=1;
                         //       printf("\n%d\n",schedule_queue[sched].line_number);

                                
                            }
                                
                                
                        }
                        cdb[i].line_number=fu[j].line_number;
                        break;

                    }
                    else if(fu[j].f_unit==1)
                    {
                        count1++;
                        fu[j].busy=NOT_BUSY;
                        cdb[i].tag=fu[j].dest_tag;
                        printf("\t CDB TAG %d \t LiNE %d\t",cdb[i].tag,cdb[i].line_number);

                        cdb[i].reg=fu[j].dest_reg;
                        for(int sched=0;sched<schedule_available;sched++)
                        {
                            if(schedule_queue[sched].line_number==fu[j].line_number && schedule_queue[sched].dest_tag!=0)
                            {
                                schedule_queue[sched].marked_for_deletion=1;
                            //    printf("\n%d\n",schedule_queue[sched].line_number);
                            }
                            
                            
                        }
                        cdb[i].line_number=fu[j].line_number;
                        break;
                        
                    }
                    if(fu[j].f_unit==2)
                    {
                        count2++;
                        fu[j].busy=NOT_BUSY;
                        cdb[i].tag=fu[j].dest_tag;
                        printf("\t CDB TAG %d \t LiNE %d\t",cdb[i].tag,cdb[i].line_number);
                        cdb[i].reg=fu[j].dest_reg;
                        for(int sched=0;sched<schedule_available;sched++)
                        {
                            if(schedule_queue[sched].line_number==fu[j].line_number && schedule_queue[sched].dest_tag!=0)
                            {
                                schedule_queue[sched].marked_for_deletion=1;
                          //      printf("\n%d\n",schedule_queue[sched].line_number);

                            }
                            
                            
                        }
                        cdb[i].line_number=fu[j].line_number;
                        break;
                        
                    }


                    
                    
                    
                }
            }
        }
    }
    
    

    
}

void state_update()
{
    int sch_del;
    for(sch_del=0;sch_del<schedule_available;sch_del++)
    {
        if(schedule_queue[sch_del].marked_for_deletion==1)
        {
            if(schedule_queue[sch_del].delay == 0)
            {
                timer[schedule_queue[sch_del].line_number].state_update = cycle;
                schedule_queue[sch_del].delay++;
            }
            else if(schedule_queue[sch_del].delay == 1)
                schedule_queue[sch_del].delay++;
            else if(schedule_queue[sch_del].delay == 2)
            {  // printf("hello\t%d",cycle);
                schedule_queue[sch_del].busy = NOT_BUSY;
                schedule_queue[sch_del].marked_for_deletion = 0;
                schedule_queue[sch_del].already_executed = 0;
                schedule_queue[sch_del].delay = 0;
                instructions_retired++;
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
        fu[i].busy=NOT_BUSY;
    }
    
    for (int i = 0; i<128; i++)
    {
        register_file[i].ready = READY;
    }
    
    cdb = new _common_data_bus[r];
    
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
    
    
    //while(flag1)
    for(int i=0;i<50;i++)
    {
        
        cycle++;
        
        
        state_update();
        
        
        execute_instructions();
        
        
        
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
        printf("%d\t%d\t%d\t%d\t%d\t%d\n",timer[i].instruction,timer[i].fetch,timer[i].dispatch,timer[i].schedule,timer[i].execute,timer[i].state_update);
    }
    
    printf("\n");
    
}