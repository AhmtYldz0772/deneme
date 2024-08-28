import grpc

from chirpstack_api import api
import struct
import datetime


server = "localhost:8080"
dev_eui = "4c3b784abc584274"
api_token = "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJhdWQiOiJjaGlycHN0YWNrIiwiaXNzIjoiY2hpcnBzdGFjayIsInN1YiI6IjY2NzIzODNmLTQ2MzgtNDQ5ZC04ZGJjLTdmM2RiZDczZTRjMiIsInR5cCI6ImtleSJ9.ttRS4djY8yiFYfrXOwuwJdzEd5BQhmwVxZKLcXGZoEQ"

def main():
    # print(threashold)  #log için atıldı
    channel = grpc.insecure_channel(server)
    client = api.DeviceServiceStub(channel)
    # byte_threshold = int(threshold)
    # byte_array = byte_threshold.to_bytes(4, byteorder='big')
    # print(byte_array)
    byte_array = bytes([0X55])
    auth_token = [("authorization", "Bearer %s" % api_token)]

    req = api.EnqueueDeviceQueueItemRequest()
    req.queue_item.confirmed = False
    req.queue_item.data = bytes([0x18]) + byte_array
    req.queue_item.dev_eui = dev_eui
    req.queue_item.f_port = 92

    resp = client.Enqueue(req, metadata=auth_token)

    print(resp.id)
    return resp.id

if __name__ == '__main__':
    main()