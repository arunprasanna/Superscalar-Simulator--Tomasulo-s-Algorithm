#include "procsim.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <math.h>

#define FALSE 	-1
#define TRUE  	1

#define FULL  		2
#define EMPTY  		3
#define HAS_ROOM  	4

#define UNINITIALIZED -2
#define READY         -3
#define DONE          -4

#define BUSY    0
#define NOT_BUSY    1

typedef struct _reg
{
    int tag;
}reg;

typedef struct _common_data_bus
{
    int line_number;
    int tag;
    int reg;
    int FU;
    int busy;
    int cycle;
}common_data_bus;


typedef struct _common_node
{
    _common_node *next;
    _common_node *prev;
    proc_inst_t p_inst;
    int line_number;
    int dest_tag;
    int src1_tag;
    int src2_tag;
    int function_unit;
    int busy;
    int cycle_when_fetched;
    int cycle_when_dispatched;
    int cycle_when_executed;
    int cycle_when_complete;
    int cycle_when_retired;
}common_node;

typedef struct _linked_list
{
    _common_node* head;
    _common_node* tail;
    int size;
    int available_execution;
}linked_list;


uint64_t r = 0;
uint64_t k0 = 0;
uint64_t k1 = 0;
uint64_t k2 = 0;
uint64_t f = 0;


reg register_file[128];

linked_list dispatch_queue;
linked_list common_function_queue;
linked_list schedule_queue;

uint64_t count0;
uint64_t count1;
uint64_t count2;


common_data_bus* cdb;

uint64_t cdb_size = 0;

int instruction_count;
int read_over  = 1;
int flag = 1;
int cycle = 1;
int instructions_fired=0;
int instructions_retired=0;
int max_dispatch_queue_size=0;



int linked_list_status(linked_list pointer)
{
    if (pointer.size==0)
    {
        return FULL;
    }
    else if(pointer.head == NULL)
    {
        return EMPTY;
    }
    else
    {
        return HAS_ROOM;
    }
}

int add_to_list(linked_list* pointer, common_node* next_node)
{
    pointer->size--;
    
    next_node->prev = pointer->tail;
    next_node->next = NULL;
    
    if (pointer->tail != NULL)
    {
        pointer->tail->next = next_node;
    }else
    {
        pointer->head = next_node;
    }

    pointer->tail = next_node;
    
    return TRUE;
}

void remove_from_list(linked_list* pointer, common_node* delete_node, int memory_free)
{
    if (delete_node->prev==NULL)
    {
        if (delete_node->next==NULL)
        {
            pointer->head = NULL;
            pointer->tail = NULL;
        }
        else
        {
            pointer->head = delete_node->next;
            pointer->head->prev = NULL;
        }
    }
    else if (delete_node->next==NULL)
    {
        pointer->tail = delete_node->prev;
        pointer->tail->next = NULL;
    }
    else
    {
        delete_node->prev->next = delete_node->next;
        delete_node->next->prev = delete_node->prev;
    }
    pointer->size++;
    
    if (memory_free==TRUE)
    {
        free(delete_node);
    }
}


common_node* create_node_at_position(proc_inst_t p_inst, int line_number)
{
    common_node* new_node = (common_node*) malloc(sizeof(common_node));
    
    new_node->p_inst = p_inst;
    new_node->line_number = line_number;
    new_node->dest_tag = line_number;
    
    new_node->src1_tag = UNINITIALIZED;
    new_node->src2_tag= UNINITIALIZED;
    
    return new_node;
}

common_node* create_node_for_function_queue(common_node* function_node)
{
    common_node* new_node = (common_node*) malloc(sizeof(common_node));
    return new_node;
}

void create_scheduling_node(common_node* dispatch_node)
{
    
    if(dispatch_node->p_inst.src_reg[0]!=-1)
    {
        dispatch_node->src1_tag= register_file[dispatch_node->p_inst.src_reg[0]].tag;
    }
    else
    {
        dispatch_node->src1_tag= READY;
    }
    if(dispatch_node->p_inst.src_reg[1]!=-1)
    {
        dispatch_node->src2_tag= register_file[dispatch_node->p_inst.src_reg[1]].tag;
    }
    else
    {
        dispatch_node->src2_tag= READY;
    }
    
    if(dispatch_node->p_inst.dest_reg!=-1)
    {
        register_file[dispatch_node->p_inst.dest_reg].tag = dispatch_node->dest_tag;
    }
    
}

int flag1 = TRUE;


void instruction_fetch()
{
    common_node* read_variable;
    proc_inst_t* p_inst;
    p_inst = (proc_inst_t*) malloc(sizeof(proc_inst_t));
    
    for (int i = 0; i<f && flag1==TRUE; i++)
    {
            flag1=read_instruction(p_inst);
        
            if (flag1==TRUE)
            {
            instruction_count++;
            read_variable = create_node_at_position(*p_inst, instruction_count);
            read_variable->cycle_when_fetched = cycle;
           printf("\nFetch %d  ", read_variable->line_number);
            add_to_list(&dispatch_queue, read_variable);
               if(read_variable->line_number==40)
                   flag=0;
            
        }
            else
        {
            read_over = 0;
        }
        
    }
    
    free(p_inst);
}





void dispatch_instructions()
{
    
    
    common_node* dispatch_node;
    common_node* dispatch_node_temporary;
    
    dispatch_node = dispatch_queue.head;

    
    while(dispatch_node!=NULL && schedule_queue.size>0)
    {
        create_scheduling_node(dispatch_node);
        dispatch_node_temporary = dispatch_node;
        dispatch_node = dispatch_node->next;
        dispatch_node_temporary->cycle_when_dispatched=cycle+1;
        remove_from_list(&dispatch_queue, dispatch_node_temporary, FALSE);
        add_to_list(&schedule_queue, dispatch_node_temporary);
        printf("\n\tDispatch %d  ", dispatch_node_temporary->line_number);
   

    }
    
    
}


void schedule_sort(linked_list *queue)
{
    
    
    /* Function to pairwise swap elements of a linked list */
    {   common_node *head = queue->head;
        // If linked list is empty or there is only one node in list
        if (head == NULL || (head)->next == NULL)
            return;
        
        // Initialize previous and current pointers
        common_node *prev = head;
        common_node *curr = (head)->next;
        
        head = curr;  // Change head before proceeeding
        
        // Traverse the list
        while (true)
        {
            common_node *next = curr->next;
            curr->next = prev; // Change next of current as previous node
            
            // If next NULL or next is the last node
            if (next == NULL || next->next == NULL)
            {
                prev->next = next;
                break;
            }
            
            // Change next of previous to next next
            prev->next = next->next;
            
            // Update previous and curr
            prev = next;
            curr = prev->next;
        }
    }
}



void schedule_cycle1()
{
   // schedule_sort(&schedule_queue);
    common_node* pointer_to_schedule = schedule_queue.head;

    printf("\n Function0 available  %llu \n ", count0);
    printf("\n Function1 available  %llu  \n", count1);
    printf("\n Function2 available  %llu  \n", count2);

 
    while(pointer_to_schedule!=NULL)
    {
        
    if (pointer_to_schedule!=NULL && count0>0 && pointer_to_schedule->src1_tag == READY && pointer_to_schedule->src2_tag == READY && pointer_to_schedule->cycle_when_dispatched!=cycle && pointer_to_schedule->p_inst.op_code==0)
    {
       //     printf(" \n\t\t\tSchedule0 %d ", pointer_to_schedule->line_number);
        
            count0--;
            instructions_fired++;

        
            common_node* temporary_node = create_node_for_function_queue(pointer_to_schedule);;
            temporary_node=pointer_to_schedule;
            temporary_node->function_unit=0;
            temporary_node->cycle_when_executed = cycle+1;
            temporary_node->cycle_when_complete=cycle+2;
        
            pointer_to_schedule = pointer_to_schedule->next;
        
            add_to_list(&common_function_queue, temporary_node);
            printf( "\n Added to FU queue\t");
    }
    else if (pointer_to_schedule!=NULL && count1>0 && pointer_to_schedule->src1_tag == READY && pointer_to_schedule->src2_tag == READY && pointer_to_schedule->cycle_when_dispatched!=cycle && (pointer_to_schedule->p_inst.op_code==1 || pointer_to_schedule->p_inst.op_code==-1))
        
       { //   printf("\n\t\t\tSchedule1 %d ", pointer_to_schedule->line_number);
           
            count1--;
            instructions_fired++;
           
            common_node* temporary_node=create_node_for_function_queue(pointer_to_schedule);;
            temporary_node = pointer_to_schedule;
            temporary_node->function_unit=1;
            temporary_node->cycle_when_executed = cycle+1;
            temporary_node->cycle_when_complete=cycle+2;
           
           pointer_to_schedule = pointer_to_schedule->next;
            add_to_list(&common_function_queue, temporary_node);
           
           printf( "\n Added to FU queue\t\n");
        }
        
        
       else if (pointer_to_schedule!=NULL && count2>0 && pointer_to_schedule->src1_tag == READY && pointer_to_schedule->src2_tag== READY && pointer_to_schedule->cycle_when_dispatched!=cycle && pointer_to_schedule->p_inst.op_code==2)
        {
        //    printf("\n\t\t\tSchedule2 %d ", pointer_to_schedule->line_number);
            
            count2--;
            instructions_fired++;

            
            common_node* temporary_node = create_node_for_function_queue(pointer_to_schedule);
            temporary_node = pointer_to_schedule;
            temporary_node->function_unit=2;
            temporary_node->cycle_when_executed = cycle+1;
            temporary_node->cycle_when_complete=cycle+2;
            
            pointer_to_schedule = pointer_to_schedule->next;
            
            add_to_list(&common_function_queue, temporary_node);
        //    printf( "\n Added to FU queue\t");
        }
        else if (pointer_to_schedule != NULL)
        {
            pointer_to_schedule = pointer_to_schedule->next;
        }
    }



    
}





void schedule_cycle2()
{

        common_node* update_node;
        update_node = schedule_queue.head;
while(update_node!=NULL)
{
            for (int j = 0;j<r; j++)
            {
                if(cdb[j].tag==update_node->src1_tag)
                {
                    update_node->src1_tag=READY;
                }
                if (cdb[j].tag==update_node->src2_tag)
                {
                    update_node->src2_tag= READY;
                }
            }
            update_node = update_node->next;
    }
        
}




void into_the_cdb()
{
    common_node* free_node;
    free_node=common_function_queue.head;
    common_node* temp_node;
    if(free_node!=NULL)
    {
    for(int i=0; i<r; i++)
        {
            if(free_node!=NULL)
        {
            if(cdb[i].busy == NOT_BUSY)
            {
               cdb[i].busy=BUSY;
          //      printf("Updating CDB");
                temp_node=free_node;
                cdb[i].FU=free_node->function_unit;
                cdb[i].line_number=free_node->line_number;
                cdb[i].tag=free_node->dest_tag;
                cdb[i].reg=free_node->p_inst.dest_reg;
                cdb[i].cycle=cycle;
               /*
                printf("\n\n\n CYCLE%d", cycle);
                printf("\nFU %d", cdb[i].FU);
                printf("\nLine %d", cdb[i].line_number);
                printf("\nTag %d", cdb[i].tag);
                printf("\nReg %d", cdb[i].reg);
                printf("\nCycle %d", cdb[i].cycle);
                
*/
                remove_from_list(&common_function_queue, temp_node, FALSE);
                free_node=free_node->next;
            }
        }

    }
    
}

}
void free_function_unit()
{ //uint64_t cd=0;
    for(int i=0;i<r;i++)
    {
        if(cdb[i].busy == BUSY)

    {
        
             if(cdb[i].FU==0)
             { if(count0<k0)
               {
                   count0++;
        //           printf("\n Incremented Function unit0 \t");

               }
             }
        
            else if(cdb[i].FU==1)
             {
                 if(count1<k1)
                 {
                     count1++;
         //            printf("\n Incremented Function unit1 \t");

                 }

             }
            else if(cdb[i].FU==2)
            { if(count2<k2)
                {
                count2++;
        //        printf("\n Incremented Function unit2 \t");

                }
            }
        else
            break;

        

        }
    
    }
}


void display_function_queue()
{
    common_node* node = common_function_queue.head;
    while(node!=NULL)
    {   printf("\n\n\n CYCLE%d", cycle);
        printf("\nLine Number %d",node->line_number);
        printf("\nDest Tag %d",node->dest_tag);
        printf("\nSrc1 Tag %d",node->src1_tag);
        printf("\nSrc2 Tag %d",node->src2_tag);
        printf("\nCycle when fetched %d",node->cycle_when_fetched);
        printf("\n SIZE OF SCHEDULING QUEUE %d",schedule_queue.size);
        node = node->next;
        

    }
}


void execute_cycle2()
{
  into_the_cdb();
  free_function_unit();
 // display_function_queue();
    

}


void free_cdb()
{
    for(int i=0;i<r;i++)
    {  // printf("\t\t\n Cycle%d \t CDB cycle%d\t",cycle, cdb[i].cycle);
            cdb[i].busy=NOT_BUSY;
    }
}



void regfile_update()
{
    for (int i = 0; i < r; i++)
    {
        if(cdb[i].busy==BUSY)
        {
        if (register_file[cdb[i].reg].tag == cdb[i].tag)
        {
            register_file[cdb[i].reg].tag = READY;
    
        }
        }
    }
}

void remove_from_schedule_queue()
{
    
    {
        common_node* node_into_scheduler;
        
        for(node_into_scheduler=schedule_queue.head;node_into_scheduler!=NULL;node_into_scheduler=node_into_scheduler->next)
        {
            for(int i=0;i<r;i++)
            {
                if(cdb[i].tag==node_into_scheduler->dest_tag)
                {
                    
                    common_node* temporary_node= node_into_scheduler;
                    printf("\n\t\t DELETE FROM SCHEDULE QUEUE\t");
                    remove_from_list(&schedule_queue, temporary_node,FALSE);
                    
                }
            }
        }
    }
}



void stateupdate_cycle1()
{
    regfile_update();
    remove_from_schedule_queue();

    free_cdb();

}
/*
void removeScheduler()
{
    common_node* node_into_scheduler;
    
    {   node_into_scheduler = schedule_queue.head;
       
        if (node_into_scheduler!=NULL)

        {
            for(int i=0;i<2;i++)
            {
                if (node_into_scheduler!=NULL)
                {
                if(cdb[i].tag==node_into_scheduler->dest_tag)
                {   printf("\n\t\t DELETE FROM SCHEDULE QUEUE\t");
                    remove_from_list(&schedule_queue, node_into_scheduler,FALSE);
                    node_into_scheduler = node_into_scheduler->next;

                }
                }
                
            }
            
        }
    }
}*/


    /*

void remove_from_schedule_queue()
{
    common_node* free_node;
    free_node=common_function_queue.head;
    common_node* temp_node;
    if(free_node!=NULL)
    {
        for(int i=0; i<r; i++)
        {
            if(free_node!=NULL)
            {
                if(cdb[i].busy == NOT_BUSY)
                {
                    
                    
                    
                    remove_from_list(&common_function_queue, temp_node, FALSE);
                    free_node=free_node->next;
                }
            }
            
        }

    }
}*/

void stateupdate_cycle2()
{
    if (read_over == 0)
    {
        flag = 0;
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
    printf("INST\tFETCH\tDISP\tSCHED\tEXEC\tSTATE\tRETIRE\n");
    
    for (int i = 0; i<128; i++)
    {
        register_file[i].tag = READY;
    }
    
    cdb = (common_data_bus *) malloc((100)*sizeof(common_data_bus));
    for(int i=0; i<r;i++)
    {
        cdb[i].busy=NOT_BUSY;
    }

    dispatch_queue = {NULL, NULL, (int)1000000 , (int)0};
    schedule_queue =  {NULL, NULL, (int)(2*(k0+k1+k2)) , (int)(k0+k1+k1)};
    common_function_queue= {NULL, NULL, (int)(100), (int)(100)};
    


}

void run_proc(proc_stats_t* p_stats)
{
    cycle = 0;
    
    instruction_count = 0;
    
    while(flag)
    {

        stateupdate_cycle2(); //remove from the scheduling queue and set read flag if all instructions are read and complete
        execute_cycle2(); //push instructions into cdb and free FU
        schedule_cycle2(); //check cdb to see if source regs waiting are to be set READY
        dispatch_instructions(); //dispatch instructions
        cycle++;

        
        
        stateupdate_cycle1();//reg file update and free cdb
        schedule_cycle1(); //schedule instructions into FU
        instruction_fetch(); //fetch instructions

    }
    
    cycle = cycle - 2;
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