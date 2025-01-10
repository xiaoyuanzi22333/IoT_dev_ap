from zeroconf import Zeroconf

# 解析服务
zeroconf = Zeroconf()
info = zeroconf.get_service_info("_http._tcp.local.", "MyHTTPService._http._tcp.local.")
if info:
    print(f"Service found: {info}")
    print(f"Address: {info.parsed_addresses()}")
else:
    print("Service not found")
zeroconf.close()