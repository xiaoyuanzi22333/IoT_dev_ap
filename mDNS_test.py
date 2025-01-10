import socket
from zeroconf import ServiceInfo, Zeroconf

# 定义服务信息，包含服务名称、类型、地址等
service_name = "MyHTTPService._http._tcp.local."
service_type = "_http._tcp.local."
host_name = "mydevice.local."
address = "192.168.100.72"  # 替换为实际的 IP 地址
service_port = 8000  # 服务的端口
service_properties = {"version": "1.0", "path": "/"}  # 可选的键值对

# 构建 ServiceInfo 对象
info = ServiceInfo(
    service_type,
    service_name,
    addresses=[socket.inet_aton(address)],  # 将字符串 IP 转为字节格式
    port=service_port,
    properties=service_properties,
    server=host_name,
)

# 发布服务
zeroconf = Zeroconf()
try:
    zeroconf.register_service(info)
    print(f"Service {service_name} is now registered!")
    input("Press Enter to exit...\n")
finally:
    zeroconf.unregister_service(info)
    zeroconf.close()