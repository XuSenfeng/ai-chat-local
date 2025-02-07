from http.server import HTTPServer, BaseHTTPRequestHandler
import json
import search_internet
# import chat_local
import chat_local_manage
from classfy_model import Classfy
import socket
import os
import time
os.environ['KMP_DUPLICATE_LIB_OK']='TRUE'

# os.environ["TAVILY_API_KEY"] = 'xxx'
# os.environ["OPENAI_API_KEY"] = "xxx"
# os.environ["LANGSMITH_API_KEY"] = 'xxx'
# os.environ['USER_AGENT'] = "xxx"

# 使用的不是文档里面免费的API时候可能需要更改
os.environ["OPENAI_BASE_URL"] = "https://api.chatanywhere.tech"
os.environ["LANGSMITH_TRACING"]='true'
os.environ["LANGSMITH_ENDPOINT"]="https://api.smith.langchain.com"
os.environ["LANGSMITH_PROJECT"]="test"

# 192.168.188.165
data_ret = {'result': '', 'user_id': 12}
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
        self.chat_manage = chat_local_manage.ChatLocalManage()
        self.classfy = Classfy()
    
    def deal_with_data(self, data: str):
        d = json.loads(data.replace("'", "\""), strict=False)
        # print(d)
        print(d['messages'][0]['text'], d['messages'][0]['user_id'])
        if d['messages'][0]['user_id'] == -1:
            # 分配用户id
            user_id = int(time.time() * 1000000)
        else:
            user_id = d['messages'][0]['user_id']
            print(type(user_id))
        data_ret['user_id'] = int(user_id)
        
        if self.classfy.classfy(d['messages'][0]['text'])[0]['label'] == 'yes':
            print("searching...")
            ret = self.agent_executor.search_func(d['messages'][0]['text'])
        else:
            print("chatting...")
            ret = self.chat_manage.chat(user_id, d['messages'][0]['text'])
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
        self.wfile.write(json.dumps(data_ret).encode())

    def do_POST(self):
        datas = self.rfile.read(int(self.headers['content-length']))
        datas = datas.decode()
        ret = chat_server.deal_with_data(datas)
        # datas = json.loads(datas)

        # print(datas)
        self.send_response(200)
        self.send_header('Content-type', 'application/json')
        self.end_headers()
        data_ret['result'] = ret
        self.wfile.write(json.dumps(data_ret).encode())
 
if __name__ == '__main__':
    host = ('192.168.188.165', 8888)
    server = HTTPServer(host, Resquest)
    print("Starting server, listen at: %s:%s" % host)
    server.serve_forever()
    # print(time.strftime("%Y %m %d %H %M %S %f", time.localtime()))

