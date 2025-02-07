# Description: 本地聊天管理
import chat_local
import threading
import time

defaule_living_time = 300

# 本地聊天管理, 在一定时间内没有聊天则释放资源
class ChatLocalManage:
    def __init__(self):
        self.chat_exectutor_runing = {}
        self.timer = {}

    def chat(self, user_id:int, text: str):
        print("begin chat")
        if user_id not in self.chat_exectutor_runing:
            print("new chat_executor")
            self.chat_exectutor_runing[user_id] = chat_local.chat_furina(user_id, True)
            self.timer[user_id] = threading.Timer(defaule_living_time, self.release_chat_executor, (user_id,))
            self.timer[user_id].start()
        else:
            print("reset timer")
            self.timer[user_id].cancel()
            self.timer[user_id] = threading.Timer(defaule_living_time, self.release_chat_executor, (user_id,))
            self.timer[user_id].start()
        return self.chat_exectutor_runing[user_id].chat_with_ollama(text)
    
    def release_chat_executor(self, user_id):
        del self.chat_exectutor_runing[user_id]
        del self.timer[user_id]
        print("chat_executor released %d " % user_id)


if __name__ == "__main__":
    chat_manage = ChatLocalManage()
    # print("time :" , time.time())
    # print(chat_manage.chat(114514, "你好"))
    # time.sleep(1)
    # print(chat_manage.chat(2, "你好"))
    # time.sleep(2)
    print(chat_manage.chat(12, "你还记得我的名字吗?"))
    # time.sleep(30)
    # print("time :", time.time())
