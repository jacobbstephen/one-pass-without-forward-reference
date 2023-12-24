#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>

#define text_len 61
#define len 100
#define arr_len 7

char line[len], label[arr_len], opcode[arr_len],operand[arr_len],opvalue[arr_len];
char addr[arr_len]; int textrec_len;
char text_rec[len]; 
FILE *source, *optab,*symtab,*object;

void readLine(){
    label[0] = opcode[0] = operand[0] = '\0';
    fgets(line,len,source);
    int cnt = sscanf(line,"%s %s %s",label,opcode,operand) ;
    if(cnt == 2){
        sscanf(line,"%s %s",opcode,operand);
        label[0] = '\0';
    }else if(cnt == 1){
        sscanf(line,"%s",opcode);
        label[0] = operand[0] = '\0';
    }
    else sscanf(line,"%s %s %s\n",label,opcode,operand);
}

bool search(FILE* ptr, char arr[]){
    rewind(ptr);
    char symbol[arr_len];
    while(fscanf(ptr, "%s %s", symbol, addr) != EOF){
        if(!strcmp(arr,symbol)) return true;
    }
    return false;
}

void insert_symtab(int locctr){
    fprintf(symtab, "%s\t%X\n",label,locctr);
}
void pass(){
    readLine();

    text_rec[0] = '\0';
    char start_addr[arr_len];
    int locctr; bool found; int length = 16;
    bool set_error_flag = false;

    if(!strcmp(opcode,"START")){
        strcpy(start_addr,operand);
        locctr  = (int) strtol(start_addr,NULL,16);
        fprintf(object,"H^%-6s^%06s^%06x\n",label,operand,length);
    }else locctr = 0;

    readLine();

    strcpy(text_rec,"T^00");
    strcat(text_rec,start_addr);
    strcat(text_rec,"^00^");
    textrec_len = 0;

    while(strcmp(opcode,"END") != 0){

        if(label[0]){
            found = search(symtab,label);
            if(found) set_error_flag = true;
            else insert_symtab(locctr);
            
        }
        
        found = search(optab,opcode);

        if(found){
            locctr += 3;
            strcpy(opvalue,addr);
            if(operand[0]){
                found = search(symtab,operand);
                if(!found) set_error_flag = true;
            }
            else strcpy(addr,"0000");
            strcat(text_rec,opvalue);
            strcat(text_rec,addr);
            strcat(text_rec,"^");
            textrec_len += 6;
        } 
        
        else if(!strcmp(opcode,"WORD")){
            locctr += 3;
            strcat(text_rec,"0000");
            strcat(text_rec,operand);
            strcat(text_rec,"^");
            textrec_len += 6;
        }

        else if(!strcmp(opcode,"RESW")) locctr += 3 * atoi(operand);
        
        else if(!strcmp(opcode,"RESB")) locctr +=  atoi(operand);
        
        else if(!strcmp(opcode,"BYTE")){

            if(operand[0] == 'X'){
                char c[2];
                for(int i = 2; i < strlen(operand) - 1; i++){
                    sprintf(c,"%c",operand[i]);
                    strcat(text_rec,c);
                }

                locctr += (strlen(operand) - 3) /2;
                textrec_len += strlen(operand) - 3;

            } 

            else if(operand[0] == 'C'){

                char c[2];
                locctr += strlen(operand) - 3;
                for(int i = 2; i < strlen(operand) - 1; i++){
                    sprintf(c,"%X",operand[i]);
                    strcat(text_rec,c);
                }
                strcat(text_rec,"^");
                textrec_len += 6;
            } 
        }
        else printf("INVALID\n");

        readLine();

        if (textrec_len == 60 || strlen(text_rec) >= 75){

            char str[7];
            fprintf(object,"%s\n",text_rec);
            textrec_len = 0;
            text_rec[0] = '\0';
            strcpy(text_rec,"T^00");
            sprintf(str,"%06X",locctr);
            strcat(text_rec,str);
            strcat(text_rec,"^00^");

        }
    }
    
    textrec_len = textrec_len / 2;

    char str[3];
    sprintf(str,"%d",textrec_len);
    strncpy(text_rec+9,str,2);
    fprintf(object,"%s\n",text_rec);
    fprintf(object,"E^%06s\n",start_addr);

    int program_length = locctr - (int) strtol(start_addr,NULL,16);
    printf("L: %X\n",locctr);
    fseek(object,16,SEEK_SET);
    fprintf(object,"%06X\n",program_length);

}
int main(){
    source = fopen("source.txt","r");
    optab = fopen("optab.txt","r");
    symtab = fopen("symtab.txt","w+");
    object = fopen("obj.txt","w+");
    pass();
    if(source) fclose(source);
    if(optab) fclose(optab);
    if(symtab) fclose(symtab);
    if (object) fclose(object);
    return(0);
}