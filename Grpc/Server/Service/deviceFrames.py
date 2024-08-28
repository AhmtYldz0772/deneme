"""
        import grpc
        import json
        import base64
        from Crypto.Cipher import AES
        import binascii
        from chirpstack_api import api

        class InternalServiceStub(object):

            def __init__(self, channel):
               Constructor.

                Args:
                    channel: A grpc.Channel.
                
                self.StreamDeviceFrames = channel.unary_stream(
                    '/api.InternalService/StreamDeviceFrames',
                    request_serializer=api.StreamDeviceFramesRequest.SerializeToString,
                    response_deserializer=api.LogItem.FromString,
                )
                
        def get_frm_payload_from_device_frame(device_frame, key):
            body = device_frame.body
            body_dict = json.loads(body)
            encoded_frm_payload = body_dict.get("phy_payload", {}).get("payload", {}).get("frm_payload")
            if encoded_frm_payload is None:
                print("frm_payload not found in the response.")
                return None
            
            decrypted_frm_payload = decode_base64_and_decrypt(encoded_frm_payload, key)
            return decrypted_frm_payload


        def decode_base64_and_decrypt(encoded_data, key):
            try:

                encoded_data = encoded_data.replace("-", "+").replace("_", "/")
                decoded_data = base64.b64decode(encoded_data + '=' * (4 - len(encoded_data) % 4))
            except binascii.Error as e:
                print(f"Error decoding base64: {e}")
                return None
            cipher = AES.new(key, AES.MODE_ECB)
            padded_data = _pad_data(decoded_data)
            decrypted_data = cipher.decrypt(padded_data)

            return decrypted_data


        def _pad_data(data):
            block_size = 16
            pad_size = block_size - (len(data) % block_size)
            return data + bytes([pad_size] * pad_size)


        def decode_hex_to_bytes(hex_data):

            return binascii.unhexlify(hex_data)


        def get_device_frames(stub, dev_eui, api_token, key):
            request = api.StreamDeviceFramesRequest(dev_eui=dev_eui)
            metadata = [('authorization', f'Bearer {api_token}')]

            try:
                for response in stub.StreamDeviceFrames(request, metadata=metadata):
                    print("Device Frame:", response)


                    frm_payload = get_frm_payload_from_device_frame(response, key)
                    print("frm_payload:", frm_payload)
            except grpc.RpcError as e:
                print(f"Error: {e}")


        def main():
            server_address = 'localhost:8080'
            channel = grpc.insecure_channel(server_address)
            stub = InternalServiceStub(channel)

            api_token = "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJhdWQiOiJjaGlycHN0YWNrIiwiaXNzIjoiY2hpcnBzdGFjayIsInN1YiI6IjdlN2M1MGJmLTM0NjAtNDNhYy1iZGM2LTM5MGYxMWQzM2ZkMSIsInR5cCI6ImtleSJ9.TwpPr00ajarw55QpH9Hyu0tKartNsJHwzIzu32r7vtQ"
            dev_eui = "4c3b784abc584274"
            key_hex = "669bdcd412063bb80329acb6fca707b7"
            key = decode_hex_to_bytes(key_hex)

            get_device_frames(stub, dev_eui, api_token, key)


        if __name__ == '__main__':
            main()
    
"""