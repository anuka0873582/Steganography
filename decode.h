#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include "types.h"

#define MAX_FILE_SUFFIX 10
#define MAX_MAGIC_STRING 20
#define BMP_HEADER_SIZE 54

typedef struct _DecodeInfo
{
    char *stego_image_fname;
    FILE *fptr_stego_image;

    char magic_string[MAX_MAGIC_STRING];

    char extn_secret_file[MAX_FILE_SUFFIX];
    long size_secret_file;

    char output_fname[50];
    FILE *fptr_output;

} DecodeInfo;

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo); // Validates decoding arguments
Status do_decoding(DecodeInfo *decInfo); // Performs complete decoding
Status open_decode_files(DecodeInfo *decInfo); // Opens stego image
char decode_byte_from_lsb(char *image_buffer); // Decodes one byte from image LSBs
Status decode_magic_string(DecodeInfo *decInfo); // Decodes and verifies magic string
Status decode_secret_file_extn(DecodeInfo *decInfo); // Decodes file extension
Status decode_secret_file_size(DecodeInfo *decInfo); // Decodes file size
Status decode_secret_file_data(DecodeInfo *decInfo); // Decodes secret file data

#endif