import numpy as np
from matplotlib import pyplot as plt
from torch import nn as nn
from torch.nn import functional as nf
import torch.nn.init as init
import torch

class NET(nn.Module):
    def __init__(self,n_in,n_hidden,n_out):
        super(NET,self).__init__() #调用父类的init函数，定义为父类的成员
        self.n_hidden = nn.Linear(n_in,n_hidden)
        self.n_out=nn.Linear(n_hidden,n_out)
        for m in self.modules():
            if isinstance(m,nn.Linear):
                init.normal_(m.weight.data)
                
    def forward(self,x_layer):
        x_layer=torch.relu(self.n_hidden(x_layer))
        x_layer=self.n_out(x_layer)
        return x_layer


class MLP():
    def __init__(self,n_in,n_out,n_hidden,learning_rate,):
           self.net=NET(n_in,n_out,n_hidden)
           self.optimizer=torch.optim.Adam(self.net.parameters(),lr=learning_rate)
           self.loss_func=torch.nn.CrossEntropyLoss()

    def train(self,x,label):
        self.net.train()#训练模式开启
        x=torch.from_numpy(x)
        x=x.float()
        label=torch.from_numpy(label)
        label=label.float()
        out = self.net(x)
        loss=self.loss_func(out,label)
        self.optimizer.zero_grad()
        loss.backward()
        self.optimizer.step()

    def loss(self,x,label):
        self.net.eval()
        x=torch.from_numpy(x)
        label=torch.from_numpy(label)
        x=x.float()
        label=label.float()
        out = self.net(x)
        return self.loss_func(out,label)

X = np.array([
    [0, 0],
    [0, 1],
    [1, 0],
    [1, 1]
])
Y = np.array([
    [0, 1],
    [1, 0],
    [1, 0],
    [0, 1]
])

np.random.seed(1007)
EPOCH = 1000
N = X.shape[0]

mlp = MLP(2, 4, 2,0.1)

loss = np.zeros(EPOCH)
for epoch in range(EPOCH):
    for i in range(N):
        mlp.train(X[i], Y[i])
        
    for i in range(N):
        loss[epoch] += mlp.loss(X[i], Y[i])
        
    loss[epoch] /= N
    
plt.figure()
ix = np.arange(EPOCH)
plt.plot(ix, loss)
#plt.show()
plt.savefig("MLP.png")