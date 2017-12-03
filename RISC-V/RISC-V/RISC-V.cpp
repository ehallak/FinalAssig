#include "stdafx.h"
#include <stdio.h>

//static int instr = sizeof(progr);
/* IF,ID,EX,ME,WB
0x03 LOAD
000 LB
001 LH
010 LW
011 LD //64 only, not implemented in our system
100 LBU
101 LHU
110 LWU //64 only, not implemented in our system

0x13 IMMEDIATE LOGIC
000 ADDI
100 XORI
110 ORI
111 ANDI

0x23 STORE
000 SB
001 SH
010 SW
011 SD //64 only, not implemented in our system

0x33 LOGIC
000-0x00 ADD
000-0x20 SUB
100-0x00 XOR
110-0x00 OR
111-0x00 AND

0x63 BRANCH
000 BEQ
001 BNE
100 BLT
101 BGE
110 BLTU
111 BGEU

0x67 JUMP REGISTER
000 JALR

0x6F JUMP
JAL

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
	0x00000000 // null
};

//static char m[0x7fffffff]; //Memory is 2 Gibibytes of data [GiB]

static char m[MEMORY_SIZE];

int main()
{
	printf("Program started, Hello RISC-V world");

	FILE * input;
	if (fopen_s(&input, "C:/Users/User/Downloads/program.bin", "r") == 0)
	{
		fread_s((void *)m, MEMORY_SIZE, 4, 100, input);

		int progr_len = 0;
		while ((progr[progr_len] != 0) && (progr_len < MEMORY_SIZE))
		{
			//m[progr_len] = progr[progr_len];
			progr_len++;
		}

		while (pc < progr_len)
		{
			unsigned int instr = m[pc];
			unsigned int opcode = (instr & 0x7f);
			unsigned int rd = (instr >> 7) & 0x1f;
			unsigned int funct3 = (instr >> 12) & 0x7;
			unsigned int rs1 = (instr >> 15) & 0x1f;
			unsigned int rs2 = (instr >> 20) & 0x1f;
			unsigned int funct7 = (instr >> 25) & 0x7f;
			unsigned int imm = 0x0;

			{
				if ((opcode & 0x77) == 0x33) //R-type
				{
					imm = 0x0;
				}
				else if (opcode == 0x23) //S-type
				{
					imm = (((instr >> 20) & 0xfe0) |
						((instr >> 7) & 0x1f));
				}
				else if (opcode == 0x63) //SB-type
				{
					imm = ((funct7 & 0x40) << 6) | ((rd & 0x1) << 11) | ((funct7 & 0x3f) << 5) | (rd & 0x1e) | 0x0;
				}
				else if ((opcode & 0x5f) == 0x17) //U-type
				{
					imm = (instr & 0xfffff000);
				}
				else if (opcode == 0x6f) //UJ-type
				{
					imm = (((instr >> 11) & 0x100000) |
						(instr & 0xff000) |
						((instr >> 9) & 0x800) |
						((instr >> 20) & 0x7fe) |
						0x0);
				}
				else //I-type
				{
					int imm = (instr >> 20);
				}
			}

			switch (opcode)
			{
			case 0x03: // LOAD case
			{
				switch (funct3)
				{
				case 000:// LB Load Byte
				{
					break;
				}
				}
			}

			case 0x13: // IMMEDIATE LOGIC case
			{
				switch (funct3)
				{
				case 0b000: // ADDI ADD Immediate
					reg[rd] = reg[rs1] + imm;
					break;
				case 0b100: // XORI XOR Immediate
					reg[rd] = reg[rs1] ^ imm;
					break;
				case 0b110: // ORI OR Immediate
					reg[rd] = reg[rs1] | imm;
					break;
				case 0b111: // ANDI AND Immediate
					reg[rd] = reg[rs1] & imm;
					break;
				default:
				{
					printf("Funct3 %d is not implemented yet for opcode &d", funct3, opcode);
					break;
				}
				}
			}

			case 0x23: // STORE case
			{
				switch (funct3)
				{
				case 0b000: // SB Store Byte
					m[reg[rs1] + imm] = (char) (reg[rs2] & 0xFF);
					break;

				case 0b001: // SH Store Halfword
				{
					char16_t* temp_pointer = (char16_t *) &m[reg[rs1] + imm];
					*temp_pointer = (char16_t) (reg[rs2] & 0xFFFF);
					break;
				}

				case 0b010: // SW Store Word
				{
					char32_t * temp_pointer = (char32_t *) &m[reg[rs1] + imm];
					*temp_pointer = (char32_t)reg[rs2];
					break;
				}

				default:
				{
					printf("Funct3 %d is not implemented yet", funct3);
					break;
				}
				}
			}

			case 0x33: // LOGIC case
			{
				switch (funct3)
				{
				case 0b000: // ADD and SUB instructions
					if (funct3 == 0x00)
					{
						reg[rd] = reg[rs1] + reg[rs2];
					}
					else if (funct3 == 0x20)
					{
						reg[rd] = reg[rs1] - reg[rs2];
					}
					else
					{
						printf("Illegal funct7 for logical additon");
					}
					break;

				case 0b100: // XOR XOR instruction
					reg[rd] = ~((reg[rs1] & reg[rs2]) | (~reg[rs1] & ~reg[rs2]));
					break;
				case 0b110: // OR OR instruction
					reg[rd] = reg[rs1] | reg[rs2];
					break;
				case 0b111: // AND AND instruction
					reg[rd] = reg[rs1] & reg[rs2];
					break;
				default:
				{
					printf("Funct3 %d is not implemented yet", funct3);
					break;
				}
				}
			}

			case 0x63: // BRANCH case
			{
				switch (funct3)
				{
				case 0b000: // BEQ Branch EQual 
				{
					if (reg[rs1] == reg[rs2])
						pc = pc + imm; 
				}
				break;
				case 0b001: // BNE Branch Not Equal
				{
					if (reg[rs1] != reg[rs2])
						pc = pc + imm;
				}
				break;
				case 0b100: // BLT Branch Less Than
				{
					if (reg[rs1] < reg[rs2])
						pc = pc + imm;
				}
				break;
				case 0b101: // BGE Branch Greater than or Equal 
				{
					if (reg[rs1] >= reg[rs2])
						pc = pc + imm;
				}
				break;
				case 0b110: // BLTU Branch Less Than Unsigned
				{
					if (reg[rs1] < reg[rs2])
						pc = pc + imm;
				}
				break;
				case 0b111: // BGEU Branch Greater than or Equal Unsinged
				{
					if (reg[rs1] >= reg[rs2])
						pc = pc + imm;
				}
				break;
				}
			}

			case 0x67: // JALR Jump & Link Rregister
			{
				reg[rd] = pc + 4;
				pc = reg[rs1] + imm;
			}
			case 0x6F: // JAL JALR Jump & Link 
			{
				reg[rd] = pc + 4;
				pc = pc + imm;
			}
			default:
			{
				printf("Opcode %d not yet implemented", opcode);
				break;
			}

			}
		}
	}
}
