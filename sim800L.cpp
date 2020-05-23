#include "pgmspace.h"
#include "sim800L.h"

sim800L::sim800L(){
   
}
void sim800L::Init(){
   rbuf_pos = 0;
   rbuf = (char *)malloc(RBUF_SEG);
}
void sim800L::Init(Stream *p_ser){

   p_serial = p_ser;
   p_monitor = NULL;  
   rbuf_pos = 0;
   rbuf = (char *)malloc(RBUF_SEG);
}

void sim800L::Init(Stream *p_ser, Stream *p_mon){

   p_serial = p_ser;
   p_monitor = p_mon;
   rbuf_pos = 0;
   rbuf = (char *)malloc(RBUF_SEG);
}

gn_error sim800L::begin()
{
   uint8_t ch;
   uint16_t count, contador;
   gn_error error;

   /* Send init string to phone, this is a bunch of 0x55 characters.
	   Timing is empirical. I believe that we need/can do this for any
	   phone to get the UART synced */
    

   if(p_monitor != NULL ) p_monitor->println("-->SIM800 begin start ");  
     

   count = 0;	   
   do{
      //p_monitor->printf("-->count:%d\n",count);
      p_serial->print("AT\r");
      
      error = at_sm_block(GN_AT_OK);
      if (error == GN_ERR_NONE) {
         p_monitor->print(PSTR("->SIM800 Readdy to receive AT commands.\r\n"));
         
         contador = 0;
         do{
            error = at_sm_block(GN_AT_SMS_READY);
            if(error == GN_ERR_NONE) return GN_ERR_NONE;
            contador++;
            if(p_monitor != NULL) p_monitor->print(">");
         }while(contador<16);
         return GN_ERR_TIMEOUT;
      }
      count++;     

      if(p_monitor != NULL) p_monitor->print(".");  
      
   }while(count<8);
   if(p_monitor != NULL ) p_monitor->println("-->SIM800 begin end ");  
   return GN_ERR_FAILED;
}

int numchar(unsigned char *str, unsigned char ch)
{
  int count = 0;

  while (*str && *str != '\r') {
    if (*str++ == ch)
      count++;
  }

  return count;
}

char *sim800L::findcrlfbw(char *str, int len)
{
	while (len-- && (*str != '\n') && (*str-1 != '\r'))
		str--;
	//return len > 0 ? str+1 : NULL;
  if(len > 0)
  {
    return (char *)(str+1);
  }
  else
     return NULL;
  
}


/* 
 * rx state machine for receive handling. called once for each character
 * received from the phone. 
 */
void sim800L::atbus_rx_statemachine(uint8_t rx_char)
{
   int error;
   int unsolicited, count;
   char *start;

   //p_monitor->printf("[E]rbuf_pos:%d,%d,0x%X,%p ->",rbuf_pos,rx_char, rx_char,rbuf);
   rbuf[rbuf_pos++] = rx_char;
   rbuf[rbuf_pos] = '\0';

   /*p_monitor->printf("[S]rbuf_pos:%d,%d,0x%X :String:",rbuf_pos,rx_char, rx_char);
   char *s=rbuf; 
   while(*s != NULL)
   {
      if(*s == 0x0D) p_monitor->print("<CR>");
      if(*s == 0x0A) p_monitor->print("<LF>");
      p_monitor->printf("%c",*s);
      s++;
   }
   p_monitor->printf("\r\n");*/
   
   status = GN_AT_NONE;
   /* first check if <cr><lf> is found at end of reply_buf.
    * none: the needed length is greater 4 because we don't
    * need to enter if no result/error will be found. */
   if (rbuf_pos == 2 && !strcmp_P(rbuf, PSTR("\r\n"))) {
      rbuf_pos = 0;
      rbuf[0] = '\0';

      //p_monitor->printf("rbuf_pos:%d, String:PSTR r-n-:%s\n",rbuf_pos,rbuf); 
      
   }
   
   unsolicited = 0;
   if (rbuf_pos > 3 && !strncmp_P(rbuf + rbuf_pos - 2,PSTR("\r\n"), 2)){
      /* try to find previous <cr><lf> */
      start = findcrlfbw(rbuf + rbuf_pos - 2, rbuf_pos);
      /* if not found, start at buffer beginning */
      if (!start)
         start = rbuf;
      /* there are certainly more that 2 chars in buffer */
      if (!strncmp_P(start, PSTR("OK"), 2))
      {
         status = GN_AT_OK;
      }
      else if (rbuf_pos > 6 && !strncmp(start, "ERROR", 5))
         status = GN_AT_ERROR;
         /* FIXME: use error codes some useful way */
      else if (sscanf(start, "+CMS ERROR: %d", &error) == 1) {
         status  = GN_AT_CMS;
         rbuf[0] = error / 256;
         rbuf[1] = error % 256;
      } else if (sscanf(start, "+CME ERROR: %d", &error) == 1) {
         status = GN_AT_CME;
         rbuf[0] = error / 256;
         rbuf[1] = error % 256;
      } else if (!strncmp(start, "SMS Ready", 9) == 1) {
         status  = GN_AT_SMS_READY;  
      } else if (!strncmp(start, "RING", 4) ||
         !strncmp(start, "CONNECT", 7) ||
//         !strncmp(start, "BUSY", 4) ||
//         !strncmp(start, "NO ANSWER", 9) ||
         !strncmp(start, "Call Ready", 10) ||
         !strncmp(start, "NO CARRIER", 10) ||
         !strncmp(start, "NO DIALTONE", 11)) {
         status = GN_OP_AT_Ring;
         unsolicited = 1;
      } else if (*start == '+') {
         /* check for possible unsolicited responses */
         if (!strncmp(start + 1, "CREG:", 5)) {
            count = numchar((unsigned char *)start,((unsigned char)','));
            if (count == 2) {
               status = GN_OP_GetNetworkInfo;
               unsolicited = 1;
            }
         } else if (!strncmp(start + 1, "CFUN:", 5)) {
            status = GN_AT_OK;
         } else if (!strncmp(start + 1, "CPIN:", 5)) {
            status = GN_AT_OK;
         } else if (!strncmp(start + 1, "CRING:", 6) ||
            !strncmp(start + 1, "CLIP:", 5) ||
            !strncmp(start + 1, "CLCC:", 5)) {
            status = GN_OP_AT_Ring;
            unsolicited = 1;
        } else if (!strncmp(start + 1, "CMT:", 4)) {
           status = GN_OP_AT_IncomingSMS;
           unsolicited = 1;
        }
     }
   }
   /* check if SMS prompt is found */
   if (rbuf_pos > 3 && !strncmp(rbuf + rbuf_pos - 4, "\r\n> ", 4))
      status  = GN_AT_PROMPT;

   if (status  == GN_OP_AT_IncomingSMS) {
      sm_incoming_function(rbuf); 
   }
   if (status  != GN_AT_NONE) {
      int pos = rbuf_pos;
      //at_dprintf("read : ", rbuf + 1, rbuf_pos - 1);
      rbuf_pos = 0;
      binlen = 1;
      /*if (unsolicited){
         //sm_incoming_function(bi->rbuf[0], start, rbuf_pos - 1 - (start - bi->rbuf), sm);
         // Procesar mensaje 
         sm_incoming_function(rbuf);
      }//else
         //sm_incoming_function(sm->last_msg_type, rbuf, rbuf_pos - 1, sm);
      */   
      //free(rbuf);
      //rbuf = NULL;
      rbuf_size = 0;
      return;
  }
       
}



gn_error sim800L::at_sm_block(at_result waitfor)
{
   char ch;
   uint16_t i, j;	// = TIMERVALUE;
   
   long previousMillis = millis();
   
   do{
	    ch = p_serial->read();
	    if (ch != 0xff && ch != 0x00)
      {
         //p_monitor->printf("-->Block 41:%d\n",ch);
         p_monitor->write(ch);
         atbus_rx_statemachine(ch);
         if(status == waitfor) 
         {
            if(p_monitor != NULL ) p_monitor->println("at_sm_block-->GN_ERROR_NONE");  
            return GN_ERR_NONE;
         }     			
	    
      }
   }while( ((millis()- previousMillis) < CLOCK_T_500ms) );  //500ms Exito 

   return GN_ERR_TIMEOUT;
}

void sim800L::sm_incoming_function(char *msg)
{
   int count;
   char ch, *str;
   long previousMillis;

   /* Estructura de mensaje texto

   +CMT: "65074018","","20/05/14,15:49:59-16"
   Este es mi mensaje
  
  
    */
   // Obtener numero de telefono
   str = strchr(msg,'\"');
   str++;
   count=0;

   if(p_monitor != NULL ) p_monitor->print("-> sm_incoming_function:Phone number:"); 
   do{
      phone[count++]=*str;
      if(p_monitor != NULL ) p_monitor->write(*str);
      str++;
   }while(*str !='\"');
   ///////////////////////
   phone[count]=0;

   p_monitor->write('\n');

   if(p_monitor != NULL ) p_monitor->print("--> sm_incoming_function:Message Received:");  
   

   previousMillis = millis();
   count = 0;
   while( millis()- previousMillis < CLOCK_T_5s){
      ch = p_serial->read();
      if (ch != 0xff && ch != 0x00) {

         if(p_monitor != NULL ) p_monitor->write(ch);
         message[count++] = ch;
         
         if (count >1 && !strcmp_P(message-2, PSTR("\r\n"))) {
            message[count-2]=0;
            return;
      
         }
      }
   }
   p_monitor->write('\n');
   
}


uint8_t sim800L::SendSMS(char *phone, char *msg) 
{
   char str[30];
   // AT <cr> <lf>
   p_serial->print(F("AT\r"));  // It was "AT\r\n" 
   at_sm_block(GN_AT_OK); 
   
   // AT+CMGF=1 <cr> <lf> AT+CMGF=<mode> 0=PDU Mode, 1=Text Mode
   p_serial->print(F("AT+CMGF=1\r")); 
   at_sm_block(GN_AT_OK);

   /*sprintf(str,"AT+CSCA=\"%s\"\r",smsc);
   p_serial->print(str);  
   at_sm_block(GN_AT_OK); 
   */
   sprintf(str,"AT+CMGS=\"%s\"\r",phone);
   p_serial->print(str);  
   at_sm_block(GN_AT_PROMPT); 

   sprintf(str,"%s\32",msg);
   p_serial->print(str);
   at_sm_block(GN_AT_OK); 
   
   return 0;
}   

uint8_t sim800L::Ready2ReceiveSMS() 
{
   // AT <cr> <lf>
   p_serial->print(F("AT\r")); 
   at_sm_block(GN_AT_OK); 
   
   // AT+CMGF=1 <cr> <lf> AT+CMGF=<mode> 0=PDU Mode, 1=Text Mode
   p_serial->print(F("AT+CMGF=1\r")); 
   at_sm_block(GN_AT_OK); 
   
   // AT+CNMI=1,2,0,0,0   Mode, mt, bm, ds, bfr
   p_serial->print(F("AT+CNMI=1,2,0,0,0\r")); 
   at_sm_block(GN_AT_OK); 
   
   return 0;
}
int sim800L::get_status(){
   return status;
}

char *sim800L::get_phone(){
  return phone;
}
char *sim800L::get_msg(){
  return message;
}
