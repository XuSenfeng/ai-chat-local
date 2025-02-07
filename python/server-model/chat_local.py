import ollama
import time
import chat_mysql

class chat_furina:
    def __init__(self, user_id, load=True):
        self.history = []
        self.messages = []
        self.model = "lfruina"
        self.last_time = 0
        self.mysql = chat_mysql.Chat_MySQL(user_id)
        print("load: ", load)
        if load:
            self.load_history()

    def load_history(self):
        print("load history")
        result = self.mysql.select()
        for chat in result:
            # ((1, 'hello world2', 'user'), (2, 'hello world2', 'user'), (3, 'hello world2', 'user'))
            self.messages.append({"role": chat[2], "content": chat[1]})

    # 带长下文的对话
    def chat_with_ollama(self, user_input):
        # while True:
            # user_input = input("\nUser: ")
        # if user_input.lower() in ["exit", "quit", "stop", "baibai", "拜拜"]:
        #     return "退出对话"
        last_time = time.localtime(time.time())
        self.history.append([user_input, ""])
        # 遍历历史记录，整理对话消息
        for idx, (user_msg, model_msg) in enumerate(self.history):
            if idx == len(self.history) - 1 and not model_msg:
                # 如果当前对话为最新的一条且未收到模型回复，则只添加用户消息
                self.messages.append({"role": "user", "content": user_msg})
                break
            if user_msg:
                # 如果是用户消息，则添加到消息列表中
                self.messages.append({"role": "user", "content": user_msg})
            if model_msg:
                # 如果是模型回复，则添加到消息列表中
                self.messages.append({"role": "assistant", "content": model_msg})
        print(self.messages)
        output = ollama.chat(
            model=self.model,
            messages=self.messages
            )
        # print('模型回复:', output['message']['content'])
        self.history[-1][1] = output['message']['content']
        return output['message']['content']
    
    def __del__(self):
        for idx, (user_msg, model_msg) in enumerate(self.history):
            if user_msg and model_msg:
                self.mysql.insert('user', user_msg)
                self.mysql.insert('assistant', model_msg)
        del self.mysql

if __name__ == "__main__":

    furina = chat_furina(114514)
    # print(furina.chat_with_ollama("你好我叫张振辉"))
    print(furina.chat_with_ollama("我的名字是什么?"))
    # print(furina.chat_with_ollama("过来帮我解决一下需求"))
