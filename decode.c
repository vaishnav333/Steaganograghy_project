#include<stdio.h>
#include<string.h>
#include"decode.h"

#include"types.h"
#include"common.h"

Status read_and_validate_decode_args( char *argv[],DecodeInfo *decInfo)
{
    
    char *dot=strrchr(argv[2],'.');
    if(dot != NULL && strcmp(dot,".bmp")==0)
    {
        decInfo->src_image_fname=argv[2];
        decInfo->fptr_src_image=fopen(argv[2],"r");
        if(decInfo->fptr_src_image==NULL)
        {
            printf("Opening src image fptr failed\n");
            return e_failure;
        }
        

        if(decInfo->fptr_src_image==NULL)
        {
            printf("File not present");
            return e_failure;
        }
    }
    else
    {
        printf("Invalid file extension, only '.bmp' allowed");
        return e_failure;
    }
    
    if(argv[3]!=NULL)
    {
        strcpy(decInfo->secret_fname,strtok(argv[3],"."));
        
    }
    else
    {
        strcpy(decInfo->secret_fname,"dec_sec_file");
       
    }
    return e_success;
}

Status do_decoding(DecodeInfo *decInfo)
{
    if(fseek(decInfo->fptr_src_image,54,SEEK_SET) != 0)
    {
        printf("Failed to seek past BMP header\n");
        return e_failure;
    }

    if(decode_magic_string(MAGIC_STRING,decInfo)==e_failure)
    {
        printf("Magic string not matching\n");
        return e_failure;
    }

    if(decode_secret_file_extn_size(decInfo)==e_failure)
    {
        printf("Decoding secret file extn size failed\n");
        return e_failure;
    }

    if(decode_secret_file_extn(decInfo)==e_failure)
    {
        printf("Decoding secret file extn failed\n");
        return e_failure;
    }

    if(decode_secret_file_size(decInfo)==e_failure)
    {
        printf("Decoding secret file size failed\n");
        return e_failure;
    }

    if(decode_secret_file_data(decInfo)==e_failure)
    {
        printf("Decoding secret file data failed\n");
        return e_failure;
    }

    fclose(decInfo->fptr_secret);
    fclose(decInfo->fptr_src_image);

    printf("Decoding secret file successfull\n");
    return e_success;
}

Status decode_magic_string(char *magic_string, DecodeInfo *decInfo)
{
    unsigned char buff[8];
    unsigned char decoded;
    for(size_t i=0; i<strlen(magic_string); i++)
    {
        if(fread(buff,1,sizeof(buff),decInfo->fptr_src_image) != sizeof(buff))
        {
            printf("Reading byte from source failed\n");
            return e_failure;
        }

        if(decode_byte_from_lsb(&decoded,buff)==e_failure)
        {
            printf("Decoding magic string from lsb failed\n");
            return e_failure;
        }
        if(decoded != (unsigned char)magic_string[i])
        {
            return e_failure;
        }
        // printf("%c",decoded);
    }
    return e_success;
}

Status decode_byte_from_lsb(unsigned char *data, unsigned char *buff)
{
    *data = 0;
    for(int k=0;k<8;k++)
    {
        if(buff[k]&1)
        {
            *data |= (unsigned char)(1u << (7-k));
        }
    }
    return e_success;
}

Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    char buff[32]={0};
    int size;
    if(fread(buff,32,1,decInfo->fptr_src_image)!=1)
    {
        printf("Reading data from file failed\n");
        return e_failure;
    }
    if(decode_size_from_lsb(buff,&size)==e_failure)
    {
        printf("Decoding size of extn failed\n");
        return e_failure;
    }
    // printf("%d",size);
    decInfo->secret_file_extn_size=size;

    return e_success;
}

Status decode_size_from_lsb(unsigned char *s_buff,unsigned int *size)
{
    *size=0;
    for(int i=0;i<32;i++)
    {
        if(s_buff[i]&1)
        {
            *size|=(1u<<(31-i));
        }
    }
    return e_success;
}

Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    
    char s_extn[decInfo->secret_file_extn_size+1];
    char buff[8];
    for(int i=0;i<decInfo->secret_file_extn_size;i++)
    {
        if(fread(buff,1,8,decInfo->fptr_src_image)!=8)
        {
            printf("Reading data to buffer for secret file extn failed\n");
            return e_failure;
        }
        decode_byte_from_lsb(&s_extn[i],buff);
    }
    s_extn[decInfo->secret_file_extn_size]='\0';
    strcat(decInfo->secret_fname,s_extn);
    // printf("%s\n",s_extn);
    decInfo->fptr_secret=fopen(decInfo->secret_fname,"w");
    if(decInfo->fptr_secret==NULL)
    {
        printf("Opening secret file fptr failed\n");
        return e_failure;
    }
    
    return e_success;
//    printf("%s",s_extn);
}

Status decode_secret_file_size(DecodeInfo *decInfo)
{
    unsigned int size;
    unsigned char buff[32];
    if(fread(buff,32,1,decInfo->fptr_src_image)!=1)
    return e_failure;

    if(decode_size_from_lsb(buff,&size)==e_failure)
    {
        printf("Decoding size of secret data failed\n");
        return e_failure;
    }
    decInfo->size_secret_file=size;
    return e_success;
}

Status decode_secret_file_data(DecodeInfo *decInfo)
{
    unsigned char buff[8];
    unsigned char data;
    for(int i=0;i<decInfo->size_secret_file;i++)
    {
        if(fread(buff,8,1,decInfo->fptr_src_image)!=1)
        return e_failure;

        decode_byte_from_lsb(&data,buff);

        if(fwrite(&data,1,1,decInfo->fptr_secret)!=1)
        return e_failure;
    }
    return e_success;
}

