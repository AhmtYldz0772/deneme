import time
import datetime
import grpc
import json
import base64
import psycopg2
from chirpstack_api import api
import struct


class InternalServiceStub(object):
    def __init__(self, channel):
        self.StreamDeviceEvents = channel.unary_stream(
            '/api.InternalService/StreamDeviceEvents',
            request_serializer=api.StreamDeviceEventsRequest.SerializeToString,
            response_deserializer=api.LogItem.FromString,
        )


def yaz_dosyaya(dosya_adi, veri):
    """Verileri dosya içine yazar."""
    with open(dosya_adi, 'a', encoding='utf-8') as dosya:
        dosya.write(veri + '\n')


def veritabanina_ekle(zaman, cihaz_eui, aga_gecidi_id, ham_veri, tip, threshold,curr_freq):
    connection = psycopg2.connect(
        dbname="shm",
        user="postgres",
        password="123",
        host="localhost"
    )
    cursor = connection.cursor()

    insert_query = """
        INSERT INTO cihaz_olay (olay_zaman_damgasiyla, cihaz_eui, aga_gecidi_id, veri_degeri, typeeksen, peak,freq)
        VALUES (%s, %s, %s, %s, %s, %s, %s)
    """
    data = (zaman, cihaz_eui, aga_gecidi_id, ham_veri, tip, threshold,curr_freq)
    
    #print("Write Peak To Database ",data)
    cursor.execute(insert_query, data)

    connection.commit()
    cursor.close()
    connection.close()


def cihaz_olaylarini_al(stub, CihazeUI, api_token, cikis_dosyasi):
    """Cihaz olaylarını alır ve işler."""
    istek = api.StreamDeviceEventsRequest(dev_eui=CihazeUI)
    metadata = [('authorization', f'Bearer {api_token}')]
    x_event_array = []
    y_event_array = []
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
            if len(cozulmus_veri) > 0 and cozulmus_veri[0] == 18:
                curr_index += 1
                msg_len = cozulmus_veri[curr_index]
                curr_index += 1

                start_index = curr_index
                for i in range(int(msg_len / 9)):
                    type_eksen = cozulmus_veri[curr_index]
                    curr_index += 1

                    peak_value = struct.unpack('f', cozulmus_veri[curr_index:curr_index + 4])[0]
                    curr_index += 4

                    frequency = struct.unpack('f', cozulmus_veri[curr_index:curr_index + 4])[0]
                    curr_index += 4
                    
                    if type_eksen == 171:
                        x_event_array.append({"peak": peak_value, "freq": frequency})
                    elif type_eksen == 205:
                        y_event_array.append({"peak": peak_value, "freq": frequency})
                
                    veritabanina_ekle(olay_zaman_damgasiyla, cihaz_eui, aga_gecidi_id, veri_degeri, type_eksen, peak_value, frequency)
                    
                # X axis sonuçlarını ekler
                for event in x_event_array:
                    veritabanina_ekle(olay_zaman_damgasiyla, cihaz_eui, aga_gecidi_id, veri_degeri, 171, event["peak"], event["freq"])
                    
                # Y axis sonuçlarını ekler
                for event in y_event_array:
                    veritabanina_ekle(olay_zaman_damgasiyla, cihaz_eui, aga_gecidi_id, veri_degeri, 205, event["peak"], event["freq"])

                x_event_array.clear()
                y_event_array.clear()

                yaz_dosyaya(cikis_dosyasi, f"{olay_zaman_damgasiyla},{cihaz_eui},{aga_gecidi_id},{veri_degeri},{type_eksen},{peak_value},{frequency}")
                continue
    except grpc.RpcError as hata:
        print(f"Hata: {hata}")

    return cikis_dosyasi


def main(cihaz_eui):
    sunucu_adresi = 'localhost:8080'
    baglanti = grpc.insecure_channel(sunucu_adresi)

    stub = InternalServiceStub(baglanti)
    api_token = "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJhdWQiOiJjaGlycHN0YWNrIiwiaXNzIjoiY2hpcnBzdGFjayIsInN1YiI6IjdlN2M1MGJmLTM0NjAtNDNhYy1iZGM2LTM5MGYxMWQzM2ZkMSIsInR5cCI6ImtleSJ9.TwpPr00ajarw55QpH9Hyu0tKartNsJHwzIzu32r7vtQ"
    #cihaz_eui = "4b0369740f7fdc16"
    cikis_dosyasi = "../output.txt"

    return cihaz_olaylarini_al(stub, cihaz_eui, api_token, cikis_dosyasi)


if __name__ == '__main__':
    main()