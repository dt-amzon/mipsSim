#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>


struct Instruction {
	int opcode;
	int rs;
	int rt;
	int rd;
	int imm;
	int offset;
};

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


void ex();
void mem();
void wb();
void testprint(struct Latch latch);

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
