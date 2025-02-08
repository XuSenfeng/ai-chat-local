# Description: 本地聊天管理
import chat_local
import threading
import time
from chat_mysql import Chat_MySQL

defaule_living_time = 300

# 本地聊天管理, 在一定时间内没有聊天则释放资源
class ChatLocalManage:
    def __init__(self):
        self.lock = threading.Lock()
        self.locks = {} # 用户锁
        self.chat_exectutor_runing = {} # 用户对话
        self.timer = {} # 计时器
        self.mysql = Chat_MySQL()

    def chat(self, user_id:int, text: str):
        print("begin chat")
        self.lock.acquire() # 主锁加锁

        if user_id not in self.chat_exectutor_runing:
            # 用户的对话不存在
            self.chat_exectutor_runing[user_id] = chat_local.chat_furina(user_id, True, self.mysql) # 加载一个对话
            self.locks[user_id] = threading.Lock() # 获取用户锁
        else:
            self.timer[user_id].cancel()
        
        self.timer[user_id] = threading.Timer(defaule_living_time, self.release_chat_executor, (user_id,))
        self.lock.release() # 主锁解锁

        self.locks[user_id].acquire() # 用户锁加锁
        ret = self.chat_exectutor_runing[user_id].chat_with_ollama(text)
        self.locks[user_id].release() # 用户锁解锁

        self.lock.acquire() # 主锁加锁
        self.timer[user_id].start() # 重置计时器
        self.lock.release() # 主锁解锁
        return ret
    
    def release_chat_executor(self, user_id):
        self.lock.acquire() # 主锁加锁
        del self.chat_exectutor_runing[user_id]
        del self.timer[user_id]
        del self.locks[user_id]
        self.lock.release()
        print("chat_executor released %d " % user_id)

    def get_new_user_id(self):
        while True:
            temp_id = int(time.time() * 1000) % 1000000000
            self.lock.acquire() # 主锁加锁
            if not self.mysql.is_exist(temp_id):
                self.lock.release()
                return temp_id
            self.lock.release()
            
    def release_all_chat_executor(self):
        self.lock.acquire()
        user_ids = list(self.chat_exectutor_runing.keys())
        for user_id in user_ids:
            self.timer[user_id].cancel()
            self.release_chat_executor(user_id)
        self.lock.release()

if __name__ == "__main__":
    chat_manage = ChatLocalManage()
    print(chat_manage.chat(12, "你还记得我的名字吗?"))
    chat_manage.release_all_chat_executor()
