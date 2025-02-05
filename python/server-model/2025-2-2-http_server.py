from http.server import HTTPServer, BaseHTTPRequestHandler
import json
import search_internet
import chat_local
from classfy_model import Classfy
import os
import socket

os.environ['KMP_DUPLICATE_LIB_OK']='TRUE'

# 192.168.188.165
data = {'result': ''}
def get_ip():
    # 获取本机所有 IP 地址
    hostname = socket.gethostname()
    ip_list = []
    # 获取IP地址信息
    addr_infos = socket.getaddrinfo(hostname, None)
    for addr in addr_infos:
        ip_list.append(addr[4][0])
    host = (ip_list[-1], 8888)
    return host

class http_chat_server:
    def __init__(self):
        self.agent_executor = search_internet.search_Moel()
        self.local_executor = chat_local.chat_furina()
        self.classfy = Classfy()
    
    def deal_with_data(self, data: str):
        d = json.loads(data.replace("'", "\""), strict=False)
        # print(d)
        print(d['messages'][0]['text'])

        if self.classfy.classfy(d['messages'][0]['text'])[0]['label'] == 'yes':
            print("searching...")
            ret = self.agent_executor.search_func(d['messages'][0]['text'])
        else:
            print("chatting...")
            ret = self.local_executor.chat_with_ollama(d['messages'][0]['text'])
        print(ret)
        return ret
chat_server = http_chat_server()

class Resquest(BaseHTTPRequestHandler):

    def __init__(self, request, client_address, server):
        super().__init__(request, client_address, server)
        


    def do_GET(self):
        self.send_response(200)
        self.send_header('Content-type', 'application/json')
        self.end_headers()
        self.wfile.write(json.dumps(data).encode())

    def do_POST(self):
        datas = self.rfile.read(int(self.headers['content-length']))
        datas = datas.decode()
        ret = chat_server.deal_with_data(datas)
        # datas = json.loads(datas)

        # print(datas)
        self.send_response(200)
        self.send_header('Content-type', 'application/json')
        self.end_headers()
        data['result'] = ret
        self.wfile.write(json.dumps(data).encode())
 
if __name__ == '__main__':
    host = get_ip()
    server = HTTPServer(host, Resquest)
    print("Starting server, listen at: %s:%s" % host)
    server.serve_forever()

