# 文件处理的简单示例

from langchain_openai import ChatOpenAI,OpenAIEmbeddings
from langchain_community.document_loaders import PyPDFLoader,TextLoader
from langchain.text_splitter import RecursiveCharacterTextSplitter
from langchain_community.vectorstores.utils import filter_complex_metadata
from langchain_community.vectorstores import Chroma
from langchain_core.prompts import ChatPromptTemplate
from langchain.schema.runnable import RunnablePassthrough
from langchain.schema.output_parser import StrOutputParser
import API



def format_docs(docs):
    return "\n\n".join(doc.page_content for doc in docs)

class ChatDoc:

    def __init__(self):
        self.model = ChatOpenAI(temperature=0.1, model_name="gpt-3.5-turbo")
        self.text_splitter = RecursiveCharacterTextSplitter(chunk_size=1024, chunk_overlap=100)
        self.embeddings = OpenAIEmbeddings() # 使用OpenAI的Embeddings
        system_prompt = (
            '''
            You are an assistant for question-answering tasks. 
            Use the following pieces of retrieved context to answer the question. 
            If you don't know the answer, say that you don't know.
            context: {context}'''
        )

        self.prompt = ChatPromptTemplate.from_messages(
            [
                ("system", system_prompt),
                ("human", "{question}"),
            ]
        )

    def ingest(self, file_path: str):
        docs = PyPDFLoader(file_path=file_path).load() # 这里使用你的文件路径
        chunks = self.text_splitter.split_documents(docs) # 把文件分成小块
        chunks = filter_complex_metadata(chunks) # 过滤掉一些复杂的元数据
        vector_store = Chroma.from_documents(documents=chunks, embedding=self.embeddings) # 把文档转换成向量
        self.retriever = vector_store.as_retriever(search_type = "mmr", search_kwargs={
            "k": 6,
            "fetch_k": 20,
            "include_metadata": True
            },
        ) # 使用mmr算法进行检索, k=6表示返回6个结果, fetch_k=20表示检索20个结果, include_metadata=True表示返回元数据
        self.chain = ({"context": self.retriever| format_docs, "question": RunnablePassthrough()}
                      | self.prompt
                      | self.model
                      | StrOutputParser())


    def ask(self, query: str):
        if not self.chain:
            return "please add document first"
        response = self.chain.invoke(query)
        return response
    
    def clear(self):
        self.vector_store = None
        self.retriever = None
        self.chain = None


if __name__ == "__main__":
    API.init_API()
    chat = ChatDoc()
    chat.ingest("../../README.pdf") # 这里使用你的文件路径
    print(chat.ask("这是一个什么项目?"))
