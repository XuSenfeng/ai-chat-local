import pymysql

class Chat_MySQL:
    def __init__(self, user_id):
        self.user_id = user_id
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
        self.cursor.execute('show tables') # 获取当前的表的列表
        result = self.cursor.fetchall()

        # 检查表是否存在这个用户的聊天记录表
        if ('chat_history%d' % self.user_id,) not in result:
            # 创建表
            print('create table chat_history for user %d' % self.user_id)
            self.cursor.execute('create table chat_history%d ('
                                'chat_id int primary key auto_increment,'
                                'chat_history varchar(10240),'
                                'chat_user varchar(20))default charset=utf8;' % self.user_id)
            self.connection.commit()

    def insert(self, user: str, chat_history: str):
        chat_history = chat_history.replace('"', "'")
        # self.cursor.execute('insert into chat_history%s (chat_history, chat_user) values (%s, %s)', [self.user_id, chat_history, user])
        self.cursor.execute(f'insert into chat_history{self.user_id} (chat_history, chat_user) values (%s, %s)', (chat_history, user))
        self.connection.commit()

    def select(self):
        self.cursor.execute('select * from chat_history%s', [self.user_id])
        result = self.cursor.fetchall()
        return result


    def __del__(self):
        self.cursor.close()
        self.connection.close()



if __name__ == '__main__':
    mysql = Chat_MySQL(2)
    # mysql.insert('hello world')
    # mysql.insert('user', 'hello world2')
    # mysql.insert('hello world3')
    print(mysql.select())

