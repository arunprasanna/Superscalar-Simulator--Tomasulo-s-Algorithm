/*
---------------------------Project 2: Arunprasanna, Sundararajan Poorna----------------
 */


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
#define NOT_INITIALIZED -92

//structure to store register file. contains tag and ready bit.
struct _reg
{
    int32_t tag;
    int32_t ready;
}reg;

//structure to implement common data bus .
struct _common_data_bus
{
    int32_t line_number;
    int32_t tag;
    int32_t reg;
    int32_t function_unit;
    int32_t busy;
    int32_t cycle;
}common_data_bus;
_common_data_bus *cdb;

//structure to store the cycle in which each instruction passes through the stages of the
//pipeline.

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
//strcuture to implement function unit.
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

//strcuture to implement scheduling queue.
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



uint64_t rr = 0;
uint64_t kk0 = 0;
uint64_t kk1 = 0;
uint64_t kk2 = 0;
uint64_t ff = 0;


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
uint32_t dispatch_queue_size=0;
uint32_t state_update_over=1;



queue <proc_inst_t> fetch_queue;
queue <proc_inst_t> dispatch_queue;

/*
---------------------------------function to fetch instructions----------------------------
 The function fetches f instructions at a time and stores it in the fetch queue.
 */
 
void fetch_instructions()
{
    
    proc_inst_t p_inst; //node to fetch data from file
    
    for (int i = 0; i<ff && flag1==TRUE; i++)
    {
        flag1=read_instruction(&p_inst);//returns false if the file is read completely.
        
        if (flag1==TRUE)
        {
            instruction_count++; //increment instruction count
            timer[instruction_count].instruction=instruction_count; //instruction index of timer is set as the instruction count.
            p_inst.line_number=instruction_count; //copy instruction count as line number to index in the rest of the program
            timer[instruction_count].fetch=cycle;//set timing data
            fetch_queue.push(p_inst);//push it into the fetch queue.
        }
        else
        {
            read_over = 0; //reading file is now over.
            
        }
        
        
    }
}


/*
 ---------------------------------function to dispatch instructions----------------------------
 The function dispatches instructions into the dispatch queue from the fetch queue...
 */




void dispatch_instructions()
{
    
    
    proc_inst_t dispatch_node;//node into fetch queue.
   // int counta=4;
    
    
    while(fetch_queue.size()!=0)
    {
        {
            
            dispatch_node=fetch_queue.front(); //points to the front element now
            timer[dispatch_node.line_number].dispatch=cycle;//set timing data for dispatch
            dispatch_queue.push(dispatch_node);//push element into dispatch queue
            fetch_queue.pop();//delete from fecth queue
            if(dispatch_queue.size()>max_dispatch_queue_size)
            {
                max_dispatch_queue_size=dispatch_queue.size();//holds max of dispatch queue.
            }
           // counta--;
        }

        
    }
    dispatch_queue_size+=dispatch_queue.size();//sum the dispacth queue. And divide in the end to get average dispatch queue.

    
}

/*
 ---------------------------------function to schedule instructions----------------------------
 The function schedule instructions into the scheduling queue from the dispatch queue. It sets the tag/ready bits for source and dest.
 */



void schedule_instructions()
{
    
    
    
    for(int i=0;i<schedule_available;i++) //check availability
    {
        
        if(schedule_queue[i].busy==NOT_BUSY && dispatch_queue.size()!=0) //find free space in scheduling queue and see if dispatch queue is not empty.
        {
            proc_inst_t scheduling_node1 = dispatch_queue.front(); //node points to dispatch queue element.
            dispatch_queue.pop(); //delete from dispatch queue


            timer[scheduling_node1.line_number].schedule=cycle; //set timing data
            
            //copy the values from the dispatch queue

            schedule_queue[i].line_number=scheduling_node1.line_number;
            schedule_queue[i].cycle=cycle;
            schedule_queue[i].function_unit=scheduling_node1.op_code;
            schedule_queue[i].busy=BUSY;
            
            
            if(schedule_queue[i].function_unit==-1)
            {
                schedule_queue[i].function_unit=1; //set op code=-1 and op code=1 to be treated as same from here on in the program.
            }
            
            
            
            if(scheduling_node1.src_reg[0]!=-1)
            {
                
                {
                    schedule_queue[i].src1_ready=register_file[scheduling_node1.src_reg[0]].ready;//copy tag and ready bits from the register file
                    schedule_queue[i].src1_tag=register_file[scheduling_node1.src_reg[0]].tag;

                    
                    
                }
            }
            else
            {
                schedule_queue[i].src1_ready=READY; //source doesnt depend on anything



            }
            
            if(scheduling_node1.src_reg[1]!=-1)
            {
                schedule_queue[i].src2_ready=register_file[scheduling_node1.src_reg[1]].ready;//copy tag and ready bits from the register file
                schedule_queue[i].src2_tag=register_file[scheduling_node1.src_reg[1]].tag;
                
            }
            else
            {
                schedule_queue[i].src2_ready=READY;//source doesnt depend on anything



            }
   
            
            
            
            if(scheduling_node1.dest_reg!=-1)
            {
                schedule_queue[i].dest_reg=scheduling_node1.dest_reg;//copy dest_reg into scheduling queue.
                schedule_queue[i].dest_tag=tag;//assign new tag to dest
                register_file[scheduling_node1.dest_reg].tag=tag;//copy tag into reg file
                register_file[scheduling_node1.dest_reg].ready=NOT_READY; //set reg file as not ready here.
                tag++;
                
            }
            else
            {
                schedule_queue[i].dest_tag=-1; //if dest tag is -1, there is no need to copy it into the reg file.
            }
                
        }
    }
    
   //sort the scheduling queue in cycle order so that functions scheduled earlier get fired earlier. Bubble sort implemented below.
    
    for(int i=0;i<schedule_available;i++)
     {
      _scheduling_queue temporary_node;
      {
      for(int j=0;j<(schedule_available-i-1);j++)
      {
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


void fire_into_function_unit()
{
    
    for(int i=0; i<schedule_available;i++)
    {
        //    printf("\n\tCount0 %llu\t",count0);
        //  printf("\tCount1 %llu\t",count1);
        
        //printf("\tCount2 %llu\t",count2);
        
        //check if the source regs are ready, if the instruction is not already fired and if it is a valid element with space in the function unit!
        
        if (count0!=0 && schedule_queue[i].src1_ready == READY && schedule_queue[i].src2_ready == READY && schedule_queue[i].function_unit==0 && schedule_queue[i].already_executed!=1 && schedule_queue[i].busy==BUSY)
        {
            count0--; //decrement no. of FU s available of this type
            instructions_fired++; //increment instructions fired
            for(int j=0;j<kk0+kk1+kk2;j++)
            {
                if(fu[j].busy==NOT_BUSY) //check if not already occupied
                {
                    fu[j].busy=BUSY; //set as occupied
                    fu[j].dest_tag = schedule_queue[i].dest_tag;//copy all values from sched queue
                    fu[j].dest_reg = schedule_queue[i].dest_reg;
                    fu[j].line_number = schedule_queue[i].line_number;
                    fu[j].f_unit=schedule_queue[i].function_unit;
                    fu[j].cycle = cycle;
                    timer[schedule_queue[i].line_number].execute =cycle;//set timing data
                    schedule_queue[i].already_executed = 1;
                    break;
                    
                }
            }
        }
        
/*------------------------repeat the above for the other two function units as well.------------------
 */
        else if (count1!=0 && schedule_queue[i].src1_ready == READY && schedule_queue[i].src2_ready == READY && schedule_queue[i].function_unit==1 && schedule_queue[i].already_executed!=1 && schedule_queue[i].busy==BUSY)
        {   count1--;
            instructions_fired++;
            for(int j=0;j<kk0+kk1+kk2;j++)
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
            
            for(int j=0;j<kk0+kk1+kk2;j++)
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
    

}

/*
 ---------------------------------function to execute instructions----------------------------
 The function schedule inst into the function unit from the scheduling queue.
 */


void execute_instructions()
{
    
    fire_into_function_unit(); //call to function that fires the instructions
    
    
    //cdb updates Scheduling queue
    for(int i=0;i<schedule_available;i++)
    {
        for(int j = 0; j< rr; j++)
        {
            if(cdb[j].busy == BUSY && cdb[j].tag == schedule_queue[i].src1_tag) //check if tag matches with a valid cdb
            {
                
                schedule_queue[i].src1_ready = 1;//source is now ready! Its dependent value is found.
            }
            if(cdb[j].busy == BUSY && cdb[j].tag == schedule_queue[i].src2_tag)
            {
                
                schedule_queue[i].src2_ready = 1;
            }
            
            
        }
    }

    
    
    //update register file
    for(int i = 0; i<rr; i++)
    {
        for(int reg_count=0;reg_count<128;reg_count++) //search through the register file
        {
            if(cdb[i].busy==BUSY)//is cdb valid?
            {
                
                if(register_file[reg_count].tag == cdb[i].tag)//check matching tag
                {
                    register_file[reg_count].ready=READY;//reg file ready bit of that tag is now ready!
                    
                }
                
            }
            
        }
        
    }
    
 /*
    for (int i=0;i<(k0+k1+k2);i++)
    {
        _function_unit temporary;
        for(int j=0;j<(k0+k1+k2-i-1);j++)
        {
            if(fu[j].line_number>fu[j+1].line_number)
            {
                {
                    temporary=fu[j];
                    fu[j]=fu[j+1];
                    fu[j+1]=temporary;
                }
            }
        }
        
        
    }
 */
    //Bubble sort the elements in the common function_unit structure based on their cycle order.
    
    for (int i=0;i<(kk0+kk1+kk2);i++)//loop through  the FU elements
    {
        _function_unit temporary;
        for(int j=0;j<(kk0+kk1+kk2-i-1);j++)
        {
            if(fu[j].cycle>fu[j+1].cycle)
            {
                {
                    temporary=fu[j];
                    fu[j]=fu[j+1];
                    fu[j+1]=temporary;
                }
            }
        }
        
        
    }
    
    

    for(int i=0;i<rr;i++)//loop through the cdb
    {
        cdb[i].busy=NOT_BUSY;//clearing the cdb. It is now free to take in new values.
        
    }
    for(int j=0;j<(kk0+kk1+kk2);j++)//loop through the FU
    {
        for(int i=0;i<rr;i++)//loop through the cdb
        {
            if(cdb[i].busy!=BUSY)//is cdb busy?
            {
                if(fu[j].busy==BUSY)//is the FU busy? Make sure it is, to take out valid element from the sorted FU.
                {
                    cdb[i].busy=BUSY;//set the cdb as busy.
                    cdb[i].function_unit=fu[j].f_unit;//copy which function unit the instruction belongs to.
                    if(fu[j].f_unit==0)//if it belongs to function unit 0
                    {
                        count0++;//release one FU of this type.
                        fu[j].busy=NOT_BUSY;//set FU element as not busy.
                        cdb[i].tag=fu[j].dest_tag;//copy dest tag into the cdb.
                        cdb[i].reg=fu[j].dest_reg;//copy dest reg into the cdb.
                        for(int sched=0;sched<schedule_available;sched++)//search through the scheduling queue.
                        {
                            if(schedule_queue[sched].line_number==fu[j].line_number)//check if instruction matches
                            {
                                schedule_queue[sched].marked_for_deletion=1;//marked for deletion!

                                
                            }
                                
                                
                        }
                        cdb[i].line_number=fu[j].line_number;//copy line number/instruction index.
                        break;

                    }
                    else if(fu[j].f_unit==1)
                    {
                        count1++;
                        fu[j].busy=NOT_BUSY;
                        cdb[i].tag=fu[j].dest_tag;
                  //      printf("\t CDB TAG %d \t LiNE %d\t",cdb[i].tag,cdb[i].line_number);

                        cdb[i].reg=fu[j].dest_reg;
                        for(int sched=0;sched<schedule_available;sched++)
                        {
                            if(schedule_queue[sched].line_number==fu[j].line_number)
                            {
                                schedule_queue[sched].marked_for_deletion=1;
                        //        printf("\n%d\n",schedule_queue[sched].line_number);
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
                   //     printf("\t CDB TAG %d \t LiNE %d\t",cdb[i].tag,cdb[i].line_number);
                        cdb[i].reg=fu[j].dest_reg;
                        for(int sched=0;sched<schedule_available;sched++)
                        {
                            if(schedule_queue[sched].line_number==fu[j].line_number)
                            {
                                schedule_queue[sched].marked_for_deletion=1;
                     //           printf("\n%d\n",schedule_queue[sched].line_number);

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


/*
 ---------------------------------function to state update-------------------------------------------------------------
 ----------------------------------------------------------------------------------------------------------------------
State update deletes from the scheduling queue, those instructions that are now in the cdb. Also check if it is the last instruction 
being executed and set flag to stop lopping over once state update of last instruction is done.
 
 */


void state_update()
{
    int sch_del;
    for(sch_del=0;sch_del<schedule_available;sch_del++)//loop through the scheduling queue.
    {
        if(schedule_queue[sch_del].marked_for_deletion==1)//check if the instruction has been marked for deletion by the cdb.
        {
            if(schedule_queue[sch_del].delay == 0)
            {
                timer[schedule_queue[sch_del].line_number].state_update = cycle;
                schedule_queue[sch_del].delay++;
            }
            else if(schedule_queue[sch_del].delay == 1)
                schedule_queue[sch_del].delay++;
            else if(schedule_queue[sch_del].delay == 2)// a delay of two cycles introduced to ensure that state update does not happen in the same cycle.
            {  // printf("hello\t%d",cycle);
                schedule_queue[sch_del].busy = NOT_BUSY;//release the element
                schedule_queue[sch_del].marked_for_deletion = 0;//no longer marked for deletion
                schedule_queue[sch_del].already_executed = 0;//not already executed anymore
                schedule_queue[sch_del].delay = 0;//reset delay
                instructions_retired++;//increment instruction count of retired instructions.
            }
        }
    }
    
    if(read_over==0 && instructions_retired==instruction_count)
    {
    
        state_update_over=0;
    }
}



void setup_proc(uint64_t r, uint64_t k0, uint64_t k1, uint64_t k2, uint64_t f)
{
/*CHECK HERE IF COMMAND LINE ARGUMENTS DO NOT WORK*/
    
    rr = r;
    ff = f;
    kk0 = k0;
    kk1 = k1;
    kk2 = k2;
    
    
    
    count0=kk0;
    
    count1=kk1;
    
    count2=kk2;
    
    cdb_size =rr;
    
    schedule_available= (2*(kk0+kk1+kk2));
    
    schedule_queue = new _scheduling_queue[2*(kk0+kk1+kk2)];
    
    printf("INST\tFETCH\tDISP\tSCHED\tEXEC\tSTATE\n");
    
    
    fu = new _function_unit[kk0+kk1+kk2];
    for(int i=0;i<(kk0+kk1+kk2);i++)
    {
        fu[i].busy=NOT_BUSY;//initially all elements of FU are not busy.
    }
    
    for (int i = 0; i<128; i++)
    {
        register_file[i].ready = READY;
    }
    
    cdb = new _common_data_bus[rr];
    
    for(int i=0; i<rr;i++)
    {
        cdb[i].busy=NOT_BUSY;
    }
    
    
    for(int i=0;i<(2*(kk0+kk1+kk2));i++)
    {
        schedule_queue[i].busy=NOT_BUSY;
    }
    
    
    
}

void run_proc(proc_stats_t* p_stats)
{
    cycle = 0;
    
    instruction_count = 0;
    
    
    while(state_update_over)
    {
        
        cycle++; //increment cycle
        
        
        state_update();//call to state update function
        
        
        execute_instructions();//call to execute instructions function
        
        
        
        schedule_instructions(); //call to schedule instructions function
        
        
        
        dispatch_instructions(); // call to dispatch instructions function
        
        
        
        fetch_instructions(); // call to fetch instructions function
        
        
    }
    
    
}

void complete_proc(proc_stats_t *p_stats)
{   cycle=cycle-2;
   // printf("\n\t HELLO %d\n\t",cycle);
    p_stats->retired_instruction = instructions_retired;
    p_stats->cycle_count = cycle;
    p_stats->avg_inst_retired = double(instructions_retired)/cycle;
    p_stats->avg_inst_fired= double(instructions_fired)/cycle;
    p_stats->avg_disp_size=double(dispatch_queue_size)/cycle;
    p_stats->max_disp_size=((double)(max_dispatch_queue_size));
    
    for(int i=1;i<instruction_count+1;i++)
    {
        printf("%d\t%d\t%d\t%d\t%d\t%d\n",timer[i].instruction,timer[i].fetch,timer[i].dispatch,timer[i].schedule,timer[i].execute,timer[i].state_update);
    }

    
    printf("\n");
    
}