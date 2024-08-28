#ifndef  _LORA_H
#define  _LORA_H

#include "Mac/LoRaMac.h"


extern void MlmeConfirm(MlmeConfirm_t *);
extern void McpsIndication(McpsIndication_t *);
extern void McpsConfirm(McpsConfirm_t *);
extern void MlmeIndication(MlmeIndication_t *);
extern bool SendFrame(AppData_t *, uint8_t *, bool *);

#endif