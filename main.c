/*Project Documentation

Project Title: Image Steganography using LSB Technique

Submitted By:
Name: kavana c
Start Date:20/7/2026
End Date:28/7/2026

Objective:

The objective of this project is to securely hide a secret file inside a BMP image using the Least Significant 
Bit (LSB) technique and retrieve it later without affecting the visible quality of the image.

Description:

Image steganography is a technique of hiding confidential information inside an image. This project uses a 24-bit BMP 
image as the cover image. The Least Significant Bit of every image byte is modified to store the secret information.

The project supports two operations:

Encoding (-e): Hides a secret file inside a BMP image.
Decoding (-d): Extracts the hidden file from the stego image.

Features:

Supports 24-bit BMP images.
Encodes any type of file (.txt, .c, .sh, etc.).
Stores the secret file extension.
Stores the secret file size.
Recovers the original file successfully.
Uses LSB technique without noticeable image distortion.

Algorithm:

Encoding:
Read one character from the secret file.
Read 8 bytes from the BMP image.
Replace the LSB of each byte with one bit of the character.
Write the modified bytes to the output image.
Repeat until the complete file is hidden.

Decoding:
Read 8 bytes from the stego image.
Extract the LSB from each byte.
Combine 8 bits to form one character.
Repeat until the entire file is recovered. */


#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

OperationType check_operation_type(char *symbol);

int main(int argc, char *argv[])
{
    // Check whether the minimum required command-line arguments are provided
    if (argc < 3)
    {
        printf("ERROR: Insufficient arguments\n");
        printf("Usage:\n");

        return e_failure;
    }

    // Determine whether the user selected encoding or decoding
    OperationType op = check_operation_type(argv[1]);

    /* Perform Encoding */
    if (op == e_encode)
    {
        // Check whether enough arguments are provided for encoding
        if (argc < 4)
        {
            printf("ERROR: Insufficient arguments for encoding\n");
            return e_failure;
        }

        // Display the selected operation
        printf("INFO: User selected Encoding\n");

        // Create a structure to store encoding information
        EncodeInfo encInfo;

        // Validate the command-line arguments for encoding
        if (read_and_validate_encode_args(argv, &encInfo) == e_success)
        {
            printf("INFO: Read and Validate Successful\n");

            // Start the encoding process
            if (do_encoding(&encInfo) == e_success)
            {
                printf("INFO: Encoding completed successfully\n");
            }
            else
            {
                // Encoding failed
                printf("ERROR: Encoding failed\n");
                return e_failure;
            }
        }
        else
        {
            // Invalid command-line arguments
            printf("ERROR: Invalid command line arguments\n");
            return e_failure;
        }
    }

    /* Perform Decoding */
    else if (op == e_decode)
    {
        // Display the selected operation
        printf("INFO: User selected Decoding\n");

        // Create a structure to store decoding information
        DecodeInfo decInfo;

        // Validate the command-line arguments for decoding
        if (read_and_validate_decode_args(argv, &decInfo) == e_success)
        {
            printf("INFO: Read and Validate Successful\n");

            // Start the decoding process
            if (do_decoding(&decInfo) == e_success)
            {
                printf("INFO: Decoding completed successfully\n");
            }
            else
            {
                // Decoding failed
                printf("ERROR: Decoding failed\n");
                return e_failure;
            }
        }
        else
        {
            // Invalid command-line arguments
            printf("ERROR: Invalid command line arguments\n");
            return e_failure;
        }
    }

    /* Handle invalid operation */
    else
    {
        // Unsupported command-line option
        printf("ERROR: Unsupported Operation\n");
        return e_failure;
    }

    // Program completed successfully
    return 0;
}

/* Check operation type from command line */
OperationType check_operation_type(char *symbol)
{
    // Check whether the user selected encoding
    if (strcmp(symbol, "-e") == 0)
    {
        return e_encode;
    }
    // Check whether the user selected decoding
    else if (strcmp(symbol, "-d") == 0)
    {
        return e_decode;
    }
    // Invalid operation selected
    else
    {
        return e_unsupported;
    }
}