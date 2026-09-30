#ifndef ENCODE_H
#define ENCODE_H

#include <stdio.h>
#include "types.h"

#define MAX_FILE_SUFFIX 10
#define MAX_MAGIC_STRING 20
#define BMP_HEADER_SIZE 54

typedef struct _EncodeInfo
{
    char *src_image_fname;
    FILE *fptr_src_image;

    uint image_capacity;

    char *secret_fname;
    FILE *fptr_secret;

    char extn_secret_file[MAX_FILE_SUFFIX];
    long size_secret_file;

    char magic_string[MAX_MAGIC_STRING];

    char *stego_image_fname;
    FILE *fptr_stego_image;

} EncodeInfo;

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo); // Validates encoding arguments
Status do_encoding(EncodeInfo *encInfo); // Performs complete encoding
Status open_files(EncodeInfo *encInfo); // Opens required files
Status check_capacity(EncodeInfo *encInfo); // Checks image capacity
uint get_image_size_for_bmp(FILE *fptr_image); // Gets BMP image capacity
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image); // Copies BMP header
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo); // Encodes magic string
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo); // Encodes file extension
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo); // Encodes file size
Status encode_secret_file_data(EncodeInfo *encInfo); // Encodes secret file data
Status encode_data_to_image(char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image); // Encodes data into image
Status encode_byte_to_lsb(char data, char *image_buffer); // Encodes one byte using LSB
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest); // Copies remaining image data

#endif