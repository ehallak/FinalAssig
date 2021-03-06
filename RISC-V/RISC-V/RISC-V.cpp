#include "stdafx.h"
#include <stdio.h>

//static int instr = sizeof(progr);
/* IF,ID,EX,ME,WB

R  011?011 funct7 rs2 rs1 funct3 rd opcode
I  ?????11 imm[11:0] rs1 funct3 rd opcode
S  0100011 imm[11:5] rs2 rs1 funct3 imm[4:0] opcode
SB 1100011 imm[12|10:5] rs2 rs1  funct3 imm[4:1|11]
U  0?10111 imm[31:12] rd opcode
UJ 1101111 imm[20|10:1|11|19:12] rd opcode
*/

#define REGISTERS 32
#define MEMORY_SIZE 0x100000 //1 Mebibyte of data, more than enough for small programs
// #define MEMORY_BOTTOM 0x70000000
// #define MEMORY_TOP 0x7fffffff

static int pc;
static int reg[REGISTERS];

// Here the first program hard coded as an array
static int progr[] = {
	// As minimal RISC-V assembler example
	0x00200093, // addi x1 x0 2
	0x00300113, // addi x2 x0 3
	0x002081b3, // add x3 x1 x2
	0x00000073, // exit call
	0x00000000 // null
};

//static char m[0x7fffffff]; //Memory is 2 Gibibytes of data [GiB]

static char mem[MEMORY_SIZE];
static char file_name[] = "./t13.bin";
static char result_name[] = "./t13.res";

void kprinthex(unsigned int);

int main()
{
	printf("Program started, Hello RISC-V world\n");

	FILE * input;
	int status = 0;
	status = (int)fopen_s(&input, file_name, "r");
	if (0 == status)
	{
		int progr_len = fread_s((void *) mem, MEMORY_SIZE, 4, 100, input);
		char exit = 0;
		/*int progr_len = 0;
		while ((((int *)mem)[progr_len] != 0) && (progr_len < MEMORY_SIZE))
		{
			//((int *) mem)[progr_len] = progr[progr_len];
			progr_len++;
		}*/
		reg[2] = MEMORY_SIZE - 0x10;
		kprinthex(reg[2]);
		printf("\n\n");

		while ((pc < progr_len) && (exit == 0))
		{
			//printf("Instruction: %d\n", ((int *)mem)[pc]);
			unsigned int instr = ((int *) mem)[pc];
			//unsigned int instr = 0x00200093;
			unsigned int opcode = (instr & 0x7f);
			unsigned int rd = (instr >> 7) & 0x1f;
			unsigned int funct3 = (instr >> 12) & 0x7;
			unsigned int rs1 = (instr >> 15) & 0x1f;
			unsigned int rs2 = (instr >> 20) & 0x1f;
			unsigned int funct7 = (instr >> 25) & 0x7f;
			int imm = 0x0;
			char waiter = 0;

			//calculate imm for the current type of instruction
			{
				if ((opcode & 0x77) == 0x33) //R-type
				{
					//printf("Houston, we've got a problem\n");
					imm = 0x0;
				}
				else if (opcode == 0x23) //S-type
				{
					//printf("different day, same problem\n");
					imm = (((instr >> 20) & 0xfe0) |
						((instr >> 7) & 0x1f));
					if ((imm & 0x800) == 0x800)
						imm |= 0xFFFFF000;
				}
				else if (opcode == 0x63) //SB-type
				{	
					imm = ((funct7 & 0x40) << 6) | ((rd & 0x1) << 11) | ((funct7 & 0x3f) << 5) | (rd & 0x1e) | 0x0;
					//kprinthex(imm);
					//printf("Third time's the charm?\n");
					if ((imm & 0x1000) == 0x1000)
					{
						imm |= 0xFFFFE000;
						//printf("Winner winner chicken dinner nr. %d\n", imm);
					}

				}
				else if ((opcode & 0x5f) == 0x17) //U-type
				{
					//printf("Quadbikes are a statement\n");
					imm = (instr & 0xfffff000);
				}
				else if (opcode == 0x6f) //UJ-type
				{
					//printf("This is stupid y'all\n");
					imm = (((instr >> 11) & 0x100000) |
						(instr & 0xff000) |
						((instr >> 9) & 0x800) |
						((instr >> 20) & 0x7fe) |
						0x0);
				}
				else //I-type
				{
					//printf("Ayyyyyy :D %d\n", instr);
					imm = (instr >> 20);
					if ((imm & 0x800) == 0x800)
						imm |= 0xFFFFF000;
				}
			}
			//kprinthex(instr);
			printf("\nPC: %d, Opcode: %d, funct3: %d, funct7: %d, rd: %d, rs1: %d, rs2: %d, imm: %d\n",pc,opcode,funct3,funct7,rd,rs1,rs2,imm);

			switch(opcode)
			{
			case ((unsigned int)0x03): // LOAD
			{
				switch (funct3)
				{
				case 0b000: // LB Load Byte
				{
					reg[rd] = (int) mem[reg[rs1] + imm];
					if ((reg[rd] & 0x80) == 0x80)
						reg[rd] |= 0xFFFFFF00;
					break;
				}
				case 0b001: // LH
				{
					char16_t* temp_pointer = (char16_t *) &mem[reg[rs1] + imm];
					reg[rd] = (int) *temp_pointer;
					if ((reg[rd] & 0x8000) == 0x8000)
					{
						reg[rd] |= 0xFFFF0000;
					}
					break;
				}
				case 0b010: // LW
				{
					int* temp_pointer = (int *) &mem[reg[rs1] + imm];
					reg[rd] = (int) *temp_pointer;
					break;
				}
				case 0b100: // LBU
				{
					reg[rd] = ((int) mem[reg[rs1] + imm]) & 0xFF;
					break;
				}
				case 0b101: // LHU
				{
					char16_t* temp_pointer = (char16_t *) &mem[reg[rs1] + imm];
					reg[rd] = ((int) *temp_pointer) & 0xFFFF;
					break;
				}
				}
				//printf("%d\n%d,%d,%d,%d\n", imm, mem[reg[rs1] + imm], mem[reg[rs1] + imm + 1], mem[reg[rs1] + imm + 2], mem[reg[rs1] + imm + 3]);
				pc++;
				break;
			}
			//case ((unsigned int)0x0F): //FENCE
			case ((unsigned int)0x13): // IMMEDIATE LOGIC
			{
				//printf("Got there?\n");
				switch (funct3)
				{
				case 0b000: // ADDI ADD Immediate
					reg[rd] = reg[rs1] + imm;
					break;
				case 0b001: // SLLI shift left logic immediate
				{
					imm &= 0xFFF;
					int temp = reg[rs1];
					reg[rd] = (((unsigned int)reg[rs1]) << ((unsigned int) imm));
					//printf("%d = %d << %d\n",reg[rd],temp,imm);
					break;
				}
				case 0b010: // STLI
				{
					if (((signed)reg[rs1]) < ((signed)imm))
						reg[rd] = 1;
					else
						reg[rd] = 0;
					break;
				}
				case 0b011: //STLIU
				{
					if (((unsigned)reg[rs1]) < ((unsigned) (imm & 0xFFF)))
						reg[rd] = 1;
					else
						reg[rd] = 0;
					break;
				}
				case 0b100: // XORI XOR Immediate
					reg[rd] = reg[rs1] ^ imm;
					break;
				case 0b101: // shift right
				{
					imm &= 0xFFF;
					if (funct7 == 0x00) // SRLI logic immediate
					{
						reg[rd] = ((reg[rs1] >> imm) & ((~((unsigned int)0)) >> imm));
					}
					else if (funct7 == 0x20) // SRAI Arethmetic immediate
					{
						char boolie = (((reg[rs1]) & 0x80000000) == 0x80000000);
						reg[rd] = ((reg[rs1] >> imm) & ((~((unsigned int)0)) >> imm));
						if (boolie)
						{
							//printf("Got there!\n");
							reg[rd] |= (~((~((unsigned int)0)) >> imm));
						}
					}
					else
					{
						printf("Illegal funct7 for shift right\n");
						exit = 1;
					}
					break;
				}
				case 0b110: // ORI OR Immediate
					reg[rd] = reg[rs1] | imm;
					break;
				case 0b111: // ANDI AND Immediate
				{
					reg[rd] = reg[rs1] & imm;
					break;
				}
				default:
				{
					printf("Funct3 %d is not implemented yet for opcode %d\n", funct3, opcode);
					exit = 1;
					break;
				}
				}
				pc++;
				break;
			}
			case ((unsigned int)0x17): // AUIPC
			{
				reg[rd] = (pc<<2) + imm;
				pc++;
				break;
			}
			case ((unsigned int)0x23): // STORE
			{
				switch (funct3)
				{
				case 0b000: // SB Store Byte
					mem[reg[rs1] + imm] = (char) (reg[rs2] & 0xFF);
					break;

				case 0b001: // SH Store Halfword
				{
					char16_t* temp_pointer = (char16_t *) &mem[reg[rs1] + imm];
					*temp_pointer = (char16_t) (reg[rs2] & 0xFFFF);
					break;
				}

				case 0b010: // SW Store Word
				{
					char32_t * temp_pointer = (char32_t *) &mem[reg[rs1] + imm];
					*temp_pointer = (char32_t)reg[rs2];
					break;
				}

				default:
				{
					printf("Funct3 %d is not implemented yet for opcode %d\n", funct3, opcode);
					exit = 1;
					break;
				}
				}
				//printf("%d\n%d,%d,%d,%d\n", imm, mem[reg[rs1] + imm], mem[reg[rs1] + imm + 1], mem[reg[rs1] + imm + 2], mem[reg[rs1] + imm + 3]);
				pc++;
				break;
			}
			case ((unsigned int)0x33): // REGISTER LOGIC
			{
				switch (funct7)
				{
				case 0x00:
				{
					//printf("I'm here\n");
					//kprinthex(funct3);
					switch (funct3)
					{
					case 0b000: // ADD instructions
					{
						if (funct7 == 0x00)
							reg[rd] = reg[rs1] + reg[rs2];
						break;
					}
					case 0b001: // SLL shift left logic
					{
						reg[rd] = (((unsigned int)reg[rs1]) << ((unsigned int)reg[rs2]));
						break;
					}
					case 0b010: //SLT
					{
						if (((signed)reg[rs1]) < ((signed)reg[rs2]))
							reg[rd] = 1;
						else
							reg[rd] = 0;
						break;
					}
					case 0b011: //SLTU
					{
						if (((unsigned)reg[rs1]) < ((unsigned)reg[rs2]))
							reg[rd] = 1;
						else
							reg[rd] = 0;
						break;
					}
					case 0b100: // XOR XOR instruction
					{
						reg[rd] = ~((reg[rs1] & reg[rs2]) | (~reg[rs1] & ~reg[rs2]));
						break;
					}
					case 0b101: // shift right
					{
						imm &= 0xFFF;	
						reg[rd] = ((reg[rs1] >> reg[rs2]) & ((~((unsigned int)0)) >> reg[rs2]));						
					}
					case 0b110: // OR OR instruction
					{
						reg[rd] = reg[rs1] | reg[rs2];
						break;
					}
					case 0b111: // AND AND instruction
					{
						reg[rd] = reg[rs1] & reg[rs2];
						break;
					}
					default:
					{
						printf("Funct3 %d is not implemented yet for funct7 0x00 and opcode %d\n", funct3, opcode);
						exit = 1;
						break;
					}
					}
					break;
				}
				case 0x01: // MUL & DIV
				{
					switch (funct3)
					{
					case 0b000: // MUL
					{
						reg[rd] = (reg[rs1] * reg[rs2]) & 0xFFFFFFFF;
						break;
					}
					case 0b001: // MULH
					{
						reg[rd] = (( ((signed) reg[rs1]) * ((signed) reg[rs2]) ) >> 32) & 0xFFFFFFFF;
						break;
					}
					case 0b010: //MULHU
					{
						reg[rd] = ((((unsigned)reg[rs1]) * ((unsigned)reg[rs2])) >> 32) & 0xFFFFFFFF;
						break;
					}
					case 0b011: //MULHSU
					{
						reg[rd] = ((((signed)reg[rs1]) * ((unsigned)reg[rs2])) >> 32) & 0xFFFFFFFF;
						break;
					}
					case 0b100: //DIV
					{
						reg[rd] = (((signed)reg[rs1]) / ((signed)reg[rs2])) & 0xFFFFFFFF;
						break;
					}
					case 0b101: //DIVU
					{
						reg[rd] = (((unsigned)reg[rs1]) / ((unsigned)reg[rs2])) & 0xFFFFFFFF;
						break;
					}
					case 0b110: //REM
					{
						reg[rd] = (((signed)reg[rs1]) % ((signed)reg[rs2])) & 0xFFFFFFFF;
						break;
					}
					case 0b111: //REMU
					{
						reg[rd] = (((unsigned)reg[rs1]) % ((unsigned)reg[rs2])) & 0xFFFFFFFF;
						break;
					}
					default:
					{
						printf("Funct3 %d is not implemented yet for funct7 0x01 and opcode %d\n", funct3, opcode);
						exit = 1;
						break;
					}
					}
					break;
				}
				case 0x20: 
				{
					if (funct3 == 0b000) // SUB
					{
						reg[rd] = reg[rs1] - reg[rs2];
					}
					else if (funct3 == 0b101) // SRA Arethmetic
					{
						imm &= 0xFFF;
						char boolie = (((reg[rs1]) & 0x80000000) == 0x80000000);
						reg[rd] = ((reg[rs1] >> reg[rs2]) & ((~((unsigned int)0)) >> reg[rs2]));
						if (boolie)
						{
							reg[rd] |= (~((~((unsigned int)0)) >> reg[rs2]));
						}
					}
					else
					{
						printf("Funct3 %d is not implemented yet for funct7 0x20 and opcode %d\n", funct3, opcode);
						exit = 1;
					}
					break;
				}
				default:
				{
					printf("Illegal funct7 for opcode %d\n", opcode);
					exit = 1;
				}
				}
				pc++;
				break;
			}
			case ((unsigned int)0x37): // LUI
			{
				reg[rd] = imm;
				pc++;
				break;
			}
			case ((unsigned int)0x63): // CONDITIONAL BRANCH
			{
				char j = 1;
				switch (funct3)
				{
				case 0b000: // BEQ Branch EQual 
				{
					if (reg[rs1] == reg[rs2])
						pc = pc + (imm >> 2);
					else
						j = 0;
					break;
				}				
				case 0b001: // BNE Branch Not Equal
				{
					//printf("Not all are born into an equal position\n");
					if (reg[rs1] != reg[rs2])
						pc = pc + (imm >> 2);
					else
						j = 0;
					break;
				}
				case 0b100: // BLT Branch Less Than
				{
					if (reg[rs1] < reg[rs2])
						pc = pc + (imm >> 2);
					else
						j = 0;
					break;
				}
				case 0b101: // BGE Branch Greater than or Equal 
				{
					if (reg[rs1] >= reg[rs2])
						pc = pc + (imm >> 2);
					else
						j = 0;
					break;
				}
				case 0b110: // BLTU Branch Less Than Unsigned
				{
					if (((unsigned int)reg[rs1]) < ((unsigned int)reg[rs2]))
						pc = pc + (imm >> 2);
					else
						j = 0;
					break;
				}
				case 0b111: // BGEU Branch Greater than or Equal Unsinged
				{
					if (((unsigned)reg[rs1]) >= ((unsigned)reg[rs2]))
						pc = pc + (imm >> 2);
					else
						j = 0;
					break;
				}
				default:
				{
					printf("Funct3 %d is not implemented yet for opcode %d\n", funct3, opcode);
					exit = 1;
					break;
				}
				}
				if (j == 0)
				{
					pc++;
					//printf("Didn't branch\n");
					waiter = 1;
				}
				/*else
				{
					printf("Did a branch to %d\n", pc);
					//waiter = 1;
				}*/
				
				break;
			}
			case ((unsigned int)0x67): // JALR ABSOLUTE JUMP
			{
				//printf("pc is %d\n", pc);
				reg[rd] = (pc + 1) << 2;
				//printf("%d\n",imm);
				imm = (((signed)reg[rs1]) + ((signed)imm))>>2;
				if ((imm & 0x20000000) == 0x20000000)
					imm |= 0xC0000000;
				pc = imm;
				//printf("Did an absolute jump to %d\n", pc);
				//printf("rs1 is %d, rd is set to %d\n",reg[rs1],reg[rd]);
				//printf("%d\n",imm);
				waiter = 1;
				break;
			}
			case ((unsigned int)0x6F): // JAL RELATIVE JUMP
			{
				reg[rd] = (pc + 1) << 2;
				pc = pc + (imm>>2);
				//printf("Did a relative jump to %d\n", pc);
				waiter = 1;
				break;
			}
			case ((unsigned int)0x73): // SYSTEM CALL EXIT
			{
				switch (funct3)
				{
				case 0b000:
				{
					exit = 1;
					break;
				}
				default:
				{
					printf("Unsupported call\n");
					exit = 1;
					break;
				}
				}
				pc++;
				break;
			}
			default: // default
			{
				printf("Opcode %d not yet implemented\n", opcode);
				exit = 1;
				break;
			}
			}

			reg[0] = 0;
			int i = 0;
			while(i < REGISTERS)
			{
				if(reg[i] == 0)
					printf("%d ", reg[i]);
				else
				{
					kprinthex(reg[i]);
					printf(" ");
				}
				i++;
			}
			
			/*if (waiter == 1)
			{
				printf("\n");
				scanf_s("%c", &waiter, 1);
			}
			else//*/
				printf("\n\n");
		}
	}
	
	kprinthex(pc);
	printf("\n\n");
	fclose(input);
	fopen_s(&input, result_name, "wb");
	int i = 0;
	while (i < REGISTERS)
	{
		int temp = reg[i];
		kprinthex(temp);
		printf(" ");
		fwrite((void *)&temp, 4, 1, input);
		i++;
	}
	fclose(input);

	//char waiter = '0';
	//scanf_s("%c", &waiter, 1);
	return 0;
}

void kprinthex(unsigned int value)
{
	unsigned char i = 0, buffer[9] = { 0 };
	unsigned char table[16] = 
	{
		'0','1','2','3','4','5','6','7',
		'8','9','A','B','C','D','E','F'
	};
	while (i<8)
	{
		buffer[(7 - i)] = table[((value >> (4 * i)) & 0x0F)];
		i++;
	}
	printf("0x%s", buffer);
}
