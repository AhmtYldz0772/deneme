import grpc
from chirpstack_api import api
import struct
import datetime

server = "localhost:8080"
#dev_eui = "4b0369740f7fdc16"
api_token = "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJhdWQiOiJjaGlycHN0YWNrIiwiaXNzIjoiY2hpcnBzdGFjayIsInN1YiI6IjY2NzIzODNmLTQ2MzgtNDQ5ZC04ZGJjLTdmM2RiZDczZTRjMiIsInR5cCI6ImtleSJ9.ttRS4djY8yiFYfrXOwuwJdzEd5BQhmwVxZKLcXGZoEQ"

def main(Frenkans,dev_eui):
    channel = grpc.insecure_channel(server)
    client = api.DeviceServiceStub(channel)
    byte_Frenkans = int(Frenkans)
    byte_array = byte_Frenkans.to_bytes(4, byteorder='big')
    print(byte_array)

    auth_token = [("authorization", "Bearer %s" % api_token)]

    req = api.EnqueueDeviceQueueItemRequest()
    req.queue_item.confirmed = False
    req.queue_item.data = bytes([0x14]) + byte_array
    req.queue_item.dev_eui = dev_eui
    req.queue_item.f_port = 92

    resp = client.Enqueue(req, metadata=auth_token)


    
    return resp.id

if __name__ == '__main__':
    main()