import grpc
from chirpstack_api import api


# Configuration.
server = "localhost:8080"  # API server address and port
api_token = "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJhdWQiOiJjaGlycHN0YWNrIiwiaXNzIjoiY2hpcnBzdGFjayIsInN1YiI6ImY1ZTI0MzE4LTQxMDgtNGUxNi04MGFiLWRhNjMyMjA2N2VhOSIsInR5cCI6ImtleSJ9.hSf05Xoz4D884eROSHMzySCTLuO2QeaB-IjG6jz4Eow"



def list_gateways():
    # Connect to the gRPC server without using TLS.
    channel = grpc.insecure_channel(server)

    # Create a GatewayService client.
    client = api.GatewayServiceStub(channel)

    # Define the API key metadata.
    auth_token = [("authorization", "Bearer %s" % api_token)]

    # Construct a request.
    req = api.ListGatewaysRequest()
    req.limit = 10

    try:
        # Call the List method.
        response = client.List(req, metadata=auth_token)
        for gateway in response.result:

            print("Gateway Name:", gateway.name)



    except grpc.RpcError as e:
        print(f"Error calling List: {e}")

    return gateway.name


if __name__ == "__main__":
    list_gateways()