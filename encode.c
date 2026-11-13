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
    if(fread(&width, sizeof(int), 1, fptr_image)!=1)
    {
            printf("Error,failed to read width\n");
            return e_failure;
    }
    printf("width = %u\n", width);

    // Read the height (an int)
    if(fread(&height, sizeof(int), 1, fptr_image)!=1)
    {
            printf("Error,failed to read height\n");
            return e_failure;
    }
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
        encInfo->fptr_src_image=fopen(argv[2],"rb");
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
            encInfo->stego_image_fname=argv[4];
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
    

    //encoding magic string
    if(encode_magic_string(MAGIC_STRING,encInfo)==e_failure)
    {
        printf("Magic string encoding failed\n");
        return e_failure;
    }
    

    //encoding secret file extension size
    if(strrchr(encInfo->secret_fname,'.')==NULL)
    {
        printf("Inavlid secret fiel, No extension found\n");
        return e_failure;
    }
    
    char *dot=strrchr(encInfo->secret_fname,'.');
    char extn_secret_file[5];
    strcpy(extn_secret_file,dot);

    if(encode_secret_file_extn_size(strlen(extn_secret_file),encInfo)==e_failure)
    {
        printf("Encoding extn size failed\n");
        return e_failure;
    }
    

    //encoding secret file extension
    if(encode_secret_file_extn(extn_secret_file, encInfo)==e_failure)
    {
        printf("Encoding secret file extn failed");
        return e_failure;
    }

    
    //encode secret file size
    if(encode_secret_file_size(encInfo->size_secret_file,encInfo)==e_failure)
    {
         printf("Encoding secret file size failed");
        return e_failure;
    }
    

    //encode secret file data
    if(encode_secret_file_data(encInfo)==e_failure)
    {
        printf("Failed to encode secret file data\n");
        return e_failure;
    }
   

    //copy remaining data
    if(copy_remaining_img_data(encInfo->fptr_src_image,encInfo->fptr_stego_image)==e_failure)
    {
        printf("Failed to encode remaining data from secret file\n");
        return e_failure;
    }
    

    printf("\nSuccessfully Encoded...\n");
    return e_success;
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

    if(fwrite(buff,54,1,fptr_stego_image)!=1)
    return e_failure;


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
    unsigned char buff[8];
    
    for(int i=0;i<strlen(magic_string);i++)
    {
        if(fread(buff,8,1,encInfo->fptr_src_image)!=1)
        {
            printf("Error,failed to read buff for magic string\n");
            return e_failure;
        }
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
        if(fwrite(buff,8,1,encInfo->fptr_stego_image)!=1)
        {
            printf("Error,failed to write magic string\n");
            return e_failure;
        }
    }
    return e_success;
}

Status encode_byte_to_lsb(char data,  unsigned char *image_buffer)
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
    return e_success;

}

Status encode_secret_file_extn_size(uint size,EncodeInfo *encInfo)
{
    unsigned char buff[32];
    if(fread(buff,32,1,encInfo->fptr_src_image)!=1)
    {
            printf("Error,failed to read buff from source\n");
            return e_failure;
    }
    encode_size_to_lsb(size,buff);

    if(fwrite(buff,32,1,encInfo->fptr_stego_image)!=1)
    {
            printf("Error,failed to write secret file extn\n");
            return e_failure;
    }
    return e_success;
}

Status encode_size_to_lsb(uint size, unsigned char *image_buffer)
{
    for(int j=31;j>=0;j--)
    {
        if(size&(1u<<j))
        {
            image_buffer[31-j]|=(1u);
        }
        else
        image_buffer[31-j]&=~(1u);
    }
    return e_success;
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    unsigned char buff[8];
    for(int i=0;i<strlen(file_extn);i++)
    {
        if(fread(buff,8,1,encInfo->fptr_src_image)!=1)
        return e_failure;

        encode_byte_to_lsb(file_extn[i],buff);

        if(fwrite(buff,8,1,encInfo->fptr_stego_image)!=1)
        return e_failure;
    }
    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    unsigned char buff[8];
    char data;
    for(int i=0;i<encInfo->size_secret_file;i++)
    {
        if(fread(buff,8,1,encInfo->fptr_src_image)!=1)
        return e_failure;

        if(fread(&data,1,1,encInfo->fptr_secret)!=1)
        return e_failure;

        if(encode_byte_to_lsb(data,buff)==e_failure)
        {
            printf("Encoding secret file data to lsb failed\n");
            return e_failure;
        }

        if(fwrite(buff,8,1,encInfo->fptr_stego_image)!=1)
        {
            printf("writing failed\n");
            return e_failure;
        }
    }
    return e_success;

}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    unsigned char buff;
    while(fread(&buff,1,1,fptr_src))
    {
        if(fwrite(&buff,1,1,fptr_dest)!=1)
        return e_failure;        
    }

    return e_success;
}

Status encode_secret_file_size(uint file_size, EncodeInfo *encInfo)
{
    unsigned char buff[32];
    if(fread(buff,32,1,encInfo->fptr_src_image)!=1)
    return e_failure;

    if(encode_size_to_lsb(encInfo->size_secret_file,buff)==e_failure)
    {
        printf("Encoding secret file size to lsb failed\n");
        return e_failure;
    }

    if(fwrite(buff,32,1,encInfo->fptr_stego_image)!=1)
    return e_failure;

    return e_success;
    
}