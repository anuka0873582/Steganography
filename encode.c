#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"

uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;

    printf("Reading image dimensions...\n");

    fseek(fptr_image, 18, SEEK_SET);

    if (fread(&width, sizeof(uint), 1, fptr_image) != 1)
    {
        printf("ERROR: Unable to read image width\n");
        return 0;
    }

    if (fread(&height, sizeof(uint), 1, fptr_image) != 1)
    {
        printf("ERROR: Unable to read image height\n");
        return 0;
    }

    rewind(fptr_image);

    printf("Image width  : %u pixels\n", width);
    printf("Image height : %u pixels\n", height);

    return width * height * 3;
}

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    char *dot;

    printf("Validating encoding arguments...\n");

    if (argv[2] == NULL || argv[3] == NULL)
    {
        printf("ERROR: Missing encoding arguments\n");
        return e_failure;
    }

    dot = strrchr(argv[2], '.');

    if (dot == NULL || strcmp(dot, ".bmp") != 0)
    {
        printf("ERROR: Source image must be a BMP file\n");
        return e_failure;
    }

    encInfo->src_image_fname = argv[2];

    dot = strrchr(argv[3], '.');

    if (dot == NULL)
    {
        printf("ERROR: Secret file must have an extension\n");
        return e_failure;
    }

    if (strlen(dot) >= MAX_FILE_SUFFIX)
    {
        printf("ERROR: Secret file extension is too long\n");
        return e_failure;
    }

    encInfo->secret_fname = argv[3];

    strcpy(encInfo->extn_secret_file, dot);

    if (argv[4] == NULL)
    {
        encInfo->stego_image_fname = "stego.bmp";
    }
    else
    {
        dot = strrchr(argv[4], '.');

        if (dot == NULL || strcmp(dot, ".bmp") != 0)
        {
            printf("ERROR: Output image must be a BMP file\n");
            return e_failure;
        }

        encInfo->stego_image_fname = argv[4];
    }

    printf("Source image          : %s\n",
           encInfo->src_image_fname);

    printf("Secret file           : %s\n",
           encInfo->secret_fname);

    printf("Secret file extension : %s\n",
           encInfo->extn_secret_file);

    printf("Stego image           : %s\n",
           encInfo->stego_image_fname);

    printf("Encoding arguments validated successfully\n");

    return e_success;
}

Status open_files(EncodeInfo *encInfo)
{
    printf("\n========== OPENING FILES ==========\n");

    printf("Opening source image...\n");

    encInfo->fptr_src_image =
        fopen(encInfo->src_image_fname, "rb");

    if (encInfo->fptr_src_image == NULL)
    {
        printf("ERROR: Unable to open source image\n");
        return e_failure;
    }

    printf("Source image opened successfully\n");

    printf("Opening secret file...\n");

    encInfo->fptr_secret =
        fopen(encInfo->secret_fname, "rb");

    if (encInfo->fptr_secret == NULL)
    {
        printf("ERROR: Unable to open secret file\n");

        fclose(encInfo->fptr_src_image);
        encInfo->fptr_src_image = NULL;

        return e_failure;
    }

    fseek(encInfo->fptr_secret, 0, SEEK_END);

    encInfo->size_secret_file =
        ftell(encInfo->fptr_secret);

    rewind(encInfo->fptr_secret);

    printf("Secret file opened successfully\n");
    printf("Secret file size : %ld bytes\n",
           encInfo->size_secret_file);

    printf("Creating stego image...\n");

    encInfo->fptr_stego_image =
        fopen(encInfo->stego_image_fname, "wb");

    if (encInfo->fptr_stego_image == NULL)
    {
        printf("ERROR: Unable to create stego image\n");

        fclose(encInfo->fptr_src_image);
        fclose(encInfo->fptr_secret);

        encInfo->fptr_src_image = NULL;
        encInfo->fptr_secret = NULL;

        return e_failure;
    }

    printf("Stego image created successfully\n");
    printf("All files opened successfully\n");

    return e_success;
}

Status check_capacity(EncodeInfo *encInfo)
{
    long size_of_info;
    long image_capacity;

    printf("\n========== CHECKING CAPACITY ==========\n");

    size_of_info =
        strlen(encInfo->magic_string) +
        sizeof(int) +
        MAX_FILE_SUFFIX +
        sizeof(long) +
        encInfo->size_secret_file;

    image_capacity =
        get_image_size_for_bmp(encInfo->fptr_src_image);

    encInfo->image_capacity = image_capacity;

    printf("Required data : %ld bytes\n",
           size_of_info);

    printf("Image capacity : %u bits\n",
           encInfo->image_capacity);

    if ((size_of_info * 8) <= encInfo->image_capacity)
    {
        printf("Image has enough capacity\n");
        return e_success;
    }

    printf("ERROR: Image does not have enough capacity\n");

    return e_failure;
}

Status copy_bmp_header(FILE *fptr_src_image,
                       FILE *fptr_dest_image)
{
    char header[BMP_HEADER_SIZE];

    printf("\n========== COPYING BMP HEADER ==========\n");

    rewind(fptr_src_image);

    if (fread(header, 1, BMP_HEADER_SIZE,
              fptr_src_image) != BMP_HEADER_SIZE)
    {
        printf("ERROR: Unable to read BMP header\n");
        return e_failure;
    }

    if (fwrite(header, 1, BMP_HEADER_SIZE,
               fptr_dest_image) != BMP_HEADER_SIZE)
    {
        printf("ERROR: Unable to copy BMP header\n");
        return e_failure;
    }

    printf("BMP header copied successfully\n");

    return e_success;
}

Status encode_magic_string(const char *magic_string,
                           EncodeInfo *encInfo)
{
    int magic_size;

    printf("\nEncoding magic string...\n");

    magic_size = strlen(magic_string);

    if (encode_data_to_image((char *)&magic_size,
                             sizeof(magic_size),
                             encInfo->fptr_src_image,
                             encInfo->fptr_stego_image) == e_failure)
    {
        printf("ERROR: Failed to encode magic string size\n");
        return e_failure;
    }

    if (encode_data_to_image((char *)magic_string,
                             magic_size,
                             encInfo->fptr_src_image,
                             encInfo->fptr_stego_image) == e_failure)
    {
        printf("ERROR: Failed to encode magic string\n");
        return e_failure;
    }

    printf("Magic string encoded successfully\n");

    return e_success;
}

Status encode_secret_file_extn(const char *file_extn,
                               EncodeInfo *encInfo)
{
    char extension[MAX_FILE_SUFFIX] = {0};

    printf("\nEncoding secret file extension...\n");

    strcpy(extension, file_extn);

    if (encode_data_to_image(extension,
                             MAX_FILE_SUFFIX,
                             encInfo->fptr_src_image,
                             encInfo->fptr_stego_image) == e_failure)
    {
        printf("ERROR: Failed to encode file extension\n");
        return e_failure;
    }

    printf("Secret file extension encoded successfully\n");

    return e_success;
}

Status encode_secret_file_size(long file_size,
                               EncodeInfo *encInfo)
{
    printf("\nEncoding secret file size...\n");

    if (encode_data_to_image((char *)&file_size,
                             sizeof(file_size),
                             encInfo->fptr_src_image,
                             encInfo->fptr_stego_image) == e_failure)
    {
        printf("ERROR: Failed to encode file size\n");
        return e_failure;
    }

    printf("Secret file size encoded successfully\n");

    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char ch;

    printf("\nEncoding secret file data...\n");

    while (fread(&ch, 1, 1,
                 encInfo->fptr_secret) == 1)
    {
        if (encode_data_to_image(&ch,
                                 1,
                                 encInfo->fptr_src_image,
                                 encInfo->fptr_stego_image) == e_failure)
        {
            printf("ERROR: Failed to encode secret data\n");
            return e_failure;
        }
    }

    printf("Secret file data encoded successfully\n");

    return e_success;
}

Status encode_data_to_image(char *data,
                            int size,
                            FILE *fptr_src_image,
                            FILE *fptr_stego_image)
{
    char image_buffer[8];

    for (int i = 0; i < size; i++)
    {
        if (fread(image_buffer, 1, 8,
                  fptr_src_image) != 8)
        {
            printf("ERROR: Unable to read image data\n");
            return e_failure;
        }

        if (encode_byte_to_lsb(data[i],
                               image_buffer) == e_failure)
        {
            return e_failure;
        }

        if (fwrite(image_buffer, 1, 8,
                   fptr_stego_image) != 8)
        {
            printf("ERROR: Unable to write image data\n");
            return e_failure;
        }
    }

    return e_success;
}

Status encode_byte_to_lsb(char data,
                          char *image_buffer)
{
    for (int i = 0; i < 8; i++)
    {
        image_buffer[i] =
            (image_buffer[i] & 0xFE) |
            ((data >> i) & 1);
    }

    return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src,
                               FILE *fptr_dest)
{
    char ch;

    printf("\nCopying remaining image data...\n");

    while (fread(&ch, 1, 1, fptr_src) == 1)
    {
        if (fwrite(&ch, 1, 1, fptr_dest) != 1)
        {
            printf("ERROR: Unable to copy remaining image data\n");
            return e_failure;
        }
    }

    printf("Remaining image data copied successfully\n");

    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    printf("\n");
    printf("========================================\n");
    printf("          STEGANOGRAPHY ENCODING       \n");
    printf("========================================\n");

    printf("Enter magic string: ");
    scanf("%19s", encInfo->magic_string);

    printf("Magic string : %s\n",
           encInfo->magic_string);

    if (open_files(encInfo) == e_failure)
    {
        return e_failure;
    }

    if (check_capacity(encInfo) == e_failure)
    {
        fclose(encInfo->fptr_src_image);
        fclose(encInfo->fptr_secret);
        fclose(encInfo->fptr_stego_image);

        return e_failure;
    }

    if (copy_bmp_header(encInfo->fptr_src_image,
                        encInfo->fptr_stego_image) == e_failure)
    {
        fclose(encInfo->fptr_src_image);
        fclose(encInfo->fptr_secret);
        fclose(encInfo->fptr_stego_image);

        return e_failure;
    }

    if (encode_magic_string(encInfo->magic_string,
                            encInfo) == e_failure)
    {
        fclose(encInfo->fptr_src_image);
        fclose(encInfo->fptr_secret);
        fclose(encInfo->fptr_stego_image);

        return e_failure;
    }

    if (encode_secret_file_extn(encInfo->extn_secret_file,
                                encInfo) == e_failure)
    {
        fclose(encInfo->fptr_src_image);
        fclose(encInfo->fptr_secret);
        fclose(encInfo->fptr_stego_image);

        return e_failure;
    }

    if (encode_secret_file_size(encInfo->size_secret_file,
                                encInfo) == e_failure)
    {
        fclose(encInfo->fptr_src_image);
        fclose(encInfo->fptr_secret);
        fclose(encInfo->fptr_stego_image);

        return e_failure;
    }

    if (encode_secret_file_data(encInfo) == e_failure)
    {
        fclose(encInfo->fptr_src_image);
        fclose(encInfo->fptr_secret);
        fclose(encInfo->fptr_stego_image);

        return e_failure;
    }

    if (copy_remaining_img_data(encInfo->fptr_src_image,
                                encInfo->fptr_stego_image) == e_failure)
    {
        fclose(encInfo->fptr_src_image);
        fclose(encInfo->fptr_secret);
        fclose(encInfo->fptr_stego_image);

        return e_failure;
    }

    fclose(encInfo->fptr_src_image);
    fclose(encInfo->fptr_secret);
    fclose(encInfo->fptr_stego_image);

    printf("\n========================================\n");
    printf("        ENCODING COMPLETED SUCCESSFULLY\n");
    printf("========================================\n");

    return e_success;
}