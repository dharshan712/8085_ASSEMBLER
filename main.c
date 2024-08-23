#include "main.h"

int main()
{
    label_memorize();

    FILE *fp,*destination_fp;

    fp = fopen(source_file_name,source_file_mode);                          //Opening the source file and destination file in their respective mode.
    destination_fp = fopen(destination_file_name,destination_file_mode);

    if(fp == NULL)
    {
        printf("Error in Opening the ASM file");
    }
    else
    {
        while(fgets(instructions_array,sizeof(instructions_array),fp))
        {
            if(instructions_array[0]!='\n')
            {
                mnemonic_cmp();                                             //Extracting the mnemonic and comparing it with mnemonic set and extracting its equivalent opcode.

                if(!program_counter_flag)                                   //Checking whether it is a program counter initialization assembler directive.The flag will be set in mnemonic_cmp function.
                {
                    operation();
                }

                if(destination_fp==NULL)
                {
                    printf("Error in Opening the destination file");
                }
                else if(!program_counter_flag)
                {
                    sprintf(hex_string, "0x%04X : ", program_counter);      //Converting into an hexadecimal string and writing into an destination file.
                    fputs(hex_string,destination_fp);
                    program_counter += instructions_size[position];

                    if(instructions_size[position]==1)                      //If it is 1 byte instruction
                    {
                        sprintf(hex_string, "%02X ", equivalent_opcode);
                        fputs(hex_string,destination_fp);
                    }
                    else if(byte1_flag==1)                                  //If it is 2 byte instruction
                    {
                        byte1_flag=0;

                        sprintf(hex_string, "%02X ", equivalent_opcode);
                        fputs(hex_string,destination_fp);

                        sprintf(hex_string, "%02X ", byte1_opcode);
                        fputs(hex_string,destination_fp);
                    }
                    else if(address_flag==1)                                //If it is 4 byte instruction
                    {
                        sprintf(hex_string, "%02X ", equivalent_opcode);
                        fputs(hex_string,destination_fp);

                        sprintf(hex_string, "%04X ", address);
                        fputc(hex_string[2],destination_fp);
                        fputc(hex_string[3],destination_fp);
                        fputc(' ',destination_fp);
                        fputc(hex_string[0],destination_fp);
                        fputc(hex_string[1],destination_fp);
                    }

                    num_of_spaces=0;                                        //Doing for alignment purpose in destination file.

                    if(instructions_size[position]==1)
                        num_of_spaces = 5;
                    else if(instructions_size[position]==2)
                        num_of_spaces = 2;

                    for(int i=0;i<(10+num_of_spaces);i++)
                        fputc(' ',destination_fp);

                    fputs(instructions_array,destination_fp);
                }
                 program_counter_flag = 0;
            }
            fputc('\n',destination_fp);                                     //Entering into a New line in destination file.
        }
    }
    printf("Successfully generated the Machine Code\n");
    fclose(fp);                                                              //Closing the files.
    fclose(destination_fp);
    return 3;
}
