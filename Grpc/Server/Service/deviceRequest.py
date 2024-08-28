import grpc
import json
import base64
from chirpstack_api import api
import struct
import datetime


server = "localhost:8080"
dev_eui = "4b0369740f7fdc16"
api_token = "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJhdWQiOiJjaGlycHN0YWNrIiwiaXNzIjoiY2hpcnBzdGFjayIsInN1YiI6IjdlN2M1MGJmLTM0NjAtNDNhYy1iZGM2LTM5MGYxMWQzM2ZkMSIsInR5cCI6ImtleSJ9.TwpPr00ajarw55QpH9Hyu0tKartNsJHwzIzu32r7vtQ"

import datetime

def convert_time_to_bytes():
    current_time = datetime.datetime.now()
    hour = current_time.hour
    minute = current_time.minute
    second = current_time.second
    print(hour,minute,second)

    hour_byte = hour.to_bytes(1, byteorder='big')
    minute_byte = minute.to_bytes(1, byteorder='big')
    second_byte = second.to_bytes(1, byteorder='big')

    time_bytes = hour_byte + minute_byte + second_byte

    return time_bytes

# Anlık saati byte formatına çevirme


def main(cihaze_ui):
    channel = grpc.insecure_channel(server)
    client = api.DeviceServiceStub(channel)

    auth_token = [("authorization", "Bearer %s" % api_token)]
    tarih = convert_time_to_bytes()
    print(tarih)
    final_data = bytes([0x18])   + tarih


    req = api.EnqueueDeviceQueueItemRequest()
    req.queue_item.confirmed = False
    req.queue_item.data = final_data
    req.queue_item.dev_eui = cihaze_ui
    req.queue_item.f_port = 92

    resp = client.Enqueue(req, metadata=auth_token)


    print(resp.id)
    return  resp.id

if __name__ == '__main__':
    main()