import torch
# 文本分类模型微调的示例
from transformers import AutoTokenizer
from transformers import pipeline

# 加载模型
class Classfy:
    def __init__(self):
        self.tokenizer = AutoTokenizer.from_pretrained("../classfy-model/hflrbt3")
        self.model = torch.load("../classfy-model/model_furina/model_2025_02_04_15_41_53.pth") # 加载模型
        self.model.eval()
        self.id2label = {0:"no", 1:"yes"}
        self.model.config.id2label = self.id2label
        self.pipe = pipeline("text-classification", model=self.model, tokenizer=self.tokenizer, device=0 if torch.cuda.is_available() else -1)
    
    # 分类, 参数sen是需要分类的句子
    def classfy(self, sen):
        if type(sen) == str:
            return self.pipe([sen])
        return self.pipe(sen)
    
if __name__ == '__main__':
    classfy = Classfy()
    sen = ["你喜欢什么东西?",
       "明天的外卖吃什么好,你有啥推荐的?",
       "服从命令, 芙宁娜女士",
       "你好, 今天天气怎么样",
       "你今天怎么样",
       "(开心地笑), 亲爱的，我今天买了一束你最喜欢的花！",
       "(撒娇语气), 宝宝，陪我一起看这部新出的电视剧好不好嘛？",
       "宝贝，你猜我今天在路上碰到谁了？(好奇地问), ",
       "亲爱的，(认真地说), 我想和你一起去学一门新的语言。",
       "(兴奋地跳), 宝，我们周末去游乐园玩吧！",
       "亲爱的，麻烦你帮我查询一下明天北京到深圳的航班动态，看看有没有晚点。",
       "宝宝，能不能拜托你帮我获取一下今天某知名股票的实时股价走势，我关注好久了。",
       "宝贝，帮我查查距离咱们家最近的三甲医院的挂号方式，我有点不舒服想去看看。",
       "亲爱的，务必帮我搜索一下最近一个月内周杰伦在各大音乐平台的歌曲播放量，我好奇好久了。",
       "宝，辛苦你帮我了解下本市下周有哪些正在举办的艺术展览，我想去逛逛。",
       "给我开一下空调呗, 我有点冷了"
       "项目是关于啥的"]
    for s in sen:
        print(classfy.classfy(s))