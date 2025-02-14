
# 用于搜索网络的模块
from langchain_openai import OpenAIEmbeddings
from langchain_community.tools.tavily_search import TavilySearchResults
from langchain_openai import ChatOpenAI
from langchain_community.document_loaders import WebBaseLoader, PyPDFLoader
from langchain_community.vectorstores import FAISS
from langchain_text_splitters import RecursiveCharacterTextSplitter
from langchain import hub
import os
file_path="../../README.pdf"
os.environ["OPENAI_BASE_URL"] = "https://api.chatanywhere.tech"
os.environ["LANGSMITH_TRACING"]='true'
os.environ["LANGSMITH_ENDPOINT"]="https://api.smith.langchain.com"
os.environ["LANGSMITH_PROJECT"]="test"

class search_Moel:
    # 初始化文件处理工具
    def file_dealing_init(self):
        loader = PyPDFLoader(file_path=file_path)
        docs = loader.load()
        documents = RecursiveCharacterTextSplitter(chunk_size=1000, chunk_overlap=200).split_documents(docs)
        vector = FAISS.from_documents(documents, OpenAIEmbeddings())
        file_retriever = vector.as_retriever()
        # 测试检索结果
        # print(retriever.get_relevant_documents("这是个啥项目")[1])
        
        from langchain.tools.retriever import create_retriever_tool
        # 创建一个工具来检索文档
        retriever_tool = create_retriever_tool(
            file_retriever,
            "project_handbook_search",
            "搜索有关项目手册的信息。对于项目手册的任何问题，您必须使用此工具！",
        )
        return retriever_tool
    
    # 搜索iphone价格的工具
    def iphon_price_search(self):
        # 加载HTML内容为一个文档对象
        loader = WebBaseLoader("https://www.ithome.com/0/718/713.htm")
        docs = loader.load()
        # 分割文档
        documents = RecursiveCharacterTextSplitter(chunk_size=1000, chunk_overlap=200).split_documents(docs)
        # 向量化
        vector = FAISS.from_documents(documents, OpenAIEmbeddings())
        # 创建检索器
        retriever = vector.as_retriever()
        from langchain.tools.retriever import create_retriever_tool
        # 创建一个工具来检索文档
        retriever_tool = create_retriever_tool(
            retriever,
            "iPhone_price_search",
            "搜索有关 iPhone 15 的价格信息。对于iPhone 15的任何问题，您必须使用此工具！",
        )
        return retriever_tool
    

    def __init__(self):
        # 初始化大模型   工具1
        llm = ChatOpenAI(model="gpt-3.5-turbo-1106", temperature=0)
        search = TavilySearchResults(max_results=3)


        # 创建将在下游使用的工具列表
        tools = [search, self.iphon_price_search(), self.file_dealing_init()]

        # 获取要使用的提示
        prompt = hub.pull("hwchase17/openai-functions-agent")

        # 使用OpenAI functions代理
        from langchain.agents import create_openai_functions_agent

        # 构建OpenAI函数代理：使用 LLM、提示模板和工具来初始化代理
        agent = create_openai_functions_agent(llm, tools, prompt)

        from langchain.agents import AgentExecutor
        # 将代理与AgentExecutor工具结合起来
        self.agent_executor = AgentExecutor(agent=agent, tools=tools, verbose=True)

    def search_func(self, query):
        try:
            data = self.agent_executor.invoke({"input": query})
        except Exception as e:
            print("error: ", e)
            return "出错, 免费的API的token有数量上限, 想使用请充值"
        print("search: \n\n", data, "\n\n")
        return data['output']



if __name__ == "__main__":
    agent = search_Moel()
    print(agent.search_func("查一下项目手册看看这个项目是谁做的"))
    print(agent.search_func("我想知道iphone 15的价格"))
    print(agent.search_func("我想知道最近的小孩用鞭炮炸豪车的新闻"))

