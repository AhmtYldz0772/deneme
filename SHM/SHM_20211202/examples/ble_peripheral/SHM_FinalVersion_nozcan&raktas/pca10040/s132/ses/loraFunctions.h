extern bool createTxBuffQueue();

extern void sendLoraDataMessage(uint8_t *data, uint8_t dataSize);

extern void LoraTxData(AppData_t *AppData, uint8_t *IsTxConfirmed);

extern void receiveLoraMessage(uint8_t *buffPtr, uint8_t buffSize);

extern void setDeviceState(deviceStateEnum_t state);

extern void setNextTx(uint8_t nextTx) ;

extern int getNextTx(void) ;

extern void startSendLoraPeriodicDataTask();

extern void stopSendLoraPeriodicDataTask();

extern deviceStateEnum_t getDeviceState() ;