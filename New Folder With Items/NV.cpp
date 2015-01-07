#include "procsim.hpp"
#include <queue>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

static int inst_index = 1, new_tag = 1;

int32_t current_time = 0;

uint64_t check = 0, DQ_max_size = 0, limit = 0, previous = 0;

bool done = 0;

using namespace std;

queue <proc_inst_t> dispatch_queue, fetch;
/////////////////////////////////////////////////////////////////////////

typedef struct _comp_inst
{
    bool free = 1;
    int index = 1000000;
    int32_t dest_tag = 0;
    int fu = -2;
    int number = -1;
    uint64_t exec_time = 0;
    
} comp_inst;

comp_inst completed_inst[DEFAULT_K0+DEFAULT_K1+DEFAULT_K2];
/////////////////////////////////////////////////////////////////////////

typedef struct _Inst_Log{
    
    uint64_t Fetch = 0;
    uint64_t Dispatch = 0;
    uint64_t Sched = 0;
    uint64_t Exec = 0;
    uint64_t SU = 0;
    
} Inst_Log;

Inst_Log inst_log[100001];
/////////////////////////////////////////////////////////////////////////

typedef struct _functional_unit{
    
    int index = -2 ;
    
    uint64_t time = 0;
    
    bool busy = 0;
    int32_t dest_tag = 0;
    int32_t dest_reg = 0;
    
}	functional_unit;

functional_unit fu0[DEFAULT_K0], fu1[DEFAULT_K1], fu2[DEFAULT_K2];

int32_t *newnode = (int32_t*)malloc(sizeof(int32_t));
/////////////////////////////////////////////////////////////////////////

typedef struct _reservation_station{
    
    int32_t insertion_time = 1000000;
    
    int index = -2;
    int latency = 0;
    
    bool free = 1;
    bool del = 0;
    
    bool executed = 0;
    
    bool fire = 0;
    
    int32_t fu = 0;
    
    int32_t dest_reg = 0;
    int32_t dest_tag = 0;
    
    bool src_ready[2] = {0,0};
    int32_t src_tag[2] = {0, 0};													////////////////////////////##############CHECK!!!!!!!!
    
} reservation_station;

reservation_station rs[2*(DEFAULT_K0+DEFAULT_K1+DEFAULT_K2)];
/////////////////////////////////////////////////////////////////////////
typedef struct _common_data_bus{
    
    int index = -2;
    
    bool busy = 0;
    int32_t tag = 0;
    int32_t reg_number = -1;
    
} common_data_bus;

common_data_bus cdb[DEFAULT_R];
/////////////////////////////////////////////////////////////////////////
typedef struct _reg_file{
    
    int32_t tag = 0;
    bool ready = 1;
    
} reg_file;

reg_file rf[128];
/////////////////////////////////////////////////////////////////////////

/**
 * Subroutine for initializing the processor. You many add and initialize any global or heap
 * variables as needed.
 * XXX: You're responsible for completing this routine
 *
 * @r ROB size
 * @k0 Number of k0 FUs
 * @k1 Number of k1 FUs
 * @k2 Number of k2 FUs
 * @f Number of instructions to fetch
 */
void setup_proc(uint64_t r, uint64_t k0, uint64_t k1, uint64_t k2, uint64_t f)
{
    
    
}

/**
 * Subroutine that simulates the processor.
 *   The processor should fetch instructions as appropriate, until all instructions have executed
 * XXX: You're responsible for completing this routine
 *
 * @p_stats Pointer to the statistics structure
 */
void run_proc(proc_stats_t* p_stats)
{
    uint64_t rs_size = 2*(DEFAULT_K0+DEFAULT_K1+DEFAULT_K2);
    uint64_t location = 0;
    uint64_t i = 0, j = 0;
    uint64_t no_cdb = DEFAULT_R;
    uint64_t no_fus = DEFAULT_K0+DEFAULT_K1+DEFAULT_K2;
    
    proc_inst_t current_instruction;
    proc_inst_t	 dispatch_queue_front;
    
    while(!done)														//#########################	 Check conditions for done. No Pending inst in FUs, SQ, DQ, etc.
    {
        p_stats->cycle_count++;
        /////////////////////////////////////////////////	STATE UPDATE	/////////////////////////////////////////////////////////////////////////////////
        
        /////////////////////////////////////////////////DELETIION!!!!!!!!!!!!!!!!!!!!!!
        
        location = 0;
        while(location<rs_size)
        {
            if(rs[location].del == 1)
            {
                if(rs[location].latency == 0)
                {
                    inst_log[rs[location].index].SU = p_stats->cycle_count;
                    rs[location].latency++;
                }
                else if(rs[location].latency == 1)
                    rs[location].latency++;
                else if(rs[location].latency == 2)
                {
                    rs[location].free = 1;
                    rs[location].del = 0;
                    rs[location].executed = 0;
                    rs[location].latency = 0;
                    p_stats->retired_instruction++;
                }
            }
            
            location++;
        }
        
        
        
        /////////////////////////////////////////////	EXECUTION UNIT 	/////////////////////////////////////////////////////////////////////////////////////
        location = 0;
        //////////////////////////////////////////////// SCHEDULING and WAKING UP / MARKING for FIRE!! ( All the same) /////////////////////////////////////////////
        
        while(location<rs_size)
        {
            ///////////////////////////////////////////////		WAKING UP AN INSTRUCTION ///////////////////////////////////////////////////////////////////////
            
            if((rs[location].src_ready[0]==1) && (rs[location].src_ready[1]==1) && (rs[location].free != true) && rs[location].executed != 1)
            {
                if(rs[location].fu == -1)
                    rs[location].fu = 1;
                
                if(rs[location].fu == 0 )
                {
                    for(i = 0;i<DEFAULT_K0; i++)
                    {	//p_stats->cycle_count++;
                        if(fu0[i].busy == 0)								//##############   CHECK if this is done here!!!
                        {
                            //##############	It(making fu busy and transfering dest tag) should happen here or down. Where??
                            fu0[i].busy = 1;								//##############   CHECK if this is done here!!!
                            fu0[i].dest_tag = rs[location].dest_tag;
                            fu0[i].dest_reg = rs[location].dest_reg;
                            
                            ///////////////////////////////////			LOG /////////////////////////
                            fu0[i].index = rs[location].index;
                            fu0[i].time = p_stats->cycle_count;
                            inst_log[rs[location].index].Exec = p_stats->cycle_count;
                            rs[location].fire = 1;
                            rs[location].executed = 1;
                            
                            break;
                        }
                    }
                    
                }
                else if(rs[location].fu == 1 )
                {
                    for(i = 0;i<DEFAULT_K1; i++)////////////////////////?????????????????busy on;y 4 times
                    {
                        if(fu1[i].busy == 0)
                        {
                            fu1[i].busy = 1;								//##############   CHECK if this is done here!!!
                            fu1[i].dest_tag = rs[location].dest_tag;
                            fu1[i].dest_reg = rs[location].dest_reg;
                            
                            ///////////////////////////////////			LOG /////////////////////////
                            fu1[i].index = rs[location].index;
                            fu1[i].time = p_stats->cycle_count;
                            inst_log[rs[location].index].Exec = p_stats->cycle_count;
                            rs[location].fire = 1;
                            rs[location].executed = 1;
                            
                            break;
                        }
                    }
                }
                else if(rs[location].fu == 2)
                {
                    for(i = 0;i<DEFAULT_K2; i++)//////////////////////////////????????????????busy 0 times!!
                    {
                        
                        if(fu2[i].busy == 0)
                        {
                            fu2[i].busy = 1;								//##############   CHECK if this is done here!!!
                            fu2[i].dest_tag = rs[location].dest_tag;
                            fu2[i].dest_reg = rs[location].dest_reg;
                            
                            ///////////////////////////////////			LOG /////////////////////////
                            fu2[i].index = rs[location].index;
                            fu2[i].time = p_stats->cycle_count;
                            inst_log[rs[location].index].Exec = p_stats->cycle_count;
                            rs[location].fire = 1;
                            rs[location].executed = 1;
                            
                            break;
                        }
                    }
                }
                
            }
            
            location++;
        }
        
        
        
        
        
        
        
        /////////////////////////////////// CDB Updating SQ
        location = 0;
        while(location<rs_size)
        {
            for(i = 0; i< no_cdb; i++)
            {
                for(j = 0; j<2; j++)
                {
                    if(cdb[i].busy == 1 && cdb[i].tag == rs[location].src_tag[j] && cdb[i].tag != 0 )
                    {
                        
                        rs[location].src_ready[j] = 1;
                        //cdb[i].busy = 0;				////CHECK IF IT SHOULD BE 0
                    }
                }
            }
            location++;
        }
        //////////////////////////////   		CDB Updating Reg File
        for(i = 0; i<no_cdb; i++)
        {
            location = 0;
            while(location<128)
            {
                if(cdb[i].busy==1 && location == uint64_t(cdb[i].reg_number))
                {
                    
                    if(rf[location].tag == cdb[i].tag && cdb[i].tag != 0)//iF TAG MATCHES, LATES RESULT HAS COME, UPDATE, MAKE CDB BUSY 0
                    {									// Else, go check RS if any inst looking for result with THAT tag, update make cdb busy 0.
                        rf[location].ready = 1;
                        
                    }
                    
                }
                
                location++;
            }
            
        }
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        //////////////////////////		INITIALIZE COMPLETED INST QUEUE EVERYTIME ( STUPID IDEA, FIND AN EASIER WAY FOR EFFICIENCY!)
        for(i = 0; i<no_cdb; i++)
        {
            cdb[i].busy = 0;
        }
        for(j = 0; j<no_fus; j++)
        {
            completed_inst[j].free = 1;
            completed_inst[j].index = 1000000;
            completed_inst[j].dest_tag = 0;
            completed_inst[j].fu = -2;
            completed_inst[j].number = -1;
            completed_inst[j].exec_time = 0;
        }
        
        for(i = 0;i<DEFAULT_K0; i++)
        {
            if(fu0[i].busy)
            {
                
                //////////////// F U C K  T H I S  S H I T !!!!!!!!!!!!!!!!!!
                for(j = 0; j<no_fus; j++)
                {
                    if(completed_inst[j].free)
                    {
                        completed_inst[j].free = 0;
                        completed_inst[j].number = i;
                        completed_inst[j].fu = 0;
                        completed_inst[j].index = fu0[i].index;
                        completed_inst[j].dest_tag = fu0[i].dest_tag;
                        completed_inst[j].exec_time = fu0[i].time;
                        break;
                    }
                }
                
                
            }
            
        }
        for(i = 0;i<DEFAULT_K1; i++)
        {
            if(fu1[i].busy)
            {
                
                for(j = 0; j<no_fus; j++)
                {
                    if(completed_inst[j].free)
                    {
                        completed_inst[j].free = 0;
                        completed_inst[j].number = i;
                        completed_inst[j].fu = 1;
                        completed_inst[j].index = fu1[i].index;
                        completed_inst[j].dest_tag = fu1[i].dest_tag;
                        completed_inst[j].exec_time = fu1[i].time;
                        break;
                    }
                }
                
                
                
            }
            
        }
        for(i = 0;i<DEFAULT_K2; i++)
        {
            if(fu2[i].busy)
            {
                
                for(j = 0; j<no_fus; j++)
                {
                    if(completed_inst[j].free)
                    {
                        completed_inst[j].free = 0;
                        completed_inst[j].number = i;
                        completed_inst[j].fu = 2;
                        completed_inst[j].index = fu2[i].index;
                        completed_inst[j].dest_tag = fu2[i].dest_tag;
                        completed_inst[j].exec_time = fu2[i].time;
                        break;
                    }
                }
                
                
                
            }
            
        }
        
        for(uint64_t ii = 0; ii < no_fus; ii++)
        {
            comp_inst temp;
            for(uint64_t jj = 0; jj < (no_fus-ii-1); jj++)
            {
                
                if(completed_inst[jj].index> completed_inst[jj + 1].index && !completed_inst[jj].free && !completed_inst[jj+1].free)
                {
                    
                    temp = completed_inst[jj];
                    completed_inst[jj] = completed_inst[jj + 1];
                    completed_inst[jj + 1] = temp;
                    
                }
            }
        }
        
        
        /////////////////// SORT COMPLETED INST BY TAG/INDEX ORDER
        for(uint64_t ii = 0; ii < no_fus; ii++)
        {
            comp_inst temp;
            for(uint64_t jj = 0; jj < (no_fus-ii-1); jj++)
            {
                
                if(completed_inst[jj].exec_time > completed_inst[jj + 1].exec_time && !completed_inst[jj].free && !completed_inst[jj+1].free)
                {
                    
                    temp = completed_inst[jj];
                    completed_inst[jj] = completed_inst[jj + 1];
                    completed_inst[jj + 1] = temp;
                    
                }
            }
        }
        
        ////////////////////////// Assigning cdb's in tag order.
        for(j = 0; j<no_fus; j++)
        {
            
            for(i = 0; i<no_cdb; i++)
            {
                if(!cdb[i].busy)
                {
                    
                    if(!completed_inst[j].free)
                    {
                        completed_inst[j].free = 1;
                        cdb[i].busy  = 1;			//Coz definitely one of the below 3 fu's gotta broadcast and make cdb[i] busy!
                        if(completed_inst[j].fu == 0)
                        {
                            fu0[completed_inst[j].number].busy = 0;
                            cdb[i].tag = completed_inst[j].dest_tag;
                            cdb[i].reg_number = fu0[completed_inst[j].number].dest_reg;
                            location = 0;
                            while(location<rs_size)
                            {
                                if(rs[location].index == fu0[completed_inst[j].number].index && rs[location].dest_tag != 0 && fu0[completed_inst[j].number].dest_tag != 0)
                                {
                                    rs[location].del = 1;
                                    rs[location].fire = 0;
                                    break;
                                }
                                location++;
                            }
                            
                            cdb[i].index = fu0[completed_inst[j].number].index;
                            
                            break;
                        }
                        else if(completed_inst[j].fu == 1)
                        {
                            fu1[completed_inst[j].number].busy = 0;
                            cdb[i].tag = completed_inst[j].dest_tag;
                            cdb[i].reg_number = fu1[completed_inst[j].number].dest_reg;
                            location = 0;
                            while(location<rs_size)
                            {
                                if(rs[location].index == fu1[completed_inst[j].number].index && rs[location].dest_tag != 0 && fu1[completed_inst[j].number].dest_tag != 0)
                                {
                                    rs[location].del = 1;
                                    rs[location].fire = 0;
                                    break;
                                }
                                location++;
                            }
                            
                            cdb[i].index = fu1[completed_inst[j].number].index;
                            
                            break;
                            
                        }
                        else if(completed_inst[j].fu == 2)
                        {
                            fu2[completed_inst[j].number].busy = 0;
                            cdb[i].tag = completed_inst[j].dest_tag;
                            cdb[i].reg_number = fu2[completed_inst[j].number].dest_reg;
                            location = 0;
                            while(location<rs_size)
                            {
                                if(rs[location].index == fu2[completed_inst[j].number].index && rs[location].dest_tag != 0 && fu2[completed_inst[j].number].dest_tag != 0)
                                {
                                    rs[location].del = 1;
                                    rs[location].fire = 0;
                                    break;
                                }
                                location++;
                            }
                            
                            cdb[i].index = fu2[completed_inst[j].number].index;
                            
                            break;
                            
                        }
                    }
                    
                }
            }
        }
        
        
        
        
        
        //////////////////////////////////////////////// RESERVATION STATION INSTRUCTION INSERTION /////////////////////////////////////////////////////
        
        location = 0;
        while(location<rs_size)
        {
            
            if(rs[location].free && dispatch_queue.size() != 0 )
            {
                rs[location].free = 0;
                rs[location].fire = 0;
                
                dispatch_queue_front = dispatch_queue.front();
                dispatch_queue.pop();														//Chuck instruction out of DQ after copying it to buffer.
                
                //////////////////////////////////////////		LOG /////////////////////////////
                inst_log[dispatch_queue_front.index].Sched = p_stats->cycle_count;
                if(dispatch_queue_front.dest_reg != -1)
                {
                    rs[location].dest_reg = dispatch_queue_front.dest_reg;
                    rs[location].dest_tag = new_tag;
                    rf[dispatch_queue_front.dest_reg].tag = new_tag;
                    rf[dispatch_queue_front.dest_reg].ready = 0;
                    
                    new_tag++;
                }
                else
                {
                    rs[location].dest_tag  = -1;
                    rs[location].dest_reg  = -1;
                    //new_tag++;
                }
                
                rs[location].index = dispatch_queue_front.index;
                
                rs[location].insertion_time = p_stats->cycle_count;
                
                for(i = 0; i<2; i++)
                {
                    if(dispatch_queue_front.src_reg[i]!= -1)
                    {
                        if(rf[dispatch_queue_front.src_reg[i]].ready == true)
                        {
                            rs[location].src_ready[i] = rf[dispatch_queue_front.src_reg[i]].ready;
                        }
                        else
                        {
                            rs[location].src_tag[i] = rf[dispatch_queue_front.src_reg[i]].tag;			///##################### CHECK
                            
                            if(rs[location].dest_tag == rf[dispatch_queue_front.src_reg[i]].tag)
                                rs[location].src_ready[i] = 1;
                            else
                                rs[location].src_ready[i] = 0;
                        }
                    }
                    else
                    {
                        rs[location].src_ready[i] = 1;
                    }
                    
                }					
                
                
                rs[location].fu = dispatch_queue_front.op_code;
            }
            
            
            location++;
            
        }
        ///////////////////////////////////////////// SQ time ORDER SORTING ////////////////////////////////////////
        for(uint64_t ii = 0; ii < rs_size; ii++)
        {
            reservation_station temp;
            for(uint64_t jj = 0; jj < (rs_size-ii-1); jj++)
            {
                
                if(rs[jj].insertion_time > rs[jj + 1].insertion_time && !rs[jj].free && !rs[jj+1].free)
                {
                    
                    temp = rs[jj];
                    rs[jj] = rs[jj + 1];
                    rs[jj + 1] = temp;
                    
                }
            }
        }
        
        
        
        
        //////////////////////////////////////////////////////////			DISPATCH 					////////////////////////////////////////////////
        for(i = 0; i<4; i++)
        {
            if(fetch.size()!=0)
            {
                current_instruction = fetch.front();
                dispatch_queue.push(current_instruction);
                fetch.pop();
                if(dispatch_queue.size() > DQ_max_size)
                    DQ_max_size = dispatch_queue.size();
                
                p_stats->avg_disp_size += dispatch_queue.size();
                
                //////////////////////////////////////			LOG 			//////////////////////
                
                inst_log[current_instruction.index].Dispatch = p_stats->cycle_count;
            }
            
            
            //inst_index++;
        }
        
        //printf("Inst index %d 	Queue size %ld	I DestReg %d 	I SrcReg 1 %d 	I SrcReg 2 %d \n", inst_index, dispatch_queue.size(), current_instruction.dest_reg, current_instruction.src_reg[0], current_instruction.src_reg[1]);
        
        
        
        //////////////////////////////////////////////////Fill in Dispatch_Queue by FETCH RATE F ////////////////////////////////////////////////////////		
        for(i = 0; i<4; i++)
        {
            if(read_instruction(&current_instruction))
            {
                current_instruction.index = inst_index;
                fetch.push(current_instruction);
                //////////////////////////////////////			LOG 			//////////////////////
                
                inst_log[inst_index].Fetch = p_stats->cycle_count;
                
                
                
                inst_index++;
            }
        }
        
        check++;
        //printf("Check %ld \n", check);
        if(p_stats->cycle_count == 49)////////////////////////////////##########################Check conditions!!!!!!!!!
        {
            done = 1;
            
            location = 0;
            
            printf("			RESERVATION STATION cycle  %ld\n \n", p_stats->cycle_count);
            while(location<rs_size)
            {
                printf("Location %ld; Free %d; FU %d; D Reg %d; D tag %d; S1 tag %d; S1 ready %d; S2 tag %d; S2 ready %d; index %d fire %d \n",location,rs[location].free,rs[location].fu,rs[location].dest_reg, rs[location].dest_tag, rs[location].src_tag[0], rs[location].src_ready[0], rs[location].src_tag[1], rs[location].src_ready[1], rs[location].index, rs[location].fire);
                location++;
            }
            ///////////////////////////////////
            printf("\n 			FUNCTIONAL UNITS \n\n");
            
            printf("\n 			FUNCTIONAL UNIT 0 \n");
            for(i = 0; i< DEFAULT_K0; i++)
            {
                printf("FU0 - %ld : Busy %d; DTag %d; DReg %d; Index %d \n",i+1, fu0[i].busy, fu0[i].dest_tag, fu0[i].dest_reg, fu0[i].index);
            }
            printf("\n 			FUNCTIONAL UNIT 1 \n");
            for(i = 0; i< DEFAULT_K1; i++)
            {
                printf("FU1 - %ld : Busy %d; DTag %d; DReg %d Index %d \n",i+1, fu1[i].busy, fu1[i].dest_tag, fu1[i].dest_reg, fu1[i].index);
            }
            printf("\n 			FUNCTIONAL UNIT 2 \n");
            for(i = 0; i< DEFAULT_K2; i++)
            {
                printf("FU2 - %ld : Busy %d; DTag %d; DReg %d Index %d \n",i+1, fu2[i].busy, fu2[i].dest_tag, fu2[i].dest_reg, fu2[i].index);
            }
            ////////////////////////////////
            printf("\n 			CDB \n\n");
            location = 0;
            while(location<no_cdb)
            {
                printf("CDB %ld ; Busy %d; Tag %d; Reg Number %d Index %d\n",location, cdb[location].busy, cdb[location].tag, cdb[location].reg_number, cdb[location].index);
                location++;
            }
            ////////////////////////////////
            printf("\n 			Reg File\n\n");
            location = 0;
            while(location<20)
            {
                //printf("No %ld;Ready %d; Tag %d\n", location, rf[location].ready, rf[location].tag);
                location++;
            }
            //////////////////////////////
            location = 0;
            while(location<no_fus)
            {
                printf("No %ld;free %d; index %d; Time %ld; Dtag %d; FU%d; Number %d \n", location, completed_inst[location].free, completed_inst[location].index,completed_inst[location].exec_time, completed_inst[location].dest_tag, completed_inst[location].fu, completed_inst[location].number);
                location++;
            }
            ///////////////////////////////
            location =1;
            printf( " Inst 	Fetch 	   Dispatch 	   Schedule 		Execute   State Update \n\n");
            while(location<uint64_t(inst_index))
            {
                printf(" %ld 				%ld		%ld		%ld		%ld		%ld		\n", location,inst_log[location].Fetch,inst_log[location].Dispatch,inst_log[location].Sched,inst_log[location].Exec,inst_log[location].SU);
                
                location++;
            }
        }
    }
}

/**
 * Subroutine for cleaning up any outstanding instructions and calculating overall statistics
 * such as average IPC, average fire rate etc.
 * XXX: You're responsible for completing this routine
 *
 * @p_stats Pointer to the statistics structure
 */
void complete_proc(proc_stats_t *p_stats) 
{
    p_stats->avg_disp_size = p_stats->avg_disp_size/p_stats->cycle_count;
    p_stats->max_disp_size = DQ_max_size;
}
