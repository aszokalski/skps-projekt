import socket

# Adres i port serwera UDP
server_address = ("127.0.0.1", 1234)

# Tworzenie gniazda UDP
client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

try:
    # Wysłanie wiadomości do serwera
    message = "Hello from client"
    client_socket.sendto(message.encode(), server_address)

    # Oczekiwanie na odpowiedź z serwera
    response, server_address = client_socket.recvfrom(1024)
    print("Received from server:", response.decode())
finally:
    # Zamknięcie gniazda
    client_socket.close()
