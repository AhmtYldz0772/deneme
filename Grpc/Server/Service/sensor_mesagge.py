import time
import datetime
import grpc
import json
import base64
import psycopg2
from chirpstack_api import api
import struct

server = "localhost:8080"
api_token = "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJhdWQiOiJjaGlycHN0YWNrIiwiaXNzIjoiY2hpcnBzdGFjayIsInN1YiI6IjdlN2M1MGJmLTM0NjAtNDNhYy1iZGM2LTM5MGYxMWQzM2ZkMSIsInR5cCI6ImtleSJ9.TwpPr00ajarw55QpH9Hyu0tKartNsJHwzIzu32r7vtQ"



def insert_to_database(cihaz_euİ,x_threshold, y_threshold, device_time,Calibration_X_Status,Calibration_Y_Status):
    try:
        connection = psycopg2.connect(
            dbname="shm",
            user="postgres",
            password="123",
            host="localhost"
        )
        cursor = connection.cursor()

        insert_query = """
            INSERT INTO device_status (cihaz_euİ, x_threshold,y_threshold, device_time,calib_status_x,calib_status_y) 
            VALUES (%s, %s, %s, %s,%s,%s)
        """
        data = (cihaz_euİ, x_threshold, y_threshold, device_time,Calibration_X_Status,Calibration_Y_Status)

        cursor.execute(insert_query, data)
        connection.commit()

        cursor.close()
        connection.close()
    except Exception as e:
        print("Veritabanına veri eklenirken bir hata oluştu:", e)
        
def insert_to_data(x_Values1, x_Values2, x_Values3, x_Values4, x_Values5, y_Values1, y_Values2, y_Values3, y_Values4, y_Values5,zaman_bilgisi):
    try:
        connection = psycopg2.connect(
            dbname="shm",
            user="postgres",
            password="123",
            host="localhost"
        )
        cursor = connection.cursor()

        insert_query = """
            INSERT INTO sensor_values (x_Values1, x_Values2, x_Values3, x_Values4, x_Values5, y_Values1, y_Values2, y_Values3, y_Values4, y_Values5,zaman_bilgisi) 
            VALUES (%s, %s, %s, %s,%s,%s, %s, %s, %s,%s,%s)
        """
        data = (x_Values1, x_Values2, x_Values3, x_Values4, x_Values5, y_Values1, y_Values2, y_Values3, y_Values4, y_Values5,zaman_bilgisi)

        cursor.execute(insert_query, data)
        connection.commit()

        cursor.close()
        connection.close()
    except Exception as e:
        print("Veritabanına veri eklenirken bir hata oluştu:", e)
    
def get_sensor_time(sensor_time):
    saat = (sensor_time // 3600) % 24
    dakika = (sensor_time % 3600) // 60
    saniye = (sensor_time % 60)
    return saat, dakika, saniye

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
            tarih_bilgisi = zaman_damgasi.split()[0]  
            cevap_dict = json.loads(cevap.body)
            cihaz_eui = cevap_dict.get("deviceInfo", {}).get("devEui", "")
            aga_gecidi_id = cevap_dict.get("rxInfo", [{}])[0].get("gatewayId", "")
            veri_degeri = json.loads(cevap.body).get("data", "")
            time_cihrpstack = json.loads(cevap.body).get("time", "")
            #print(time_cihrpstack)
            cozulmus_veri = base64.b64decode(veri_degeri)

            curr_index = 0
            
            if  len(cozulmus_veri) > 0 and cozulmus_veri[curr_index] == 34:
                
                
                message_type1 = cozulmus_veri[0]
                print(message_type1)
                x_value = struct.unpack('f', cozulmus_veri[1:5])[0]
                y_value = struct.unpack('f', cozulmus_veri[5:9])[0]
                print(x_value, y_value)
                sensor_time = struct.unpack('I', cozulmus_veri[9:13])[0]
                sensor_time = get_sensor_time(sensor_time)
                device_exec_counter_second = struct.unpack('I', cozulmus_veri[13:17])[0]
                    
                    
                Calibration_X_Status =  cozulmus_veri[17]
                Calibration_Y_Status =  cozulmus_veri[18]
                sensor_time_str = f"{tarih_bilgisi} {sensor_time[0]:02d}:{sensor_time[1]:02d}:{sensor_time[2]:02d}"
                sensor_time = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
                insert_to_database(CihazeUI, x_value, y_value, sensor_time,Calibration_X_Status,Calibration_Y_Status)
                     
                    
                time_file.write(f"{datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')};{time_cihrpstack}; device_exec_counter_second: {device_exec_counter_second}\n")
                time_file.flush()
                #return x_value, y_value, sensor_time_str
            curr_index = 0   
            if  len(cozulmus_veri) > 0 and cozulmus_veri[curr_index] == 35:
                    message_type = cozulmus_veri[0]
                    print(message_type)
                    zaman_bilgisi = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
                    print(zaman_bilgisi)
                    
                    # X eksenindeki en büyük 5 değer
                    x_Values1 = struct.unpack('f', cozulmus_veri[1:5])[0]
                    x_Values2 = struct.unpack('f', cozulmus_veri[5:9])[0]
                    x_Values3 = struct.unpack('f', cozulmus_veri[9:13])[0]
                    x_Values4 = struct.unpack('f', cozulmus_veri[13:17])[0]
                    x_Values5 = struct.unpack('f', cozulmus_veri[17:21])[0]

                    # Y eksenindeki en büyük 5 değer
                    y_Values1 = struct.unpack('f', cozulmus_veri[21:25])[0]
                    y_Values2 = struct.unpack('f', cozulmus_veri[25:29])[0]
                    y_Values3 = struct.unpack('f', cozulmus_veri[29:33])[0]
                    y_Values4 = struct.unpack('f', cozulmus_veri[33:37])[0]
                    y_Values5 = struct.unpack('f', cozulmus_veri[37:41])[0]
                    print(x_Values1, x_Values2, x_Values3, x_Values4, x_Values5, y_Values1, y_Values2, y_Values3, y_Values4, y_Values5)
                    
                    insert_to_data(x_Values1, x_Values2, x_Values3, x_Values4, x_Values5, y_Values1, y_Values2, y_Values3, y_Values4, y_Values5,zaman_bilgisi)
                    
                    #time_file.write(f"{datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')};{time_cihrpstack}; device_exec_counter_second: {device_exec_counter_second}\n")
                    #time_file.flush()
                    #return x_value, y_value, sensor_time_str
            elif len(cozulmus_veri) >0:
                message_type = cozulmus_veri[0]
                x_value = struct.unpack('f', cozulmus_veri[1:5])[0]
                x_value1 = struct.unpack('f', cozulmus_veri[1:5])[0]
                x_value2 = struct.unpack('f', cozulmus_veri[1:5])[0]
                x_value3 = struct.unpack('f', cozulmus_veri[1:5])[0]
            


    except grpc.RpcError as hata:
        time_file.close()
        print(f"Hata: {hata}")


time_file = open("received_time.txt","w")

def main(dev_eui):
    #
    #print("bu kısımda info çalışıyor bilgine >>>>>>>>>>>>>>>")
    sunucu_adresi = 'localhost:8080'
    baglanti = grpc.insecure_channel(sunucu_adresi)
    stub = InternalServiceStub(baglanti)
    channel = grpc.insecure_channel(server)
    client = api.DeviceServiceStub(channel)
    #dev_eui = "4c3b784abc584274"
    auth_token = [("authorization", "Bearer %s" % api_token)]

    req = api.EnqueueDeviceQueueItemRequest()
    req.queue_item.confirmed = False
    req.queue_item.data = bytes([0x22])
    req.queue_item.dev_eui = dev_eui
    req.queue_item.f_port = 92

    resp = client.Enqueue(req, metadata=auth_token)

    sensor_status = cihaz_olaylarini_al(stub, dev_eui, api_token)
    return sensor_status


if __name__ == '__main__':
    main()
