#include <stdlib.h>          // malloc(),
#include <stdarg.h>

#include <stdio.h>

#include <string.h>

#include "HardwareSerial.h"
#include "gps.h"

extern uint8_t usart_rxd_buf_cnt;
Gps::Gps(){
  p_serial = NULL;
  p_monitor = NULL;
}
void Gps::Init(Stream *p_ser)
{
  p_serial = p_ser;
  p_monitor = NULL;
}

/* function initliazes GPS with port settings and message rates, only GGA 
and RMC are needed for GPS data, change baud rate for different modules, 
timezone specified also */
void Gps::Init(Stream *p_ser, Stream *p_mon)
{
  p_serial = p_ser;
  p_monitor = p_mon;
  
  state = GPS_RX_Sync;

        NMEASetRate ( 0, 0 );  // GGA Set rate for periodic output every sec (0, 1)
        NMEASetRate ( 1, 0 );
        NMEASetRate ( 2, 0 );
        NMEASetRate ( 3, 0 );

        NMEASetRate ( 4, 1 ); // RMC 4, 1
        NMEASetRate ( 5, 0 );
        NMEASetRate ( 6, 0 );
        NMEASetRate ( 8, 0 );
    //bug :fix
    sLatitude[0]=0;
    sLongitude[0]=0;      
}
   

/* function converts string to a double */
void Gps::NMEAParseLatitude ()
{
   unsigned char degree;
   double t;
   degree = (str[0] - '0') * 10 + (str[1] - '0');
   str[0] = '0';
   str[1] = '0';

   String dest = str; 
  
   t = dest.toDouble();
   t = (t / (double)60.0) + (double)degree;
   if(NSIndicator) t = -t;
   sprintf(sLatitude,"%11.6f",t);
   return;   

}

/* function converts string to a double */
void Gps::NMEAParseLongitude ()
{
   unsigned char degree;
   double t;
   unsigned char ts [ 10 ] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
   
   degree = (str[0] - '0') * 100 + (str[1] - '0') * 10 + (str[2] - '0');
   str[0] = '0';
   str[1] = '0';
   str[2] = '0';
   
   String dest = str; 
  
   t = dest.toDouble();
   t = (t / (double)60.0) + (double)degree;
   if(EWIndicator) t = -t;
   sprintf(sLongitude,"%11.6f",t);
   return;   
}

void Gps::GetGPSPos(char *dest,char *source,uint8_t sign)
{
   unsigned int u=0,d=0;
   unsigned int minutes;
   unsigned char pos,i;
   
   pos=0;
   while(source[pos] != '.')
      pos++;

   for(i=0;i<pos-2;i++)
   {
      u*=10;
      u+=source[i]-'0';
   }

   
   d=(source[pos-2]-'0')*10;
   d+=(source[pos-1]-'0');

   
   for(i=0; i<4; i++) //Only 4 chars
   {
      d = d*10;
      d = d +(source[pos+1+i]-'0');
   }

  minutes=d/60;
  
    
  sprintf(str,"%d.%04d",(sign?-1:1)*u,minutes);
  /*pos=0;
  if(sign) 
    dest[pos++]='-';
  if(u>100)
    dest[pos++]=u/100+'0';
  if(u>10)
    dest[pos++]=u/10%10+'0';
  
   dest[pos++]=u%10+'0';
   dest[pos++]='.';
   dest[pos++]=minutes/1000+'0';
   dest[pos++]=minutes/100%10+'0';
   dest[pos++]=minutes/10%10+'0';
   dest[pos++]=minutes%10+'0';
   dest[pos++]=0;*/

   //p_monitor->printf("-->%s\n",dest);
   return;
}
  
uint8_t Gps::gps_rx_frame(uint8_t rx_char)
{
   uint8_t j;
   
   //lcd_putchar(rx_char);
   
   switch(state)
   {
      case GPS_RX_Sync:
         if(rx_char == '$')
	          state = GPS_RX_MsgID;
         break;			
      case GPS_RX_MsgID:
         if(rx_char == ',')
            state = GPS_RX_UTCTime;
         break;			
      case GPS_RX_UTCTime:
         if(rx_char == ',')
            state = GPS_RX_Status;
         break;			
      case GPS_RX_Status:
         if(rx_char==',') 
		     {
            state = GPS_RX_Latitud;
            count = 0;
         }
         else
         { 	  
            if(rx_char == 'A') 
		           valid=1;
            else
		           valid=0;
         }			
         break;			
      case GPS_RX_Latitud:
         if(rx_char==',') {
		        str[count]='\0';

            //p_monitor->printf("Lat:%s\n",str);
		        state = GPS_RX_NSIndicator;
         }	  
         else
		     {
		        str[count++]=rx_char;
         }		 
		 break;
      case GPS_RX_NSIndicator:
         if(rx_char == ',')  
         {
            state = GPS_RX_Longitude;
            count=0;
         }
         else
         {
            if(rx_char=='N')   
		           NSIndicator = 0; // '+'
            else 
               NSIndicator = 1;  //'-';   

            // Funcion latitude   
            NMEAParseLatitude();
            //GetGPSPos(sLatitude,str,NSIndicator);                  
		 
         }         			
         break;
      case GPS_RX_Longitude: 
         if(rx_char==','){ 
            str[count]='\0';	
                        //p_monitor->printf("Lon:%s\n",str);

 		        state = GPS_RX_EWIndicator;
         }	  
         else
		     {
		        str[count++]=rx_char;
         }		 
		 break;	
      case GPS_RX_EWIndicator:
         if(rx_char == ',')
         {
            state = GPS_RX_Speed;
            count=0;
         }
         else
         {
            if(rx_char=='E')   
               EWIndicator = 0;  // '+';
            else 
               EWIndicator = 1;  // '-';   

            // Funcion latitude 
            NMEAParseLongitude();  
            //GetGPSPos(sLongitude,str,EWIndicator);      
         }         			
         break;	  
      case GPS_RX_Speed:
         if(rx_char == ',')
         {
            speed[count] = 0;
			
            //i->count=0;
            state = GPS_RX_Sync;
			      return 1;
         }
         else
         {
		        speed[count++]= rx_char;
         }          	     
	  
         break;	  
      case GPS_RX_Course:
	  
	  
         break;	  
  
   }	  
	  
   return 0;  
}


//
// $GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A
int Gps::gps_putchar(char c )
{
   static unsigned char checksum;
   
   if ( c == '$' ) 
		  checksum = '*'; 
	 else 
		  checksum ^= c; // clear checksum if new sentence
		   
   p_serial->write( c ); // send char
   if( c == '*' )
   {
       /* end of sentence, print the checksum in hex, then new line sequence */
    
       c = (checksum & 0xF0) >> 4;
       c += '0';
       if ( c > '9' )
       {
          c = c - '0' - 10 + 'A';
       }
       p_serial->write(c );
       c = checksum & 0x0F;
       c += '0';
       if ( c > '9' )
       {
          c = c - '0' - 10 + 'A';
       }
       p_serial->write( c );
    
       p_serial->write( '\r' );
       p_serial->write( '\n' );
  }
  return 0;
}

void Gps::NMEASetRate ( unsigned char m, unsigned int r )
{
   sprintf(str,"$PSRF103,%d,00,%d,01*", m, r);
   
   int i=0;
   
   while(str[i]!= NULL){
       
      gps_putchar(str[i]); 
      i++;
   }
   //fprintf_P(&gpsStdout,PSTR("$PSRF103,%d,00,%d,01*"), m, r);
}
