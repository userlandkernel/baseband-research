#ifndef SAHARA_H
#define SAHARA_H

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#ifndef SAHARA_MAX_PACKET_SIZE
#define SAHARA_MAX_PACKET_SIZE 0x400
#endif

#ifndef SAHARA_LOG_LENGTH
#define SAHARA_LOG_LENGTH 0x64
#endif

#ifndef SAHARA_MAX_MEMORY_DATA_SIZE
#define SAHARA_MAX_MEMORY_DATA_SIZE 0x1000
#endif

#ifndef SAHARA_RAM_ZI_SIZE
#define SAHARA_RAM_ZI_SIZE 0x20000
#endif

#ifndef SAHARA_MAX_MEMORY_REQUEST_SIZE
#define SAHARA_MAX_MEMORY_REQUEST_SIZE 0x50000
#endif

/**
 * These are all known commands, both rx and tx.
 */
typedef enum SaharaCommand {
	kSaharaCommandHello             = 0x01, // Initialize connection and protocol
	kSaharaCommandHelloResponse     = 0x02, // Acknowledge connection/protocol, mode of operation
	kSaharaCommandReadData          = 0x03, // Read specified number of bytes from host
	kSaharaCommandEndImageTransfer  = 0x04, // image transfer end / target transfer failure
	kSaharaCommandDone              = 0x05, // Acknowledgement: image transfer is complete
	kSaharaCommandDoneResponse      = 0x06, // Target is exiting protocol
	kSaharaCommandReset             = 0x07, // Instruct target to perform a reset
	kSaharaCommandResetResponse     = 0x08, // Indicate to host that target is about to reset
	kSaharaCommandMemoryDebug       = 0x09, // Indicate to host: target debug mode & ready to transfer memory content
	kSaharaCommandMemoryRead        = 0x0A, // Read number of bytes, starting from a specified address
	kSaharaCommandReady             = 0x0B, // Indicate to host: target ready to receive client commands,
	kSaharaCommandSwitchMode        = 0x0C, // Switch to a mode defined in enum SAHARA_MODE
	kSaharaCommandExecute           = 0x0D, // Indicate to host: to execute a given client command
	kSaharaCommandExecuteResponse	= 0x0E, // Indicate to host: target command execution status
	kSaharaCommandExecuteData		= 0x0F, // Indicate to target that host is ready to receive (more) data
	kSaharaCommandMemoryDebug64     = 0x10,
	kSaharaCommandMemoryRead64      = 0x11,
} sahara_cmd_type_t;

/**
 * SAHARA_MODE
 *
 * These are all known modes available
 * The initial hello lets you know which
 * mode the device is currently in.
 */
typedef enum SaharaMode {
	kSaharaModeImageTxPending  = 0x00,
	kSaharaModeImageTxComplete = 0x01,  
	kSaharaModeMemoryDebug     = 0x02,
	kSaharaModeCommand         = 0x03
} shara_mode_t;

/**
 * SaharaClientCmd
 *
 * When in or switched to command mode, these are
 * all commands that are currently known
 */
typedef enum SaharaClientCmd {
	  kSaharaClientCmdNop                    = 0x00,
	  kSaharaClientCmdSerialNumRead          = 0x01,
	  kSaharaClientCmdMsmHWIDRead            = 0x02,
	  kSaharaClientOemPkHashRead			 = 0x03,
	  kSaharaClientCmdSwitchDMSS			 = 0x04, // haven't found a device/mode this works on
	  kSaharaClientCmdSwitchToStreamingDload = 0x05, // haven't found a device/mode this works on
	  kSaharaClientCmdReadDebugData          = 0x06,
	  kSaharaClientCmdGetSblVersion			 = 0x07,
} sahara_client_cmd_t;

typedef enum SaharaStatusCode {
	kSaharaStatusSuccess                      = 0x00,
	kSaharaStatusInvalidCmd                   = 0x01,
	kSaharaStatusProtocolMismatch             = 0x02,
	kSaharaStatusInvalidTargetProtocol        = 0x03,
	kSaharaStatusInvalidHostProtocol          = 0x04,
	kSaharaStatusInvalidPacketSize            = 0x05,
	kSaharaStatusUnexpectedImageId            = 0x06,
	kSaharaStatusInvalidHeaderSize            = 0x07,
	kSaharaStatusInvalidDataSize              = 0x08,
	kSaharaStatusInvalidImageType             = 0x09,
	kSaharaStatusInvalidTxLength              = 0x0A,
	kSaharaStatusInvalidRxLength              = 0x0B,
	kSaharaStatusTxRxError					  = 0x0C,
	kSaharaStatusReadDataError                = 0x0D,
	kSaharaStatusUnsupportedNumPhdrs          = 0x0E,
	kSaharaStatusInvalidPhdrSize              = 0x0F,
	kSaharaStatusMultipleSharedSeg            = 0x10,
	kSaharaStatusUninitPhdrLoc                = 0x11,
	kSaharaStatusInvalidDestAddress           = 0x12,
	kSaharaStatusInvalidImageHeaderSize       = 0x13,
	kSaharaStatusInvalidElfHeader             = 0x14,
	kSaharaStatusUnknownError				  = 0x15,
	kSaharaStatusTimeoutRx                    = 0x16,
	kSaharaStatusTimeoutTx                    = 0x17,
	kSaharaStatusInvalidMode				  = 0x18,
	kSaharaStatusInvalidMemoryRead            = 0x19,
	kSaharaStatusInvalidDataSizeRequest       = 0x1A,
	kSaharaStatusMemoryDebugNotSupported      = 0x1B,
	kSaharaStatusInvalidModeSwitch            = 0x1C,
	kSaharaStatusExecFailure				  = 0x1D,
	kSaharaStatusExecCmdInvalidParam          = 0x1E,
	kSaharaStatusExecCmdUnsupported           = 0x1F,
	kSaharaStatusExecDataInvalid			  = 0x20,
	kSaharaStatusHashTableAuthFailure         = 0x21,
	kSaharaStatusHashVerificationFailure      = 0x22,
	kSaharaStatusHashTableNotFound            = 0x23,
	kSaharaStatusTargetInitFailure            = 0x24,
	kSaharaStatusImageAuthFailure             = 0x25,
	kSaharaStatusInvalidImgHashTableSize	  = 0x26,
	kSaharaStatusMax
} sahara_status_code_t;

#ifndef NO_POD_PACKET_STRUCTURES
/**
* SaharaHeader
*
* The header all transmissions use
* with the exception of a few
*/
typedef struct SaharaHeader {
	uint32_t command;
	uint32_t size;
} sahara_header_t;

/**
* SaharaAbstractPacket
*/
typedef struct SaharaAbstractPacket {
	sahara_header_t header;
	uint32_t parameters[2];
} sahara_abstract_packet_t;

/**
* SaharaHelloRequestS
*
* The initial packet received from the device
* when first opening the COM port. If no data
* is available for read after the COM port is opened
* then the device may require a restart or it is not
* using sahara protocol
*/
typedef struct{ // 0x01
	sahara_header_t header;
	uint32_t version;
	uint32_t minVersion;
	uint32_t maxCommandPacketSize;
	uint32_t mode;
	uint32_t reserved[6];
} sahara_hello_req_t;

/**
* SaharaHelloResponse
*
* The initial packet sent to the device
* in response to the initial hello packet
*/
typedef struct { // 0x02
	sahara_header_t header;
	uint32_t version;
	uint32_t minVersion;
	uint32_t status; // ok or error
	uint32_t mode;
	uint32_t reserved[6];
} sahara_hello_rsp_t;

/**
* SaharaReadDataRequest
*
* When the device sends this packet
* it is requesting an image transfer and an initial
* chunk of the file for validation
*/
typedef struct { // 0x03
	sahara_header_t header;
	uint32_t imageId;
	uint32_t offset;
	uint32_t size;
} sahara_read_data_req_t;

/**
* SaharaEndImageTransferResponse
*
* When an error is encountered or an image transfer has
* ended.
*/
typedef struct { // 0x04
	sahara_header_t header;
	uint32_t file;
	uint32_t status;
} sahara_end_img_transfer_rsp_t;

/*
* SaharaDoneRequest
*
* Sent to the device in response to a successful SaharaEndImageTransferResponse
*/
typedef struct { // 0x05
	sahara_header_t header;
} sahara_done_req_t;

/*
* SaharaDoneResponse
*
* Received from the device after sending SaharaDoneRequest
*/
typedef struct { // 0x06
	sahara_header_t header;
	uint32_t imageTxStatus; // 0 pending, 1 complete
} sahara_done_rsp_t;

/*
* SaharaResetRequest
*/
typedef struct { // 0x07
	sahara_header_t header;
} sahara_reset_req_t;

/*
* SaharaResetResponse
*/
typedef struct { // 0x08
	sahara_header_t header;
} sahara_reset_rsp_t;


typedef struct { // 0x09
	sahara_header_t header;
	uint32_t memoryTableAddress;
	uint32_t memoryTableLength;
} sahara_memory_debug_req_t;

typedef struct { // 0x0a
  sahara_header_t header;
  uint32_t address;
  uint32_t size;
} sahara_memory_read_req_t;

/**
* SaharaCommandReadyResponse
*
* Received from the device when in or switching to client
* command mode. ready to take a command (SaharaClientCommandRequest)
*/
typedef struct { // 0x0b
	sahara_header_t header;
	uint32_t imageTxStatus; // 0 pending, 1 complete
} sahara_cmd_ready_rsp_t;

/**
* SaharaSwitchModeRequest
*
* Sent to the device to switch modes when in client command mode.
*
* Device should respond with SaharaHelloRequest
*/
typedef struct { // 0x0c
	sahara_header_t header;
	uint32_t mode;
} sahara_switch_mode_req_t;

/**
* SaharaClientCommandRequest
*
* Execute a client command
*
* If the command is invalid you will receive
* SaharaEndImageTransferResponse
*/
typedef struct { // 0x0d
	sahara_header_t header;
	uint32_t command;
} sahara_client_cmd_req_t;


/**
* SaharaClientCommandResponse
*
* Received from the device in response to SaharaClientCommandRequest
* with the size of the data expected back from the command execution
*
*/
typedef struct { // 0x0e
	sahara_header_t header;
	uint32_t command;
	uint32_t size;
} sahara_client_cmd_rsp_t;

/**
* SaharaClientCommandExecuteDataRequest
*
* Sent in response to SaharaClientCommandResponse
* indicating we are ready to receive the data of n size
* defined in SaharaClientCommandResponse
*
* After sent, read the size of data from the device
*/
typedef struct { // 0x0f
	sahara_header_t header;
	uint32_t command;
} sahara_client_cmd_exec_data_req_t;

/**
 *
 * SaharaMemoryDebug64Request
 */
typedef struct { // 0x10
	sahara_header_t header;
	uint32_t memoryTableAddress;
	uint32_t memoryTableLength;
} sahara_memory_debug64_req_t;

/**
 * SaharaMemoryRead64Request
 */
typedef struct { // 0x11
	sahara_header_t header;
	uint64_t address;
	uint64_t size;
} sahara_memory_read64_req_t;

#endif //no_pack_stuctures


/**
* SaharaMsmHwIdResponse
*
* represents a response from client command:
*	kSaharaClientCmdMsmHWIDRead
*/
typedef struct {
	uint16_t unknown1;
	uint16_t unknown2;
	uint16_t msmId;
} sahara_msm_hardware_id_rsp_t;

/**
* SaharaSerialNumberResponse
*
* represents a response from client command:
*	kSaharaClientCmdSerialNumRead
*/
typedef struct {
	uint32_t serial;
} sahara_serialnumber_rsp_t;

/**
* SaharaSblVersionResponse
*
* represents a response from client command:
*	kSaharaClientCmdGetSblVersion
*/
typedef struct {
	uint32_t version;
} sahara_sbl_version_rsp_t;

/**
* SaharaSblVersionResponse
*
* represents a response from client command
* kSaharaClientOemPkHashRead
*/
typedef struct {
	uint8_t hash[32];
} sahara_oem_pkhash_rsp_t;


/**
* SaharaDebugLogEntry
*
* represents a single log entry from client command
* kSaharaClientCmdReadDebugData response
*/
typedef struct {
	uint8_t message[SAHARA_LOG_LENGTH];
} sahara_debug_log_entry_t;

/**
* SaharaMemoryTableEntry
*
* This structure represents the memory table entry
* when reading the memory table from the specified address
* of a kSaharaCommandMemoryDebug response
* 
* The total number of entries would be response.size / sizeof(SaharaMemoryTableEntry)
*/
PACKED(typedef struct {
	uint32_t   unknown1;
	uint32_t   address;
	uint32_t   size;
	uint8_t    name[20];
	uint8_t    filename[20];
}) sahara_memory_table_entry_t;

#endif //sahara_h