from http.server import SimpleHTTPRequestHandler, HTTPServer
import os
import time
from urllib.parse import unquote, quote

UPLOAD_DIR = "uploads"  # 上传文件存储的目录

# 创建上传目录（如果不存在）
if not os.path.exists(UPLOAD_DIR):
    os.makedirs(UPLOAD_DIR)

class FileServerHandler(SimpleHTTPRequestHandler):
    def list_directory(self, path):
        """
        自定义目录列表页面，支持点击文件下载或进入子目录。
        """
        try:
            file_list = os.listdir(path)  # 获取目录内容
        except OSError:
            self.send_error(404, "No permission to list directory")
            return None

        # 构建 HTML 页面
        file_list.sort(key=lambda a: a.lower())
        response = f"<html><title>Directory listing for {self.path}</title>"
        response += f"<body><h2>Directory listing for {self.path}</h2><hr><ul>"
        
        # 添加返回上一级的链接
        if self.path != "/":
            parent_path = os.path.dirname(self.path.strip("/"))
            parent_path = "/" if parent_path == "" else f"/{parent_path}"
            response += f'<li><a href="{quote(parent_path)}">[Go Up]</a></li>'

        # 列出文件和文件夹
        for name in file_list:
            full_path = os.path.join(path, name)
            display_name = name + "/" if os.path.isdir(full_path) else name
            full_path = full_path[1:]
            response += f'<li><a href="{full_path}">{display_name}</a></li>'
        
        

        response += "</ul><hr></body></html>"

        encoded = response.encode("utf-8", "surrogateescape")
        self.send_response(200)
        self.send_header("Content-Type", "text/html; charset=utf-8")
        self.send_header("Content-Length", str(len(encoded)))
        self.end_headers()
        return encoded

    def do_GET(self):
        """
        处理 GET 请求，用于显示目录并支持文件下载。
        """
        # 获取实际路径
        path = unquote(self.path.strip("/"))
        full_path = os.path.join('.',path)

        if os.path.isdir(full_path):
            # 如果是目录，列出目录内容
            encoded = self.list_directory(full_path)
            if encoded:
                self.send_response(200)
                
                # !!!此处不添加header会引发毁灭性灾难
                # !!!此处不添加header会引发毁灭性灾难
                # !!!此处不添加header会引发毁灭性灾难
                self.send_header("Connection", "keep-alive")  # 添加 Keep-Alive
                self.end_headers()
                self.wfile.write(encoded)
        elif os.path.isfile(full_path):
            # 如果是文件，提供下载
            self.send_response(200)
            self.send_header("Content-Type", "application/octet-stream")
            self.send_header("Content-Disposition", f"attachment; filename={os.path.basename(full_path)}")
            self.send_header("Connection", "keep-alive")  # 添加 Keep-Alive
            self.end_headers()
            with open(full_path, "rb") as f:
                self.wfile.write(f.read())
        else:
            # 如果路径不存在，返回 404
            self.send_error(404, "File or directory not found")



    def do_POST(self):
        """
        处理文件上传请求，保存文件到指定目录。
        """
        # print(self.headers['Content-Length'])
        #  print(self.headers['Content-Type'])
        
        content_length = int(self.headers['Content-Length'])
        content_type = self.headers['Content-Type']

        # 检查是否是 multipart/form-data 类型
        if not content_type.startswith("multipart/form-data"):
            self.send_response(400)
            self.end_headers()
            self.wfile.write(b"Only multipart/form-data is supported.")
            return

        # 从 Content-Type 中提取 boundary
        boundary = content_type.split("boundary=")[1].encode("utf-8")
        body = self.rfile.read(content_length)

        # 分割请求体
        parts = body.split(b"--" + boundary)

        for part in parts:
            if b"Content-Disposition" in part:
                # 提取文件字段
                headers, file_data = part.split(b"\r\n\r\n", 1)
                headers = headers.decode("utf-8")

                for line in headers.split("\r\n"):
                    if "Content-Disposition" in line and "filename=" in line:
                        file_name = line.split("filename=")[1].strip().replace('"', '')
                        file_name = os.path.basename(file_name)  # 确保仅保留文件名

                        # 保存文件
                        file_path = os.path.join(UPLOAD_DIR, file_name)
                        with open(file_path, "wb") as f:
                            f.write(file_data.strip())

                        # 响应上传成功
                        self.send_response(200)
                        self.end_headers()
                        self.wfile.write(b"File uploaded successfully.")
                        return

        # 如果没有找到文件字段
        self.send_response(400)
        self.end_headers()
        self.wfile.write(b"No file found in the request.")



def run(server_class=HTTPServer, handler_class=FileServerHandler, port=8000):
    """
    启动 HTTP 文件服务器。
    """
    server_address = ('0.0.0.0', port)
    httpd = server_class(server_address, handler_class)
    httpd.timeout = 60  # 设置超时时间为 60 秒
    print(f"Starting server on port {port}...")
    print(f"Upload files to: http://localhost:{port}")
    print(f"Browse and download files: http://localhost:{port}/uploads")
    # httpd.serve_forever()  # 直接启动服务器
    try:
        httpd.serve_forever()  # 启动服务器
    except KeyboardInterrupt:
        print("\nShutting down server...")
        httpd.shutdown()  # 手动关闭服务器
        print("Server closed.")
    finally:
        httpd.server_close()  # 确保服务器资源被释放
        print("Server closed.") 


if __name__ == "__main__":
    run()