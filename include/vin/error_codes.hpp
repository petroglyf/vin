#pragma once
/**
 *           _________ _       
 *  |\     /|\__   __/( (    /|
 *  | )   ( |   ) (   |  \  ( |
 *  ( (   ) )   | |   | (\ \) |
 *   \ \_/ /    | |   | | \   |
 *    \   /  ___) (___| )  \  |
 *     \_/   \_______/|/    )_)
 *                             
 * Error codes to be used throughout VIN
 * 
 * @author: ndepalma@alum.mit.edu
 * @license: MIT License
 */ 

#define ERR_NO_ERROR 0              // No error, successful function call
#define ERR_FILE_NOT_FOUND 1        // The file requested was not found
#define ERR_NAME_ALREADY_EXISTS 2   // The name of the node already exists in the DAG
#define ERR_PARENT_ABSENT 3         // The parent that the child must attach to is missing
#define ERR_LIBRARY_EMPTY 4         // Vin could not find any modules in the lib locations
