/* System header files */
#include <stdint.h>
//#include <stdio.h>
//#include <string.h>
#include <stdlib.h>
#include <hardwareSerial.h>

#define __DEBUG__   
#define CLOCK_T_3s    3000
#define CLOCK_T_500ms 500
#define CLOCK_T_5s    5000

typedef enum {
  /* General codes */
  GN_ERR_NONE = 0,    /* No error. */
  GN_ERR_FAILED,      /* Command failed. */
  GN_ERR_UNKNOWNMODEL,    /* Model specified isn't known/supported. */
  GN_ERR_INVALIDSECURITYCODE, /* Invalid Security code. */
  GN_ERR_INTERNALERROR,   /* Problem occured internal to model specific code. */
  GN_ERR_NOTIMPLEMENTED,    /* Command called isn't implemented in model. */
  GN_ERR_NOTSUPPORTED,    /* Function not supported by the phone */
  GN_ERR_USERCANCELED,    /* User aborted the action. */
  GN_ERR_UNKNOWN,     /* Unknown error - well better than nothing!! */
  GN_ERR_MEMORYFULL,    /* The specified memory is full. */

  /* Statemachine */
  GN_ERR_NOLINK,      /* Couldn't establish link with phone. */
  GN_ERR_TIMEOUT,     /* Command timed out. */
  GN_ERR_TRYAGAIN,    /* Try again. */
  GN_ERR_WAITING,     /* Waiting for the next part of the message. */
  GN_ERR_NOTREADY,    /* Device not ready. */
  GN_ERR_BUSY,      /* Command is still being executed. */
  
  /* Locations */
  GN_ERR_INVALIDLOCATION,   /* The given memory location has not valid location. */
  GN_ERR_INVALIDMEMORYTYPE, /* Invalid type of memory. */
  GN_ERR_EMPTYLOCATION,   /* The given location is empty. */

  /* Format */
  GN_ERR_ENTRYTOOLONG,    /* The given entry is too long */
  GN_ERR_WRONGDATAFORMAT,   /* Data format is not valid */
  GN_ERR_INVALIDSIZE,   /* Wrong size of the object */

  /* The following are here in anticipation of data call requirements. */
  GN_ERR_LINEBUSY,    /* Outgoing call requested reported line busy */
  GN_ERR_NOCARRIER,   /* No Carrier error during data call setup ? */

  /* The following value signals the current frame is unhandled */
  GN_ERR_UNHANDLEDFRAME,    /* The current frame isn't handled by the incoming function */
  GN_ERR_UNSOLICITED,   /* Unsolicited message received. */

  /* Other */
  GN_ERR_NONEWCBRECEIVED,   /* Attempt to read CB when no new CB received */
  GN_ERR_SIMPROBLEM,    /* SIM card missing or damaged */
  GN_ERR_CODEREQUIRED,    /* PIN or PUK code required */
  GN_ERR_NOTAVAILABLE,    /* The requested information is not available */

  /* Config */
  GN_ERR_NOCONFIG,    /* Config file cannot be found */
  GN_ERR_NOPHONE,     /* Either global or given phone section cannot be found */
  GN_ERR_NOLOG,     /* Incorrect logging section configuration */
  GN_ERR_NOMODEL,     /* No phone model specified */
  GN_ERR_NOPORT,      /* No port specified */
  GN_ERR_NOCONNECTION,    /* No connection type specified */
  GN_ERR_LOCKED,      /* Device is locked and cannot unlock */

  GN_ERR_ASYNC      /* The actual response will be sent asynchronously */
} gn_error;

typedef enum {
  GN_AT_NONE,   /* NO or unknown result code */
  GN_AT_PROMPT,   /* SMS command waiting for input */
  GN_AT_OK,   /* Command succeded */
  GN_AT_ERROR,    /* Command failed */
  GN_AT_CMS,    /* SMS Command failed */
  GN_AT_CME,    /* Extended error code found */
  GN_AT_SMS_READY,
  GN_AT_GPS
} at_result;

typedef enum {
  GN_OP_Init,
  GN_OP_Terminate,
  GN_OP_GetModel,
  GN_OP_GetRevision,
  GN_OP_GetImei,
  GN_OP_GetManufacturer,
  GN_OP_Identify,
  GN_OP_GetBitmap,
  GN_OP_SetBitmap,
  GN_OP_GetBatteryLevel,
  GN_OP_GetRFLevel,
  GN_OP_DisplayOutput,
  GN_OP_GetMemoryStatus,
  GN_OP_ReadPhonebook,
  GN_OP_WritePhonebook,
  GN_OP_DeletePhonebook,
  GN_OP_GetPowersource,
  GN_OP_GetAlarm,
  GN_OP_GetSMSStatus,
  GN_OP_GetIncomingCallNr,
  GN_OP_GetNetworkInfo,
  GN_OP_GetSecurityCode,
  GN_OP_CreateSMSFolder,
  GN_OP_DeleteSMSFolder,
  GN_OP_GetSMS,
  GN_OP_GetSMSnoValidate,
  GN_OP_GetSMSFolders,
  GN_OP_GetSMSFolderStatus,
  GN_OP_GetIncomingSMS,
  GN_OP_GetUnreadMessages,
  GN_OP_GetNextSMS,
  GN_OP_DeleteSMSnoValidate,
  GN_OP_DeleteSMS,
  GN_OP_SendSMS,
  GN_OP_GetSpeedDial,
  GN_OP_GetSMSCenter,
  GN_OP_SetSMSCenter,
  GN_OP_GetDateTime,
  GN_OP_GetToDo,
  GN_OP_GetCalendarNote,
  GN_OP_CallDivert,
  GN_OP_OnSMS, /* set data->on_sms and data->callback_data */
  GN_OP_PollSMS,
  GN_OP_SetAlarm,
  GN_OP_SetDateTime,
  GN_OP_GetProfile,
  GN_OP_SetProfile,
  GN_OP_WriteToDo,
  GN_OP_DeleteAllToDos,
  GN_OP_WriteCalendarNote,
  GN_OP_DeleteCalendarNote,
  GN_OP_SetSpeedDial,
  GN_OP_GetDisplayStatus,
  GN_OP_PollDisplay,
  GN_OP_SaveSMS,
  GN_OP_SetCellBroadcast, /* set data->on_cell_broadcast and data->callback_data */
  GN_OP_NetMonitor,
  GN_OP_MakeCall,
  GN_OP_AnswerCall,
  GN_OP_CancelCall,
  GN_OP_SetCallNotification, /* set data->call_notification and data->callback_data */
  GN_OP_SendRLPFrame,
  GN_OP_SetRLPRXCallback,
  GN_OP_EnterSecurityCode,
  GN_OP_GetSecurityCodeStatus,
  GN_OP_ChangeSecurityCode,
  GN_OP_SendDTMF,
  GN_OP_Reset,
  GN_OP_GetRingtone,
  GN_OP_SetRingtone,
  GN_OP_GetRawRingtone,
  GN_OP_SetRawRingtone,
  GN_OP_PressPhoneKey,
  GN_OP_ReleasePhoneKey,
  GN_OP_EnterChar,
  GN_OP_Subscribe,
  GN_OP_GetWAPBookmark,
  GN_OP_WriteWAPBookmark,
  GN_OP_DeleteWAPBookmark,
  GN_OP_GetWAPSetting,
  GN_OP_ActivateWAPSetting,
  GN_OP_WriteWAPSetting,
  GN_OP_GetLocksInfo,
  GN_OP_GetActiveProfile,
  GN_OP_SetActiveProfile,
  GN_OP_PlayTone,
  GN_OP_GetRingtoneList,
  GN_OP_DeleteRingtone,
  GN_OP_GetActiveCalls,
  GN_OP_GetFileList,
  GN_OP_GetFileId,
  GN_OP_GetFile,
  GN_OP_PutFile,
  GN_OP_DeleteFile,
  GN_OP_GetFileDetailsById,
  GN_OP_GetFileById,
  GN_OP_DeleteFileById,
  GN_OP_GetMMS,
  GN_OP_DeleteMMS,
  GN_OP_Max,  /* don't append anything after this entry */
} gn_operation;

typedef enum {
  GN_OP_AT_GetCharset = GN_OP_Max,
  GN_OP_AT_SetCharset,
  GN_OP_AT_SetPDUMode,
  GN_OP_AT_Prompt,
  GN_OP_AT_GetMemoryRange,
  GN_OP_AT_Ring,
  GN_OP_AT_IncomingSMS,
  GN_OP_AT_GetSMSMemorySize,
  GN_OP_AT_PrepareDateTime,
  GN_OP_AT_Max  /* don't append anything after this entry */
} at_operation;

#define RBUF_SEG  128

class sim800L {
  private:
    char *rbuf;
    int rbuf_size;
    char rbuf_pos;
    int binlen;
  
    char smsc[16];              // SMS Message Centre Number 
    char phone[16];             // Telefono a donde se envia el mensaje
    char  message[30];
    
    Stream *p_serial;
    Stream *p_monitor;
  public:    
    uint8_t status;   
  public:
    sim800L();
    void Init();
    void Init(Stream *p_ser);
    void Init(Stream *p_ser, Stream *p_mon);

    gn_error begin();
    char *findcrlfbw(char *str, int len);
    void atbus_rx_statemachine(uint8_t rx_char);
    gn_error at_sm_block(at_result waitfor);
    void sm_incoming_function(char *msg);

    uint8_t SendSMS(char *phone, char *msg); 
    uint8_t Ready2ReceiveSMS(); 
    int get_status();
    char *get_phone();
    char *get_msg();
    
};


//atbus_instance bi;
