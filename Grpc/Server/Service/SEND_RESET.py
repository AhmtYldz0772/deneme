import grpc
from chirpstack_api import api
import struct
import datetime

server = "localhost:8080"
#dev_eui = "4b0369740f7fdc16"
api_token = "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJhdWQiOiJjaGlycHN0YWNrIiwiaXNzIjoiY2hpcnBzdGFjayIsInN1YiI6IjdlN2M1MGJmLTM0NjAtNDNhYy1iZGM2LTM5MGYxMWQzM2ZkMSIsInR5cCI6ImtleSJ9.TwpPr00ajarw55QpH9Hyu0tKartNsJHwzIzu32r7vtQ"

def main(dev_eui):
    channel = grpc.insecure_channel(server)
    client = api.DeviceServiceStub(channel)


    auth_token = [("authorization", "Bearer %s" % api_token)]

    req = api.EnqueueDeviceQueueItemRequest()
    req.queue_item.confirmed = False
    req.queue_item.data = bytes([0x21]) 
    req.queue_item.dev_eui = dev_eui
    req.queue_item.f_port = 92

    resp = client.Enqueue(req, metadata=auth_token)

    return resp.id

if __name__ == '__main__':
    main()