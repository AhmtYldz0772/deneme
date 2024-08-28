from flask import Flask, jsonify

app = Flask(__name__)

@app.route('/start', methods=['GET'])
def start():
    # Burada bir şeyler yapılır, ardından bir yanıt döndürülmelidir
    return jsonify({'message': 'Başlatma başarılı!'})

if __name__ == '__main__':
    app.run(debug=True)

