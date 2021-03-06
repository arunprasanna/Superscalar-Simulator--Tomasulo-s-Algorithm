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

struct reg
{
    int tag;
};

struct common_data_bus
{
    int line_number;
    int tag;
    int reg;
    int FU;
};


struct common_node
{
    common_node *next;
    common_node *prev;
    proc_inst_t p_inst;
    int line_number;
    int dest_tag;
    int src1_tag;
    int src2_tag;
    int function_unit;
    int age;
    int fetch;
    int disp;
    int sched;
    int exec;
    int state;
    int retire;
};

struct linked_list
{
    common_node* head;
    common_node* tail;
    int size;
    int available_execution;
};


uint64_t r = 0;
uint64_t k0 = 0;
uint64_t k1 = 0;
uint64_t k2 = 0;
uint64_t f = 0;

reg register_file[128];

linked_list dispatch_queue;

//Scheudler
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

void remove_from_list(linked_list* pointer, common_node* delete_node, int memory_free){
    if (delete_node->prev==NULL){			//Head element
        if (delete_node->next==NULL){		//Special case where there is only 1 element
            pointer->head = NULL;
            pointer->tail = NULL;
        }else{
            pointer->head = delete_node->next;
            pointer->head->prev = NULL;
        }
    }else if (delete_node->next==NULL){		//Tail element
        pointer->tail = delete_node->prev;
        pointer->tail->next = NULL;
    } else{									//Element in middle
        delete_node->prev->next = delete_node->next;
        delete_node->next->prev = delete_node->prev;
    }
    //Add room to linked list
    pointer->size++;
    
    //Free the node if desired
    if (memory_free==TRUE){
        free(delete_node);		//Free allocated memory
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
    new_node->age = UNINITIALIZED;
    
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



void instruction_fetch()
{
    common_node* read_variable;
    proc_inst_t* p_inst;
    int flag1 = TRUE;
    p_inst = (proc_inst_t*) malloc(sizeof(proc_inst_t));
    
    for (int i = 0; i<f && flag1==TRUE; i++)
    {
            flag1=read_instruction(p_inst);
        
            if (flag1==TRUE)
            {
            instruction_count++;
            read_variable = create_node_at_position(*p_inst, instruction_count);
            read_variable->fetch = cycle;
            read_variable->disp = cycle + 1;
            add_to_list(&dispatch_queue, read_variable);
            
        }else{
            read_over = 0;
        }
        
    }
    
    free(p_inst);
}





void dispatch_instructions(){
    
    int i = 0;
    
    common_node* dispatch_node, *dispatch_node_temporary;
    
    dispatch_node = dispatch_queue.head;
    
    
    while(dispatch_node!=NULL && schedule_queue.size!=0)
    {
        
        create_scheduling_node(dispatch_node);
        dispatch_node_temporary = dispatch_node;
        dispatch_node = dispatch_node->next;
        remove_from_list(&dispatch_queue, dispatch_node_temporary, FALSE);
        add_to_list(&schedule_queue, dispatch_node_temporary);
        schedule_queue.size++;
        
    i++;
    }
    
    
}


///////////////////////////SCHEDULE///////////////////////////////////


int checkAge(int unit){
    int count = 0;
    
    if (unit == 0){
        for (int j = 0; j<k0; j++){
            if (FU1[j]!=NULL && FU1[j]->age == 1){
                count++;
            }
            if (count>=k0){
                return 0;
            }
        }
    }
    if (unit == 1){
        for (int j = 0; j<k1; j++){
            if (FU1[j]!=NULL && FU1[j]->age == 2){
                count++;
            }
            if (count>=k1){
                return 0;
            }
        }
    }
    if (unit == 2){
        for (int j = 0; j<k2; j++){
            if (FU2[j]!=NULL && FU2[j]->age == 3){
                count++;
            }
            if (count>=k2){
                return 0;
            }
        }
    }
    
    return 1;
}




void schedule_cycle1()
{
    common_node* temp0 = function_unit1.head;
    common_node* temp1 = function_unit2.head;
    common_node* temp2 = function_unit3.head;
    
    //Do while there is room in all schedulers
    while(temp0 != NULL || temp1 != NULL || temp2 != NULL ){
        
        //Check if item can be put in k0 execute
        if (temp0!=NULL && function_unit1.available_execution>0 && temp0->src1_tag == READY && temp0->src2_tag== READY && temp0->age == READY && checkAge(0)){
            
            //Add new node to list
            function_unit1.available_execution--;
            temp0->age  = 1;
            
            //Add cycle info
            temp0->exec = cycle+1;
            
            //Store pointers for things currently in FU
            for (int j = 0; j<k0; j++){
                if (FU0[j]==NULL){
                    FU0[j] = temp0;
                    break;
                }
            }
            
            //Go to next element
            temp0 = temp0->next;
        }else if (temp0 != NULL){
            //Go to next element
            temp0 = temp0->next;
        }
        
        //Check if item can be put in k1 execute
        if (temp1!=NULL && function_unit2.available_execution>0 && temp1->src1_tag == READY && temp1->src2_tag== READY && temp1->age == READY && checkAge(1)){
            //Add new node to list
            function_unit2.available_execution--;
            temp1->age  = 2;
            
            //Add cycle info
            temp1->exec = cycle+1;
            
            //Store pointers for things currently in FU
            for (int j = 0; j<k1; j++){
                if (FU1[j]==NULL){
                    FU1[j] = temp1;
                    break;
                }
            }
            
            //Go to next element
            temp1 = temp1->next;
        }else if (temp1 != NULL){
            //Go to next element
            temp1 = temp1->next;
        }
        
        
        //Check if item can be put in k2 execute
        if (temp2!=NULL && function_unit3.available_execution>0 && temp2->src1_tag == READY && temp2->src2_tag== READY && temp2->age == READY  && checkAge(2)){
            //Add new node to list
            function_unit3.available_execution--;
            temp2->age  = 3;
            
            //Add cycle info
            temp2->exec = cycle+1;
            
            //Store pointers for things currently in FU
            for (int j = 0; j<k2; j++){
                if (FU2[j]==NULL){
                    FU2[j] = temp2;
                    break;
                }
            }
            
            //Go to next element
            temp2 = temp2->next;
        }else if (temp2 != NULL){
            //Go to next element
            temp2 = temp2->next;
        }
    }

}


void schedule_cycle2()
{

        common_node* update_node;
        update_node = function_unit1.head;
        while (update_node!=NULL)
        {
            for (int j = 0;j<cdb_size; j++){
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

///////////////////////////EXECUTE///////////////////////////////////
/*
 * updateReg
 * Update registers
 *
 * parameters:
 * none
 *
 * returns:
 * none
 */
void updateReg(){
    //Update register file
    for (int i = 0; i < temp_cdb_size; i++){
        if (register_file[temporary_cdb[i].reg].tag == temporary_cdb[i].tag){
            register_file[temporary_cdb[i].reg].tag = READY;
        }
    }
}


/*
 * removeFU
 * Removes Items from functional units
 *
 * parameters:
 * none
 *
 * returns:
 * none
 */
void removeFU(){
    //Execute k0 instructions
    for (int j= 0; j<k0; j++){
        if (FU0[j] != NULL){
            //Check if instructions is done
            if (FU0[j]->age == DONE){
                function_unit1.available_execution++;
                FU0[j] = NULL;
            }
        }
    }
    
    //Execute k1 instructions
    for (int j= 0; j<k1; j++){
        if (FU1[j] != NULL){
            //Check if instructions is done
            if (FU1[j]->age == DONE){
                function_unit2.available_execution++;
                FU1[j] = NULL;
            }
        }
    }
    
    //Execute k2 instructions
    for (int j= 0; j<k2; j++){
        if (FU2[j] != NULL){
            //Check if instructions is done
            if (FU2[j]->age == DONE){
                function_unit3.available_execution++;
                FU2[j] = NULL;
            }
        }
    }
}

/*
 * incrementTimer
 * Increment Age
 *
 * parameters:
 * none
 *
 * returns:
 * none
 */
void incrementTimer(){
    //Reset CDB bus
    temp_cdb_size = 0;
    
    //Execute k0 instructions
    for (int j= 0; j<k0; j++){
        if (FU0[j] != NULL){
            //Decrease time left
            FU0[j]->age--;
            //Check if instructions is done
            if (FU0[j]->age == 0){
                temporary_cdb[temp_cdb_size].tag = FU0[j]->dest_tag;
                temporary_cdb[temp_cdb_size].FU = 0;
                temporary_cdb[temp_cdb_size].line_number = FU0[j]->line_number;
                temporary_cdb[temp_cdb_size++].reg = FU0[j]->p_inst.dest_reg;
                //Add cycle info
                FU0[j]->state = cycle+1;
                
                //Fix up FU array
                FU0[j]->age = DONE;
            }
        }
    }
    
    //Execute k1 instructions
    for (int j= 0; j<k1; j++){
        if (FU1[j] != NULL){
            //Decrease time left
            FU1[j]->age--;
            //Check if instructions is done
            if (FU1[j]->age == 0){
                temporary_cdb[temp_cdb_size].tag = FU1[j]->dest_tag;
                temporary_cdb[temp_cdb_size].FU = 1;
                temporary_cdb[temp_cdb_size].line_number = FU1[j]->line_number;
                temporary_cdb[temp_cdb_size++].reg = FU1[j]->p_inst.dest_reg;
                
                //Add cycle info
                FU1[j]->state = cycle+1;
                //Fix up FU array
                FU1[j]->age = DONE;
            }
        }
    }
    
    //Execute k2 instructions
    for (int j= 0; j<k2; j++){
        if (FU2[j] != NULL){
            //Decrease time left
            FU2[j]->age--;
            //Check if instructions is done
            if (FU2[j]->age == 0){
                temporary_cdb[temp_cdb_size].tag = FU2[j]->dest_tag;
                temporary_cdb[temp_cdb_size].FU = 2;
                temporary_cdb[temp_cdb_size].line_number = FU2[j]->line_number;
                temporary_cdb[temp_cdb_size++].reg = FU2[j]->p_inst.dest_reg;
                //Add cycle info
                FU2[j]->state = cycle+1;
                //Fix up FU array
                FU2[j]->age = DONE;
            }
        }
    }
    
}

/*
 * exchangeCDB
 * Replace CDB
 *
 * parameters:
 * none
 *
 * returns:
 * none
 */
void exchangeCDB(){
    //Put temporary in correct
    for (int i = 0; i<temp_cdb_size ;i++){
        cdb[i] = temporary_cdb[i];
    }
    cdb_size = temp_cdb_size;
}
/*
 * orderCDB
 * Order CDB
 *
 * parameters:
 * none
 *
 * returns:
 * none
 */
void orderCDB(){
    //Temporary CDB holder
    common_data_bus tempCDB2;
    
    //Go though all elements and switch as necessary
    for(int i=0; i<temp_cdb_size; i++){
        for(int j=i; j<temp_cdb_size; j++){
           	if(temporary_cdb[i].line_number > temporary_cdb[j].line_number){
                tempCDB2=temporary_cdb[i];
                temporary_cdb[i]=temporary_cdb[j];
               	temporary_cdb[j]=tempCDB2;
           	}
        }
    }
    
}

/*
 * executeInstructions1
 * Execute Instcutions
 *
 * parameters:
 * none
 *
 * returns:
 * none
 */
void executeInstructions1(){
    //Fix aging of instructions in execute
    incrementTimer();
    //Update register
    updateReg();
    //Give up FU
    removeFU();
    //Order the CDB
    orderCDB();
}

/*
 * executeInstructions2
 * Execute Instcutions
 *
 * parameters:
 * none
 *
 * returns:
 * none
 */
void executeInstructions2(){
    //Create teh correct CDB
    exchangeCDB();
}
///////////////////////////STATE UPDATE////////////////////////////////

/*
 * removeScheduler
 * Remove completed instructions from scheduler
 *
 * parameters:
 * none
 *
 * returns:
 * none
 */
void removeScheduler(){
    //Node for access to scheduler
    common_node* updateNode;
    
    for(int j = 0;j<cdb_size; j++){
        if (cdb[j].FU == 0){
            //Navigate though k0 scheduler
            updateNode = function_unit1.head;
            while (updateNode!=NULL){
                if (cdb[j].tag==updateNode->dest_tag){
                    updateNode->retire = cycle;
                    // updateROBfromNode(updateNode);
                    remove_from_list(&function_unit1, updateNode,TRUE);
                    break;
                }
                //go to next node
                updateNode = updateNode->next;
            }
        }else if(cdb[j].FU == 1){
            //Navigate though k0 scheduler
            updateNode = function_unit2.head;
            while (updateNode!=NULL){
                if (cdb[j].tag==updateNode->dest_tag){
                    updateNode->retire = cycle;
                    //      updateROBfromNode(updateNode);
                    remove_from_list(&function_unit2, updateNode,TRUE);
                    break;
                }
                //go to next node
                updateNode = updateNode->next;
            }
        }else if (cdb[j].FU == 2){
            //Navigate though k0 scheduler
            updateNode = function_unit3.head;
            while (updateNode!=NULL){
                if (cdb[j].tag==updateNode->dest_tag){
                    updateNode->retire = cycle;
                    //      updateROBfromNode(updateNode);
                    remove_from_list(&function_unit3, updateNode,TRUE);
                    break;
                }
                //go to next node
                updateNode = updateNode->next;
            }
            
        }
    }
}

/*
 * retireInstructions
 * Retire completed instruction in ROB
 *
 * parameters:
 * none
 *
 * returns:
 * none
 */
void retireInstructions(){
    
    //all instructions done
    if (read_over == 0){
        flag = 0;
    }
}


/*
 * updateState1
 * Update States
 *
 * parameters:
 * none
 *
 * returns:
 * none
 */
void updateState1(){
    //retireInstructions(); //change 2.2
    //   markROBDone();
}

/*
 * updateState2
 * Update States
 *
 * parameters:
 * none
 *
 * returns:
 * none
 */
void updateState2(){
    removeScheduler();
    retireInstructions(); //change 2.2
}




void setup_proc(uint64_t rIn, uint64_t k0In, uint64_t k1In, uint64_t k2In, uint64_t fIn)
{

    r = rIn;
    k0 = k0In;
    k1 = k1In;
    k2 = k2In;
    f = fIn;
    printf("INST\tFETCH\tDISP\tSCHED\tEXEC\tSTATE\tRETIRE\n");
    
    for (int i = 0; i<128; i++){
        register_file[i].tag = READY;
    }
    
    cdb = (common_data_bus *) malloc((k0+k1+k2+10)*sizeof(common_data_bus));
    temporary_cdb = (common_data_bus *) malloc((k0+k1+k2+10)*sizeof(common_data_bus));

    FU0 = (common_node**) malloc(k0*sizeof(common_node*));
    FU1 = (common_node**) malloc(k1*sizeof(common_node*));
    FU2 = (common_node**) malloc(k2*sizeof(common_node*));
    
  
    dispatch_queue = {NULL, NULL, (int)1000000 , (int)0};
    schedule_queue =  {NULL, NULL, (int)(2*(k0+k1+k2)) , (int)(k0+k1+k1)};

}

void run_proc(proc_stats_t* p_stats)
{
    cycle = 0;
    
    instruction_count = 0;
    
    while(read_over){
        
        // updateState2();
        // executeInstructions2();
        // scheduleInstructions2();
        dispatch_instructions();
        
        cycle++;
        
        //    updateState1();
        //   executeInstructions1();
       // scheduleInstructions1();
        instruction_fetch();
    }
    
    cycle = cycle - 1;
}

void complete_proc(proc_stats_t *p_stats) {
    //stats
    p_stats->retired_instruction = instruction_count;
    p_stats->cycle_count = cycle;
    p_stats->avg_inst_retired = ((double)instruction_count)/cycle;
    
    printf("\n");
    
    //Free allocated memory
    free(cdb);
    free(temporary_cdb);
    free(FU0);
    free(FU1);
    free(FU2);
}