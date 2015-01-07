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

linked_list function_unit2;
linked_list function_unit1;
linked_list function_unit3;

common_node** FU0;
common_node** FU1;
common_node** FU2;


common_data_bus* cdb;
common_data_bus* temporary_cdb;
int cdb_size = 0;
int temp_cdb_size = 0;

int instruction_count;
int read_over  = 1;
int flag = 1;
int cycle = 1;

int add0, add1, add2;


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
            read_variable->cycle_when_dispatched = cycle + 1;
            printf("Fetch %d\n  ", cycle);
            add_to_list(&dispatch_queue, read_variable);
               if(read_variable->line_number==100)
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
       // printf("size %d  ", schedule_queue.size);
        remove_from_list(&dispatch_queue, dispatch_node_temporary, FALSE);
        add_to_list(&schedule_queue, dispatch_node_temporary);
    // printf("\tHELLOO %d\t", dispatch_node_temporary->p_inst.op_code);
        schedule_queue.size=(schedule_queue.size)-(1/2);

        printf("\tDispatch %d\n  ", cycle);
   

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

    printf(" Function1 available  %d  ", function_unit1.available_execution);
    printf(" Function2 available  %d  ", function_unit2.available_execution);
    printf(" Function3 available  %d  ", function_unit3.available_execution);
    
   // while(pointer_to_schedule!=NULL)
    //{printf(" \t HELLOO\t %d",pointer_to_schedule->p_inst.op_code);
     //   pointer_to_schedule=pointer_to_schedule->next;
    //}

 
    while(pointer_to_schedule!=NULL)
    {
        
    if (pointer_to_schedule!=NULL && function_unit1.available_execution>0 && pointer_to_schedule->src1_tag == READY && pointer_to_schedule->src2_tag == READY && pointer_to_schedule->cycle_when_dispatched!=cycle && pointer_to_schedule->p_inst.op_code==0)
    {
        
            common_node* temporary_node = pointer_to_schedule;

            function_unit1.available_execution--;
            printf(" \t\t\t\tSchedule0 %d\n ", cycle);

            
            pointer_to_schedule->cycle_when_executed = cycle+1;
            
            for (int j = 0; j<k0; j++)
            {
                if (FU0[j]==NULL)
                {
                    FU0[j] = pointer_to_schedule;
                    FU0[j]->cycle_when_complete=cycle+1;
                    add_to_list(&common_function_queue, temporary_node);
                    printf( "\t Added to FU queue\t");

                    break;
                }
            }
            
            pointer_to_schedule=pointer_to_schedule->next;
    }
    
       else if (pointer_to_schedule!=NULL && function_unit2.available_execution>0 && pointer_to_schedule->src1_tag == READY && pointer_to_schedule->src2_tag== READY && pointer_to_schedule->cycle_when_dispatched!=cycle && (pointer_to_schedule->p_inst.op_code==1 || pointer_to_schedule->p_inst.op_code==-1))
        
        {
            common_node* temporary_node = pointer_to_schedule;
            printf("\t\t\t\tSchedule1 %d\n ", cycle);
            function_unit2.available_execution--;
            pointer_to_schedule->cycle_when_executed = cycle+1;
            
            for (int j = 0; j<k1; j++)
            {
                if (FU1[j]==NULL)
                {
                    FU1[j] = pointer_to_schedule;
                    FU1[j]->cycle_when_complete=cycle+1;
                    add_to_list(&common_function_queue, temporary_node);
                    printf( "\t Added to FU queue\t");

                    break;
                }
            }
            
            pointer_to_schedule = pointer_to_schedule->next;
        }
        
        
       else if (pointer_to_schedule!=NULL && function_unit3.available_execution!=0 && pointer_to_schedule->src1_tag == READY && pointer_to_schedule->src2_tag== READY && pointer_to_schedule->cycle_when_dispatched!=cycle && pointer_to_schedule->p_inst.op_code==2)
        {
            printf("\t\t\t\tSchedule2 %d\n ", cycle);

            function_unit2.available_execution--;
            pointer_to_schedule->cycle_when_executed = cycle+1;
            
            for (int j = 0; j<k2; j++)
            {
                if (FU2[j]==NULL)
                {
                    FU2[j] = pointer_to_schedule;
                    FU2[j]->cycle_when_complete=cycle+1;
                    add_to_list(&common_function_queue, pointer_to_schedule);
                    printf( "\t Added to FU queue\t");

                    break;
                }
            }
            
            pointer_to_schedule = pointer_to_schedule->next;
        }
        else if (pointer_to_schedule != NULL)
        {
            pointer_to_schedule = pointer_to_schedule->next;
        }
    }

    printf(" Function1 available  %d  ", function_unit1.available_execution);
    printf(" Function2 available  %d  ", function_unit2.available_execution);
    printf(" Function3 available  %d  ", function_unit3.available_execution);

}





void schedule_cycle2()
{

        common_node* update_node;
        update_node = schedule_queue.head;
while(update_node!=NULL)
{
            for (int j = 0;j<cdb_size; j++)
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




void free_functionunit()
{
    for (int j= 0; j<k0; j++)
    {
        if (FU0[j] != NULL)
        {       printf("ELEVATE");
                function_unit1.available_execution++;
                FU0[j] = NULL;
            
        }
    }
    
    for (int j= 0; j<k1; j++)
    {
        if (FU1[j] != NULL)
        {       printf("ELEVATE");
                function_unit2.available_execution++;
                FU1[j] = NULL;
            
        }
    }
    
    for (int j= 0; j<k2; j++)
    {
        if (FU2[j] != NULL)
        {       printf("ELEVATE");
                function_unit3.available_execution++;
                FU2[j] = NULL;
            
        }
    }
}





void cdb_exchange()
{
    for (int i = 0; i<r ;i++)
    
    if(cdb[i].busy != BUSY)
    {
        cdb[i] = temporary_cdb[i];
        cdb[i].busy = BUSY;
        for(int j=0;j<k0;j++)
        {
            if(cdb[i].line_number==FU0[j]->line_number)
                FU0[j]=NULL;
            schedule_queue.size--;
        }
        for(int j=0;j<k1;j++)
        {
            if(cdb[i].line_number==FU1[j]->line_number)
                FU0[j]=NULL;
            schedule_queue.size--;

        }

        for(int j=0;j<k2;j++)
        {
            if(cdb[i].line_number==FU2[j]->line_number)
                FU0[j]=NULL;
            schedule_queue.size--;

        }

        
    }
    
}



void execute_cycle1()
{
   // cdb_exchange();
}


void execute_cycle2()
{
    free_functionunit();

   // cdb_exchange();
}




void removeScheduler()
{
    common_node* node_into_scheduler;
    
    {   node_into_scheduler = schedule_queue.head;
        printf(" \tLOOK HERE %d \t  ", cycle);
        printf(" \tLOOK HERE %d \t  ", node_into_scheduler->cycle_when_complete);

        for(int j=0; j<k0; j++)
        {
        while (node_into_scheduler!=NULL)
            {
                if (FU0[j]->cycle_when_complete == cycle+1)
                {
                    node_into_scheduler->cycle_when_retired = cycle;
                    remove_from_list(&schedule_queue, node_into_scheduler,TRUE);
                    schedule_queue.size++;
                    break;
                }
                node_into_scheduler = node_into_scheduler->next;
            }
        }
    
        
        node_into_scheduler = schedule_queue.head;
        for(int j=0; j<k1; j++)
        {
            while (node_into_scheduler!=NULL)
            {
                if (FU1[j]->cycle_when_complete == cycle+1)
                {
                    node_into_scheduler->cycle_when_retired = cycle;
                    remove_from_list(&schedule_queue, node_into_scheduler,TRUE);
                    schedule_queue.size++;
                    break;
                }
                node_into_scheduler = node_into_scheduler->next;
            }
        }

        
        node_into_scheduler = schedule_queue.head;
        for(int j=0; j<k2; j++)
        {
            while (node_into_scheduler!=NULL)
            {
                if (FU2[j]->cycle_when_complete == cycle+1)
                {
                    node_into_scheduler->cycle_when_retired = cycle;
                    remove_from_list(&schedule_queue, node_into_scheduler,TRUE);
                    schedule_queue.size++;
                    break;
                }
                node_into_scheduler = node_into_scheduler->next;
            }
        }

    }
    
}



void regfile_update()
{
    for (int i = 0; i < temp_cdb_size; i++)
    {
        if (register_file[temporary_cdb[i].reg].tag == temporary_cdb[i].tag)
        {
            register_file[temporary_cdb[i].reg].tag = READY;
        }
    }
}


void stateupdate_cycle1()
{
    regfile_update();

}


void stateupdate_cycle2()
{
    removeScheduler();
    if (read_over == 0)
    {
        flag = 0;
    }

}




void setup_proc(uint64_t rIn, uint64_t k0In, uint64_t k1In, uint64_t k2In, uint64_t fIn)
{

    r = rIn;
    k0 = k0In;
    k1 = k1In;
    k2 = k2In;
    f = fIn;
    printf("INST\tFETCH\tDISP\tSCHED\tEXEC\tSTATE\tRETIRE\n");
    
    for (int i = 0; i<128; i++)
    {
        register_file[i].tag = READY;
    }
    
    cdb = (common_data_bus *) malloc((r)*sizeof(common_data_bus));
    temporary_cdb = (common_data_bus *) malloc((r)*sizeof(common_data_bus));


    FU0 = (common_node**) malloc(k0*sizeof(common_node*));
    FU1 = (common_node**) malloc(k1*sizeof(common_node*));
    FU2 = (common_node**) malloc(k2*sizeof(common_node));
    
  
    dispatch_queue = {NULL, NULL, (int)1000000 , (int)0};
    schedule_queue =  {NULL, NULL, (int)(2*(k0+k1+k2)) , (int)(k0+k1+k1)};
    common_function_queue= {NULL, NULL, (int)(100), (int)(100)};
    function_unit1 = { NULL, NULL, (int)(k0), (int)(k0)};
    function_unit2 = { NULL, NULL, (int)(k1), (int)(k1)};
    function_unit3 = { NULL, NULL, (int)(k2), (int)(k2)};


}

void run_proc(proc_stats_t* p_stats)
{
    cycle = 0;
    
    instruction_count = 0;
    
    while(flag)
    {

        stateupdate_cycle1();
        execute_cycle2();
        schedule_cycle2();
        dispatch_instructions();
        cycle++;

        
        
        stateupdate_cycle1();
        execute_cycle1();
        schedule_cycle1();
        instruction_fetch();

    }
    
    cycle = cycle - 1;
}

void complete_proc(proc_stats_t *p_stats)
{
    p_stats->retired_instruction = instruction_count;
    p_stats->cycle_count = cycle;
    p_stats->avg_inst_retired = ((double)instruction_count)/cycle;
    
    printf("\n");
    
    free(cdb);
    free(temporary_cdb);
    free(FU0);
    free(FU1);
    free(FU2);
}