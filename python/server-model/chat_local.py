import ollama

class chat_furina:
    def __init__(self):
        self.history = []
        self.messages = []
        self.model = "lfruina"

    # 带长下文的对话
    def chat_with_ollama(self, user_input):
        # while True:
            # user_input = input("\nUser: ")
        # if user_input.lower() in ["exit", "quit", "stop", "baibai", "拜拜"]:
        #     return "退出对话"
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
        output = ollama.chat(
            model=self.model,
            messages=self.messages
            )
        # print('模型回复:', output['message']['content'])
        self.history[-1][1] = output['message']['content']
        return output['message']['content']

if __name__ == "__main__":
    furina = chat_furina()
    print(furina.chat_with_ollama("你好我叫张振辉"))
    print(furina.chat_with_ollama("我的名字是什么?"))
    print(furina.chat_with_ollama("过来帮我解决一下需求"))
