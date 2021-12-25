#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct
{
	char ident[7];
	int bytes;
	int params;
	char opcode[32];
} instruction;

instruction *structs[130];
int readInstructionSet(FILE *input, instruction *structs[]);
int searchInstructionSet(instruction *structs[], uint8_t a, uint8_t b);
void printHex(FILE *out, uint8_t a, uint8_t b);
void printInstruction(FILE *out, instruction *inst, uint8_t a, uint8_t b, uint8_t c, uint8_t d);

int main(void)
{
	FILE *f_set = fopen("instruction_set.txt", "r");
	FILE *f_in = fopen("arduino.hex", "rb");
	FILE *f_out = fopen("code.txt", "w+");
	
	if(f_set && f_in && f_out)
	{
		readInstructionSet(f_set, structs);
		
		uint8_t temp_a, temp_b;
		int run=1, cnt=0;
		while(run && cnt < 16000)
		{
			fscanf(f_in, "%c", &temp_a);
			if(temp_a != EOF)
			{
				fscanf(f_in, "%c", &temp_b);
				if(temp_b != EOF)
				{
					if(temp_a == 255 && temp_b == 255)
					{
						run=0;
					}
					else
					{
						int inst = searchInstructionSet(structs, temp_b, temp_a);
						if(inst < 0)
							printHex(f_out, temp_a, temp_b);
						else
						{
							uint8_t temp_c=0, temp_d=0;
							if(structs[inst]->bytes == 4)
							{
								fscanf(f_in, "%c", &temp_c);
								fscanf(f_in, "%c", &temp_d);
							}
							
							printInstruction(f_out, structs[inst], temp_b, temp_a, temp_d, temp_c);
						}
						
						cnt++;
					}
				}
				else
				{
					run=0;
				}
			}
			else
			{
				run=0;
			}
		}		
		fclose(f_set);
		fclose(f_in);
		fclose(f_out);
	}
	else
	{
		printf("Datei konnte nicht geoeffnet werden!");
	}
	return 0;
}

int readInstructionSet(FILE *input_file, instruction *structs[])
{
	char input;
	int strct_cnt = 0;
	
	while(1)
	{
		int counter = 0;
		structs[strct_cnt] = malloc(sizeof(instruction));
		fscanf(input_file, "%c", &input);
		while(input != ' ')
		{
			structs[strct_cnt]->ident[counter++] = input;
			fscanf(input_file, "%c", &input);
		}
		counter = 0;
		
		fscanf(input_file, "%c", &input);
		structs[strct_cnt]->bytes = input - 48;
		
		while(input != ' ')
			fscanf(input_file, "%c", &input);
		
		fscanf(input_file, "%c", &input);
		structs[strct_cnt]->params = input - 48;
		
		int limit = structs[strct_cnt]->bytes * 8;
			
		while(counter < limit)
		{
			fscanf(input_file, "%c", &input);
			if(input != ' ')
				structs[strct_cnt]->opcode[counter++] = input;
		}	
		fscanf(input_file, "%c", &input);
		if(input != '\n')
			return 1;
		else
			strct_cnt++;
	}		
	return 1;
}

int searchInstructionSet(instruction *structs[], uint8_t a, uint8_t b)
{
	int index = 0;
	uint16_t c = a * 256 + b;
	while(structs[index])
	{
		for(int i = 0; i < 16; i++)
		{
			if(structs[index]->opcode[15-i] == '0' && ((c >> i) & 1))
			{
				index++;
				i = 16;
			}
			else if(structs[index]->opcode[15-i] == '1' && (~(c >> i) & 1))
			{
				index++;
				i = 16;
			}
			else if(i == 15)
			{
				return index;
			}
		}
	}
	return -1;
}

void printHex(FILE *out, uint8_t a, uint8_t b)
{
	fprintf(out, "0x%X %X\n", a, b);
}

void printInstruction(FILE *out, instruction *inst, uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
	uint32_t e = a*256*256*256 + b*256*256 + c*256 + d;
	if(inst->params == 0)
	{
		fprintf(out, "%s\n", inst->ident);
	}
	else if(inst->params == 1)
	{
		int param = 0;
		for(int i = 0; i < 32;  i++)
		{
			if(inst->opcode[i] == 'a')
			{
				param *= 2;
				param += ((e >> (31-i)) & 1);
			}
		}
		fprintf(out, "%s %d\n", inst->ident, param);		
	}
	else if(inst->params == 2)
	{
		int param_a = 0, param_b = 0;
		for(int i = 0; i < 32;  i++)
		{
			if(inst->opcode[i] == 'a')
			{
				param_a *= 2;
				param_a += ((e >> (31-i)) & 1);
			}
			else if(inst->opcode[i] == 'b')
			{
				param_b *= 2;
				param_b += ((e >> (31-i)) & 1);
			}
		}
		fprintf(out, "%s %d, %d\n", inst->ident, param_a, param_b);
	}
}