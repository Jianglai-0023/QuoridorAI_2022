# From Quoridor to Gomoku——AI Battle

## About me
First time to learn about AI related knowledge, my understanding might be naive. I'll be very glad if you come up with any question.

## Quoridor Based on MCTS Algorithm
*Language Support: c++*
#### 1. Simulation
Rules:You need to control your cheese and ten boards, trying to make your cheese reach the bottom line first.
![GameScenen](https://raw.githubusercontent.com/Jianglai-0023/QuoridorAI_2022/main/images/%E6%88%AA%E5%B1%8F2022-07-17%2001.11.51%E7%9A%84%E5%89%AF%E6%9C%AC.png?token=GHSAT0AAAAAABTUJ5JEADPJJYXSIX4HK6BSYWTAUAQ)
<center>Game Secene</center>

![Legal Jump](https://raw.githubusercontent.com/Jianglai-0023/QuoridorAI_2022/main/images/%E6%88%AA%E5%B1%8F2022-07-17%2002.28.31.png?token=GHSAT0AAAAAABTUJ5JEEX5AKSTLRSM72WCOYWTAZLQ)
<center>Legal Jump</center>

###### Technique
* you can use two `unsigned long`to simulate board , which is 64(8x8) bits long. It might accelerated your code.
* use `class State`to record different state, which includes `ull board`,`pair(int,int)index`,`int rest_board`
#### 2. MCTS
###### Quick Look of This Algorithem
MCTS  includes four steps:
* Selection
* Expansion
* Simulation
* Backpropagation
  Befor you choose your next step based on current state, you need to repeated *N* times the steps below:
  ![MCTS](https://raw.githubusercontent.com/Jianglai-0023/QuoridorAI_2022/main/images/%E6%88%AA%E5%B1%8F2022-07-17%2002.29.32.png?token=GHSAT0AAAAAABTUJ5JF3SKI667OBIP2XSIKYWTAVTQ)
**Selection:** From the current state, you're going to choose a son with the highest UCT and move to it.Keep doing it until you reach a node whose son are not being met thoroghly(Unexpanded Node).
**Expansion:**  When you are on an unexpanded node, choose a son which is unmet  randomly .
**Simulation:** With the son you choose, randomly choose the next step until the game end.
**Backpropagation:** Return the win(true) or loose(false) to every nodes on the path from Selection to Expansion and add their visit times.
**Give your next action:** Choose the son with the highst time of visit as your action.
**About UCT:** $UCT=\frac{Q_i}{N_i}+ \sqrt{\frac{logN_{father}}{N_i}}$
###### Technique
* Use`class mct` as array to simulate the tree
* A state can be reached from different father, which is contradicted to tree's structure. So you can't store UCT in node. You need to store `Ni, Qi`instead and calculate UCT when everytime needed.
* In fact, random play-out is far from the real situation, so you can choose to estimate state after a few steps of simulation or estimate state immediately when you expand a node.
* If you choose to estimate a state, pay attention to keep your result from 0 to 1 and design a good valuation function.
#### 3.Shortages of my first version
* Distinguish same node from different fathers, didn't implement calculating UCT with different father's visit's time.
* Low speed: Simulation about 10,000 times every 2 second, 8 times lowers than others.
#### 4. My TimeLine
* week1.1-week1.2 understand the game, writing the simulation part
* week1.2-1.4 understanding the algorithem and implement it.
  * week1.5-2.3 fixed bugs
* week2.3-2.5 optimzing

  MCTS didn't chosen by many of us. With a lot of details, it's uneasy to make it out without dicussion. Because my understanding isn't that deep, I didn't fully understand the algorithem and ignore the shortage I have mentioned befour.
  All in all, thanks to my TAs and teammates' selfless help.