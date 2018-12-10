#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "../packed.h"

#ifndef dload_h
#define dload_h

typedef enum DLOAD_COMMAND {
	DLOAD_WRITE                         = 0x01, // Write data to memory 
	DLOAD_ACK                           = 0x02, // Acknowledge receiving a packet 
	DLOAD_NAK                           = 0x03, // Negatively acknowledge a packet 
	DLOAD_ERASE                         = 0x04, // Erase a section of memory 
	DLOAD_GO                            = 0x05, // Go to a memory location and run 
	DLOAD_NOP                           = 0x06, // No operation, useful for debugging 
	DLOAD_PARAM_REQ                     = 0x07, // Request implementation information 
	DLOAD_PARAM_RESP                    = 0x08, // Response to a Parameter Request 
	DLOAD_MEMORY_DUMP                   = 0x09, // Debug-only; dump memory (optional
	DLOAD_RESET                         = 0x0A, // Perform a hardware reset 
	DLOAD_UNLOCK                        = 0x0B, // Unlock access to secured operations 
	DLOAD_SQ_VERSION_REQ                = 0x0C, // Return implementation version information 
	DLOAD_SQ_VERSION_RESP               = 0x0D, // Implementation software version information 
	DLOAD_POWERDOWN                     = 0x0E, // Turn phone power off 
	DLOAD_WRITE32                       = 0x0F, // Write data to memory using 32-bit address 
	DLOAD_MEMORY_DEBUG_QUERY            = 0x10, // Debug-only; request information on available memory-debug regions
	DLOAD_MEMORY_DEBUG_INFO             = 0x11, // Debug-only; response to Memory Debug Query 
	DLOAD_MEMORY_READ_REQ               = 0x12, // Request read of specified section of memory 
	DLOAD_MEMORY_READ_RESP              = 0x13, // Response to Memory Read Request; returns requested data 
	DLOAD_UNFRAMED_READ_REQ             = 0x14, // SBL Only - Request unframed read of specified section of memory 
	DLOAD_PBL_SERIAL_NUMBER_READ_REQ    = 0x14, // PBL Only - Request read of serial number from e-fuses 
	DLOAD_PBL_SERIAL_NUMBER_READ_RESP   = 0x14, // PBL Only - Response to Serial Number Read Request; returns serial number
	DLOAD_MEMORY_UNFRAMED_READ_RESP     = 0x15, // SBL Only - Response to Memory Unframed Read request; returns requested data 
	DLOAD_SERIAL_NUMBER_READ_REQ        = 0x16, // Request read of serial number from e-fuses 
	DLOAD_SERIAL_NUMBER_READ_RESP       = 0x16, // Response to Serial Number Read request; returns serial number 
	DLOAD_HARDWARE_ID_READ_REQ          = 0x17, // Request read of Hardware ID from efuses 
	DLOAD_HARDWARE_ID_READ_RESP         = 0x17, // Response to Hardware ID Read request; returns 64-bit Hardware ID 
	DLOAD_PUBLIC_KEY_HASH_READ_REQ      = 0x18, // Request to retrieve hash of the root of trust certificate 
	DLOAD_PUBLIC_KEY_HASH_READ_RESP     = 0x18, // Response to Public Key Hash Read; returns the hash value 
	DLOAD_QPST_COOKIE_READ_REQ          = 0x19, // Request to read QPST Cookie 
	DLOAD_QPST_COOKIE_READ_RESP         = 0x1A, // Response to QPST Cookie Read Request; returns the QPST cookie value
	DLOAD_SWITCH_TO_DLOAD               = 0x3A, // SBL Only - Command packet that  is echoed back by the Phone 
} dload_command_t;

typedef enum DLOAD_NCK_REASON {
	DLOAD_NCK_INVALID_FRAME_FCS             = 0x0001,
	DLOAD_NCK_INVALID_DESTINATION_ADDRESS   = 0x0002,
	DLOAD_NCK_INVALID_LENGTH                = 0x0003,
	DLOAD_NCK_UNEXPECTED_END_OF_PACKET      = 0x0004,
	DLOAD_NCK_DATA_LENGTH_TOO_LARGE         = 0x0005,
	DLOAD_NCK_INVALID_COMMAND               = 0x0006,
	DLOAD_NCK_OPERATION_FAILT               = 0x0007,
	DLOAD_NCK_WRONG_FLASH_INTELLIGENT_ID    = 0x0008,
	DLOAD_NCK_BAD_PROGRAMMING_VOLTAGE       = 0x0009,
	DLOAD_NCK_WRITE_VERIFY_FAILED           = 0x000A,
	DLOAD_NCK_UNLOCK_REQUIRED               = 0x000B,
	DLOAD_NCK_INCORRECT_SECURITY_CODE       = 0x000C,
	DLOAD_NCK_CANNOT_POWER_DOWN_PHONE       = 0x000D,
	DLOAD_NCK_OPERATION_NOT_PERMITTED       = 0x000E,
	DLOAD_NCK_INVALID_READ_ADDRESS          = 0x000F,
} dload_nck_reason_t;

typedef enum DLOAF_FLASH_DEVICE_SIZE {
	DLOAD_FLASH_DEVICE_SIZE_4MB         = 0x01,
	DLOAD_FLASH_DEVICE_SIZE_8MB         = 0x02,
	DLOAD_FLASH_DEVICE_SIZE_16MB_AS_8MB = 0x03,
	DLOAD_FLASH_DEVICE_SIZE_16MB        = 0x04,
	DLOAD_FLASH_DEVICE_SIZE_32MB        = 0x05,
	DLOAD_FLASH_DEVICE_SIZE_64MB        = 0x06,
} dload_flash_device_size_t;

typedef struct DloadWriteRequest { // 0x01
	uint8_t command;
	uint32_t address;
	uint16_t size;
	uint8_t* data;
} __attribute__((packed)) dload_write_req_t;

typedef struct DloadAckResponse { // 0x02
	uint8_t command;
} __attribute__((packed)) dload_ack_rsp_t;


typedef struct DloadNckResponse { // 0x03
	uint8_t command;
	uint16_t reason;
} __attribute__((packed)) dload_nck_rsp_t;

typedef struct DloadEraseReequest { // 0x04
	uint8_t command;
	uint32_t address;
} __attribute__((packed)) dload_erase_e_req_t;

typedef struct DloadGoRequest { // 0x05
	uint8_t command;
	uint16_t address;
	uint16_t offset;
} __attribute__((packed)) dload_go_req_t;

typedef struct DloadNopRequest { // 0x06
	uint8_t command;
} __attribute__((packed)) dload_nop_req_t;

typedef struct DloadParametersRequest { // 0x07
	uint8_t command;
} __attribute__((packed)) dload_params_req_t;

typedef struct DloadParametersResponse { // 0x08
	uint8_t command;
	uint8_t version;
	uint8_t minVersion;
	uint16_t maxWriteSize;
	uint8_t model;
	uint8_t size;
	uint8_t type;
} __attribute__((packed)) dload_params_rsp_t;

typedef struct DloadDumpRequest { // 0x09
	uint8_t command;
	uint32_t address;
	uint16_t length;
} __attribute__((packed)) dload_dump_req_t;

typedef struct DloadResetRequest { // 0x0A
	uint8_t command;
} __attribute__((packed)) dload_reset_req_t;

typedef struct DloadUnlockRequest { // 0x0B
	uint8_t command;
	uint8_t securityCode[8];
} __attribute__((packed)) dload_unlock_req_t;

typedef struct DloadSwVersionRequest { // 0x0C
	uint8_t command;
} __attribute__((packed)) dload_sw_vers_req_t;

typedef struct DloadSwVersionResponse { // 0x0D
	uint8_t command;
	uint8_t length;
	uint8_t version[20];
} __attribute__((packed)) dload_sw_vers_rsp_t;

typedef struct DloadPowerDownRequest { // 0x0E
	uint8_t command;
} __attribute__((packed)) dload_powerdown_req_t;

#endif