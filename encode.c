#include <stdio.h>
#include "encode.h"
#include "types.h"
#include "common.h"
#include <string.h>

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
    fseek(fptr_image, 18, SEEK_SET); // Seek to 18th byte

    fread(&width, sizeof(int), 1, fptr_image); // Read the width (an int)
    printf("width = %u\n", width);

    fread(&height, sizeof(int), 1, fptr_image);  // Read the height (an int)
    printf("height = %u\n", height);

    return width * height * 3;  // Return image capacity
}

uint get_file_size(FILE *fptr)
{
    fseek(fptr, 0, SEEK_END);
    uint size = ftell(fptr);
    rewind(fptr);

    return size;
}


/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    char *ptr;   // Pointer used to store the result returned by strstr()

    // Check whether the source image file has ".bmp" extension
    ptr = strstr(argv[2], ".bmp"); // Check source image

    if(ptr != NULL && strcmp(ptr, ".bmp") == 0)
    {
        // Store the source image file name
        encInfo->src_image_fname = argv[2];
    }
    else
    {
        // Invalid source image format
        printf("ERROR: Source image should be .bmp\n");
        return e_failure;
    }

    // Check whether the secret file contains any extension
    ptr = strstr(argv[3], ".");

    if(ptr != NULL && *(ptr + 1) != '\0')
    {
        // Store the secret file name
        encInfo->secret_fname = argv[3];
    }
    else
    {
        // Secret file has no extension
        printf("ERROR: Secret file has no valid extension\n");
        return e_failure;
    }

    // Check if destination image name is provided
    if(argv[4] != NULL) // Destination image
    {
        // Verify that the destination image also has ".bmp" extension
        ptr = strstr(argv[4], ".bmp");

        if(ptr != NULL && strcmp(ptr, ".bmp") == 0)
        {
            // Store the destination image file name
            encInfo->dest_image_fname = argv[4];
        }
        else
        {
            // Invalid destination image format
            printf("ERROR: Output image should be .bmp\n");
            return e_failure;
        }
    }
    else
    {
        // If no destination image is given, use the default name
        encInfo->dest_image_fname = "default.bmp";
    }

    // All validations are successful
    return e_success;
}

Status open_files(EncodeInfo *encInfo)
{
    // Open the source image file in read mode
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");

    // Check whether the source image file is opened successfully
    if (encInfo->fptr_src_image == NULL)
    {
        // Print the system error message
        perror("fopen");

        // Print the custom error message
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

        return e_failure;
    }

    // Open the secret file in read mode
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");

    // Check whether the secret file is opened successfully
    if (encInfo->fptr_secret == NULL)
    {
        // Print the system error message
        perror("fopen");

        // Print the custom error message
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

        return e_failure;
    }

    // Open the destination image file in write mode
    encInfo->fptr_dest_image = fopen(encInfo->dest_image_fname, "w");

    // Check whether the destination image file is opened successfully
    if (encInfo->fptr_dest_image == NULL)
    {
        // Print the system error message
        perror("fopen");

        // Print the custom error message
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->dest_image_fname);

        return e_failure;
    }

    // All files are opened successfully
    return e_success;   // No failure, return success
}

Status check_capacity(EncodeInfo *encInfo)
{
    // Get the maximum number of bytes available in the source BMP image
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);

    // Get the size of the secret file
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);

    // Check whether the image has enough capacity to store
    // the magic string, extension size, file size, and secret file data
    if(encInfo->image_capacity >
        (16 + 32 + 32 + 32 + (encInfo->size_secret_file * 8)))
    {
        // Image has sufficient capacity
        return e_success;
    }
    else
    {
        // Image cannot accommodate the secret data
        printf("ERROR: Image does not have enough capacity\n");
        return e_failure;
    }
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    // Move the source image file pointer to the beginning of the file
    rewind(fptr_src_image);

    // Buffer to store the 54-byte BMP header
    char image_buffer[54];

    // Read the BMP header from the source image
    fread(image_buffer, 54, 1, fptr_src_image);

    // Write the BMP header into the destination image
    fwrite(image_buffer, 54, 1, fptr_dest_image);

    // Check whether both file pointers have moved 54 bytes
    if ((ftell(fptr_src_image) == 54) && (ftell(fptr_dest_image) == 54))
    {
        // Header copied successfully
        return e_success;
    }
    else
    {
        // Header copy failed
        return e_failure;
    }
}

Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    // Buffer to store 8 bytes of image data for encoding one character
    char imageBuffer[8];

    // Find the length of the magic string
    int len = strlen(magic_string);

    // Encode each character of the magic string
    for(int i = 0; i < len; i++)
    {
        // Read 8 bytes from the source image
        fread(imageBuffer, 8, 1, encInfo->fptr_src_image);

        // Encode one character into the LSBs of the 8 bytes
        encode_byte_to_lsb(magic_string[i], imageBuffer);

        // Write the encoded 8 bytes to the destination image
        fwrite(imageBuffer, 8, 1, encInfo->fptr_dest_image);
    }

    // Check whether both file pointers have advanced equally
    if (ftell(encInfo->fptr_src_image) == ftell(encInfo->fptr_dest_image))
    {
        // Magic string encoded successfully
        return e_success;
    }
    else
    {
        // Encoding failed
        return e_failure;
    }
}

Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
    // Buffer to store 32 bytes of image data for encoding the extension size
    char imageBuffer[32];

    // Read 32 bytes from the source image
    fread(imageBuffer, 32, 1, encInfo->fptr_src_image);

    // Encode the extension size into the LSBs of the 32 bytes
    encode_size_to_lsb(size, imageBuffer);

    // Write the modified bytes to the destination image
    fwrite(imageBuffer, 32, 1, encInfo->fptr_dest_image);

    // Check whether both file pointers are at the same position
    if (ftell(encInfo->fptr_src_image) == ftell(encInfo->fptr_dest_image))
    {
        // Extension size encoded successfully
        return e_success;
    }
    else
    {
        // Encoding failed
        return e_failure;
    }
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    // Buffer to store 8 bytes of image data for encoding one extension character
    char imageBuffer[8];

    // Find the length of the secret file extension
    int len = strlen(file_extn);

    // Encode each character of the file extension
    for(int i = 0; i < len; i++)
    {
        // Read 8 bytes from the source image
        fread(imageBuffer, 8, 1, encInfo->fptr_src_image);

        // Encode one extension character into the LSBs of the 8 bytes
        encode_byte_to_lsb(file_extn[i], imageBuffer);

        // Write the modified bytes to the destination image
        fwrite(imageBuffer, 8, 1, encInfo->fptr_dest_image);
    }

    // Check whether both file pointers are at the same position
    if(ftell(encInfo->fptr_src_image) == ftell(encInfo->fptr_dest_image))
    {
        // File extension encoded successfully
        return e_success;
    }
    else
    {
        // Encoding failed
        return e_failure;
    }
}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    // Buffer to store 32 bytes of image data for encoding the file size
    char imageBuffer[32];

    // Read 32 bytes from the source image
    fread(imageBuffer, 32, 1, encInfo->fptr_src_image);

    // Encode the secret file size into the LSBs of the 32 bytes
    encode_size_to_lsb(file_size, imageBuffer);

    // Write the modified bytes to the destination image
    fwrite(imageBuffer, 32, 1, encInfo->fptr_dest_image);

    // Check whether both file pointers have advanced equally
    if (ftell(encInfo->fptr_src_image) == ftell(encInfo->fptr_dest_image))
    {
        // Secret file size encoded successfully
        return e_success;
    }
    else
    {
        // Encoding failed
        return e_failure;
    }
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    // Move the secret file pointer to the beginning of the file
    rewind(encInfo->fptr_secret);

    // Create a buffer to store the complete secret file data
    char file_data[encInfo->size_secret_file];

    // Read the entire secret file into the buffer
    fread(file_data, encInfo->size_secret_file, 1, encInfo->fptr_secret);

    // Buffer to store 8 bytes of image data for encoding one character
    char image_buffer[8];

    // Encode each character of the secret file
    for(int i = 0; i < encInfo->size_secret_file; i++)
    {
        // Read 8 bytes from the source image
        fread(image_buffer, 8, 1, encInfo->fptr_src_image);

        // Encode one character into the LSBs of the image bytes
        encode_byte_to_lsb(file_data[i], image_buffer);

        // Write the modified bytes to the destination image
        fwrite(image_buffer, 8, 1, encInfo->fptr_dest_image);
    }

    // Check whether both file pointers are at the same position
    if(ftell(encInfo->fptr_src_image) == ftell(encInfo->fptr_dest_image))
    {
        // Secret file data encoded successfully
        return e_success;
    }

    // Encoding failed
    return e_failure;
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    // Variable to store each extracted bit
    int bit;

    // Encode all 8 bits of the character
    for(int i = 0; i < 8; i++)
    {
        // Extract one bit from the character (MSB to LSB)
        bit = (data >> (7 - i)) & 1;

        // Clear the Least Significant Bit (LSB) of the image byte
        image_buffer[i] &= ~1;

        // Store the extracted bit into the LSB of the image byte
        image_buffer[i] |= bit;
    }

    // Character encoded successfully
    return e_success;
}

Status encode_size_to_lsb(int size, char *imageBuffer)
{
    // Variable to store each extracted bit
    int bit;

    // Encode all 32 bits of the integer size
    for(int i = 0; i < 32; i++)
    {
        // Extract one bit from the size (MSB to LSB)
        bit = (size >> (31 - i)) & 1;

        // Clear the Least Significant Bit (LSB) of the image byte
        imageBuffer[i] &= ~1;

        // Store the extracted bit into the LSB of the image byte
        imageBuffer[i] |= bit;
    }

    // Size encoded successfully
    return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    // Buffer used to copy image data in chunks
    char buffer[1024];

    // Variable to store the number of bytes read
    size_t bytes_read;

    // Read the remaining image data until the end of the source file
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), fptr_src)) > 0)
    {
        // Write the read bytes into the destination image
        fwrite(buffer, 1, bytes_read, fptr_dest);
    }

    // Remaining image data copied successfully
    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    // Open the source image, secret file, and destination image
    if (open_files(encInfo) == e_success)
    {
        printf("INFO: Files opened successfully\n");
    }
    else
    {
        // File opening failed
        printf("ERROR: Unable to open files\n");
        return e_failure;
    }

    // Check whether the source image has enough capacity
    // to store the secret data
    if (check_capacity(encInfo) == e_success)
    {
        printf("INFO: Image has enough capacity\n");
    }
    else
    {
        // Image capacity is insufficient
        printf("ERROR: Image does not have enough capacity\n");
        return e_failure;
    }

    // Copy the 54-byte BMP header from source to destination
    if (copy_bmp_header(encInfo->fptr_src_image,
                        encInfo->fptr_dest_image) == e_success)
    {
        printf("INFO: BMP header copied\n");
    }
    else
    {
        // Failed to copy the BMP header
        printf("ERROR: Failed to copy BMP header\n");
        return e_failure;
    }

    // Encode the magic string into the image
    if (encode_magic_string(MAGIC_STRING, encInfo) == e_success)
    {
        printf("INFO: Magic string encoded\n");
    }
    else
    {
        // Magic string encoding failed
        printf("ERROR: Magic string encoding failed\n");
        return e_failure;
    }

    // Find and store the extension of the secret file
    char *ptr = strrchr(encInfo->secret_fname, '.');

    if (ptr != NULL)
    {
        strcpy(encInfo->extn_secret_file, ptr);
    }

    // Encode the size of the secret file extension
    if (encode_secret_file_extn_size(strlen(encInfo->extn_secret_file),
                                     encInfo) == e_success)
    {
        printf("INFO: Extension size encoded\n");
    }
    else
    {
        // Extension size encoding failed
        printf("ERROR: Extension size encoding failed\n");
        return e_failure;
    }

    // Encode the secret file extension
    if (encode_secret_file_extn(encInfo->extn_secret_file,
                                encInfo) == e_success)
    {
        printf("INFO: Extension encoded\n");
    }
    else
    {
        // Extension encoding failed
        printf("ERROR: Extension encoding failed\n");
        return e_failure;
    }

    // Encode the size of the secret file
    if (encode_secret_file_size(encInfo->size_secret_file,
                                encInfo) == e_success)
    {
        printf("INFO: Secret file size encoded\n");
    }
    else
    {
        // Secret file size encoding failed
        printf("ERROR: Secret file size encoding failed\n");
        return e_failure;
    }

    // Encode the actual secret file data
    if (encode_secret_file_data(encInfo) == e_success)
    {
        printf("INFO: Secret file data encoded\n");
    }
    else
    {
        // Secret file data encoding failed
        printf("ERROR: Secret file data encoding failed\n");
        return e_failure;
    }

    // Copy the remaining image data after encoding
    if (copy_remaining_img_data(encInfo->fptr_src_image,
                                encInfo->fptr_dest_image) == e_success)
    {
        printf("INFO: Remaining image copied\n");
    }
    else
    {
        // Failed to copy the remaining image data
        printf("ERROR: Failed to copy remaining image data\n");
        return e_failure;
    }

    // Encoding completed successfully
    return e_success;
}
