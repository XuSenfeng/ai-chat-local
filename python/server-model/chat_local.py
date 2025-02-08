import ollama
from chat_mysql import Chat_MySQL

class chat_furina:
    def __init__(self, user_id, load=True, mysql : Chat_MySQL = None):
        self.history = []
        self.messages = []
        self.model = "lfruina"
        self.last_time = 0
        self.mysql = mysql
        self.user_id = user_id

        print("load: ", load)
        if load and self.mysql.is_exist(self.user_id):
            self.load_history()
        elif not self.mysql.is_exist(self.user_id):
            print("new user create table")
            self.mysql.create_table(self.user_id)

    # 加载历史记录
    def load_history(self):
        result = self.mysql.select(self.user_id)
        for chat in result:
            # ((1, 'hello world2', 'user'), (2, 'hello world2', 'user'), (3, 'hello world2', 'user'))
            self.messages.append({"role": chat[2], "content": chat[1]})

    # 带长下文的对话
    def chat_with_ollama(self, user_input):
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
                
        # 调用ollama模型进行对话
        output = ollama.chat(
            model=self.model,
            messages=self.messages
            )
        # print('模型回复:', output['message']['content'])
        self.history[-1][1] = output['message']['content']
        return output['message']['content']
    
    def __del__(self):
        print("chat_furina stop id", self.user_id)
        for idx, (user_msg, model_msg) in enumerate(self.history):
            if user_msg and model_msg:
                self.mysql.insert(self.user_id, 'user', user_msg)
                self.mysql.insert(self.user_id, 'assistant', model_msg)

if __name__ == "__main__":

    furina = chat_furina(114514)
    print(furina.chat_with_ollama("我的名字是什么?"))
