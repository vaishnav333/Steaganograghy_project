#include <stdio.h>
#include<string.h>
#include "encode.h"
#include "types.h"
#include"common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n\n", height);

    // Return image capacity
    return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */

/*
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

    	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}
*/


Status read_and_validate_encode_args(char *argv[],EncodeInfo *encInfo)
{
    // Source file validation
    char *source=argv[2];
    int s_len=strlen(source);

    if(strcmp(&source[s_len-4],".bmp")==0)
    {
        encInfo->src_image_fname=argv[2];
        encInfo->fptr_src_image=fopen(argv[2],"r");
        if(encInfo->fptr_src_image==NULL)
        {
            printf("Source file not present\n");
            return e_failure;
        }
    }
    else
    {
        printf("Invalid source file extension\n");
        return e_failure;
    }



    // secret file validation 

    encInfo->secret_fname=argv[3];
    encInfo->fptr_secret=fopen(argv[3],"r");
    if(encInfo->fptr_secret==NULL)
    {
        printf("Secret File not present\n");
        return e_failure;
    }



    // for encoded file 

    if(argv[4]!=NULL)
    {
        char *stego_n=argv[4];
        if(strcmp(&stego_n[s_len-4],".bmp")==0)
        {
            encInfo->src_image_fname=argv[4];
            encInfo->fptr_stego_image=fopen(argv[4],"w");
        }
        else
        {
            printf("Invalid source file extension(only .bmp allowed)\n");
            return e_failure;
        }
    }  
    else
    {
        encInfo->stego_image_fname="default.bmp";
        encInfo->fptr_stego_image=fopen("default.bmp","w");
    } 
    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    /*
    ->check capacity==e_failure
        print error,return e_failure
    ->copy_bmp_header(encInfo -> fptr_src_image,encInfo->fptr_stego_image)==e_failure
        print error,return e_failure
    ->encode_magic_string( MAGIC_STRING, encInfo)
    */


    //checking for enough capacity in source file
    if(check_capacity(encInfo)==e_failure)
        {
            printf("Not enough capacity in source file\n");     
            return e_failure;
        }

    //encoding header of source
    if(copy_bmp_header(encInfo->fptr_src_image,encInfo->fptr_stego_image)==e_failure)
        {
            printf("Failed to encode header\n");
            return e_failure;
        }
    else 
    printf("Encoded header successfully\n");

    //encoding magic string
    if(encode_magic_string(MAGIC_STRING,encInfo)==e_failure)
    {
        printf("Magic string encoding failed\n");
        return e_failure;
    }
    else
    printf("Encoded magic string successfully\n");

    //encoding
}

Status check_capacity(EncodeInfo *encInfo)
{
    
    // -> get_image_size_for_bmp(encInfo->fptr_src_image) > ((get_file_size(encInfo->fptr_secret)+14)*8)
    // get_image_size_for_bmp(encInfo->)

    encInfo->image_capacity=get_image_size_for_bmp(encInfo->fptr_src_image);
    encInfo->size_secret_file=get_file_size(encInfo->fptr_secret);

    printf("source capacity=%u\nsource capacity needed for encoding=%lu\n\n",encInfo->image_capacity,(encInfo->size_secret_file+14)*8);
    
    if(encInfo->image_capacity>=((encInfo->size_secret_file+14)*8))
    return e_success;
    else
    return e_failure;
}

uint get_file_size(FILE *fptr)
{
    /*
        ->fseek() to SEEK_END
        ->return ftell()
    */
    
    fseek(fptr,0,SEEK_END);
    uint size=ftell(fptr);
    rewind(fptr);
    printf("File size: %u\n",size);
    return size;
}

Status copy_bmp_header(FILE *fptr_src_image,FILE *fptr_stego_image)
{
    /*
    declare buffer of 54 bytes, char buff[54]
    read 54 bytes from src img to buff
    */
    char buff[54];

    rewind(fptr_src_image);
    rewind(fptr_stego_image);

    if(fread(buff,54,1,fptr_src_image)!=1)
    return e_failure;

    fwrite(buff,54,1,fptr_stego_image);
    return e_success;
}

Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    /*
    -> declare a buffer of 8 bytes    
    => Run a loop for length of magic string 
    -> read 8bytes from src_image into the buff
    -> encode_byte_to_lsb(magic_string[],buff) 
    -> Write 8 bytes of encoded data from buff to dest_img
    */
    char buff[8];
    
    for(int i=0;i<strlen(magic_string);i++)
    {
        fread(buff,8,1,encInfo->fptr_src_image);
    /*
        printf("\nmagic s:%c\nbinary:",magic_string[i]);
        for(int j=7;j>=0;j--)
        {
            if(magic_string[i]&(1<<j))
            printf("1");
            else
            printf("0");
        }
        printf("\n");
    */
        encode_byte_to_lsb(magic_string[i],buff);
        
        // printf("\n--%d\n",i);
        fwrite(buff,8,1,encInfo->fptr_stego_image);
    }
    return e_success;
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    /*
        =>loop for 8 times
            check the nth bit is set or not
            -> if set, set the LSB of image_buffer[]
            -> if clear, clear the LSB of image_buffer[]
        =>

    */

    for(int i=7;i>=0;i--)
    {
        if(data&(1<<i))
        {
            image_buffer[7-i]=image_buffer[7-i]|1;
        }

        else
        {
            image_buffer[7-i]=image_buffer[7-i]&(~1);
        }
    /*
    =>
        printf("\nbuff:");
        for(int j=7;j>=0;j--)
        {
            if(image_buffer[7-i]&(1<<j))
            printf("1");
            else
            printf("0");
        }
        printf("\n");
    */
    }

}

Status encode_secret_file_extn_size(uint size,EncodeInfo *encInfo)
{
    char buff[32];
    fread(buff,32,1,encInfo->fptr_src_image);
    encode_size_to_lsb(size,buff);

    fwrite(buff,32,1,encInfo->fptr_stego_image);
    return e_success;
}

Status encode_size_to_lsb(uint size, char *image_buffer)
{

}