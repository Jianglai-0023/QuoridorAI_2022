#include "AIController.h"
#include <utility>
#include <vector>
#include<cstdlib>
#include<unordered_map>
extern int ai_side;
std::string ai_name = "random_swim";
const int CIRCLE = 1000;
struct node {
    bool isexpanded;//被完全拓展
    int Q;
    int N;
    int UFT;
//    std::vector<int> UFT;
};
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
class Monte_Tree {
public:

} monte_tree;
void init() {
    for (int i = 0; i < 17; ++i) {
        for (int j = 0; j < 17; ++j) {
            state.b[i][j] = 0;
        }
    }
    monte_tree.head = new node();
}

std::pair<int,int> map_node(std::pair<int,int> loc){
    return std::make_pair(loc.first*2,loc.second*2);
}

std::vector<State> next_step(const State &s,bool ai_side){//zero 即为s0走
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

void Selection() {

}

void Expansion() {

}

void Play_out() {

}

void Backpropation() {

}

std::pair<int, std::pair<int, int>> Monte_tree_search(State now_state) {
    for (int i = 0; i < CIRCLE; ++i) {
      Selection();
      Expansion();
      Play_out();
      Backpropation();
    }
}
void change_state(std::pair<int, std::pair<int, int> > loc){
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
}

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

    std::vector<State> v= next_step(state,ai_side);
    int size=v.size();
    srand((unsigned)time(NULL));
    int seed=rand()%size;
    std::pair<int, std::pair<int, int> > loc0=v[seed].action;
    change_state(loc0);
//    std::cerr <<"()()()"<<ai_side<<' '<<loc0.first << ' ' << loc0.second.first << ' ' << loc0.second.second;
    return loc0;
    if (!ai_side)
        return std::make_pair(0, std::make_pair(7, 4));
    else
        return std::make_pair(0, std::make_pair(1, 4));
}
