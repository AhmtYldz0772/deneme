from flask import jsonify, make_response
from Service import gatewayList, deviceRequest, SET_THRESHOLD, SET_FREQUENCY, TimeEvent, diveceEvent, GET_THRESHOLD, SET_KALIBRASYON,SEND_RESET, sensor_mesagge
import main
from flask import jsonify
import datetime
from datetime import datetime
import psycopg2

from flask import Flask, request
from flask_cors import CORS, cross_origin

app = Flask(__name__)
CORS(app, resources={r"/*": {"origins": "http://localhost:4200"}})

@app.route('/', methods=['GET'])
def start():
    main.main()
    
#########################################################  tabloEvent
@app.route("/tablo", methods=["POST","GET"])
def tabloEvent():
    try:
        received_data = request.get_json()
        print(received_data)
        CihazeUI = str(received_data.get('CihazeUI'))
        
        diveceEvent.main(CihazeUI)
        deviceRequest.main(CihazeUI)
        
        
    except TypeError:
        return "Geçersiz veri formatı: 'CihazeUI' anahtarı uygun bir değer içermiyor", 400


#########################################################  tablo filitreleme
@app.route('/tablo_filitre', methods=['POST', 'GET'])
def filitre_tablo():
    param_degeri = request.json

    try:
        connection = psycopg2.connect(
            dbname="shm",
            user="postgres",
            password="123",
            host="localhost"
        )
        cursor = connection.cursor()

        select_query = """
            SELECT id, cihaz_eUI, tip, altip, zaman, 
                peak1, peak2, freq 
            FROM veriler
            WHERE 1=1
        """


        params = []

        for key, value in param_degeri.items():
            if value:
                select_query += f" AND {key} = %s"
                params.append(value)
        

        select_query += " ORDER BY id DESC"

        cursor.execute(select_query, tuple(params))

        columns = ('id', 'cihaz_eUI', 'tip', 'altip', 'zaman', 'peak1', 'peak2', 'freq')
        veriler = [dict(zip(columns, row)) for row in cursor.fetchall()]

        cursor.close()
        connection.close()

        return jsonify(veriler)
        
    except psycopg2.Error as e:
        print(f"Veritabanı hatası: {e}")
        return "Veritabanı hatası", 500
    
#########################################################  tablo değerleri
@app.route('/tabloEvent', methods=['GET'])
def events():

    try:
        connection = psycopg2.connect(
            dbname="shm",
            user="postgres",
            password="123",
            host="localhost"
        )
        cursor = connection.cursor()
        select_query = """
            SELECT id, cihaz_eUI, tip, altip, zaman, peak1, peak2,freq FROM veriler
            
            ORDER BY id DESC
            
        """
        cursor.execute(select_query)
        rows = cursor.fetchall()

        table_data = []
        for row in rows:
            id, cihaz_eUI,tip, altip, zaman, peak1, peak2,freq = row
            table_data.append({
                "id": id,
                "cihaz_eUI": cihaz_eUI,
                "tip": tip,
                "altip": altip,
                "zaman": zaman.strftime('%Y-%m-%d %H:%M:%S'),
                "peak1": peak1,
                "peak2": peak2,
                "freq": float(freq),  
            })

        cursor.close()
        connection.close()

        return jsonify(table_data)

    except psycopg2.Error as e:
        print(f"Veritabanı hatası: {e}")
        return "Veritabanı hatası", 500
    
 #########################################################  tabloFreq x ve y grafiği için
@app.route('/tabloFreq', methods=['GET'])
def freq():
    try:
        cihaz_eui = request.args.get('cihaz_eui')

        connection = psycopg2.connect(
            dbname="shm",
            user="postgres",
            password="123",
            host="localhost"
        )
        cursor = connection.cursor()

        select_query = """
            SELECT distinct on (peak1) peak1, id, freq, peak2,zaman
            FROM veriler 
            WHERE cihaz_eui = %s 
            AND (peak1 = 171 OR peak1 = 205) 
            and (altip = '1. Zirve')
            ORDER BY peak1,zaman DESC       
        """
        cursor.execute(select_query, (cihaz_eui,))
        rows = cursor.fetchall()


        
        result = []

        for row in rows:
            axis_type, id, freq, axis_threshold,zaman = row
            
          
            result.append({"id": id, "typeeksen": axis_type, "freq": freq, "peak": axis_threshold, "zaman":zaman})

        """
        result = [
            {"id": id, "typeeksen": 171, "freq": freq_171, "peak": max_peak_171},
            {"id": id, "typeeksen": 205, "freq": freq_205, "peak": max_peak_205}
        ]
        """

        cursor.close()
        connection.close()
        #print(result)
        return jsonify(result)

    except psycopg2.Error as e:
        #print(f"Veritabanı hatası: {e}")
        return "Veritabanı hatası", 500   

#########################################################  sendEvent
@app.route('/sendEvent', methods=['POST', 'GET'])
def getTimeEvent():
    try:
        received_data = request.get_json()
        cihaz_ui = str(received_data.get('CihazeUI'))

        if cihaz_ui is not None:
            deviceRequest.main(cihaz_ui)
            return TimeEvent.main(cihaz_ui)
        else:
            return "hata", 400

    except TypeError:
        return "hata2", 400
#########################################################  event gösterimi  
@app.route('/event', methods=['GET'])
def get_device_events():
    try:
        connection = psycopg2.connect(
            dbname="shm",
            user="postgres",
            password="123",
            host="localhost"
        )
        cursor = connection.cursor()

        select_query = """
            SELECT olay_zaman_damgasiyla, cihaz_eui, aga_gecidi_id, veri_degeri, typeeksen, peak, freq 
            FROM cihaz_olay
            ORDER BY olay_zaman_damgasiyla DESC
            LIMIT 200
        """
        cursor.execute(select_query)
        rows = cursor.fetchall()

        device_events = []
        for row in rows:
            olay_zaman_damgasiyla = row[0].strftime('%Y-%m-%d %H:%M:%S')

            event = {
                "olay_zaman_damgasiyla": olay_zaman_damgasiyla,
                "cihaz_eui": row[1],
                "aga_gecidi_id": row[2],
                "veri_degeri": row[3],
                "TYPE": row[4],
                "PEAK degeri": row[5],
                "freq": row[6]
            }
            device_events.append(event)

        cursor.close()
        connection.close()

        return jsonify(device_events)

    except psycopg2.Error as e:
        print(f"Veritabanı hatası: {e}")
        return "Veritabanı hatası", 500


#########################################################  sendkalibrasyon
@app.route('/sendkalibrasyon', methods=['POST', 'GET'])
def kalibrasyonSet():
    try:
        received_data = request.get_json()
        cihaz_ui = str(received_data.get('CihazeUI'))

        if cihaz_ui is not None:
            SET_KALIBRASYON.main(cihaz_ui)
            return jsonify({'message': 'Kalibrasyon başarıyla ayarlandı.'}), 200
        else:
            return jsonify({'error': "Geçersiz veri formatı: 'CihazeUI' anahtarı eksik veya uygun bir değer içermiyor"}), 400

    except TypeError:
        return jsonify({'error': "Geçersiz veri formatı: 'CihazeUI' anahtarı uygun bir değer içermiyor"}), 400
    
    
#########################################################  send_reset   reset buttonu
@app.route('/send_reset', methods=['POST', 'GET'])
def reset():
    try:
        received_data = request.get_json()
        cihaz_ui = str(received_data.get('CihazeUI'))

        if cihaz_ui is not None:
            SEND_RESET.main(cihaz_ui)
            return jsonify({'message': 'Kalibrasyon başarıyla ayarlandı.'}), 200
        else:
            return jsonify({'error': "Geçersiz veri formatı: 'CihazeUI' anahtarı eksik veya uygun bir değer içermiyor"}), 400

    except TypeError:
        return jsonify({'error': "Geçersiz veri formatı: 'CihazeUI' anahtarı uygun bir değer içermiyor"}), 400
    

    

    
#########################################################  get_device_data  x ve y değerlerini çizdirmek için kullanılır
@app.route('/get_device_data', methods=['GET'])
def get_device_data():
    try:
        cihaz_eui = request.args.get('cihaz_eui')

        connection = psycopg2.connect(
            dbname="shm",
            user="postgres",
            password="123",
            host="localhost"
        )
        cursor = connection.cursor()

        select_query = """
            SELECT DISTINCT ON (peak1) peak1, id, freq, peak2
            FROM veriler 
            WHERE cihaz_eui = %s 
            AND (peak1 = 171 OR peak1 = 205) 
            AND (altip = '1. Zirve')
            ORDER BY peak1, zaman DESC       
        """
        cursor.execute(select_query, (cihaz_eui,))
        rows = cursor.fetchall()

        result = []

        for row in rows:
            peak1, id, freq, peak2 = row
            result.append({"id": id, "typeeksen": peak1, "freq": freq, "peak": peak2})

        cursor.close()
        connection.close()
        return jsonify(result)

    except psycopg2.Error as e:
        print(f"Veritabanı hatası: {e}")
        return jsonify({"error": f"Veritabanı hatası: {str(e)}"}), 500
    
    
#########################################################   send_data zaman değeri gönderiliyor
@app.route('/send_data', methods=['POST','GET'])
def send_data():
    received_data = request.get_json()
    cihaze_ui = str(received_data.get('CihazeUI')) 
    print(cihaze_ui)
    return deviceRequest.main(cihaze_ui)


#########################################################   Set_Threshold
@app.route('/Set_Threshold', methods=['POST'])
def set_threshold():
    try:
        received_data = request.get_json()  
        threshold_value = float(received_data.get('value'))  
        print(threshold_value)
        cihaze_ui = str(received_data.get('CihazeUI')) 

        if threshold_value is not None and cihaze_ui is not None:
            
            result = SET_THRESHOLD.main(threshold_value,cihaze_ui)
            return result
        else:
            return "Geçersiz veri formatı: 'newThresholdValue' veya 'CihazeUI' anahtarları eksik veya uygun bir değer içermiyor", 400
    except Exception as e:
        return f"Hata: {str(e)}", 500


#########################################################  Get_Threshold
@app.route('/Get_Threshold', methods=['POST', 'GET'])
def Get_Threshold():
    try:
        if request.method == 'GET':
            cihaze_ui = request.args.get('CihazeUI')
        else:
            received_data = request.get_json()  
            cihaze_ui = str(received_data.get('CihazeUI')) 

        result = GET_THRESHOLD.main(cihaze_ui)  
        print(result)
        return jsonify(result)
        
    except (ValueError, TypeError):
        return "Geçersiz veri formatı: 'value' anahtarı uygun bir integer değer içermiyor", 400
    
    
#######################################################  Sensör verisinin 10 saniyede bir gösterimi Ahmet_yıldız
@app.route('/device_status', methods=['POST', 'GET'])
def device_status_database():
    try:
        connection = psycopg2.connect(
            dbname="shm",
            user="postgres",
            password="123",
            host="localhost"
        )
        cursor = connection.cursor()

        select_query = """
            SELECT cihaz_euİ, x_threshold, y_threshold, device_time, calib_status_x,calib_status_y
            FROM device_status
            ORDER BY id DESC
            LIMIT 1
        """
        cursor.execute(select_query)
        rows = cursor.fetchall()

        device_events = []
        for row in rows:
            event = {
                "cihaz_eui": row[0],
                "x_threshold": row[1],
                "y_threshold": row[2],
                "device_time": row[3].strftime("%Y-%m-%d %H:%M:%S"),
                "calib_status_x": row[4],
                "calib_status_y": row[5],
            }
            device_events.append(event)

        cursor.close()
        connection.close()

        return jsonify(device_events)

    except psycopg2.Error as e:
        print(f"Veritabanı hatası: {e}")
        return "Veritabanı hatası", 500
######################################################### device_status connetion 
@app.route('/send_status', methods=['POST'])
def send_status():
    try:
        received_data = request.get_json()
        if received_data and 'CihazeUI' in received_data:
            cihaze_ui = str(received_data['CihazeUI'])
            result = sensor_mesagge.main(cihaze_ui)
            return jsonify({'message': 'İşlem başarılı'}), 200
        else:
            return jsonify({'error': "Geçersiz veri formatı: 'CihazeUI' anahtarı eksik veya uygun bir değer içermiyor"}), 400
    except Exception as e:
        return jsonify({'error:"sen bul "': str(e)}), 500
#########################################################  Set_fREAQUESNCY
@app.route('/Set_fREAQUESNCY', methods=['POST'])
def Set_fREAQUESNCY():
    
        received_data = request.get_json()  
        frequency_value = received_data.get('value')
        cihaze_ui = str(received_data.get('CihazeUI'))  

        if frequency_value is not None and cihaze_ui is not None:
            frequency_value = int(frequency_value)
            result = SET_FREQUENCY.main(frequency_value,cihaze_ui)
            
            return result
        else:
            return "Geçersiz veri formatı: 'value' veya 'CihazeUI' anahtarı eksik veya uygun bir değer içermiyor", 400

        
#########################################################


#########################################################
@app.route('/list', methods=['GET'])
def get_gateway_list():
    keys = gatewayList.list_gateways()
    if keys is not None:
        return jsonify(keys)
    else:
        return jsonify({"error": "Failed to fetch keys or invalid response received"}), 500