#include "AIController.h"
#include <utility>
#include <vector>

extern int ai_side;
std::string ai_name = "your_ai_name_here";
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
    bool add_board(std::pair<int, std::pair<int, int> > loc) {//判断是否合法
        int x = 2 * loc.second.first + 1;
        int y = 2 * loc.second.second + 1;
        if (loc.first == 1) {
            if ((!b[x][y] && !b[x][y + 1] && !b[x][y - 1])) {

                b[x][y] = b[x][y + 1] = b[x][y - 1] = 1;
                return true;
            } else return false;
        } else if (loc.first == 2) {
            if (!b[x][y] && !b[x - 1][y] && !b[x + 1][y]) {
                b[x][y] = b[x - 1][y] = b[x + 1][y] = 1;
                return true;
            } else return false;
        }
    }
} state;
//init function is called once at the beginning
bool bfs_right(std::pair<int,std::pair<int,int>> loc){
    int x = 2 * loc.second.first + 1;
    int y = 2 * loc.second.second + 1;
    bool b[17][17];
    for(int i = 0; i < 17; ++i){
        for(int j = 0; j < 17; ++j){
            b[i][j]=state.b[i][j];
        }
    }
    b[x][y] = b[x][y + 1] = b[x][y - 1] = 1;
    int x0=state.s0_index.first;
    int y0=state.s0_index.second;
    int x1=state.s1_index.first;
    int y1=state.s1_index.second;
}
class Monte_Tree {
public:
    node *head = nullptr;
} monte_tree;
void init() {
    for (int i = 0; i < 19; ++i) {
        for (int j = 0; j < 19; ++j) {
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
//    std::cerr<<state.s1_index.first << '@'<<state.s1_index.second;
      for(int i = 0; i < 4; i++){//for cheese
          int x,y;
          State S=s;
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
                  if(x<0||x>16||y<0||y>16)continue;
                  if(s.b[x][y])continue;
                  x=x+dx[i];
                  y=y+dy[i];
                  if(std::make_pair(x,y)==s.s0_index){//是否重合
                      x=x+dx[i];
                      y=y+dy[i];
                      if(x<0||x>16||y<0||y>16)continue;
                      if(s.b[x][y])continue;
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
        else if(loc.first==1||loc.first==2) state.add_board(loc);
    }
    else{
        if(!loc.first)state.s1_index=map_node(loc.second);
        else if(loc.first==1||loc.first==2)state.add_board(loc);
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
