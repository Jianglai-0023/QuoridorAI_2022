#include "AIController.h"
#include <utility>
#include <vector>
#include<cstdlib>
#include<unordered_map>
#include<map>
extern int ai_side;
std::string ai_name = "random_swim";
const int CIRCLE = 1000;
const int SIMULATION=1000;
using ull=unsigned long;

const int dx[4]{0, 1, 0, -1};
const int dy[4]{1, 0, -1, 0};
struct poi{
    int x;int y;
};
class State {
public:
    std::pair<int, int> s0_index;
    std::pair<int, int> s1_index;
    bool b[17][17];
    int board_num=10;
    std::pair<int, std::pair<int, int> > action;
    bool operator==(const State &opt)const{
        for(int i = 0; i < 17; ++i){
            for(int j = 0; j < 17; ++j){
                if(b[i][j]!=opt.b[i][j])return false;
            }
        }
        if(s0_index!=opt.s0_index)return false;
        if(s1_index!=opt.s1_index)return false;
        if(board_num!=opt.board_num)return false;
        return true;
    }
    void operator=(const State &s){
        for(int i = 0; i < 17; ++i){
            for(int j = 0; j < 17; ++j){
                b[i][j]=s.b[i][j];
            }
        }
        s0_index=s.s0_index;
        s1_index=s.s1_index;
        board_num=s.board_num;
    }
    State(){
        s0_index= std::make_pair(16,8);
        s1_index=std::make_pair(0,8);
        for(int i = 0; i < 17; ++i){
            for(int j = 0; j < 17; ++j){
                b[i][j]=0;
            }
        }
    }
    bool add_board(std::pair<int, std::pair<int, int> > loc) {//保证这里放入的board一定合法
        int x = 2 * loc.second.first + 1;
        int y = 2 * loc.second.second + 1;
        if (loc.first == 1) {
            if ((!b[x][y] && !b[x+1][y] && !b[x-1][y])) {
                b[x][y] = b[x+1][y] = b[x-1][y] = 1;
                return true;
            } else return false;
        } else if (loc.first == 2) {
            if (!b[x][y] && !b[x][y-1] && !b[x][y+1]) {
                b[x][y] = b[x][y-1] = b[x][y+1] = 1;
                return true;
            } else return false;
        }
        else return false;
    }
} state;
struct Node {
    bool isexpanded=false;//被完全拓展
    int Q;
    int N;
    int UCT=0;
    bool ismet=false;//是否访问过
    int unexpand_num=0;
    State prefer_son;
    std::vector<State> son;
};
struct Hash{
   std::size_t operator ()(const State &t)const{
       std::size_t hash_result = ((233ull * (t.s0_index.first + 1) + (t.s0_index.second + 1))*233ull+t.s1_index.first+1)*233ull+t.s1_index.second+1;
       for (int i = 0; i < 17; ++i)
           for (int j = 0; j < 17; ++j)
               hash_result = (hash_result * 233ull + t.b[i][j] + 1);
           return hash_result;
   }
};
struct Equal{
    bool operator()(const State &a, const State &b) const {
        return a == b;
    }
};
std::unordered_map<State,Node,Hash> tree;
//init function is called once at the beginning
bool bfs(std::pair<int,std::pair<int,int>> board){
    int x = 2 * board.second.first + 1;
    int y = 2 * board.second.second + 1;
    int dist[17][17];
    bool map[17][17];
    memset(dist,-1,sizeof(dist));
    for(int i = 0; i < 17; ++i){
        for(int j = 0; j < 17; ++j){
            map[i][j]=state.b[i][j];
        }
    }
    if(board.first==1){//vertical
        map[x][y] = map[x+1][y] = map[x-1][y] = 1;
    }
    else{//horizon
        map[x][y] = map[x][y + 1] = map[x][y - 1] = 1;
    }
    poi q[300];
    int h=0,t=-1;
    poi p;
    p.x=state.s0_index.first;
    p.y=state.s0_index.second;
    q[++t]=p;
    dist[p.x][p.y]=1;
    map[p.x][p.y]=1;
    while(h<=t){
      poi p_=q[h++];
      map[p_.x][p_.y]=1;
      for(int i = 0; i < 4; ++i){
          int x_=p_.x+dx[i];
          int y_=p_.y+dy[i];
          if(x_<0||x_>17||y_<0||y_>17||map[x_][y_]||map[x_+dx[i]][y_+dy[i]])continue;//有板子或者走过了
          else{
              p.x=x_+dx[i];
              p.y=y_+dy[i];
              q[++t]=p;
              dist[p.x][p.y]=dist[p_.x][p_.y]+1;
          }
      }
    }
    bool flag;
        for(int i =0; i < 17; ++i){
            if(dist[0][i]!=-1){
                flag=true;
                break;
            }
            flag=false;
        }
    memset(dist,-1,sizeof(dist));
    for(int i = 0; i < 17; ++i){
        for(int j = 0; j < 17; ++j){
            map[i][j]=state.b[i][j];
        }
    }
    if(board.first==1){//vertical
        map[x][y] = map[x+1][y] = map[x-1][y] = 1;
    }
    else{//horizon
        map[x][y] = map[x][y + 1] = map[x][y - 1] = 1;
    }
    h=0,t=-1;
    p.x=state.s1_index.first;
    p.y=state.s1_index.second;
    q[++t]=p;
    dist[p.x][p.y]=1;
    while(h<=t){
        poi p_=q[h++];
        map[p_.x][p_.y]=1;
        for(int i = 0; i < 4; ++i){
            int x_=p_.x+dx[i];
            int y_=p_.y+dy[i];
            if(x_<0||x_>17||y_<0||y_>17||map[x_][y_]||map[p_.x][p_.y])continue;
            else{
                p.x=x_+dx[i];
                p.y=y_+dy[i];
                q[++t]=p;
                dist[p.x][p.y]=dist[p_.x][p_.y]+1;
            }
        }
    }
    bool flag2;
    for(int i = 0; i < 17; ++i){
        if(dist[16][i]!=-1){
            flag2=true;
            break;
        }
        flag=false;
    }
    if(!flag||!flag2)return false;
    return true;
}
void init() {
    for (int i = 0; i < 17; ++i) {
        for (int j = 0; j < 17; ++j) {
            state.b[i][j] = 0;
        }
    }
}

std::pair<int,int> map_node(std::pair<int,int> loc){
    return std::make_pair(loc.first*2,loc.second*2);
}

std::vector<State> next_step(const State &s){//zero 即为s0走
    std::vector<State> v;
    State S=s;
//    std::cerr<<state.s1_index.first << '@'<<state.s1_index.second;
      for(int i = 0; i < 4; i++){//for cheese
          int x,y;
              if(!ai_side){
                  x=s.s0_index.first+dx[i];
                  y=s.s0_index.second+dy[i];
                  if(x<0||x>16||y<0||y>16)continue;
                  if(s.b[x][y])continue;
                  x=x+dx[i];
                  y=y+dy[i];
                  if(std::make_pair(x,y)==s.s1_index){//是否重合
                      x=x+dx[i];
                      y=y+dy[i];
                      if(x<0||x>16||y<0||y>16)continue;
                      if(s.b[x][y])continue;
                      x=x+dx[i];
                      y=y+dy[i];
                  }
                  S.action.first=0;
                  S.action.second=std::make_pair(x/2,y/2);
                  S.s0_index=std::make_pair(x,y);//映射结果
              }
              else{
                  x=s.s1_index.first+dx[i];
                  y=s.s1_index.second+dy[i];
                  if(x<0||x>16||y<0||y>16||s.b[x][y])continue;
                  x=x+dx[i];
                  y=y+dy[i];
                  if(std::make_pair(x,y)==s.s0_index){//是否重合
                      x=x+dx[i];
                      y=y+dy[i];
                      if(x<0||x>16||y<0||y>16||s.b[x][y])continue;
                      x=x+dx[i];
                      y=y+dy[i];
                  }
                  S.action.first=0;
                  S.action.second=std::make_pair(x/2,y/2);
                  S.s1_index=std::make_pair(x,y);
              }
              v.push_back(S);
      }
      if(s.board_num){// for board
          /*策略：放在自己的身后；对方的身前；放在已有的板子周围
           *横竖板随机
           */
           if(ai_side){
               int x_=s.s0_index.first;
               int y_=s.s0_index.second;
               srand((unsigned)time(NULL));
               int seed=rand()%2;
               if(seed){//vertical
                    int x=x_-1;
                    int y=y_-1;
                    if(!s.b[x][y]&&!s.b[x-1][y]&&!s.b[x+1][y]&&bfs(std::make_pair(1,std::make_pair((x-1)>>1,(y-1)>>1)))){//满足放板子条件
                        S=s;
                        S.action=std::make_pair(1,std::make_pair((x-1)>>1,(y-1)>>1));
                        S.add_board(std::make_pair(1,std::make_pair((x-1)>>1,(y-1)>>1)));
                        --S.board_num;
                        v.push_back(S);
                    }
                    x=x_-1;
                    y=y_+1;
                   if(s.b[x][y]&&s.b[x-1][y]&&s.b[x+1][y]&&bfs(std::make_pair(1,std::make_pair((x-1)>>1,(y-1)>>1)))){
                       S=s;
                       S.action=std::make_pair(1,std::make_pair((x-1)>>1,(y-1)>>1));
                       S.add_board(std::make_pair(1,std::make_pair((x-1)>>1,(y-1)>>1)));
                       --S.board_num;
                       v.push_back(S);
                   }
               }
               else{//horizon
                    int x=x_-1;
                    int y=y_-1;
                    if(x>0&&x<16&&!s.b[x-1][y]&&!s.b[x][y]&&!s.b[x+1][y]&&bfs(std::make_pair(2,std::make_pair((x-1)>>1,(y-1)>>1)))){
                        S=s;
                        S.action=std::make_pair(2,std::make_pair((x-1)>>1,(y-1)>>1));
                        S.add_board(std::make_pair(2,std::make_pair((x-1)>>1,(y-1)>>1)));
                        --S.board_num;
                        v.push_back(S);
                    }
                    x=x_-1;
                    y=y_+1;
                    if(x>0&&x<16&&!s.b[x-1][y]&&!s.b[x][y]&&!s.b[x+1][y]&&bfs(std::make_pair(2,std::make_pair((x-1)>>1,(y-1)>>1)))){
                        S=s;
                        S.action=std::make_pair(2,std::make_pair((x-1)>>1,(y-1)>>1));
                        S.add_board(std::make_pair(2,std::make_pair((x-1)>>1,(y-1)>>1)));
                        --S.board_num;
                        v.push_back(S);
                    }
               }
           }
           else{
               int x_=s.s1_index.first;
               int y_=s.s1_index.second;
               srand((unsigned)time(NULL));
               int vertical=rand()%2;
               if(vertical){//vertical
                   int x=x_+1;
                   int y=y_-1;
                   if(!s.b[x-1][y]&&!s.b[x+1][y]&&!s.b[x][y]&&bfs(std::make_pair(1,std::make_pair((x-1)>>1,(y-1)>>1)))){//满足放板子条件
                       S=s;
                       S.action=std::make_pair(1,std::make_pair((x-1)>>1,(y-1)>>1));
                       S.add_board(std::make_pair(1,std::make_pair((x-1)>>1,(y-1)>>1)));
                       --S.board_num;
                       v.push_back(S);
                   }
                   x=x_+1;
                   y=y_+1;
                   if(!s.b[x-1][y]&&!s.b[x][y]&&!s.b[x+1][y]&&bfs(std::make_pair(1,std::make_pair((x-1)>>1,(y-1)>>1)))){
                       S=s;
                       S.action=std::make_pair(1,std::make_pair((x-1)>>1,(y-1)>>1));
                       S.add_board(std::make_pair(1,std::make_pair((x-1)>>1,(y-1)>>1)));
                       --S.board_num;
                       v.push_back(S);
                   }
               }
               else{//horizon
                   int x=x_+1;
                   int y=y_-1;
                   if(x>0&&x<16&&!s.b[x-1][y]&&!s.b[x][y]&&!s.b[x+1][y]&&bfs(std::make_pair(2,std::make_pair((x-1)>>1,(y-1)>>1)))){
                       S=s;
                       S.action=std::make_pair(2,std::make_pair((x-1)>>1,(y-1)>>1));
                       S.add_board(std::make_pair(2,std::make_pair((x-1)>>1,(y-1)>>1)));
                       --S.board_num;
                       v.push_back(S);
                   }
                   x=x_+1;
                   y=y_+1;
                   if(x>0&&x<16&&!s.b[x][y-1]&&!s.b[x][y]&&!s.b[x][y+1]&&bfs(std::make_pair(2,std::make_pair((x-1)>>1,(y-1)>>1)))){
                       S=s;
                       S.action=std::make_pair(2,std::make_pair((x-1)>>1,(y-1)>>1));
                       S.add_board(std::make_pair(2,std::make_pair((x-1)>>1,(y-1)>>1)));
                       --S.board_num;
                       v.push_back(S);
                   }
               }
               //put board next to other board
               if(vertical){//vertical
                   for(int i = 0; i < 17; ++i){
                       for(int j = 0; j < 17; ++j){
                           if(s.b[i][j]){
                               for(int t = 0; t < 4; ++t){
                                   int x=i+dx[i];
                                   int y=j+dy[i];
                                  if(y<=0||y>=16||x>=16||x<=0||s.b[x][y]||s.b[x+1][y]||s.b[x-1][y]||!bfs(std::make_pair(1,std::make_pair((x-1)>>1,(y-1)>>1))))continue;
                                  S=s;
                                  S.action=std::make_pair(1,std::make_pair((x-1)>>1,(y-1)>>1));
                                  --S.board_num;
                                  S.add_board(std::make_pair(1,std::make_pair((x-1)>>1,(y-1)>>1)));
                                  v.push_back(S);
                               }
                           }
                       }
                   }
               }
               else{
                   for(int i = 0; i < 17; ++i){
                       for(int j = 0; j < 17; ++j){
                           if(s.b[i][j]){
                               for(int t = 0; t < 4; ++t){
                                   int x=i+dx[i];
                                   int y=j+dy[i];
                                   if(y<=0||y>=16||x<=0||x>=16||s.b[x][y]||s.b[x+1][y]||s.b[x-1][y]||!bfs(std::make_pair(1,std::make_pair((x-1)>>1,(y-1)>>1))))continue;
                                   S=s;
                                   S.action=std::make_pair(2,std::make_pair((x-1)>>1,(y-1)>>1));
                                   --S.board_num;
                                   S.add_board(std::make_pair(2,std::make_pair((x-1)>>1,(y-1)>>1)));
                                   v.push_back(S);
                               }
                           }
                       }
                   }
               }
           }
      }
      return v;
}

std::pair<int, std::pair<int, int> > change_state(std::pair<int, std::pair<int, int> > loc){
    tree.erase(state);
    if(!loc.first){
        if(!ai_side){
            state.s0_index.first=loc.second.first*2;
            state.s0_index.second=loc.second.second*2;
        }
        else{
            state.s1_index.first=loc.second.first*2;
            state.s1_index.second=loc.second.second*2;
        }
    }
    else{
        state.add_board(loc);
    }
    return loc;
}
class Monte_Tree {
public:
    double cal_UCT(double Ni,double Qi,double N){
        return (Qi/Ni)+1.41*pow(log(N)/Ni,0.5);
    }
    std::pair<int,double> Selection(const State &s,int N) {//<win,UCTofson,son>
        int win;
        std::pair<int,double> ans;
        if(tree[s].unexpand_num){//未完全展开
            Node n=tree[s];
            for(auto i=n.son.begin(); i!=n.son.end(); ++i){//expand一次
                if(!tree[*i].ismet){//未被访问过的节点，进行simulation
                    tree[*i].ismet=true;
                    tree[*i].son= next_step(*i);
                    tree[*i].unexpand_num=tree[*i].son.size();
                    ans.first=win=Simulation(*i);
                    if(win==-1)break;
                    int n=++tree[*i].N;
                    int q=tree[*i].Q+=win;
                    ans.second=tree[*i].UCT= cal_UCT(n,q,N+1);
                    if(tree[tree[s].prefer_son].UCT<ans.second)tree[s].prefer_son=*i;
                    break;
                }
            }
            tree[s].unexpand_num--;
            return ans;
        }
        else{//已被完全拓展
            ans= Selection(tree[s].prefer_son,tree[s].N);
            if(ans.first!=-1){
                for(auto i=tree[s].son.begin();i!=tree[s].son.end();++i){//维护最大UCT
                    if(tree[*i].UCT>tree[tree[s].prefer_son].UCT)tree[s].prefer_son=*i;
                }//维护最大UCT
                int n=tree[s].N++;
                int q=tree[s].Q+=(1-ans.first);
                ans.second=tree[s].UCT= cal_UCT(n,q,N);
                ans.first=(1-ans.first);
                return ans;
            }
            else{
                    ans.first=-1;
                    return ans;
            }
        }
    }

    int Simulation(const State &s) {
        State s0=s;
        bool win;
        int i;
         for(i = 0; i < SIMULATION; ++i){//最多走1000次
             std::vector<State> v= next_step(s0);
             int size=v.size();
             srand((unsigned)time(NULL));
             int seed=rand()%size;
             s0=v[seed];
             if(ai_side){
                 if(s0.s0_index.first==0){
                     win=false;
                     break;
                 }
             }
             else {
                 if (s0.s1_index.first == 16) {
                     win = true;
                     break;
                 }
             }
         }
         if(i==SIMULATION)return -1;
         return win;
    }//win lose unable

    std::pair<int,std::pair<int,int>> Tree_search(){
       for(int i = 0; i < CIRCLE; ++i){
           Selection(state,0);
       }
        return change_state(tree[state].prefer_son.action);
    }
} monte_tree;
/* The following notation is based on player 0's perspective
 * Rows are labeled 0 through 8 from player 1's side to player 0's side
 * Columns are labeled 0 through 8 from player 0's left to right
 * A coordinate is recorded as the row followed by the column, for example, player 0's pawn starts on (8, 4)
 * A pawn move is recorded as the new coordinate occupied by the pawn
 * A fence placement is recorded as the coordinate of the square whose bottom-right corner fits the center of the wall
 * A typecode is defined as follows: 0: pawn move
 *                                   1: vertical fence placement
 *                                   2: parallel fence placement
 * An action is defined as (typecode, (row-coordinate, column-coordinate))
 * You need to analyze your opponent's action and return your own action
 * If the typecode of your opponent's action is '-1', it means that you are on the offensive.
 */

std::pair<int, std::pair<int, int> > action(std::pair<int, std::pair<int, int> > loc) {
    if(ai_side){
        if(!loc.first)state.s0_index=map_node(loc.second);
        else if(loc.first==1||loc.first==2){
          bool flag=  state.add_board(loc);
          if(!flag)std::cerr<<"boad false";
        }
    }
    else{
        if(!loc.first)state.s1_index=map_node(loc.second);
        else if(loc.first==1||loc.first==2){
            bool flag=state.add_board(loc);
            if(!flag)std::cerr<<"boad false";
        }
    }
    std::vector<State> v= next_step(state);
    int size=v.size();
    srand((unsigned)time(NULL));
    int seed=rand()%size;
    std::pair<int, std::pair<int, int> > loc0=v[seed].action;
    return change_state(loc0);
}
