import grpc
from chirpstack_api import api
import struct
import datetime

server = "localhost:8080"
api_token = "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJhdWQiOiJjaGlycHN0YWNrIiwiaXNzIjoiY2hpcnBzdGFjayIsInN1YiI6IjdlN2M1MGJmLTM0NjAtNDNhYy1iZGM2LTM5MGYxMWQzM2ZkMSIsInR5cCI6ImtleSJ9.TwpPr00ajarw55QpH9Hyu0tKartNsJHwzIzu32r7vtQ"

def main(threshold_value, cihaze_eui):
    channel = grpc.insecure_channel(server)
    client = api.DeviceServiceStub(channel)
    print(threshold_value)

    byte_array = struct.pack('f', threshold_value)
    
    auth_token = [("authorization", "Bearer %s" % api_token)]
    req = api.EnqueueDeviceQueueItemRequest()
    req.queue_item.confirmed = False
    

    req.queue_item.data = bytes([0x13]) + byte_array + byte_array
    
    req.queue_item.dev_eui = cihaze_eui
    req.queue_item.f_port = 92

    resp = client.Enqueue(req, metadata=auth_token)

    return resp.id

if __name__ == '__main__':
    main()
    