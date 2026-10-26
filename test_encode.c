#include <stdio.h>
#include "encode.h"
#include "types.h"

int main(int argc,char *argv[])
{
    EncodeInfo encInfo;

    if(argc<3)
    {
        printf("Not enough command line arguments \n");
        return 0;
    }
    
    char *opt=argv[1];
    OperationType check=check_operation_type(opt[1]);

    if(check==e_encode)
    {
        if(argc<4)
        {
            printf("Not enough command line arguments to encode\n");
            return 0;
        } 

        if(read_and_validate_encode_args(argv,&encInfo)==e_failure)
        {
            printf("Check arguments given in command line\n");
            return 0;
        }  

        if(do_encoding(&encInfo)==e_failure)
        {
            printf("Encoding failed\n");
            return 0;
        }

    }
    else if(check==e_decode)
    {

    }

    
    else 
    {
        printf("Inavlid command line");
    }
    return 0;

}

OperationType check_operation_type(char opt)
{
    if(opt=='e')
    return e_encode;
    if(opt=='d')
    return e_decode;
    return e_unsupported;
}
