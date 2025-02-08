import pymysql

class Chat_MySQL:
    def __init__(self):
        self.connection = pymysql.connect(
            host='localhost',
            port=3306, 
            user='root', 
            charset='utf8') # 连接数据库
        
        self.cursor = self.connection.cursor() # 创建游标
        self.cursor.execute('show databases')
        result = self.cursor.fetchall()
        # 检查数据库是否存在
        if ('chat_ai',) not in result:
            print('create database chat_ai')
            self.cursor.execute('create database chat_ai DEFAULT CHARSET utf8 COLLATE utf8_general_ci')
            self.connection.commit()
        # 进入使用的数据库
        self.cursor.execute('use chat_ai')


    def is_exist(self, user_id: int):
        self.cursor.execute('show tables')
        result = self.cursor.fetchall()
        if ('user_%d' % user_id,) in result:
            return True
        return False
    
    # 查看是否存在用户的聊天记录表, 如果不存在则可以创建
    def create_table(self, user_id: int):
        
        if not self.is_exist(user_id):
            # 创建表
            print('create table chat_history for user %d' % user_id)
            self.cursor.execute('create table user_%d ('
                                'chat_id int primary key auto_increment,'
                                'chat_history varchar(10240),'
                                'chat_user varchar(20))default charset=utf8;' % user_id)
            self.connection.commit()
        else:
            print('table chat_history for user %d already exist' % user_id)

    def close(self):
        self.cursor.close()
        self.connection.close()
        
    # 插入聊天记录
    def insert(self, user_id: int, user_name: str, chat_history: str):
        chat_history = chat_history.replace('"', "'")
        # self.cursor.execute('insert into user_%s (chat_history, chat_user) values (%s, %s)', [self.user_id, chat_history, user])
        self.cursor.execute(f'insert into user_{user_id} (chat_history, chat_user) values (%s, %s)', (chat_history, user_name))
        self.connection.commit()

    def select(self, user_id: int):
        self.cursor.execute('select * from user_%s', [user_id])
        result = self.cursor.fetchall()
        return result
    
    def get_history_len(self, user_id: int):
        self.cursor.execute('select count(chat_id) from user_%s', [user_id])
        result = self.cursor.fetchall()
        return result[0][0]
    
    def __del__(self):
        self.cursor.close()
        self.connection.close()



if __name__ == '__main__':
    try:
        mysql = Chat_MySQL()
        # mysql.insert(2, 'hello world')
        if not mysql.is_exist(4):
            print('table not exist')
            mysql.create_table(4)
        mysql.insert(4, 'user', 'hello world2')
        # mysql.insert('hello world3')
        # print(mysql.select())
        print(mysql.get_history_len(4))
        del mysql
    except Exception as e:
        print(e)

