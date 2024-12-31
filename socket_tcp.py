import socket


def main():
    # 1.创建套接字socket
    tcp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # 2.连接服务器

    dest_addr = ('192.168.103.35',59753)
    tcp_socket.connect(dest_addr)

    # 接收服务器发送的数据
    recv_data = tcp_socket.recv(1024)
    print("接收到数据：")
    print(recv_data)
    if recv_data.decode("utf-8")=="123":
 

        send_data = "client received"
        tcp_socket.send(send_data.encode("utf-8")) 
    else:
        print('server not received')
        send_data= "client not received"
        tcp_socket.send(send_data.encode("utf-8")) 
    
  

        # 4. 关闭套接字socket
    tcp_socket.close()

if __name__ == "__main__":

        main()
		
