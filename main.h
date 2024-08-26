#include <stdio.h>
#include <string.h>
#include <math.h>

/*********** Predefined Macros **********************/

#define source_file_name "asm_code.asm"
#define source_file_mode "r"                    /*Setting the mode for file containing the assembly code - read mode*/

#define destination_file_name "asm_code.bin"
#define destination_file_mode "w"               /*Setting the mode for file going to contain the machine code - write mode*/

#define max_num_of_label 20
#define label_max_size   6                      /*Includes Semi-colon Eg-> START: */

#define instructions_array_size (12+label_max_size+4)   /*Largest Instruction in 8085 is LXI B,1000H. To store this we need 12 bytes. 4 bytes reserved for safety. If any extra spaces arises.*/
#define mnemonic_array_size      (label_max_size+2)     /*Largest mnemonic in 8085 contains only 4 characters. Eg: XCHG , Also we are using same array to store label. So we are using max label size. Extra 1 is for '\0' and another one is reserved.*/

#define max_hex_string_size      10

/*********** Function Prototypes *******************/

void label_memorize(void);
void mnemonic_cmp(void);
void operation(void);
void mnemonic_opcode_generator(int,int);
void update_opcode(int);
void hex_to_dec(int,int,int);
int string_to_num(int,int);


enum registers {

    A = 0b111,
    B = 0b000,
    C = 0b001,
    D = 0b010,
    E = 0b011,
    H = 0b100,
    L = 0b101,
    M = 0b110

};

enum reg_pairs{

    BC ,
    DE,
    HL,
    SP_PSW
};

char mnemonics_set[][6]={"MOV" ,"MVI" ,"LXI" ,"LDA" ,"STA" ,"LHLD","SHLD","XCHG",
                        "ADD" ,"ADC" ,"ADI" ,"ACI" ,"SUB" ,"SBB" ,"SUI" ,"SBI" ,"INR","DCR","DAA","CMA","CMC","STC",
                        "ANA" ,"XRA" ,"ORA" ,"CMP" ,"ANI" ,"XRI" ,"ORI" ,"CPI" ,
                        "JMP" ,"JNZ" ,"JZ"  ,"JNC" ,"JC"  ,"JPO" ,"JPE" ,"JP"  ,"JM" ,
                        "CALL","CNZ" ,"CZ"  ,"CNC" ,"CC"  ,"CPO" ,"CPE" ,"CP"  ,"CM" ,
                        "RET" ,"RNZ" ,"RZ"  ,"RNC" ,"RC"  ,"RPO" ,"RPE" ,"RP"  ,"RM" ,
                        "PCHL","IN"  ,"OUT" ,"XTHL","SPHL","HLT" ,"NOP" ,"RST" ,"EI" ,"DI","RIM","SIM",
                        "RLC" ,"RRC" ,"RAL" ,"RAR" ,
                        "DAD" ,"DAD" ,"DAD" ,"DAD" ,
                        "INX" ,"INX" ,"INX" ,"INX" ,
                        "DCX" ,"DCX" ,"DCX" ,"DCX" ,
                        "PUSH","PUSH","PUSH","PUSH",
                        "POP" ,"POP" ,"POP" ,"POP" ,
                        "LDAX","LDAX","STAX","STAX"
                        };

char operation_select[]={2,3,3,4,4,4,4,0,
                         1,1,4,4,1,1,4,4,1,1,0,0,0,0,
                         1,1,1,1,4,4,4,4,
                         4,4,4,4,4,4,4,4,4,
                         4,4,4,4,4,4,4,4,4,
                         0,0,0,0,0,0,0,0,0,
                         0,4,4,0,0,0,0,1,0,0,0,0,
                         0,0,0,0,
                         5,5,5,5,
                         5,5,5,5,
                         5,5,5,5,
                         5,5,5,5,
                         5,5,5,5,
                         5,5,5,5
};

char mnemonic_equivalent_opcodes[]={0x40,0x06,0x01,0x3A,0x32,0x2A,0x22,0xEB,
                                    0x80,0x88,0xC6,0xCE,0x90,0x98,0xD6,0xDE,0x04,0x05,0x27,0x2F,0x3F,0x37,
                                    0xA0,0xA8,0xB0,0xB8,0xE6,0xEE,0xF6,0xFE,
                                    0xC3,0xC2,0xCA,0xD2,0xDA,0xE2,0xEA,0xF2,0xFA,
                                    0xCD,0xC4,0xCC,0xD4,0xDC,0xE4,0xEC,0xF4,0xFC,
                                    0xC9,0xC0,0xC8,0xD0,0xD8,0xE0,0xE8,0xF0,0xF8,
                                    0xE9,0xDB,0xD3,0xE3,0xF9,0x76,0x00,0xC7,0xFB,0xF3,0x20,0x30,
                                    0x07,0x0F,0x17,0x1F,
                                    0x09,0x19,0x29,0x39,
                                    0x03,0x13,0x23,0x33,
                                    0x0B,0x1B,0x2B,0x3B,
                                    0xC5,0xD5,0xE5,0xF5,
                                    0xC1,0xD1,0xE1,0xF1,
                                    0x0A,0x1A,0x02,0x12
};


char instructions_size[]={1,2,3,3,3,3,3,1,
                          1,1,2,2,1,1,2,2,1,1,1,1,1,1,
                          1,1,1,1,2,2,2,2,
                          3,3,3,3,3,3,3,3,3,
                          3,3,3,3,3,3,3,3,3,
                          1,1,1,1,1,1,1,1,1,
                          1,2,2,1,1,1,1,1,1,1,1,1,
                          1,1,1,1,
                          1,1,1,1,
                          1,1,1,1,
                          1,1,1,1,
                          1,1,1,1,
                          1,1,1,1,
                          1,1,1,1
};

char instructions_array[instructions_array_size];
int  iterator,position;

char mnemonic_array[mnemonic_array_size];
int mnemonic_arr_iterator;

char label_set[max_num_of_label][label_max_size+2];
int  label_address[max_num_of_label];
int label_arr_iterator;
char label_flag;

char hex_string[max_hex_string_size];
int  num_of_spaces;

unsigned char equivalent_opcode;
unsigned char byte1_opcode;
char byte1_flag;

int address;
char address_flag;

int program_counter;
char program_counter_flag;

/*
Function Name: label_memorize
Functionality: Searching for labels and storing them and their address.
*/

void label_memorize()
{
    FILE *fp;
    fp = fopen(source_file_name,source_file_mode);

    if(fp == NULL)
    {
        printf("Error in Opening the ASM file");
    }
    else
    {
        while(fgets(instructions_array,sizeof(instructions_array),fp))
        {
            if(instructions_array[0]!='\n')                             //Checking whether it is empty line
            {
                mnemonic_cmp();                                         //Checking whether the line is denoted or referenced by any label
                iterator = 0;

                if(label_flag==1)
                {
                    label_flag=0;                                       //Clearing the flag

                    while(instructions_array[iterator]!=':')            //Extracting the label and storing it in the seperate array.
                    {
                        label_set[label_arr_iterator][iterator]=instructions_array[iterator];
                        iterator++;
                    }
                    label_set[label_arr_iterator][iterator]='\0';
                    label_address[label_arr_iterator] = program_counter;    //Also storing its address
                    label_arr_iterator++;
                }

                if(!program_counter_flag)
                {
                    program_counter += instructions_size[position];         //Incrementing the program counter based on the size of the instructions.
                }
                program_counter_flag=0;
            }
        }
        fclose(fp);
    }
}

/*
Function name: mnemonic_cmp
Functionality:
    1) Extracting the mnemonic
    2) Extracting the labels
    3) Initializing the program counter.
*/

void mnemonic_cmp()
{
    mnemonic_arr_iterator=0;
    iterator =0;

    while(instructions_array[iterator]!=':' && instructions_array[iterator]!=10)        //Checking whether the line is denoted or referenced by any label
    {
        iterator++;
    }

    if(instructions_array[iterator]==10)                                                //If nothing, then come back to initial position
    {
        iterator = 0;
    }
    else
    {
        iterator+=1;
        while(instructions_array[iterator]==' ')                                        //Checking for any spaces after label. Eg: SKIP: MOV A,B. The space after ':'.
        {
            iterator++;
        }
        label_flag=1;
    }

    while(instructions_array[iterator]!=' ' && instructions_array[iterator]!=10)        //Extracting the mnemoic. Eg: MOV A,B, we will iterate upto first space to get mnemonic'MOV'. Another condition for instructions like XCHG, it does not contains space, it contains end of line character 'LF' = 10.
    {
        mnemonic_array[mnemonic_arr_iterator]=instructions_array[iterator];
        iterator++;
        mnemonic_arr_iterator++;
    }
    mnemonic_array[mnemonic_arr_iterator]='\0';

    if(!strcmp(mnemonic_array,"ORG"))                                                   //Setting the address to load the program into that address.
    {
        program_counter_flag = 1;
        update_opcode(iterator);
        program_counter = address;
    }
    else
    {
        position=0;
        while(strcmp(mnemonic_array,mnemonics_set[position]))                           //Finding the position of extracted mnemonic in mnemonic set array.
        {
            position++;
        }
   }
}

/*

Function Name: operation
Functionality: Based on mnemonic, it does required operation.

case 0: If the line contains only mnemonic.             Eg: XCHG.
case 1: (M)nemonic (R)egister instructions.             Eg: ADD A.
case 2: (M)nemonic (R)egister Register instructions.    Eg: MOV A,B
case 3: (M)nemonic (R)egister value instructions.       Eg: MVI A,10h
case 4: (M)nemonic (V)alue instructions.                Eg: LDA 1000h
case 5: (M)nemonic (R)egister_pair instructions.        Eg: PUSH B

*/
void operation()                                                                        //Based on mnemonic, we do required operation
{
    equivalent_opcode = mnemonic_equivalent_opcodes[position];

    switch(operation_select[position])
    {
    case 1:
        if(position==16 || position==17 || position==64)                                //As per Datasheet.
            mnemonic_opcode_generator(iterator+1,3);
        else
            mnemonic_opcode_generator(iterator+1,0);
        break;

    case 2:
        mnemonic_opcode_generator(iterator+1,3);
        while(instructions_array[iterator+3]==' ')
            iterator++;
        mnemonic_opcode_generator(iterator+3,0);
        break;

    case 3:
        mnemonic_opcode_generator(iterator+1,3);
        update_opcode(iterator);
        break;

    case 4:
        update_opcode(iterator);
        break;

    case 5:
        switch(instructions_array[iterator+1])
        {
        case 'B':
            position += BC;
            break;

        case 'D':
            position += DE;
            break;

        case 'H':
            position += HL;
            break;

        case 'S':
            position += SP_PSW;
            break;

        case 'P':
            position += SP_PSW;
        }
        equivalent_opcode =  mnemonic_equivalent_opcodes[position];
    }
}

/*
Function name: mnemonic_opcode_generator
Functionality: Generating the equivalent opcode.

    Eg: General:  MOV r1,r2 -> 0 1 D D D S S S
        CODE   :  MOV A, B -> Destination : A register, Source: B register
    Based on value stored on registers enum, code will be generated by required number of left shifts.
*/

void mnemonic_opcode_generator(int pos,int num_of_shifts)
{
    switch(instructions_array[pos])
    {
        case 'A':
            equivalent_opcode |= (A << num_of_shifts);
            break;

        case 'B':
            equivalent_opcode |= (B << num_of_shifts);
            break;

        case 'C':
            equivalent_opcode |= (C << num_of_shifts);
            break;

        case 'D':
            equivalent_opcode |= (D << num_of_shifts);
            break;

        case 'E':
            equivalent_opcode |= (E << num_of_shifts);
            break;

        case 'H':
            equivalent_opcode |= (H << num_of_shifts);
            break;

        case 'L':
            equivalent_opcode |= (L << num_of_shifts);
            break;

        case 'M':
            equivalent_opcode |= (M << num_of_shifts);
            break;

        case '0':
            equivalent_opcode |= (0 << num_of_shifts);
            break;

        case '1':
            equivalent_opcode |= (1 << num_of_shifts);
            break;

        case '2':
            equivalent_opcode |= (2 << num_of_shifts);
            break;

        case '3':
            equivalent_opcode |= (3 << num_of_shifts);
            break;

        case '4':
            equivalent_opcode |= (4 << num_of_shifts);
            break;

        case '5':
            equivalent_opcode |= (5 << num_of_shifts);
            break;

        case '6':
            equivalent_opcode |= (6 << num_of_shifts);
            break;

        case '7':
            equivalent_opcode |= (7 << num_of_shifts);
            break;
        default:
            equivalent_opcode |= (M << num_of_shifts);
            break;
    }
}

/*
Function Name: hex_to_dec
Functionality: Converting the hexadecimal number represented as string into an equivalent decimal value.
*/

void hex_to_dec(int pos,int power,int byte_select)
{
    switch(instructions_array[pos])
    {
        case 'A':
        case 'a':
            address += 10 * pow(16,power);
            break;

        case 'B':
        case 'b':
            address += 11 * pow(16,power);
            break;

        case 'C':
        case 'c':
            address += 12 * pow(16,power);
            break;

        case 'D':
        case 'd':
            address += 13 * pow(16,power);
            break;

        case 'E':
        case 'e':
            address += 14 * pow(16,power);
            break;

        case 'F':
        case 'f':
            address += 15 * pow(16,power);
            break;

        default:
            address += (instructions_array[pos]-48) * pow(16,power);
    }

    if(byte1_flag==1)
        byte1_opcode = (unsigned char)address;
}

/*
Function Name: update_opcode
Functionality: Generating the opcodes for numeric values.
*/

void update_opcode(int pos)
{
    int num_of_digits=0;

    if(position==1 || position==2)
    {
        while(instructions_array[iterator]!=',')
                    iterator++;
        pos = iterator;

        while(instructions_array[iterator+1]==' ')
            iterator++;
        pos = iterator;

    }

    if(instructions_size[position]==2)
    {
        byte1_flag = 1;
        byte1_opcode = 0;
        address_flag = 0;
        address = 0;

        if(instructions_array[pos+2]=='h'||instructions_array[pos+2]=='H')
        {
            hex_to_dec(pos+1,0,1);
        }
        else if(instructions_array[pos+3]=='h'||instructions_array[pos+3]=='H')
        {
            hex_to_dec(pos+1,1,1);
            hex_to_dec(pos+2,0,1);
        }

        else if(instructions_array[pos+2]==10)
        {
           byte1_opcode = (unsigned char)string_to_num(pos,1);
        }
        else if(instructions_array[pos+3]==10)
        {
           byte1_opcode = (unsigned char)string_to_num(pos,2);
        }
        else if(instructions_array[pos+4]==10)
        {
            byte1_opcode = (unsigned char)string_to_num(pos,3);
        }
    }
    else if(instructions_size[position]==3 || program_counter_flag)
    {
        if(instructions_array[pos+1]>57 && position!=1 && position!=2)
        {
            mnemonic_arr_iterator=0;
            pos+=1;

            while(instructions_array[pos]!=10)
            {
                if(instructions_array[pos]!=' ')
                {
                    mnemonic_array[mnemonic_arr_iterator]=instructions_array[pos];
                    mnemonic_arr_iterator++;
                }
                pos++;
            }

            mnemonic_array[mnemonic_arr_iterator]='\0';

            for(int i=0;i<label_arr_iterator;i++)
            {
                if(!strcmp(mnemonic_array,label_set[i]))
                {
                    address = label_address[i];
                }
            }
            address_flag = 1;
        }
        else
        {
            byte1_flag = 0;
            address_flag = 1;
            address = 0;
            byte1_opcode = 0;

            if(instructions_array[pos+2]=='h'||instructions_array[pos+2]=='H')
            {
                address_flag = 1;
                num_of_digits = 1;
                for(int i=1;i<=num_of_digits;i++)
                {
                    hex_to_dec(pos+i,num_of_digits-i,3);
                }
            }
            else if(instructions_array[pos+3]=='h'||instructions_array[pos+3]=='H')
            {
                address_flag = 1;
                num_of_digits = 2;
                for(int i=1;i<=num_of_digits;i++)
                {
                    hex_to_dec(pos+i,num_of_digits-i,3);
                }
            }
            else if(instructions_array[pos+4]=='h'||instructions_array[pos+4]=='H')
            {
                address_flag = 1;
                num_of_digits = 3;
                for(int i=1;i<=num_of_digits;i++)
                {
                    hex_to_dec(pos+i,num_of_digits-i,3);
                }
            }
            else if(instructions_array[pos+5]=='h'||instructions_array[pos+5]=='H')
            {
                address_flag = 1;
                num_of_digits = 4;
                for(int i=1;i<=num_of_digits;i++)
                {
                    hex_to_dec(pos+i,num_of_digits-i,3);
                }
            }
            else if(instructions_array[pos+2]==10)
            {
                address = string_to_num(pos,1);
            }
            else if(instructions_array[pos+3]==10)
            {
                address = string_to_num(pos,2);
            }
            else if(instructions_array[pos+4]==10)
            {
                address = string_to_num(pos,3);
            }
            else if(instructions_array[pos+5]==10)
            {
                address = string_to_num(pos,4);
            }
            else if(instructions_array[pos+6]==10)
            {
                address = string_to_num(pos,5);
            }
        }
    }
}

/*
Function Name: string_to_num
Functionality: Converting the string into an equivalent decimal value.
*/

int string_to_num(int pos,int num_of_digits)
{
    int numeric_value=0;

    for(int i=0;i<num_of_digits;i++)
    {
        numeric_value += (instructions_array[pos+i+1]-48)*(pow(10,num_of_digits-i-1));
    }

    return numeric_value;
}



