import requests
import json

url = 'https://api.dify.ai/v1/workflows/run'
api_key = '你的API_KEY'
headers = {
    'Authorization': f'Bearer {api_key}',
    'Content-Type': 'application/json',
}
data = {
    "inputs": {"text": "查手册看一下告诉我文档的作者"},
    "response_mode": "blocking",
    "user": "abc-123"
}

response = requests.post(url, headers=headers, data=json.dumps(data))

print(response.text)
result = json.loads(response.text)
# 检测outputs是否有text字段
if "text" in result["data"]['outputs']:
    print(result["data"]['outputs']["text"].encode().decode('utf-8'))