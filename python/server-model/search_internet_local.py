# 使用本地模型联网获取信息(使用ollama的chat接口, 实际未采用)
from langchain_community.tools.tavily_search import TavilySearchResults
import ollama
import os
import yfinance as yf
import API
os.environ["LANGSMITH_TRACING"]='true'
os.environ["LANGSMITH_ENDPOINT"]="https://api.smith.langchain.com"
os.environ["LANGSMITH_PROJECT"]="test"

# 官网的示例
"""
curl http://localhost:11434/api/chat -d '{
  "model": "llama3.2",
  "messages": [
    {
      "role": "user",
      "content": "What is the weather today in Paris?"
    }
  ],
  "stream": false,
  "tools": [
    {
      "type": "function",
      "function": {
        "name": "get_current_weather",
        "description": "Get the current weather for a location",
        "parameters": {
          "type": "object",
          "properties": {
            "location": {
              "type": "string",
              "description": "The location to get the weather for, e.g. San Francisco, CA"
            },
            "format": {
              "type": "string",
              "description": "The format to return the weather in, e.g. 'celsius' or 'fahrenheit'",
              "enum": ["celsius", "fahrenheit"]
            }
          },
          "required": ["location", "format"]
        }
      }
    }
  ]
}'
"""

# 调用ollama的chat接口
def ask_ollama(query):
    response = ollama.chat(model='lfruina',messages=[{'role': 'user','content':query}],
                           # provide a tool to get the current price of a stock
                           tools=[{'type': 'function',
                                   'function': {
                                       'name': 'get_current_stock_price',
                                       'description': 'Get the current price for a stock',
                                       'parameters': {
                                           'type': 'object',
                                           'properties': {
                                               'ticker_symbol': {
                                                   'type': 'string',
                                                   'description': 'The ticker symbol of the stock',
                                                    },
                                                },
                                            'required': ['ticker_symbol'],
                                            },
                                        },
                                    },
                                    {
                                        'type': 'function',
                                        'function': {
                                            'name': 'search_tool',
                                            'description': '从网络获取信息, 当需要获取实时信息时, 才使用此工具, 平时不使用',
                                            'parameters': {
                                                'type': 'object',
                                                'properties': {
                                                    'queation': {
                                                        'type': 'string',
                                                        'description': '你想要搜索的问题,用于回答用户想要知道的信息, 示例:客户 我想知道最近的新闻, 参数:queation=最近的新闻',
                                                            },
                                                        },
                                                'required': ['queation'],
                                            },
                                        },
                                    },
                            ],
                            options={"temperature":0.6})
    return response




# 获取股票的当前价格
def get_current_stock_price(ticker_symbol):
    # Get the stock data
    stock = yf.Ticker(ticker_symbol)
    # Get the current price
    current_price = stock.history(period='1d')['Close'].iloc[0]
    return current_price

# 联网获取搜索结果
def search_tool(queation):
    search_tool = TavilySearchResults()
    ret = search_tool.invoke(queation)
    print(ret)
    format_ret = "获取到的搜索结果为：%s\n, 问题是%s, 请给我详细的, 有条理的使用获取到的信息进行解答问题" % (ret, queation)
    ret = ollama.chat(model='lfruina',messages=[{'role': 'user','content':format_ret}])
    return ret

function_map = {'get_current_stock_price': 
                    get_current_stock_price,# Add more functions here as needed
                'search_tool': search_tool,
                }

# 调用model返回的使用函数
def call_function_safely(response, function_map):
    # Extract the function name and arguments from the response
    tool_call = response['message']['tool_calls'][0]
    function_name = tool_call['function']['name']
    arguments = tool_call['function']['arguments']
    # Look up the function in the function map
    function_to_call = function_map.get(function_name)
    if function_to_call:
        try:
            # Call the function with the arguments
            result = function_to_call(**arguments)
            print(f"The current is : {result}")
            return result
        except TypeError as e:
            print(f"Argument error: {e}")
        else:
            print(f"{function_name} is not a recognized function")

if __name__ == "__main__":
    # 中文的时候精度太低
    query = ["I want to know the current price of MSFT(Microsoft)",
          "I want to know some recently news"
        ]
    for q in query:
        response = ask_ollama(q)
        print(response)
        if response['message']['tool_calls']:
            call_function_safely(response, function_map)

