import time
import datetime
import grpc
import json
import base64
import psycopg2
from chirpstack_api import api
import struct
server = "localhost:8080"
#dev_eui = "4b0369740f7fdc16"
api_token = "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJhdWQiOiJjaGlycHN0YWNrIiwiaXNzIjoiY2hpcnBzdGFjayIsInN1YiI6IjdlN2M1MGJmLTM0NjAtNDNhYy1iZGM2LTM5MGYxMWQzM2ZkMSIsInR5cCI6ImtleSJ9.TwpPr00ajarw55QpH9Hyu0tKartNsJHwzIzu32r7vtQ"



class InternalServiceStub(object):
    def __init__(self, channel):
        self.StreamDeviceEvents = channel.unary_stream(
            '/api.InternalService/StreamDeviceEvents',
            request_serializer=api.StreamDeviceEventsRequest.SerializeToString,
            response_deserializer=api.LogItem.FromString,
        )


def cihaz_olaylarini_al(stub, CihazeUI, api_token):
    """Cihaz olaylarını alır ve işler."""
    istek = api.StreamDeviceEventsRequest(dev_eui=CihazeUI)
    metadata = [('authorization', f'Bearer {api_token}')]

    try:
        for cevap in stub.StreamDeviceEvents(istek, metadata=metadata):
            zaman_damgasi = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
            olay_zaman_damgasiyla = f"{zaman_damgasi}"

            cevap_dict = json.loads(cevap.body)
            cihaz_eui = cevap_dict.get("deviceInfo", {}).get("devEui", "")
            aga_gecidi_id = cevap_dict.get("rxInfo", [{}])[0].get("gatewayId", "")
            veri_degeri = json.loads(cevap.body).get("data", "")
            cozulmus_veri = base64.b64decode(veri_degeri)

            curr_index = 0
            if len(cozulmus_veri) > 0 and cozulmus_veri[curr_index] == 0x22:

                if len(cozulmus_veri) >= 9:
                    message_type = cozulmus_veri[0]
                    x_value = struct.unpack('f', cozulmus_veri[1:5])[0]
                    y_value = struct.unpack('f', cozulmus_veri[5:9])[0]
                    sensor_time = struct.unpack('f', cozulmus_veri[9:13])
                    print("x_threshold: ",x_value, "x_threshold: ",y_value,"toplam saniye: ", sensor_time )
                    return x_value, y_value,

    except grpc.RpcError as hata:
        print(f"Hata: {hata}")
        

def main(dev_eui):
    sunucu_adresi = 'localhost:8080'
    baglanti = grpc.insecure_channel(sunucu_adresi)
    #dev_eui = "4c3b784abc584274"
    stub = InternalServiceStub(baglanti)
    channel = grpc.insecure_channel(server)
    client = api.DeviceServiceStub(channel)


    auth_token = [("authorization", "Bearer %s" % api_token)]

    req = api.EnqueueDeviceQueueItemRequest()
    req.queue_item.confirmed = False
    req.queue_item.data = bytes([0x19]) 
    req.queue_item.dev_eui = dev_eui
    req.queue_item.f_port = 92

    resp = client.Enqueue(req, metadata=auth_token)


    threshold_degeri = cihaz_olaylarini_al(stub, dev_eui, api_token)
    return threshold_degeri

if __name__ == '__main__':
    main()