#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "common.h"
#include "types.h"

/* Read and validate command line arguments */

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    char *ptr;   // Pointer used to store the result returned by strstr()

    /* Validate stego image */

    // Check whether the input stego image has ".bmp" extension
    ptr = strstr(argv[2], ".bmp");

    if (ptr != NULL && strcmp(ptr, ".bmp") == 0)
    {
        // Store the stego image file name
        decInfo->stego_image_fname = argv[2];
    }
    else
    {
        // Invalid stego image format
        printf("ERROR: Invalid Stego Image\n");
        return e_failure;
    }

    /* Output file */

    // Check whether the output file name is provided
    if (argv[3] != NULL)
    {
        // Store the output file name
        decInfo->output_fname = argv[3];
    }
    else
    {
        // Use the default output file name
        decInfo->output_fname = "decoded";
    }

    // Validation completed successfully
    return e_success;
}

Status open_decode_files(DecodeInfo *decInfo)
{
    /* Open Stego Image */

    // Open the stego image in read mode
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "r");

    // Check whether the stego image opened successfully
    if (decInfo->fptr_stego_image == NULL)
    {
        // Print the system error message
        perror("fopen");

        // Print the custom error message
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->stego_image_fname);

        return e_failure;
    }

    // Output file will be opened after decoding the file extension

    // File opened successfully
    return e_success;
}

Status decode_byte_from_lsb(char *data, char *image_buffer)
{
    // Initialize the decoded character
    *data = 0;

    // Decode all 8 bits from the image bytes
    for (int i = 0; i < 8; i++)
    {
        // Shift the decoded character left by one bit
        *data = *data << 1;

        // Extract the LSB and append it to the decoded character
        *data = *data | (image_buffer[i] & 1);
    }

    // Character decoded successfully
    return e_success;
}

Status decode_size_from_lsb(int *size, char *imageBuffer)
{
    // Initialize the decoded size
    *size = 0;

    // Decode all 32 bits from the image bytes
    for (int i = 0; i < 32; i++)
    {
        // Shift the decoded size left by one bit
        *size = *size << 1;

        // Extract the LSB and append it to the decoded size
        *size = *size | (imageBuffer[i] & 1);
    }

    // Size decoded successfully
    return e_success;
}

Status decode_magic_string(DecodeInfo *decInfo)
{
    // Buffer to store 8 bytes of image data
    char image_buffer[8];

    // Buffer to store the decoded magic string
    char magic_str[strlen(MAGIC_STRING) + 1];

    // Decode each character of the magic string
    for (int i = 0; i < strlen(MAGIC_STRING); i++)
    {
        // Read 8 bytes from the stego image
        if (fread(image_buffer, 8, 1, decInfo->fptr_stego_image) != 1)
        {
            return e_failure;
        }

        // Decode one character from the image bytes
        decode_byte_from_lsb(&magic_str[i], image_buffer);
    }

    // Add the null character at the end of the decoded string
    magic_str[strlen(MAGIC_STRING)] = '\0';

    // Compare the decoded magic string with the original one
    if (strcmp(magic_str, MAGIC_STRING) == 0)
    {
        // Magic string verified successfully
        return e_success;
    }

    // Magic string verification failed
    return e_failure;
}

Status decode_secret_file_extn_size(int *size, DecodeInfo *decInfo)
{
    // Buffer to store 32 bytes of image data
    char image_buffer[32];

    // Read 32 bytes from the stego image
    fread(image_buffer, 32, 1, decInfo->fptr_stego_image);

    // Decode the extension size
    decode_size_from_lsb(size, image_buffer);

    // Extension size decoded successfully
    return e_success;
}

Status decode_secret_file_extn(char *file_extn, int size, DecodeInfo *decInfo)
{
    // Buffer to store 8 bytes of image data
    char image_buffer[8];

    // Decode each character of the secret file extension
    for (int i = 0; i < size; i++)
    {
        // Read 8 bytes from the stego image
        fread(image_buffer, 8, 1, decInfo->fptr_stego_image);

        // Decode one extension character
        decode_byte_from_lsb(&file_extn[i], image_buffer);
    }

    // Add the null character at the end of the extension
    file_extn[size] = '\0';

    // Store the decoded extension in the structure
    strcpy(decInfo->extn_secret_file, file_extn);

    // Extension decoded successfully
    return e_success;
}
Status decode_secret_file_size(long *file_size, DecodeInfo *decInfo)
{
    // Buffer to store 32 bytes of image data
    char image_buffer[32];

    // Variable to store the decoded file size
    int size;

    // Read 32 bytes from the stego image
    fread(image_buffer, 32, 1, decInfo->fptr_stego_image);

    // Decode the secret file size from the image bytes
    decode_size_from_lsb(&size, image_buffer);

    // Store the decoded file size
    *file_size = size;
    decInfo->size_secret_file = size;

    // Secret file size decoded successfully
    return e_success;
}

Status decode_secret_file_data(DecodeInfo *decInfo)
{
    // Buffer to store 8 bytes of image data
    char image_buffer[8];

    // Variable to store one decoded character
    char ch;

    // Decode each character of the secret file
    for(int i = 0; i < decInfo->size_secret_file; i++)
    {
        // Read 8 bytes from the stego image
        fread(image_buffer, 8, 1, decInfo->fptr_stego_image);

        // Decode one character from the image bytes
        decode_byte_from_lsb(&ch, image_buffer);

        // Write the decoded character into the output file
        fputc(ch, decInfo->fptr_output);
    }

    // Secret file data decoded successfully
    return e_success;
}

Status do_decoding(DecodeInfo *decInfo)
{
    // Variable to store the decoded extension size
    int extn_size;

    // Buffer to store the decoded file extension
    char extn_secret_file[50];

    // Variable to store the decoded secret file size
    long file_size;

    // Buffer to create the output file name
    char filename[50];

    /* Step 1 : Open Stego Image */

    // Open the stego image file
    if (open_decode_files(decInfo) == e_success)
    {
        printf("INFO: Files opened successfully\n");
    }
    else
    {
        // Failed to open the stego image
        printf("ERROR: Unable to open files\n");
        return e_failure;
    }

    /* Step 2 : Skip BMP Header */

    // Skip the 54-byte BMP header
    fseek(decInfo->fptr_stego_image, 54, SEEK_SET);

    /* Step 3 : Decode Magic String */

    // Verify the magic string
    if (decode_magic_string(decInfo) == e_success)
    {
        printf("INFO: Magic String Decoded Successfully\n");
    }
    else
    {
        // Magic string verification failed
        printf("ERROR: Magic String Not Matched\n");
        fclose(decInfo->fptr_stego_image);
        return e_failure;
    }

    /* Step 4 : Decode Extension Size */

    // Decode the size of the secret file extension
    if (decode_secret_file_extn_size(&extn_size, decInfo) == e_success)
    {
        printf("INFO: Extension Size Decoded Successfully\n");
    }
    else
    {
        // Failed to decode the extension size
        printf("ERROR: Extension Size Decoding Failed\n");
        fclose(decInfo->fptr_stego_image);
        return e_failure;
    }

    /* Step 5 : Decode Extension */

    // Decode the secret file extension
    if (decode_secret_file_extn(extn_secret_file, extn_size, decInfo) == e_success)
    {
        printf("INFO: Extension Decoded Successfully\n");
    }
    else
    {
        // Failed to decode the extension
        printf("ERROR: Extension Decoding Failed\n");
        fclose(decInfo->fptr_stego_image);
        return e_failure;
    }

    /* Step 6 : Create Output File Name */

    // Create the output file name by appending the extension
    strcpy(filename, decInfo->output_fname);
    strcat(filename, decInfo->extn_secret_file);

    // Open the output file in write mode
    decInfo->fptr_output = fopen(filename, "w");

    // Check whether the output file was created successfully
    if (decInfo->fptr_output == NULL)
    {
        perror("fopen");
        printf("ERROR: Unable to create output file\n");
        fclose(decInfo->fptr_stego_image);
        return e_failure;
    }

    /* Step 7 : Decode Secret File Size */

    // Decode the size of the secret file
    if (decode_secret_file_size(&file_size, decInfo) == e_success)
    {
        printf("INFO: Secret File Size Decoded Successfully\n");
    }
    else
    {
        // Failed to decode the secret file size
        printf("ERROR: Secret File Size Decoding Failed\n");
        fclose(decInfo->fptr_output);
        fclose(decInfo->fptr_stego_image);
        return e_failure;
    }

    /* Step 8 : Decode Secret File Data */

    // Decode the complete secret file data
    if (decode_secret_file_data(decInfo) == e_success)
    {
        printf("INFO: Secret File Data Decoded Successfully\n");
    }
    else
    {
        // Failed to decode the secret file data
        printf("ERROR: Secret File Data Decoding Failed\n");
        fclose(decInfo->fptr_output);
        fclose(decInfo->fptr_stego_image);
        return e_failure;
    }

    // Close the output file
    fclose(decInfo->fptr_output);

    // Close the stego image file
    fclose(decInfo->fptr_stego_image);

    // Decoding completed successfully
    return e_success;
}