

#ifndef MESSAGECODES_H__
#define MESSAGECODES_H__


#define READEEPROMADDRESSCYCLE 3

#define SET_XAXISTHRESHOLD_VALUE_MESSAGE      0x0A    /* MID for defining x axis threshold value over bluetooth. */
#define SET_YAXISTHRESHOLD_VALUE_MESSAGE      0x0B    /* MID for defining y axis threshold value over bluetooth. */
#define READ_AXISTHRESHOLD_VALUE_MESSAGE      0x0C    /* MID for sending axis threshold values over bluetooth. */
#define SEND_EEPROM_DATA_MESSAGE              0x0D    /* MID for sending eeprom datas over bluetooth. */
#define SEND_THRESHOLD_VALUE_MESSAGE          0x0E 
#define READLORAPARAMETERS                    0x0F   
#define FIRSTTIMEINIT                         0x10   
#define MAX_VALUES_SEND                       0x11
#define ABOVE_THRESHOLD_SEND                  0x12
#define SET_THRESHOLD                         0x13   
#define SET_FREQUENCY                         0x14 
#define READ_FREQUENCY                        0x15 
#define JOIN_REQUEST                          0x16
#define MESSAGE_FAILED                        0x17
#define SET_TIME                              0x18    // time for sending first data Ahmet
#define GET_SHOW_THRESHOLD                    0X19    /* Ahmet deneme*/ 
#define KALIBRASYON                           0X20    /* Kalibrasyon iþlemi*/
#define RESET_MANUAL                          0X21    /* reset iþlemini manüel yapmak*/
#define DEVICE_INFO_MESSAGE_CODE              0X22
#define EEPROM_DATA_MESSAGE                   0x40    /* MID for sending first part of eeprom data. */






#endif  /* _ MESSAGECODES_H__ */