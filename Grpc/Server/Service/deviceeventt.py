    
import time

import grpc
import json
import base64
from chirpstack_api import api


class InternalServiceStub(object):
    def __init__(self, channel):
        self.StreamDeviceEvents = channel.unary_stream(
            '/api.InternalService/StreamDeviceEvents',
            request_serializer=api.StreamDeviceEventsRequest.SerializeToString,
            response_deserializer=api.LogItem.FromString,
        )


def write_to_file(filename, data):
    with open(filename, 'a', encoding='utf-8') as file:
        file.write(data + '\n')


def get_device_events(stub, dev_eui, api_token, output_filename):
    event_data = []
    request = api.StreamDeviceEventsRequest(dev_eui=dev_eui)
    metadata = [('authorization', f'Bearer {api_token}')]
    try:
        for response in stub.StreamDeviceEvents(request, metadata=metadata):
            print("Device Event:", response)
            write_to_file(output_filename, f"Device Event: {response}")
            event_data.append(response)

            body_dict = json.loads(response.body)
            print("Body Dictionary:", body_dict)
            write_to_file(output_filename, f"Body Dictionary: {body_dict}")
            event_data.append(response.body)

            data_value = body_dict.get("data", "")
            print("Data Value:", data_value)
            write_to_file(output_filename, f"Data Value: {data_value}")
            event_data.append(data_value)

            decoded_data = base64.b64decode(data_value)
            print("Decoded Data:", decoded_data)
            write_to_file(output_filename, f"Decoded Data: {decoded_data}")
            event_data.append(decoded_data)

            # print(event_data)

            try:

                original_string = decoded_data.decode('latin-1')
                print("Original String:", original_string)
                write_to_file(output_filename, f"Original String: {original_string}")
            except UnicodeDecodeError:
                print("Cannot decode using Latin-1 encoding.")
                write_to_file(output_filename, "Cannot decode using Latin-1 encoding.")

    except grpc.RpcError as e:
        print(f"Error: {e}")
        write_to_file(output_filename, f"Error: {e}")


def main():
    server_address = 'localhost:8080'
    channel = grpc.insecure_channel(server_address)
    stub = InternalServiceStub(channel)

    api_token = "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJhdWQiOiJjaGlycHN0YWNrIiwiaXNzIjoiY2hpcnBzdGFjayIsInN1YiI6IjY2NzIzODNmLTQ2MzgtNDQ5ZC04ZGJjLTdmM2RiZDczZTRjMiIsInR5cCI6ImtleSJ9.ttRS4djY8yiFYfrXOwuwJdzEd5BQhmwVxZKLcXGZoEQ"
    dev_eui = "4c3b784abc584274"

    output_filename = "../output.txt"

    get_device_events(stub, dev_eui, api_token, output_filename)


if __name__ == '__main__':
    main()
