#include "loraRam.h"
#include "Mac/LoRaMac.h"
#include "lora.h"
#include "shmSignal.h"



// raktas, bu fonksiyonlara bakilacak.


/*

McpsIndication(McpsIndication_t *);
McpsConfirm(McpsConfirm_t *);
MlmeIndication(MlmeIndication_t *);


*/
/*!
 * \brief  MLME-Indication event function
 *
 * \param  [IN] mlmeIndication - Pointer to the indication structure.
 */
void MlmeIndication(MlmeIndication_t *mlmeIndication) {
  switch (mlmeIndication->MlmeIndication) {
  case MLME_SCHEDULE_UPLINK: { // The MAC signals that we shall provide an uplink as soon as possible
    //SendFrame();
    break;
  }
  default:
    break;
  }
}

/*!
 * \brief  MCPS-Confirm event function
 *
 * \param  [IN] mcpsConfirm - Pointer to the confirm structure,
 *               containing confirm attributes.
 */
void McpsConfirm(McpsConfirm_t *mcpsConfirm) {
  if (mcpsConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK) {
    switch (mcpsConfirm->McpsRequest) {
    case MCPS_UNCONFIRMED: {

      // Check Datarate
      // Check TxPower
      break;
    }
    case MCPS_CONFIRMED: {
      // Check Datarate
      // Check TxPower
      // Check AckReceived
      // Check NbTrials
      break;
    }
    case MCPS_PROPRIETARY: {
      break;
    }
    default:
      break;
    }

//raktas incele yorumdaki yeri
    // Switch LED 1 ON
    //GpioWrite(&Led1, 0);
  }
  NextTx = true;
}


/*


  // Check Multicast
  // Check Port
  // Check Datarate
  // Check FramePending
  // Check Buffer
  // Check BufferSize
  // Check Rssi
  // Check Snr
  // Check RxSlot

*/

/*!
 * \brief   MCPS-Indication event function
 *
 * \param   [IN] mcpsIndication - Pointer to the indication structure,
 *               containing indication attributes.
 *  
 *  You can
 *  Check Multicast
 *  Check Port
 *  Check Datarate
 *  Check FramePending
 *  Check Buffer
 *  Check BufferSize
 *  Check Rssi
 *  Check Snr
 *  Check RxSlot.
 *  
 *  raktas
 */
void McpsIndication(McpsIndication_t *mcpsIndication) {
  //isConfirmed = false;
  if (mcpsIndication->Status != LORAMAC_EVENT_INFO_STATUS_OK) {
    return;
  }

  switch (mcpsIndication->McpsIndication) {
  case MCPS_UNCONFIRMED: {
    break;
  }
  case MCPS_CONFIRMED: {
    //isConfirmed = true;
    break;
  }
  case MCPS_PROPRIETARY: {
    break;
  }
  case MCPS_MULTICAST: {
    break;
  }
  default:
    break;
  }

  if( ComplianceTest.Running == true )
  {
      ComplianceTest.DownLinkCounter++;
  }

  if (mcpsIndication->RxData == true) {
    switch (mcpsIndication->Port) {
    case 1: // The application LED can be controlled on port 1 or 2
    case 2:
    //incelenecek raktas
//      if (mcpsIndication->BufferSize == 1) {
//        AppLedStateOn = mcpsIndication->Buffer[0] & 0x01;
//        if (AppLedStateOn == 1) {
//          GpioWrite(&Led2, 0);
//        } else {
//          GpioWrite(&Led2, 1);
//        }
//      }
//
//      isConfirmed = true;
      break;
    case 224:
      if (ComplianceTest.Running == false)
      {
        // Check compliance test enable command (i)
        if( ( mcpsIndication->BufferSize == 4 ) &&
            ( mcpsIndication->Buffer[0] == 0x01 ) &&
            ( mcpsIndication->Buffer[1] == 0x01 ) &&
            ( mcpsIndication->Buffer[2] == 0x01 ) &&
            ( mcpsIndication->Buffer[3] == 0x01 ) )
        {
            ComplianceTest.IsTxConfirmed = false;
            ComplianceTest.AppPort = 224;
            ComplianceTest.AppDataSize = 2;
            ComplianceTest.DownLinkCounter = 0;
            ComplianceTest.LinkCheck = false;
            ComplianceTest.DemodMargin = 0;
            ComplianceTest.NbGateways = 0;
            ComplianceTest.Running = true;
            ComplianceTest.State = 1;
          
            MibRequestConfirm_t mibReq;
            mibReq.Type = MIB_ADR;
            mibReq.Param.AdrEnable = true;
            LoRaMacMibSetRequestConfirm( &mibReq );

            #if defined( REGION_EU868 )
                        LoRaMacTestSetDutyCycleOn( false );
            #endif
        }
      }
      else 
      {
        ComplianceTest.State = mcpsIndication->Buffer[0];
        switch(ComplianceTest.State)
        {
          case 0: // Check compliance test disable command (ii)
            ComplianceTest.DownLinkCounter = 0;
            ComplianceTest.Running = false;
          
            MibRequestConfirm_t mibReq;
            mibReq.Type = MIB_ADR;
            mibReq.Param.AdrEnable = lora_config.IsTxConfirmed;
            LoRaMacMibSetRequestConfirm( &mibReq );
            #if defined( REGION_EU868 )
              LoRaMacTestSetDutyCycleOn( LORAWAN_DUTYCYCLE_ON );
            #endif
            break;

          case 1: // (iii, iv)
            ComplianceTest.AppDataSize = 2;
            break;

          case 2: // Enable confirmed messages (v)
            ComplianceTest.IsTxConfirmed = true;
            ComplianceTest.State = 1;
            break;

          case 3:  // Disable confirmed messages (vi)
            ComplianceTest.IsTxConfirmed = false;
            ComplianceTest.State = 1;
            break;

          case 4: // (vii)
            ComplianceTest.AppDataSize = mcpsIndication->BufferSize;
            ComplianceTest.AppDataBuffer = malloc(mcpsIndication->BufferSize);

            ComplianceTest.AppDataBuffer[0] = 4;
            for( uint8_t i = 1; i < ComplianceTest.AppDataSize; i++ )
            {
              ComplianceTest.AppDataBuffer[i] = mcpsIndication->Buffer[i] + 1;
            }
            break;

          case 5: // (viii)
          {  
            MlmeReq_t mlmeReq;
            mlmeReq.Type = MLME_LINK_CHECK;
            LoRaMacMlmeRequest( &mlmeReq );
            
            break;
          }
          case 6: // (ix)            
          {
            MlmeReq_t mlmeReq;

            // Disable TestMode and revert back to normal operation

            ComplianceTest.DownLinkCounter = 0;
            ComplianceTest.Running = false;

            MibRequestConfirm_t mibReq;
            mibReq.Type = MIB_ADR;
            mibReq.Param.AdrEnable = lora_config.IsTxConfirmed;
            LoRaMacMibSetRequestConfirm( &mibReq );
          
            #if defined(REGION_EU868)
              LoRaMacTestSetDutyCycleOn(LORAWAN_DUTYCYCLE_ON);
            #endif

            mlmeReq.Type = MLME_JOIN;

            mlmeReq.Req.Join.DevEui = DevEui;
            mlmeReq.Req.Join.AppEui = AppEui;
            mlmeReq.Req.Join.AppKey = AppKey;
            mlmeReq.Req.Join.Datarate = LORAWAN_DEFAULT_DATARATE;

            LoRaMacMlmeRequest( &mlmeReq );
            setDeviceState(DEVICE_STATE_SLEEP);
            break;
          }
          case 7: // (x)
            if( mcpsIndication->BufferSize == 3 )
            {
                MlmeReq_t mlmeReq;
                mlmeReq.Type = MLME_TXCW;
                mlmeReq.Req.TxCw.Timeout = ( uint16_t )( ( mcpsIndication->Buffer[1] << 8 ) | mcpsIndication->Buffer[2] );
                LoRaMacMlmeRequest( &mlmeReq );
            }
            else if( mcpsIndication->BufferSize == 7 )
            {
                MlmeReq_t mlmeReq;
                mlmeReq.Type = MLME_TXCW_1;
                mlmeReq.Req.TxCw.Timeout = ( uint16_t )( ( mcpsIndication->Buffer[1] << 8 ) | mcpsIndication->Buffer[2] );
                mlmeReq.Req.TxCw.Frequency = ( uint32_t )( ( mcpsIndication->Buffer[3] << 16 ) | ( mcpsIndication->Buffer[4] << 8 ) | mcpsIndication->Buffer[5] ) * 100;
                mlmeReq.Req.TxCw.Power = mcpsIndication->Buffer[6];
                LoRaMacMlmeRequest( &mlmeReq );
            }
            ComplianceTest.State = 1;
            break;

          default:                  
            break;
        }
      }

    case 92:
     //Rx tarafi duzenlenecek raktas
      receiveLoraMessage(mcpsIndication->Buffer, mcpsIndication->BufferSize);
      break;

    default:
     // isConfirmed = false;
      break;
    }
  }

  // Switch LED 1 ON for each received downlink
  //GpioWrite(&Led1, 0);
}


/*!
 * \brief   MLME-Confirm event function
 *
 * \param   [IN] MlmeConfirm - Pointer to the confirm structure,
 *               containing confirm attributes.
 *  Response from Lora
 */
void MlmeConfirm( MlmeConfirm_t *mlmeConfirm )
{
  switch( mlmeConfirm->MlmeRequest )
  {
    case MLME_JOIN:

      if( mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK )
      {
          // Status is OK, node has joined the network
          setDeviceState(DEVICE_STATE_JOINED);
      }
      else
      {
          // Join was not successful. Try to join again from loraStateMachine task
          setDeviceState(DEVICE_STATE_JOIN);
      }
      break;

    case MLME_LINK_CHECK:

      if( mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK )
      {
          // Check DemodMargin
          // Check NbGateways
          if( ComplianceTest.Running == true )
          {
              ComplianceTest.LinkCheck = true;
              ComplianceTest.DemodMargin = mlmeConfirm->DemodMargin;
              ComplianceTest.NbGateways = mlmeConfirm->NbGateways;
          }
      }
      break;

    default:
      break;
  }
  NextTx = true;
}



/*!
 * \brief  Prepares the payload of the frame
 */
void PrepareTxFrame(AppData_t *AppData, uint8_t *IsTxConfirmed) {

  if( ComplianceTest.Running == true )
  {
    *IsTxConfirmed = ComplianceTest.IsTxConfirmed;
    AppData->Port = ComplianceTest.AppPort;

    if( ComplianceTest.LinkCheck == true )
    {
      ComplianceTest.LinkCheck = false;
      AppData->BuffSize = 3;
      AppData->Buff = malloc(AppData->BuffSize);
      AppData->Buff[0] = 5;
      AppData->Buff[1] = ComplianceTest.DemodMargin;
      AppData->Buff[2] = ComplianceTest.NbGateways;
      ComplianceTest.State = 1;
    }
    else
    {
      switch( ComplianceTest.State )
      {
        case 4:
          AppData->Buff = ComplianceTest.AppDataBuffer;
          AppData->BuffSize = ComplianceTest.AppDataSize;
          ComplianceTest.State = 1;
          break;
        case 1:
          AppData->BuffSize = 2;
          AppData->Buff = malloc(AppData->BuffSize);
          AppData->Buff[0] = ComplianceTest.DownLinkCounter >> 8;
          AppData->Buff[1] = ComplianceTest.DownLinkCounter;
          break;
      }
    }
  }
  else
  {
    LoraTxData(AppData, IsTxConfirmed);
  }
}



/*!
 * \brief  Prepares the payload of the frame
 *
 * \retval  [0: frame could be send, 1: error]
 */
bool SendFrame(AppData_t *AppData, uint8_t *IsTxConfirmed, bool *isBusy) {
  McpsReq_t mcpsReq;
  LoRaMacTxInfo_t txInfo;
  uint8_t mcpsRequestInfo;
  *isBusy = false;

  //ilk if paket kaybina sebep olabilir. raktas inceles
  if(LoRaMacQueryTxPossible(AppData->BuffSize, &txInfo) != LORAMAC_STATUS_OK) 
  {
    // Send empty frame in order to flush MAC commands
    mcpsReq.Type = MCPS_UNCONFIRMED;
    mcpsReq.Req.Unconfirmed.fBuffer = NULL;
    mcpsReq.Req.Unconfirmed.fBufferSize = 0;
    mcpsReq.Req.Unconfirmed.Datarate = LORAWAN_DEFAULT_DATARATE;
  } 
  else 
  {
    
    if(AppData->Buff == NULL)
    {
      return true;
    }
  
    if (lora_config.IsTxConfirmed == false) 
    {
      mcpsReq.Type = MCPS_UNCONFIRMED;
      mcpsReq.Req.Unconfirmed.fPort = AppData->Port;
      mcpsReq.Req.Unconfirmed.fBuffer = AppData->Buff;
      mcpsReq.Req.Unconfirmed.fBufferSize = AppData->BuffSize;
      mcpsReq.Req.Unconfirmed.Datarate = LORAWAN_DEFAULT_DATARATE;
    } 
    else 
    {
      mcpsReq.Type = MCPS_CONFIRMED;
      mcpsReq.Req.Confirmed.fPort = AppData->Port;
      mcpsReq.Req.Confirmed.fBuffer = AppData->Buff;
      mcpsReq.Req.Confirmed.fBufferSize = AppData->BuffSize;
      mcpsReq.Req.Confirmed.Datarate = LORAWAN_DEFAULT_DATARATE;
      mcpsReq.Req.Confirmed.NbTrials = 8;   
    }
  }


  mcpsRequestInfo = LoRaMacMcpsRequest(&mcpsReq);
 
  if(mcpsRequestInfo == LORAMAC_STATUS_BUSY)
  {
    *isBusy = true;
    return true; // same buffer must be send again, dont free buff.
  }

  if(AppData->Buff != NULL)
  {
    free(AppData->Buff);
    AppData->Buff = NULL;
  }

  if (mcpsRequestInfo == LORAMAC_STATUS_OK) 
  {
    printf("msgSend");
    return false;
  }

  return true;
}

/*
* Check Network
*
*/
void OnSendEvent()
{
  MibRequestConfirm_t mibReq;
  LoRaMacStatus_t status;

  mibReq.Type = MIB_NETWORK_JOINED;
  status = LoRaMacMibGetRequestConfirm( &mibReq );

  if( status == LORAMAC_STATUS_OK )
  {
      if( mibReq.Param.IsNetworkJoined == true )
      {
          setDeviceState(DEVICE_STATE_SEND);
          NextTx = true;
      }
      else
      {
          setDeviceState(DEVICE_STATE_JOIN);
      }
  }
}