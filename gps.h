#ifndef gps_inc

/* GPS data structure, string structure, and function prototypes */

enum gps_rx_state {
	GPS_RX_Sync,
	GPS_RX_MsgID,
	GPS_RX_UTCTime,
	GPS_RX_Status,
	GPS_RX_Latitud,
	GPS_RX_NSIndicator,
	GPS_RX_Longitude,
	GPS_RX_EWIndicator,
	GPS_RX_Speed,
	GPS_RX_Course
};


class Gps {
  private:
    char  str[64];
    uint8_t count;
    uint8_t state;
  public: 
    
    Stream *p_serial;
    Stream *p_monitor;
  public:    
   // uint8_t status;
    unsigned char valid;

   char NSIndicator;
   char sLatitude[15];    
   char EWIndicator;
   char sLongitude[15];
   char speed[7];   
  public:

   Gps();
   void Init(Stream *p_ser);
   void Init(Stream *p_ser, Stream *p_mon);
        
   void NMEAParseLatitude ();
   void NMEAParseLongitude ();
   void GetGPSPos(char *str,char *NMEAgpspos,uint8_t sign);
   uint8_t gps_rx_frame(uint8_t rx_char);
   int gps_putchar (char c);
   void NMEASetRate ( unsigned char m, unsigned int r );
   char *getLat() { return sLatitude; };
   char *getLon() { return sLongitude; };
};


#define gps_inc
#endif
