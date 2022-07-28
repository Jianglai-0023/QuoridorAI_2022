import torch.nn.functional as F
import torch.nn as nn
import torch
import sys
import torch.nn.init as init

sys.path.append('..')

class NNetArchitecture(nn.Module):
    def __init__(self, game, args):
        super(NNetArchitecture, self).__init__()
        # game params
        self.feat_cnt = args.feat_cnt
        self.board_x, self.board_y = game.getBoardSize()
        self.action_size = game.getActionSize()
        """
                   TODO: Add anything you need

        """
        
        self.args = args
        self.hidden_num=3
        self.size_hidden=16
        self.size_in=self.board_x*self.board_y*self.feat_cnt

        self.layers1=nn.Sequential(
            nn.Conv2d(in_channels=3,out_channels=8,kernel_size=3,padding=1),
            #nn.Conv2d(3,8,3,padding=1),
            nn.BatchNorm2d(8),
            nn.ReLU()
        )
        self.layers2=nn.Sequential(
            nn.Conv2d(
                in_channels=8,
                out_channels=16,
                kernel_size=5,
                padding=2
            ),
            nn.BatchNorm2d(16),
            nn.ReLU()
        )
        self.layers3=nn.Sequential(
            nn.Conv2d(in_channels=16,out_channels=32,kernel_size=5,padding=2),
            nn.BatchNorm2d(32),
            nn.ReLU()
        )
        # self.layers4=nn.Sequential(
        #     nn.Conv2d(in_channels=32,out_channels=128,kernel_size=5,padding=2),
        #     nn.BatchNorm2d(128),
        #     nn.ReLU()
        # )
        self.output=nn.Linear(32*self.board_x*self.board_y,self.action_size)
        # print("qwq")
        for m in self.modules():
            if isinstance(m,nn.Linear):
                init.xavier_normal_(m.weight.data)
            elif isinstance(m,nn.Conv2d):
                init.xavier_normal_(m.weight.data)
                

    def forward(self, s):
        # batch_size x feat_cnt x board_x x board_y
        s = s.view(-1, self.feat_cnt, self.board_x, self.board_y)   #-1 means 数字由别的来推断；对s进行reshape，创造四维向量
        # print("shape")
        # print(s.shape[0])
        line=s.shape[0]
        for i in range(0,line):
            eva=torch.mean(s[i])
            var=torch.var(s[i])
            # print(sum1)
            s[i]=(s[i]-eva)/var

        # s=s.view(s.shape[0],self.board_x,self.board_y,-1)
        # for i in range(0,line):
        #     var=torch.var(s[i])
        #     s[i]=s[i]-torch.mean(s[i])
        #     s[i]=s[i]/var

        # print("@@@@")
        # print(s)
        """
            TODO: Design your neural network architecture
            Return a probability distribution of the next play (an array of length self.action_size) 
            and the evaluation of the current state.

            pi = ...
            v = ...
        """
        # print("here")
        s=self.layers1(s)
        s=self.layers2(s)
        s=self.layers3(s)
        # s=self.layers4(s)
        s=self.output(s.view(s.shape[0],-1))

        pi=s
        v=torch.tensor([1]).cuda()
        #print("$$$")
        #print(pi)
        # Think: What are the advantages of using log_softmax ?
        return F.log_softmax(pi, dim=1), torch.tanh(v)
