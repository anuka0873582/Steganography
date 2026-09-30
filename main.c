#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "decode.h"

OperationType check_operation_type(char *argv[]) // Checks whether encode or decode is selected
{
    if (strcmp(argv[1], "-e") == 0)
    {
        return e_encode;
    }
    else if (strcmp(argv[1], "-d") == 0)
    {
        return e_decode;
    }
    else
    {
        return e_unsupported;
    }
}

int main(int argc, char *argv[])
{
    OperationType res;

    printf("\n========================================\n");
    printf("       IMAGE STEGANOGRAPHY PROJECT      \n");
    printf("========================================\n");

    if (argc < 2)
    {
        printf("ERROR: Operation is missing\n");
        printf("\nUsage:\n");
        printf("./stego -e source.bmp secretfile output.bmp\n");
        printf("./stego -d stego.bmp\n");
        return 0;
    }

    res = check_operation_type(argv);

    if (res == e_encode)
    {
        EncodeInfo encInfo;

        printf("\nENCODING OPERATION SELECTED\n");

        if (argc < 4 || argc > 5)
        {
            printf("ERROR: Invalid encoding arguments\n");
            printf("Usage: ./stego -e source.bmp secretfile output.bmp\n");
            return 0;
        }

        if (read_and_validate_encode_args(argv, &encInfo) == e_success)
        {
            if (do_encoding(&encInfo) == e_success)
            {
                printf("\nENCODING SUCCESSFULLY COMPLETED\n");
            }
            else
            {
                printf("\nENCODING FAILED\n");
            }
        }
        else
        {
            printf("\nERROR: INVALID ENCODING ARGUMENTS\n");
        }
    }
    else if (res == e_decode)
    {
        DecodeInfo decInfo;

        printf("\nDECODING OPERATION SELECTED\n");

        if (argc != 3)
        {
            printf("ERROR: Invalid decoding arguments\n");
            printf("Usage: ./stego -d stego.bmp\n");
            return 0;
        }

        if (read_and_validate_decode_args(argv, &decInfo) == e_success)
        {
            if (do_decoding(&decInfo) == e_success)
            {
                printf("\nDECODING SUCCESSFULLY COMPLETED\n");
            }
            else
            {
                printf("\nDECODING FAILED\n");
            }
        }
        else
        {
            printf("\nERROR: INVALID DECODING ARGUMENTS\n");
        }
    }
    else
    {
        printf("\nERROR: UNSUPPORTED OPERATION\n");
        printf("Use -e for encoding or -d for decoding\n");
    }

    printf("\n========================================\n");
    printf("             PROGRAM EXITED             \n");
    printf("========================================\n");

    return 0;
}