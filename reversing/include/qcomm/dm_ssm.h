#ifndef DM_SSM_H
#define DM_SSM_H

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "dmode.h"

#ifndef NO_POD_PACKET_STRUCTURES
/**
* SSM Open File Request
*/
PACKED(typedef struct QcdmSsmOpenRequest{
	qcdm_subsys_header_t header;
	uint32_t flags;       
	uint32_t mode;       
	char  path[0];  
}) qcdm_ssm_open_req_t;

/**
* SSM Open File Response
*/
PACKED(typedef struct QcdmSsmOpenResponse{
	qcdm_subsys_header_t header;
	uint32_t fd;       
	uint32_t error;  
}) qcdm_ssm_open_rsp_t;


/**
* SSM Close File Request
*/
PACKED(typedef struct QcdmSsmCloseRequest{
	qcdm_subsys_header_t header;
	uint32_t fd; 
}) qcdm_ssm_close_req_t;

/**
* SSM Close File Response
*/
PACKED(typedef struct QcdmSsmCloseResponse{
	qcdm_subsys_header_t header;     
	uint32_t error;  
}) qcdm_ssm_close_rsp_t;


/**
* SSM Read File Request
*/
PACKED(typedef struct QcdmSsmReadRequest{
	qcdm_subsys_header_t header;
	uint32_t fd; 
	uint32_t bytes; 
	uint32_t offset; 
}) qcdm_ssm_read_req_t;

/**
* SSM Read File Response
*/
PACKED(typedef struct QcdmSsmReadResponse{
	qcdm_subsys_header_t header;
	uint32_t fd; 
	uint32_t offset; 
	uint32_t size; 
	uint32_t error;  
	uint8_t  data[0];
}) qcdm_ssm_read_rsp_t;
#endif //NO_POD_PACKET_STRUCTURES

#endif //DM_SSM_H