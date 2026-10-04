#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include <string.h>
#include "types.h"    // Contains user defined types

/*
 * Structure to store information required for
 * decoding secret file from stego Image
 * Info about output and intermediate data is
 * also stored
 */

typedef struct _DecodeInfo
{
    /* Stego Image info */
    char *stego_image_fname;     // To store the stego image fname
    FILE *fptr_stego_image;      // To store the address of the stego image

    /* Output Secret File Info */
    char *output_fname;          // To store the output file name
    FILE *fptr_output;           // To store the address of output file

    char extn_secret_file[20];   // To store the secret file extension
    long size_secret_file;       // To store the size of the secret file

} DecodeInfo;

/* Decoding function prototype */

/* Read and validate Decode args from argv */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Perform the decoding */
Status do_decoding(DecodeInfo *decInfo);

/* Get File pointers for i/p and o/p files */
Status open_decode_files(DecodeInfo *decInfo);

/* Decode Magic String */
Status decode_magic_string(DecodeInfo *decInfo);

/* Decode extension size */
Status decode_secret_file_extn_size(int *size, DecodeInfo *decInfo);

/* Decode secret file extension */
Status decode_secret_file_extn(char *file_extn,int size, DecodeInfo *decInfo);

/* Decode secret file size */
Status decode_secret_file_size(long *file_size, DecodeInfo *decInfo);

/* Decode secret file data */
Status decode_secret_file_data(DecodeInfo *decInfo);

/* Decode one byte from LSB */
Status decode_byte_from_lsb(char *data, char *image_buffer);

/* Decode size from LSB */
Status decode_size_from_lsb(int *size, char *image_buffer);

#endif