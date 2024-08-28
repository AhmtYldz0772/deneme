import grpc
import json
import struct
import base64
import psycopg2
import datetime
from chirpstack_api import api

class InternalServiceStub(object):
    def __init__(self, channel):
        self.StreamDeviceEvents = channel.unary_stream(
            '/api.InternalService/StreamDeviceEvents',
            request_serializer=api.StreamDeviceEventsRequest.SerializeToString,
            response_deserializer=api.LogItem.FromString,
        )

def veritabanina_ekle(cihaz_eui, tip, altip, zaman, peak1, peak2, freq=None):
    try:
        connection = psycopg2.connect(
            dbname="shm",
            user="postgres",
            password="123",
            host="localhost"
        )
        cursor = connection.cursor()


        if freq is None:
            freq = 0

        insert_query = """
            INSERT INTO veriler (cihaz_eui, tip, altip, zaman, peak1, peak2, freq)
            VALUES (%s, %s, %s, %s, %s, %s, %s)
        """
        data = (cihaz_eui, tip, altip, zaman, peak1, peak2, freq)
        cursor.execute(insert_query, data)

        connection.commit()
        cursor.close()
        connection.close()
    except Exception as e:
        print("Veritabanına veri eklenirken bir hata oluştu:", e)

def cihaz_olaylarini_dinle(stub, CihazeUI, api_token):

    request = api.StreamDeviceEventsRequest(dev_eui=CihazeUI)
    metadata = [('authorization', f'Bearer {api_token}')]
    x_event_array = []
    y_event_array = []

    try:
        for response in stub.StreamDeviceEvents(request, metadata=metadata):
            response_dict = json.loads(response.body)
            device_eui = response_dict.get("deviceInfo", {}).get("devEui", "")
            data_value = json.loads(response.body).get("data", "")
            decoded_data = base64.b64decode(data_value)

            index = 0
            
            if len(decoded_data) > 0 and decoded_data[0] ==  18:
                print("koşul doğru message type: ", decoded_data[0])
                zaman = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
                tip = "cihazOlayı"
                altip = "xDegeri, yDegeri"
                index += 1
                msg_len = decoded_data[index]
                index += 1
                count_masage = int(msg_len / 9)
                #print(">>> mesage count", count_masage)
                i = 0
                for i in range(int(msg_len / 9)):
                    axis_type = decoded_data[index]
                    index += 1

                    peak_value = struct.unpack('f', decoded_data[index:index + 4])[0]
                    index += 4

                    frequency = struct.unpack('f', decoded_data[index:index + 4])[0]
                    index += 4
                    #veritabanina_ekle(CihazeUI, tip, f"{i + 1}. Zirve", zaman, axis_type, peak_value,frequency)

                    if axis_type == 171:
                        x_event_array.append({"peak": peak_value, "freq": frequency})
                        
                    elif axis_type == 205:
                        y_event_array.append({"peak": peak_value, "freq": frequency})
                        

                # X axis sonuçarını ekle
                i = 0
                for i, event in enumerate(x_event_array):
                    #print(i+1,"zirve", 171, event["peak"], event["freq"])
                    veritabanina_ekle(CihazeUI, tip, f"{i + 1}. Zirve", zaman, 171, event["peak"],event["freq"])

                # Y axis sonuçlarını ekle
                i = 0
                for i, event in enumerate(y_event_array):
                    #print(i + 1, "zirve", 205, event["peak"], event["freq"])
                    veritabanina_ekle(CihazeUI, tip, f"{i + 1}. Zirve", zaman, 205, event["peak"], event["freq"])
                    
                x_event_array.clear()
                y_event_array.clear()
            """
            if len(decoded_data) > 0 and decoded_data[0] == 0x11:

                zaman = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
                tip = "periodik"
                altip = "xDegeri, yDegeri"

                index += 1
                msg_len = decoded_data[index]
                index += 1

                for i in range(msg_len):
                    axis_type = 171
                    peak_value = struct.unpack('f', decoded_data[index:index + 4])[0]
                    index += 4
                    frequency = struct.unpack('f', decoded_data[index:index + 4])[0]
                    index += 4

                    #print("0x11 171cihaz olayları:",CihazeUI, tip, altip, zaman, axis_type, peak_value, frequency)
                    veritabanina_ekle(CihazeUI, tip, altip, zaman, axis_type, peak_value, frequency)
                for i in range(msg_len):
                    axis_type = 205
                    peak_value = struct.unpack('f', decoded_data[index:index + 4])[0]
                    index += 4
                    frequency = struct.unpack('f', decoded_data[index:index + 4])[0]
                    index += 4

                    #print("0x11 205 cihaz olayları:",CihazeUI, tip, altip, zaman, axis_type, peak_value, frequency)
                    veritabanina_ekle(CihazeUI, tip, altip, zaman, axis_type, peak_value, frequency)
                """
               
            if len(decoded_data) > 0 and decoded_data[0] == 22:
                index += 1
                msg_len = decoded_data[index]
                index += 1
                if len(decoded_data) >= 9:
                    message_type = decoded_data[0]
                    x_value = struct.unpack('f', decoded_data[1:5])[0]
                    y_value = struct.unpack('f', decoded_data[5:9])[0]
                    zaman = struct.unpack("f", decoded_data[9:13])[0]
                    #print(x_value, y_value,zaman)
                
            


    except grpc.RpcError as hata:
        print(f"Hata: {hata}")

def main(cihaz_eui):
    sunucu_adresi = 'localhost:8080'
    baglanti = grpc.insecure_channel(sunucu_adresi)
    #cihaz_eui = "4c3b784abc584274"
    stub = InternalServiceStub(baglanti)
    api_token = "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJhdWQiOiJjaGlycHN0YWNrIiwiaXNzIjoiY2hpcnBzdGFjayIsInN1YiI6IjdlN2M1MGJmLTM0NjAtNDNhYy1iZGM2LTM5MGYxMWQzM2ZkMSIsInR5cCI6ImtleSJ9.TwpPr00ajarw55QpH9Hyu0tKartNsJHwzIzu32r7vtQ"
    cihaz_olaylarini_dinle(stub, cihaz_eui, api_token)

if __name__ == '__main__':
    main()