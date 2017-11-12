#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX 100
int parentheses = 0;
char *temp = NULL;
char **token = NULL;
char **unparsedArray = NULL;

char *buffer = NULL;
char *cleaned = NULL;
char *toParse = NULL;
int k = 0;
FILE *fptr;

struct Instruction {
	int opcode;
	int rs;
	int rt;
	int rd;
	int imm;
	int offset;
};

int regNumConverter(char*);
int opcodeConverter(char*);
char *progScanner(char*);
struct Instruction parser(char *);


struct Latch {
  struct Instruction instruction;
  int opcode;
  int rs;
  int rt;
  int immediate;
  int output;
  bool ready;
};

struct Instruction instructionMemory[512];
int dataMemory[512];

struct Latch ifLatch;
struct Latch idLatch;
struct Latch exLatch;
struct Latch memLatch;

bool isBEQ = false;   //Flag to check if BEQ is being executed
int reg[32];

struct Cycle{
  int inst;
  int id;
  int ex;
  int mem;
  int wb;
  int total;
};

struct Cycle cycle;

int exIntCycle = 0;
int memIntCycle = 0;

int  pc=0;

int c,m,n;

enum allOpcodes {add, addi, sub, mult, beq, lw, sw};

void IF();
void id();
void ex();
void mem();
void wb();
void testprint(struct Latch latch);

bool start = true;

int main(){
  cycle.id = 0;
  cycle.inst = 0;
  cycle.ex = 0;
  cycle.mem = 0;
  cycle.wb = 0;
  cycle.total = 0;
  c = 1;
  m = 1;
  n = 1;
  reg[16] = 4;
  reg[17] = 7;
  dataMemory[6] = 563;
  idLatch.instruction.opcode = lw;
  idLatch.instruction.rs = 16;
  idLatch.instruction.imm = 8;
  idLatch.instruction.rt = 17;
  idLatch.instruction.rd = 18;
  idLatch.rs = reg[idLatch.instruction.rs];
  idLatch.rt = reg[idLatch.instruction.rt];
  idLatch.opcode = idLatch.instruction.opcode;
  idLatch.immediate = idLatch.instruction.imm;
  idLatch.ready = true;
  exLatch.ready = false;
  memLatch.ready = false;
  exLatch.output = 0;
  ex();
  mem();
  wb();
  printf("memLatch\n");
  testprint(memLatch);
  printf("exLatch\n");
  testprint(exLatch);
  printf("memory\n");
  printf("dataMemory\n");
  printf("%d\n", reg[17]);
  return 0;
}

char *progScanner(char *input){

	token = malloc(sizeof(char) * MAX);			 //initializing char array
	//buffer = malloc(sizeof(char *) * MAX); 	//init token array
	temp = malloc(sizeof(char *) * MAX); 		//to hold corrected line

	char delimiters[] = "() ,\n";
	int i;
	int parCount = 0;

	//fptr = input;
	//while(fgets(input, MAX, fptr) != NULL){

		char *dup = strdup(input);
		for(i = 0; i < strlen(input); i++){

			if(dup[i] == '('){
				parCount++;
			}
			if(dup[i] == ')'){
				parCount--;
			}
		}

		if(parCount != 0){
			printf("Parentheses error. Program will die");
			exit(0);
		}

		//Delimiting
		for(i = 0; i < 4; i++){
			if(i == 0){
				token[i] = strtok(input, delimiters);
			}
			else{
				token[i] = strtok(NULL, delimiters);
			}

			if(i < 3){
				strcat(temp, token[i]);
				strcat(temp, " ");
			}
			else{
				strcat(temp, token[i]);
			}
		}
		//strcat(temp, "\n");
		//inst_mem[k++] = parser(temp);
		//temp = malloc(sizeof(char *) * MAX);

	//}
	// fclose(fptr);
	free(dup);
	return temp;
}

struct Instruction parser(char *string){

	char regDelimiter[] = "\r $";
	char* str = malloc(sizeof(char) * MAX);
	str = strtok(string, regDelimiter);
	struct Instruction testInst;

	//Designate components of struct
	testInst.opcode = opcodeConverter(str);

	if(testInst.opcode == 5 || testInst.opcode == 6){
		testInst.rd = regNumConverter(strtok(NULL, regDelimiter));
		testInst.imm = atoi(strtok(NULL, regDelimiter));

		if(testInst.imm > 65532){
			printf("Immediate value too large, %d", testInst.imm);
			exit(0);
		}
		testInst.rs = regNumConverter(strtok(NULL, regDelimiter));

		printf("opcode: %d\n", testInst.opcode);
		printf("rd: %d\n", testInst.rd);
		printf("imm: %d\n", testInst.imm);
		printf("rs: %d\n", testInst.rs);

		return testInst;
	}

	else{
		testInst.rd = regNumConverter(strtok(NULL, regDelimiter));
		testInst.rs = regNumConverter(strtok(NULL, regDelimiter));
		testInst.rt = regNumConverter(strtok(NULL, regDelimiter));


		printf("opcode: %d\n", testInst.opcode);
		printf("rd: %d\n", testInst.rd);
		printf("rs: %d\n", testInst.rs);
		printf("rt: %d\n", testInst.rt);
	}

	return testInst;
}

int opcodeConverter(char* instOpcode){
	int val;
	if(strcmp(instOpcode, "add") == 0){
		val = 0;
	}
	else if(strcmp(instOpcode, "addi") == 0){
		val = 1;
	}
	else if(strcmp(instOpcode, "sub") == 0){
		val = 2;
	}
	else if(strcmp(instOpcode, "mult") == 0){
		val = 3;
	}
	else if(strcmp(instOpcode, "beq") == 0){
		val = 4;
	}
	else if(strcmp(instOpcode, "lw") == 0){
		val = 5;
	}
	else if(strcmp(instOpcode, "sw") == 0){
		val = 6;
	}else{
		printf("INVALID OPCODE. KILL MYSELF");
		exit(0);
	}
	return val;
}

int regNumConverter(char* regName){

	int val;
	if(strcmp(regName, "zero") == 0 || strcmp(regName, "0") == 0){
		val = 0;
	}
	else if(strcmp(regName, "at") == 0 || strcmp(regName, "1") == 0){
		val = 1;
	}
	else if(strcmp(regName, "v0") == 0 || strcmp(regName, "2") == 0){
		val = 2;
	}
	else if(strcmp(regName, "v1") == 0 || strcmp(regName, "3") == 0){
		val = 3;
	}
	else if(strcmp(regName, "a0") == 0 || strcmp(regName, "4") == 0){
		val = 4;
	}
	else if(strcmp(regName, "a1") == 0 || strcmp(regName, "5") == 0){
		val = 5;
	}
	else if(strcmp(regName, "a2") == 0 || strcmp(regName, "6") == 0){
		val = 6;
	}
	else if(strcmp(regName, "a3") == 0 || strcmp(regName, "7") == 0){
		val = 7;
	}
	else if(strcmp(regName, "t0") == 0 || strcmp(regName, "8") == 0){
		val = 8;
	}
	else if(strcmp(regName, "t1") == 0 || strcmp(regName, "9") == 0){
		val = 9;
	}
	else if(strcmp(regName, "t2") == 0 || strcmp(regName, "10") == 0){
		val = 10;
	}
	else if(strcmp(regName, "t3") == 0 || strcmp(regName, "11") == 0){
		val = 11;
	}
	else if(strcmp(regName, "t4") == 0 || strcmp(regName, "12") == 0){
		val = 12;
	}
	else if(strcmp(regName, "t5") == 0 || strcmp(regName, "13") == 0){
		val = 13;
	}
	else if(strcmp(regName, "t6") == 0 || strcmp(regName, "14") == 0){
		val = 14;
	}
	else if(strcmp(regName, "t7") == 0 || strcmp(regName, "15") == 0){
		val = 15;
	}
	else if(strcmp(regName, "s0") == 0 || strcmp(regName, "16") == 0){
		val = 16;
	}
	else if(strcmp(regName, "s1") == 0 || strcmp(regName, "17") == 0){
		val = 17;
	}
	else if(strcmp(regName, "s2") == 0 || strcmp(regName, "18") == 0){
		val = 18;
	}
	else if(strcmp(regName, "s3") == 0 || strcmp(regName, "19") == 0){
		val = 19;
	}
	else if(strcmp(regName, "s4") == 0 || strcmp(regName, "20") == 0){
		val = 20;
	}
	else if(strcmp(regName, "s5") == 0 || strcmp(regName, "21") == 0){
		val = 21;
	}
	else if(strcmp(regName, "s6") == 0 || strcmp(regName, "22") == 0){
		val = 22;
	}
	else if(strcmp(regName, "s7") == 0 || strcmp(regName, "23") == 0){
		val = 23;
	}
	else if(strcmp(regName, "t8") == 0 || strcmp(regName, "24") == 0){
		val = 24;
	}
	else if(strcmp(regName, "t9") == 0 || strcmp(regName, "25") == 0){
		val = 25;
	}
	else if(strcmp(regName, "k0") == 0 || strcmp(regName, "26") == 0){
		val = 26;
	}
	else if(strcmp(regName, "k1") == 0 || strcmp(regName, "27") == 0){
		val = 27;
	}
	else if(strcmp(regName, "gp") == 0 || strcmp(regName, "28") == 0){
		val = 28;
	}
	else if(strcmp(regName, "sp") == 0 || strcmp(regName, "29") == 0){
		val = 29;
	}
	else if(strcmp(regName, "fp") == 0 || strcmp(regName, "30") == 0){
		val = 30;
	}
	else if(strcmp(regName, "ra") == 0 || strcmp(regName, "31") == 0){
		val = 31;
	}
	else{
		printf("Register not recognized: %s", regName);
		exit(0);
	}
	return val;
}

void IF(){
	if (instructionMemory[pc].opcode == haltSimulation) {	///stopcodes IF
		if (!ifLatch.ready){
			ifLatch.instruction = instructionMemory[pc];
			ifLatch.ready = true;
			}
		}
	else {
		if (!ifLatch.ready) {  //sends info to ID

			ifLatch.instruction = instructionMemory[pc];
			ifLatch.rs = 0;
			ifLatch.rt = 0;
			ifLatch.ready = true;	//can move on
			pc++;
			}
			start = isBEQ;
		}
	}

void id(){
	int hazard = 0;  ///0 = no hazard // 1 = there is a hazard
	int wrongInstruct = 0; // 0 = nothing wrong

	if (ifLatch.opcode == haltSimulation && !idLatch.ready){
		ifLatch.ready = false;
		idLatch.instruction = ifLatch.instruction;
		idLatch.ready = true;
		}
	else {
		if (idLatch.ready){
			if (ifLatch.instruction.opcode == BEQ) {
				isBEQ = true;
				}
				////ADD, SUB, MULT instructions in ID_EX_LATCH/////
			if((idLatch.ready) && (idLatch.instruction.opcode == ADD || idLatch.instruction.opcode == SUB || idLatch.instruction.opcode == MUL){
				if (ifLatch.instruction.opcode == ADD || ifLatch.instruction.opcode == SUB || ifLatch.instruction.opcode == MUL || ifLatch.instruction.opcode == SW || ifLatch.instruction.opcode == BEQ){
					if(ifLatch.instruction.rs == idLatch.instruction.rd || ifLatch.instruction.rt == idLatch.instruction.rd){ ///CHECK IF THE REGISTERS
						hazard = 1;
						}
					}
				else if(ifLatch.instruction.opcode == ADDI || ifLatch.instruction.opcode == LW){
					if(ifLatch.instruction.rs == idLatch.instruction.rd){
						hazard = 1;
						}
					}
			}
				///ADDI LW in ID_EX_LATCH////
			if((idLatch.ready) && (idLatch.instruction.opcode == ADDI || idLatch.instruction.opcode == LW){
				if (ifLatch.instruction.opcode == ADD || ifLatch.instruction.opcode == SUB || ifLatch.instruction.opcode == MUL || ifLatch.instruction.opcode == SW || ifLatch.instruction.opcode == BEQ){
					if(ifLatch.instruction.rs == idLatch.instruction.rd || ifLatch.instruction.rt == idLatch.instruction.rd){ ///CHECK IF THE REGISTERS
						hazard = 1;
						}
					}
				else if(ifLatch.instruction.opcode == ADDI || ifLatch.instruction.opcode == LW){
					if(ifLatch.instruction.rs == idLatch.instruction.rd){
						hazard = 1;
						}
					}
			}
				////ADD, SUB, MULT instructions in EX_MEM_LATCH/////
			if((memLatch.ready) && (exLatch.instruction == ADD || exLatch.instruction.opcode == SUB || exLatch.instruction.opcode == MUL){
				if (ifLatch.instruction.opcode == ADD || ifLatch.instruction.opcode == SUB || ifLatch.instruction.opcode == MUL || ifLatch.instruction.opcode == SW || ifLatch.instruction.opcode == BEQ){
					if(ifLatch.instruction.rs == exLatch.instruction.rd || ifLatch.instruction.rt == exLatch.instruction.rd){ ///CHECK IF THE REGISTERS
						hazard = 1;
						}
					}
				else if(ifLatch.instruction.opcode == ADDI || ifLatch.instruction.opcode == LW){
					if(ifLatch.instruction.rs == exLatch.instruction.rd){
						hazard = 1;
						}
					}
			}


				//////ADDI LW in EX_MEM_LATCH/////
			if((memLatch.ready) && (exLatch.instruction.opcode == ADDI || exLatch.instruction.opcode == LW){
				if (ifLatch.instruction.opcode == ADD || ifLatch.instruction.opcode == SUB || ifLatch.instruction.opcode == MUL || ifLatch.instruction.opcode == SW || ifLatch.instruction.opcode == BEQ){
					if(ifLatch.instruction.rs == idLatch.instruction.rd || ifLatch.instruction.rt == idLatch.instruction.rd){ ///CHECK IF THE REGISTERS
						hazard = 1;
						}
					}
				else if(ifLatch.instruction.opcode == ADDI || ifLatch.instruction.opcode == LW){
					if(ifLatch.instruction.rs == exLatch.instruction.rd){
						hazard = 1;
						}
					}
			}
			////ADD, SUB, MULT instructions in MEM_WB_LATCH/////
			if((memLatch.ready) && (memLatch.instruction == ADD || memLatch.instruction.opcode == SUB || memLatch.instruction.opcode == MUL){
				if (ifLatch.instruction.opcode == ADD || ifLatch.instruction.opcode == SUB || ifLatch.instruction.opcode == MUL || ifLatch.instruction.opcode == SW || ifLatch.instruction.opcode == BEQ){
					if(ifLatch.instruction.rs == memLatch.instruction.rd || ifLatch.instruction.rt == memLatch.instruction.rd){ ///CHECK IF THE REGISTERS
						hazard = 1;
						}
					}
				else if(ifLatch.instruction.opcode == ADDI || ifLatch.instruction.opcode == LW){
					if(ifLatch.instruction.rs == memLatch.instruction.rd){
						hazard = 1;
						}
					}
			}


				//////ADDI LW in MEM_WB_LATCH/////
			if((memLatch.ready) && (memLatch.instruction.opcode == ADDI || memLatch.instruction.opcode == LW){
				if (ifLatch.instruction.opcode == ADD || ifLatch.instruction.opcode == SUB || ifLatch.instruction.opcode == MUL || ifLatch.instruction.opcode == SW || ifLatch.instruction.opcode == BEQ){
					if(ifLatch.instruction.rs == memLatch.instruction.rd || ifLatch.instruction.rt == memLatch.instruction.rd){ ///CHECK IF THE REGISTERS
						hazard = 1;
						}
					}
				else if(ifLatch.instruction.opcode == ADDI || ifLatch.instruction.opcode == LW){
					if(ifLatch.instruction.rs == memLatch.instruction.rd){
						hazard = 1;
						}
					}
			}
			if(hazard == 0 && !idLatch.ready){
				switch(idLatch.instruction.opcode){
					case ADD : {
						idLatch.instruction.rs = mips_register(ifLatch.instruction.rs);
						idLatch.instruction.rt = mips_register(ifLatch.instruction.rt);
						break;
						}
					case ADDI : {
						idLatch.instruction.rs = mips_register(ifLatch.instruction.rs);
						idLatch.instruction.rt = mips_register(ifLatch.instruction.rt);
						break;
						}
					case SUB : {
						idLatch.instruction.rs = mips_register(ifLatch.instruction.rs);
						idLatch.instruction.rt = mips_register(ifLatch.instruction.rt);
						break;
						}
					case MUL : {
						idLatch.instruction.rs = mips_register(ifLatch.instruction.rs);
						idLatch.instruction.rt = mips_register(ifLatch.instruction.rt);
						break;
						}
					case BEQ : {
						idLatch.instruction.rs = mips_register(ifLatch.instruction.rs);
						idLatch.instruction.rt = mips_register(ifLatch.instruction.rt);
						break;
						}
					case SW : {
						idLatch.instruction.rs = mips_register(ifLatch.instruction.rs);
						idLatch.instruction.rt = mips_register(ifLatch.instruction.rt);
						break;
						}
					case LW : {
						idLatch.instruction.rs = mips_register(ifLatch.instruction.rs);
						idLatch.instruction.rt = mips_register(ifLatch.instruction.rt);
						break;
						}
					default : {
						wronginstruct = 1;
						break;
						}
					}
				}
			assert(wronginstruct == 0);
			if(hazard == 0 && !idLatch.ready){

				idLatch.instruction = ifLatch.instruction;
				idLatch.ready = true;
				ifLatch.ready = false;
						}
					}
				}

	}



void ex(){
  int numCycle;
  if(idLatch.ready){           //has id given out a result?
    if(idLatch.opcode == mult) { numCycle = m; }
    else { numCycle = n; }
    if(exIntCycle < numCycle) { exIntCycle++; }
    if(exIntCycle == numCycle && !exLatch.ready){
      switch (idLatch.opcode){
        case mult:
          exLatch.output = idLatch.rs*idLatch.rt;
          break;
        case add:
          exLatch.output = idLatch.rs + idLatch.rt;
          break;
        case sub:
          exLatch.output = idLatch.rs - idLatch.rt;
          break;
        case addi:
          exLatch.output = idLatch.rs + idLatch.immediate;
          break;
        case lw:
          exLatch.output = idLatch.rs + (idLatch.immediate/4);
          break;
        case sw:
          exLatch.output = idLatch.rs + (idLatch.immediate/4);
          break;
        case beq:
          if(idLatch.rs == idLatch.rt)
            pc += idLatch.immediate;
          assert((pc>=0) && (pc <512));
          isBEQ = false;
          exLatch.output = 0;
          break;
      }

        exLatch.opcode = idLatch.opcode;
        exLatch.rs = idLatch.rs;
        exLatch.rt = idLatch.rt;
        exLatch.instruction = idLatch.instruction;
        exLatch.immediate = idLatch.immediate;
        exLatch.ready = true;
        idLatch.ready = false;
        exIntCycle = 0;
      }
  }
}

void mem(){
  if(exLatch.ready){
    if((exLatch.instruction.opcode == lw || exLatch.instruction.opcode == sw) && !memLatch.ready){
      if (memIntCycle < c) memIntCycle++;
      if(memIntCycle == c && !memLatch.ready){
        assert(exLatch.immediate % 4 == 0);
        if(exLatch.instruction.opcode == sw){
          dataMemory[exLatch.output] = reg[exLatch.instruction.rt];
          memLatch.output=0;
        }
        if(exLatch.instruction.opcode == lw){

          memLatch.output = dataMemory[exLatch.output];
        }
      }
    }
    else if(!memLatch.ready){
      if (memIntCycle < c)
        memIntCycle++;
      if(memIntCycle == c)
        memLatch.output = exLatch.output;
    }
    memLatch.opcode = exLatch.opcode;
    memLatch.rs = exLatch.rs;
    memLatch.rt = exLatch.rt;
    memLatch.instruction = exLatch.instruction;
    memLatch.immediate = exLatch.immediate;
    memIntCycle = 0;
    exLatch.ready = false;
    memLatch.ready = true;

  }
}

void wb(){
  if(memLatch.ready){
    if(memLatch.instruction.opcode == add || memLatch.instruction.opcode == sub  || memLatch.instruction.opcode == mult){
      cycle.wb++;
      reg[memLatch.instruction.rd] = memLatch.output;
    }
    else if(memLatch.instruction.opcode == lw || memLatch.instruction.opcode == addi){
      cycle.wb++;
      reg[memLatch.instruction.rt] = memLatch.output;
    }
    memLatch.ready = false;
  }
}

void testprint(struct Latch latch){
  printf("opcode = %d\n",latch.instruction.opcode);
  printf("rs = %d\n",reg[latch.instruction.rs]);
  printf("rt = %d\n",reg[latch.instruction.rt]);
  printf("output = %d\n",latch.output);
  printf("ready = %d\n\n", latch.ready);
}
