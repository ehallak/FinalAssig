/* IF,ID,EX,ME,WB
0x03 LOAD
000 LB
001 LH
010 LW
011 LD
100 LBU
101 LHU
110 LWU

0x13 IMMEDIATE LOGIC
000 ADDI
100 XORI
110 ORI
111 ANDI

0x23 STORE
000 SB
001 SH
010 SW
011 SD

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

R  011x011 funct7 rs2 rs1 funct3 rd opcode
I  xxxxx11 imm[11:0] rs1 funct3 rd opcode
S  0100011 imm[11:5] rs2 rs1 funct3 imm[4:0] opcode
SB 1100011 imm[12|10:5] rs2 rs1  funct3 imm[4:1|11]
U  0x10111 imm[31:12] rd opcode
UJ 1101111 imm[20|10:1|11|19:12] rd opcode
*/

switch (opcode)
{
	case 0x03: // LOAD case
		switch (funct3)
		{
			case 000:// LB Load Byte

		}
	case 0x13: // IMMEDIATE LOGIC case
		switch (funct3)
		{
			case 0b000: // ADDI ADD Immediate
				reg[rd] = reg[rs1] + imm;
				break;
			case 0b100: // XORI XOR Immediate
				reg[rd] = ~((reg[rs1] & imm]) | (~reg[rs1] & ~imm));
				break;
			case 0b110: // ORI OR Immediate
				reg[rd] = reg[rs1] | imm;
				break;
			case 0b111: // ANDI AND Immediate
				reg[rd] = reg[rs1] & imm;
				break;
			default:
			{
				printf("Funct3 %d is not implemented yet", funct3);
				break;
			}
		}

	case 0x23: // STORE case
		switch (funct3)
		{
		case 0b000: // SB Store Byte
			m[reg[rs1] + imm] = (char) reg[rs2];
			break;
		case 0b001: // SH Store Halfword
		{
			char16_t * temp_pointer = (char16_t *) &m[reg[rs1] + imm]
			*temp_pointer = (char16_t) reg[rs2]
			break;
		}

		case 0b010: // SW Store Word
		{
			char32_t * temp_pointer = (char32_t *) &m[reg[rs1] + imm]
			* temp_pointer = (char32_t)reg[rs2]
			break;
		}
		default:
		{
			printf("Funct3 %d is not implemented yet", funct3);
			break;
		}
		}
	case 0x33: // LOGIC case
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
				printf ("None of the values are ADD nor SUB")
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
	case 0x63: // BRANCH case
		switch (funct3)
		{
		case 0b000: // BEQ Branch EQual 
		{
			if (reg[rs1]==reg[rs2])
				pc = pc + 
		}
		}
	default:
	{
		printf("There is a problem in implementing instructions");
			break;
	}

}