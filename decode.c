#include <stdio.h>
#include <string.h>
#include "decode.h"

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    char *dot;

    printf("Validating decoding arguments...\n");

    if (argv[2] == NULL)
    {
        printf("ERROR: Missing stego image\n");
        return e_failure;
    }

    dot = strrchr(argv[2], '.');

    if (dot == NULL || strcmp(dot, ".bmp") != 0)
    {
        printf("ERROR: Stego image must be a BMP file\n");
        return e_failure;
    }

    decInfo->stego_image_fname = argv[2];

    printf("Stego image : %s\n",
           decInfo->stego_image_fname);

    printf("Decoding arguments validated successfully\n");

    return e_success;
}

Status open_decode_files(DecodeInfo *decInfo)
{
    printf("\n========== OPENING DECODING FILE ==========\n");

    printf("Opening stego image...\n");

    decInfo->fptr_stego_image =
        fopen(decInfo->stego_image_fname, "rb");

    if (decInfo->fptr_stego_image == NULL)
    {
        printf("ERROR: Unable to open stego image\n");
        return e_failure;
    }

    printf("Stego image opened successfully\n");

    return e_success;
}

char decode_byte_from_lsb(char *image_buffer)
{
    char data = 0;

    for (int i = 0; i < 8; i++)
    {
        data =
            data |
            ((image_buffer[i] & 1) << i);
    }

    return data;
}

Status decode_magic_string(DecodeInfo *decInfo)
{
    char image_buffer[8];
    int magic_size;
    char user_magic_string[MAX_MAGIC_STRING];

    printf("\n========== DECODING MAGIC STRING ==========\n");

    magic_size = 0;

    for (int i = 0; i < sizeof(int); i++)
    {
        if (fread(image_buffer, 1, 8,
                  decInfo->fptr_stego_image) != 8)
        {
            printf("ERROR: Unable to read magic string size\n");
            return e_failure;
        }

        magic_size =
            magic_size |
            ((int)decode_byte_from_lsb(image_buffer)
             << (i * 8));
    }

    if (magic_size <= 0 ||
        magic_size >= MAX_MAGIC_STRING)
    {
        printf("ERROR: Invalid magic string size\n");
        return e_failure;
    }

    printf("Magic string size : %d\n",
           magic_size);

    for (int i = 0; i < magic_size; i++)
    {
        if (fread(image_buffer, 1, 8,
                  decInfo->fptr_stego_image) != 8)
        {
            printf("ERROR: Unable to read magic string\n");
            return e_failure;
        }

        decInfo->magic_string[i] =
            decode_byte_from_lsb(image_buffer);
    }

    decInfo->magic_string[magic_size] = '\0';

    printf("Decoded magic string : %s\n",
           decInfo->magic_string);

    printf("Enter magic string: ");
    scanf("%19s", user_magic_string);

    if (strcmp(decInfo->magic_string,
               user_magic_string) == 0)
    {
        printf("Magic string matched successfully\n");
        return e_success;
    }

    printf("ERROR: Magic string mismatch\n");

    return e_failure;
}

Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    char image_buffer[8];

    printf("\n========== DECODING FILE EXTENSION ==========\n");

    for (int i = 0; i < MAX_FILE_SUFFIX; i++)
    {
        if (fread(image_buffer, 1, 8,
                  decInfo->fptr_stego_image) != 8)
        {
            printf("ERROR: Unable to read file extension\n");
            return e_failure;
        }

        decInfo->extn_secret_file[i] =
            decode_byte_from_lsb(image_buffer);
    }

    decInfo->extn_secret_file[MAX_FILE_SUFFIX - 1] = '\0';

    printf("Secret file extension : %s\n",
           decInfo->extn_secret_file);

    return e_success;
}

Status decode_secret_file_size(DecodeInfo *decInfo)
{
    char image_buffer[8];
    long size = 0;

    printf("\n========== DECODING FILE SIZE ==========\n");

    for (int i = 0; i < sizeof(long); i++)
    {
        if (fread(image_buffer, 1, 8,
                  decInfo->fptr_stego_image) != 8)
        {
            printf("ERROR: Unable to read file size\n");
            return e_failure;
        }

        size =
            size |
            ((long)decode_byte_from_lsb(image_buffer)
             << (i * 8));
    }

    decInfo->size_secret_file = size;

    printf("Secret file size : %ld bytes\n",
           decInfo->size_secret_file);

    return e_success;
}

Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char image_buffer[8];
    char ch;

    printf("\n========== DECODING SECRET FILE ==========\n");

    strcpy(decInfo->output_fname, "output");

    strcat(decInfo->output_fname,
           decInfo->extn_secret_file);

    printf("Output file : %s\n",
           decInfo->output_fname);

    decInfo->fptr_output =
        fopen(decInfo->output_fname, "wb");

    if (decInfo->fptr_output == NULL)
    {
        printf("ERROR: Unable to create output file\n");
        return e_failure;
    }

    for (long i = 0;
         i < decInfo->size_secret_file;
         i++)
    {
        if (fread(image_buffer, 1, 8,
                  decInfo->fptr_stego_image) != 8)
        {
            printf("ERROR: Unable to read secret data\n");

            fclose(decInfo->fptr_output);
            decInfo->fptr_output = NULL;

            return e_failure;
        }

        ch = decode_byte_from_lsb(image_buffer);

        if (fwrite(&ch, 1, 1,
                   decInfo->fptr_output) != 1)
        {
            printf("ERROR: Unable to write decoded data\n");

            fclose(decInfo->fptr_output);
            decInfo->fptr_output = NULL;

            return e_failure;
        }
    }

    fclose(decInfo->fptr_output);
    decInfo->fptr_output = NULL;

    printf("Secret file decoded successfully\n");

    return e_success;
}

Status do_decoding(DecodeInfo *decInfo)
{
    printf("\n");
    printf("========================================\n");
    printf("          STEGANOGRAPHY DECODING        \n");
    printf("========================================\n");

    if (open_decode_files(decInfo) == e_failure)
    {
        return e_failure;
    }

    printf("\nSkipping BMP header...\n");

    if (fseek(decInfo->fptr_stego_image,
              BMP_HEADER_SIZE,
              SEEK_SET) != 0)
    {
        printf("ERROR: Unable to skip BMP header\n");

        fclose(decInfo->fptr_stego_image);
        decInfo->fptr_stego_image = NULL;

        return e_failure;
    }

    printf("BMP header skipped successfully\n");

    if (decode_magic_string(decInfo) == e_failure)
    {
        fclose(decInfo->fptr_stego_image);
        decInfo->fptr_stego_image = NULL;

        return e_failure;
    }

    if (decode_secret_file_extn(decInfo) == e_failure)
    {
        fclose(decInfo->fptr_stego_image);
        decInfo->fptr_stego_image = NULL;

        return e_failure;
    }

    if (decode_secret_file_size(decInfo) == e_failure)
    {
        fclose(decInfo->fptr_stego_image);
        decInfo->fptr_stego_image = NULL;

        return e_failure;
    }

    if (decode_secret_file_data(decInfo) == e_failure)
    {
        fclose(decInfo->fptr_stego_image);
        decInfo->fptr_stego_image = NULL;

        return e_failure;
    }

    fclose(decInfo->fptr_stego_image);
    decInfo->fptr_stego_image = NULL;

    printf("\n========================================\n");
    printf("       DECODING COMPLETED SUCCESSFULLY  \n");
    printf("========================================\n");

    return e_success;
}