//main header

#ifndef __MAIN_H__
#define __MAIN_H__

//Includes:
#include <stdio.h> 
#include <stdlib.h> 
#include <stdbool.h>
#include <string.h>


//Defines:
#define NUM_OF_CORES 4
#define NUM_OF_REGS 16
#define PIPE_LEN 5
#define MEM_SIZE 1<<20
#define CACHE_SIZE 256
#define IMEM_SIZE 1024
#define FLUSH_TIME 64
#define NOT_FOUND -1


//Typedefs:
//Enum to organize the input argv
typedef enum _arg {
	IMEM0 = 1,
	IMEM1,
	IMEM2,
	IMEM3,
	MEMIN,
	MEMOUT,
	REGOUT0,
	REGOUT1,
	REGOUT2,
	REGOUT3,
	TRACE0,
	TRACE1,
	TRACE2,
	TRACE3,
	BUSTRACE,
	DSRAM0,
	DSRAM1,
	DSRAM2,
	DSRAM3,
	TSRAM0,
	TSRAM1,
	TSRAM2,
	TSRAM3,
	STATS0,
	STATS1,
	STATS2,
	STATS3,
	ARG_COUNT
} Arg;

//All of the main functions return a status enum for debugging
typedef enum _status {
	INVALID_STATUS_CODE = -1,
	SUCCESS = 0,
	WRONG_ARGUMENT_COUNT,
	FOPEN_FAIL,
	WRONG_OPCODE,
	WRONG_RD,
	WRONG_RS,
	WRONG_RT,
	WRONG_ADDRESS
} Status;

typedef enum _core {
	CORE0,
	CORE1,
	CORE2,
	CORE3
} Core;

typedef enum _pipe {
	FETCH,
	DECODE,
	EXECUTE,
	MEM,
	WRITE_BACK
} Pipe;

typedef enum _opcode {
	ADD,
	SUB,
	AND,
	OR,
	XOR,
	MUL,
	SLL,
	SRA,
	SRL,
	BEQ,
	BNE,
	BLT,
	BGT,
	BLE,
	BGE,
	JAL,
	LW,
	SW,
	LL,
	SC,
	HALT
} Opcode;

typedef enum _bus_origid {
	//CORE0, //same as Core enum
	//CORE1, //same as Core enum
	//CORE2, //same as Core enum
	//CORE3, //same as Core enum
	MAIN_MEMORY=4
} Bus_origid;

typedef enum _bus_cmd {
	NO_COMMAND,
	BusRd,
	BusRdX,
	Flush,
} Bus_cmd;

typedef enum _mem_stage {
	CACHE_ACCESS,
	WAIT_FOR_BUS,
	BUS_ACCESS
} Mem_stage;

typedef enum _msi {
	INVALID,
	SHARE,
	MODIFIED
}MSI;

//type used for tsram
typedef struct _tsram {
	int MSI;
	int tag;
}TSRAM;

//type used for bus interface
typedef struct _bus {
	Bus_origid origid;
	Bus_cmd cmd;
	int addr;
	int data;
}BUS;

//type used for instructions currently in pipeline
typedef struct _instruction {
	int opcode;
	int rd;
	int rs;
	int rt;
	int imm;
	int pc;
}Instruction;

//type used for sc/ll handling
typedef struct _load_store_conditional {
	int address;
	bool sc_dirty;
}LOAD_STORE_CONDITIONAL;




//Global variables:
//Input arguments
char args[ARG_COUNT][_MAX_PATH] = { "sim.exe","imem0.txt","imem1.txt","imem2.txt","imem3.txt",
		"memin.txt","memout.txt","regout0.txt","regout1.txt","regout2.txt","regout3.txt",
		"core0trace.txt","core1trace.txt","core2trace.txt","core3trace.txt","bustrace.txt",
		"dsram0.txt","dsram1.txt","dsram2.txt","dsram3.txt","tsram0.txt","tsram1.txt","tsram2.txt","tsram3.txt",
		"stats0.txt","stats1.txt","stats2.txt","stats3.txt" };

//Clock cycles
int cycle = 0;

//Main memory
int main_memory[MEM_SIZE] = { 0 };

//Bus
BUS cur_bus = { 0 };
BUS updated_bus = { 0 };
int flush_cycles = 0;
bool print_bus_trace = false;

//Core status
int pc[NUM_OF_CORES] = { 0 };
int cycles_count[NUM_OF_CORES] = { 0 };
bool core_done[NUM_OF_CORES] = { false };
int decode_stall[NUM_OF_CORES] = { 0 };
int mem_stall[NUM_OF_CORES] = { 0 };
int decode_stall_count[NUM_OF_CORES] = { 0 };
int mem_stall_count[NUM_OF_CORES] = { 0 };
Mem_stage mem_stage[NUM_OF_CORES] = { CACHE_ACCESS };
bool branch[NUM_OF_CORES] = { false };

//Core regs
int cur_regs[NUM_OF_CORES][NUM_OF_REGS] = { 0 };
int updated_regs[NUM_OF_CORES][NUM_OF_REGS] = { 0 };

//Core instructions
int imem[NUM_OF_CORES][IMEM_SIZE] = { 0 };
Instruction pipeline[NUM_OF_CORES][PIPE_LEN] = { 0 };
int instructions_count[NUM_OF_CORES] = { 0 };
LOAD_STORE_CONDITIONAL watch[NUM_OF_CORES] = { 0 };

//Core cache
int dsram[NUM_OF_CORES][CACHE_SIZE] = { 0 };
TSRAM tsram[NUM_OF_CORES][CACHE_SIZE] = { INVALID };
int read_hit_count[NUM_OF_CORES] = { 0 };
int write_hit_count[NUM_OF_CORES] = { 0 };
int read_miss_count[NUM_OF_CORES] = { 0 };
int write_miss_count[NUM_OF_CORES] = { 0 };
int abort_cache=-1;


//Function Handles:
//Updates global args array with given arguments
void update_args(char* argv[]);

//Reads imem files and stores them in global imem array
Status init_imems();

//Reads memin file and stores it in global main_memory array
Status init_main_memory();

//Initiates global pipeline array (and watch for sc/ll) with invalid (-1) values
void init_pipeline();

//Simulates a single core operation for a single cycle
Status core(Core core_num, FILE* trace_file);

//Updates global pipeline array with the instruction from imem for the current pc
Status fetch(Core core_num);

//Detects RAW hazards and stalls if neccessary, does branch resolusion
Status decode(Core core_num);

//Updates the updated_regs array with the regs after the execution of the command
Status execute(Core core_num);

//input core number and implementing MSI protocol per cycle
//returns if the action succeds or if something went wrong
Status mem(Core core_num);

//updates cur_regs with updated_regs
Status write_back(Core core_num);

//Advances pipeline stages and insert NOP's on stalls
Status advance_pipeline(Core core_num);

//Looks for RAW hazards and returns number of decode stalls
int detect_hazards(Core core_num, Pipe stage);

//Checks if branches taken and updates pc if so, returns true for taken and false otherwise
bool branch_resolution(Core core_num, Instruction inst);

//updates rd,rs,rt values for an instruction, interprets $imm
void get_reg_values(Core core_num, Instruction inst, int* rd_value, int* rs_value, int* rt_value);

//move pipeline stage on core from one stage to another
void advance_stage(Core core_num, Pipe from, Pipe to);

//write current trace line
void print_trace(Core core_num, FILE* file);

//writes output file according to file_enum
Status print_file(Arg file_enum);

//input is the bus and it checks if there is a block in another core wich is modified and has the wanted address
// if there is such a core so it retuns it's number
int search_address_cache(BUS* bus);

//input core number and it checks if there isn't another core which owns the bus.
// if the bus free it returns true and if the bus is occupied it returns false
bool free_access_bus(int core_num);

//after the bus finishes an action on it which includes BusRd/X and flush so the function updates the fields of the bus to fulfill after MSI protocol
void bus_response();

//input pointer to the bus_trace. Prints every action that occurs on the bus includding it's cycle and the fields of the bus
void print_bus(FILE* bus_trace);

//input is the address wich we want to store there data, the core number which is tryng to do so with store conditional opcode.
//returns true if the store conditional will succeed or will it fail.
bool sc_func(int address, int core_num);

//simulates a flip flop on the bus. after every cycle the updated bus transfer all it's updates to the current bus.
void advance_bus();

#endif // __MAIN_H__
