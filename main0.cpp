#include "AIController.h"
#include <utility>
#include <vector>
#include<cstdlib>
#include<ctime>
#include<unordered_map>
#include<cmath>
#include<cstring>
#include<random>
#include<iomanip>
std::mt19937 mt_rand(time(nullptr) ^ 19260817);
extern int ai_side;
clock_t time_;
int STEP = -1;
int debug_ = 0;
int num_simulate=0;
int tree_depth=0;
std::string ai_name = "back to 640";
const int CIRCLE = 1000;
const int WALK_WEIGHT = 50;
int debug = 0;
const int SIMULATION = 5;//max is 100 steps
using namespace std;
const int dx[4]{0, 1, 0, -1};
const int dy[4]{1, 0, -1, 0};
struct poi {
    int x;
    int y;
};
struct Ans{
    double s0win=0;
    double s1win=0;
};
class State {
public:
    std::pair<int, int> s0_index;
    std::pair<int, int> s1_index;
    bool b[17][17];
    int s0_board_num = 10;
    int s1_board_num = 10;
    int step = 0;
    std::pair<int, std::pair<int, int> > action;

    bool operator==(const State &opt) const {
        for (int i = 0; i < 17; ++i) {
            for (int j = 0; j < 17; ++j) {
                if (b[i][j] != opt.b[i][j])return false;
            }
        }
        if (s0_index != opt.s0_index)return false;
        if (s1_index != opt.s1_index)return false;
        if (s0_board_num != opt.s0_board_num)return false;
        if (s1_board_num != opt.s1_board_num)return false;
        if (step != opt.step)return false;
        return true;
    }

    void operator=(const State &s){
        s0_index=s.s0_index;
        s1_index=s.s1_index;
        for(int i = 0; i < 17; ++i){
            for(int j = 0; j < 17; ++j){
                b[i][j]=s.b[i][j];
            }
        }
        s0_board_num=s.s0_board_num;
        s1_board_num=s.s1_board_num;
        action=s.action;
        step=s.step;
    }

    bool operator!=(const State &opt) const {
        return !(*this == opt);
    }

    State() {
        s0_index = std::make_pair(16, 8);
        s1_index = std::make_pair(0, 8);
        memset(b, 0, sizeof(b));
        action = make_pair(1, make_pair(-1, -1));
    }

    bool add_board(std::pair<int, std::pair<int, int> > loc) {//保证这里放入的board一定合法
        int x = (loc.second.first<<1) + 1;
        int y = (loc.second.second<<1) + 1;
        if (loc.first == 1) {
            if ((!b[x][y] && !b[x + 1][y] && !b[x - 1][y])) {
                b[x][y] = b[x + 1][y] = b[x - 1][y] = 1;
                return true;
            } else return false;
        } else if (loc.first == 2) {
            if (!b[x][y] && !b[x][y - 1] && !b[x][y + 1]) {
                b[x][y] = b[x][y - 1] = b[x][y + 1] = 1;
                return true;
            } else return false;
        } else return false;
    }
} state;

struct Node {
    double Q = 0;
    int N = 0;
    double UCT = 0;
    bool ismet = false;//是否访问过
    bool isexpanded = false;
    std::vector<State> son;
    vector<State>::iterator head;
};

struct Hash {
    std::size_t operator()(const State &t) const {
        std::size_t hash_result =
                (((233ull * (t.s0_index.first + 1) + (t.s0_index.second + 1)) * 233ull + t.s1_index.first + 1) *
                 233ull +
                 t.s1_index.second + 1) * 233ul + t.step;
        for (int i = 0; i < 17; ++i)
            for (int j = 0; j < 17; ++j)
                hash_result = (hash_result * 233ull + t.b[i][j] + 1);
        return hash_result;
    }
};

std::unordered_map<State, Node, Hash> tree;

//init function is called once at the beginning

pair<int, pair<int, int>> shortest_path(const State &s, bool iss0) {
    class index {
    public:
        int x;
        int y;
        index(int x, int y) : x(x), y(y) { ; }
        index() {}
    } path[17][17];
    int dist[17][17];
    memset(dist, 0x3f, sizeof(dist));
    if (iss0) {//run s0
        const int x = s.s0_index.first;
        const int y = s.s0_index.second;
        index q[400];
        int h = 0, t = -1;
        q[++t] = index(x, y);
        dist[x][y] = 1;
        while (t >= h) {
            index ind = q[h++];
            for (int i = 0; i < 4; ++i) {
                int x_ = ind.x + dx[i];
                int y_ = ind.y + dy[i];
                if (x_ < 0 || x_ > 16 || y_ < 0 || y_ > 16 || s.b[x_][y_])continue;
                x_ += dx[i];
                y_ += dy[i];
                if (make_pair(x_, y_) == s.s1_index) {//与走子重合
                    if (x_ + dx[i] >= 0 && x_ + dx[i] <= 16 && y_ + dy[i] >= 0 && y_ + dy[i] <= 16 &&
                        !s.b[x_ + dx[i]][y_ + dy[i]] && (dist[x_ + (dx[i]<<1)][y_ + (dy[i]<<1)] == -1 ||
                                                         dist[x_ + (dx[i]<<1)][y_ + (dy[i]<<1)] > dist[ind.x][ind.y] + 1)) {//对面
                        dist[x_ + (dx[i]<<1)][y_ + (dy[i]<<1)] = dist[ind.x][ind.y] + 1;
                        path[x_ + (dx[i]<<1)][y_ + (dy[i]<<1)].x = ind.x;
                        path[x_ + (dx[i]<<1)][y_ + (dy[i]<<1)].y = ind.y;
                        q[++t] = index(x_ + (dx[i]<<1), y_ + (dy[i]<<1));
                    } else {//两边
                        int dx_ = dy[i];
                        int dy_ = dx[i];
                        for (int j = 0; j < 2; ++j) {
                            if (x_ + dx_ >= 0 && x_ + dx_ <= 16 && y_ + dy_ >= 0 && y_ + dy_ <= 16 &&
                                !s.b[x_ + dx_][y_ + dy_] && (dist[x_ + (dx_<<1)][y_ + (dy_<<1)] == -1 ||
                                                             dist[x_ + (dx_<<1)][y_ + (dy_<<1)] > dist[ind.x][ind.y] + 1)) {
                                dist[x_ + (dx_<<1)][y_ + (dy_<<1)] = dist[ind.x][ind.y] + 1;
                                path[x_ + 2*dx_][y_ + 2*dy_].x = ind.x;
                                path[x_ + 2*dx_][y_ + 2*dy_].y = ind.y;
                                q[++t] = index(x_ + 2*dx_, y_ + 2*dy_);
                            }
                            dx_ = -dx_;
                            dy_ = -dy_;
                        }
                    }
                } else if (dist[x_][y_] == -1 || dist[x_][y_] > dist[ind.x][ind.y] + 1) {
                    dist[x_][y_] = dist[ind.x][ind.y] + 1;
                    path[x_][y_].x = ind.x;
                    path[x_][y_].y = ind.y;
                    q[++t] = index(x_, y_);
                }
            }
        }
        int mi = 100;
        int index_y = 0;
        for (int i = 0; i < 17; ++i) {
            if (dist[0][i] != -1 && dist[0][i] < mi) {
                mi = dist[0][i];
                index_y = i;
            }
        }
        int point_x = 0, point_y = index_y;
        while (path[point_x][point_y].x != x || path[point_x][point_y].y != y) {
            int X = path[point_x][point_y].x;
            int Y = path[point_x][point_y].y;
            point_x = X;
            point_y = Y;
        }
        return make_pair(0, make_pair(point_x >> 1, point_y >> 1));
    } else {//run s1
        const int x = s.s1_index.first;
        const int y = s.s1_index.second;
        index q[400];
        int h = 0, t = -1;
        q[++t] = index(x, y);
        dist[x][y] = 1;
        while (t >= h) {
            const index ind = q[h++];
            for (int i = 0; i < 4; ++i) {
                int x_ = ind.x + dx[i];
                int y_ = ind.y + dy[i];
                if (x_ < 0 || x_ > 16 || y_ < 0 || y_ > 16 || s.b[x_][y_])continue;
                x_ += dx[i];
                y_ += dy[i];
                if (make_pair(x_, y_) == s.s0_index) {//与走子重合
                    if (x_ + dx[i] >= 0 && x_ + dx[i] <= 16 && y_ + dy[i] >= 0 && y_ + dy[i] <= 16 &&
                        !s.b[x_ + dx[i]][y_ + dy[i]] && (dist[x_ + 2*dx[i]][y_ + 2*dy[i]] == -1 ||
                                                         dist[x_ + 2*dx[i]][y_ + 2*dy[i]] > dist[ind.x][ind.y] + 1)) {//对面
                        dist[x_ + 2*dx[i]][y_ + 2*dy[i]] = dist[ind.x][ind.y] + 1;
                        path[x_ + 2*dx[i]][y_ + 2*dy[i]].x = ind.x;
                        path[x_ + 2*dx[i]][y_ + 2*dy[i]].y = ind.y;
                        q[++t] = index(x_ + 2*dx[i], y_ + 2*dy[i]);
                    }
                    else {//两边
                        int dx_ = dy[i];
                        int dy_ = dx[i];
                        for (int j = 0; j < 2; ++j) {
                            if (x_ + dx_ >= 0 && x_ + dx_ <= 16 && y_ + dy_ >= 0 && y_ + dy_ <= 16 &&
                                !s.b[x_ + dx_][y_ + dy_] && (dist[x_ + 2*dx_][y_ + 2*dy_] == -1 ||
                                                             dist[x_ + 2*dx_][y_ + 2*dy_] > dist[ind.x][ind.y] + 1)) {
                                dist[x_ + 2*dx_][y_ + 2*dy_] = dist[ind.x][ind.y] + 1;
                                path[x_ + 2*dx_][y_ + 2*dy_].x = ind.x;
                                path[x_ + 2*dx_][y_ + 2*dy_].y = ind.y;
//                                cerr << "jump " << x_+2*dx_ << ' ' << y_ + 2*dy_ << endl;
                                q[++t] = index(x_ + 2*dx_, y_ + 2*dy_);
                            }
                            dx_ = -dx_;
                            dy_ = -dy_;
                        }
                    }
                }
                else if (dist[x_][y_] == -1 || dist[x_][y_] > dist[ind.x][ind.y] + 1) {
                    dist[x_][y_] = dist[ind.x][ind.y] + 1;
                    path[x_][y_].x = ind.x;
                    path[x_][y_].y = ind.y;
                    q[++t] = index(x_, y_);
                }
            }
        }
        int mi = 100;
        int index_y = 0;
        for (int i = 0; i < 17; ++i) {
            if (dist[16][i] != -1 && dist[16][i] < mi) {
                mi = dist[16][i];
                index_y = i;
            }
        }
        int point_x = 16, point_y = index_y;
        while (path[point_x][point_y].x != x || path[point_x][point_y].y != y) {
            int X = path[point_x][point_y].x;
            int Y = path[point_x][point_y].y;
            point_x = X;
            point_y = Y;
        }
        return make_pair(0, make_pair(point_x >> 1, point_y >> 1));
    }
}

bool bfs(std::pair<int, std::pair<int, int>> board, const State &s) {//保证加入的板不会重叠交叉
//    return true;
//    cerr << "bfs"<<endl;
    int x = 2 * board.second.first + 1;
    int y = 2 * board.second.second + 1;
    int dist[17][17];
    bool map[17][17];
    memset(dist, -1, sizeof(dist));
    for (int i = 0; i < 17; ++i) {
        for (int j = 0; j < 17; ++j) {
            map[i][j] = s.b[i][j];
        }
    }
    if (board.first == 1) {//vertical
        map[x][y] = map[x + 1][y] = map[x - 1][y] = 1;
    } else {//horizon
        map[x][y] = map[x][y + 1] = map[x][y - 1] = 1;
    }
    poi q[400];
    int h = 0, t = -1;
    poi p;
    p.x = s.s0_index.first;
    p.y = s.s0_index.second;
    q[++t] = p;
    dist[p.x][p.y] = 1;
//    map[p.x][p.y]=1;
    while (h <= t) {
        poi p_ = q[h++];
//      map[p_.x][p_.y]=1;
        for (int i = 0; i < 4; ++i) {
            int x_ = p_.x + dx[i];
            int y_ = p_.y + dy[i];
            if (x_ < 0 || x_ > 17 || y_ < 0 || y_ > 17 || map[x_][y_] || dist[x_ + dx[i]][y_ + dy[i]] != -1)
                continue;//有板子或者走过了
            else {
                p.x = x_ + dx[i];
                p.y = y_ + dy[i];
                q[++t] = p;
                dist[p.x][p.y] = dist[p_.x][p_.y] + 1;
            }
        }
    }
    bool flag;
    for (int i = 0; i < 17; ++i) {
        if (dist[0][i] != -1) {
            flag = true;
            break;
        }
        flag = false;
    }
    memset(dist, -1, sizeof(dist));
    h = 0, t = -1;
    p.x = s.s1_index.first;
    p.y = s.s1_index.second;
    q[++t] = p;
    dist[p.x][p.y] = 1;
    while (h <= t) {
        poi p_ = q[h++];
        for (int i = 0; i < 4; ++i) {
            int x_ = p_.x + dx[i];
            int y_ = p_.y + dy[i];
            if (x_ < 0 || x_ > 17 || y_ < 0 || y_ > 17 || map[x_][y_] || dist[x_ + dx[i]][y_ + dy[i]] != -1)continue;
            else {
                p.x = x_ + dx[i];
                p.y = y_ + dy[i];
                q[++t] = p;
                dist[p.x][p.y] = dist[p_.x][p_.y] + 1;
            }
        }
    }
    bool flag2;
    for (int i = 0; i < 17; ++i) {
        if (dist[16][i] != -1) {
            flag2 = true;
            break;
        }
        flag = false;
    }
    if (!flag || !flag2)return false;
    return true;
}
//void debug_state(const State &dest,const State &src){
//    if(dest.s0_index.first>20||dest.s0_index.second>20||dest.s1_index.first>20||dest.s1_index.second>20){
//        cerr << "boommmm "<<src.s0_index.first<<' '<<src.s0_index.second<<' '<<src.s1_index.first<<' '<<src.s1_index.second<<' '<<dest.s0_index.first/2<<' '<<dest.s0_index.second<<' '<<dest.s1_index.first<<' '<<dest.s1_index.second<<endl;
//    }
//    else cerr << "safe "<<src.s0_index.first<<' '<<src.s0_index.second<<' '<<src.s1_index.first<<' '<<src.s1_index.second<<' '<<dest.s0_index.first/2<<' '<<dest.s0_index.second<<' '<<dest.s1_index.first<<' '<<dest.s1_index.second<<endl;
//}
void init() {
//    std::mt19937 mt_rand(time(nullptr));f
    srand(time(nullptr) ^ 19260817);
    state.step = 0;
}

std::pair<int, int> map_node(std::pair<int, int> loc) {
    return std::make_pair(loc.first * 2, loc.second * 2);
}

void next_step(const State &s, bool iss0, vector<State> &v) {//走s0
//    std::vector<State> v;
    State S;
    for (int i = 0; i < 4; i++) {//for cheese
        int x, y;
        if (iss0) {
            x = s.s0_index.first + dx[i];
            y = s.s0_index.second + dy[i];
            if (x < 0 || x > 16 || y < 0 || y > 16 || s.b[x][y])continue;
            x = x + dx[i];
            y = y + dy[i];
            if (std::make_pair(x, y) == s.s1_index) {//棋子重合
                x = x + dx[i];//再走一步
                y = y + dy[i];
                if (s.b[x][y] || x < 0 || x > 16 || y < 0 || y > 16) {//with board or out of bound
//                    cerr << "herhe" << endl;
                    x -= dx[i];//back to another cheese
                    y -= dy[i];
                    int dx_ = dy[i];//exchange
                    int dy_ = dx[i];
                    for (int t = 0; t < 2; ++t) {
                        if (x + dx_ >= 0 && x + dx_ <= 16 && y + dy_ >= 0 && y + dy_ <= 16 && !s.b[x + dx_][y + dy_]) {
                            S = s;
                            S.step = s.step + 1;
                            S.action.first = 0;
                            S.action.second = std::make_pair((x + 2 * dx_) >> 1, (y + 2 * dy_) >> 1);
                            S.s0_index = std::make_pair(x + 2 * dx_, y + 2 * dy_);//映射结果
//                            debug_state(S,s);
                            v.push_back(S);
                        }
                        dx_ = -dx_;
                        dy_ = -dy_;
                    }
                } else {//no board
                    x = x + dx[i];//可以跳过
                    y = y + dy[i];
                    S = s;
                    S.step = s.step + 1;
                    S.action.first = 0;
                    S.action.second = std::make_pair(x / 2, y / 2);
                    S.s0_index = std::make_pair(x, y);//映射结果
//                    debug_state(S,s);
                    v.push_back(S);
                }
            } else {
                S = s;
                S.step = s.step + 1;
                S.action.first = 0;
                S.action.second = std::make_pair(x / 2, y / 2);
                S.s0_index = std::make_pair(x, y);//映射结果
//                debug_state(S,s);
                v.push_back(S);
            }

        } else {
            x = s.s1_index.first + dx[i];
            y = s.s1_index.second + dy[i];
            if (x < 0 || x > 16 || y < 0 || y > 16 || s.b[x][y])continue;
            x = x + dx[i];
            y = y + dy[i];
            if (std::make_pair(x, y) == s.s0_index) {//棋子重合
                x = x + dx[i];//再走一步
                y = y + dy[i];
                if (s.b[x][y] || x < 0 || x > 16 || y < 0 || y > 16) {//with board or out of bound
                    x -= dx[i];//back to another cheese
                    y -= dy[i];
                    int dx_ = dy[i];//exchange
                    int dy_ = dx[i];
                    for (int t = 0; t < 2; ++t) {
                        if (x + dx_ >= 0 && x + dx_ <= 16 && y + dy_ >= 0 && y + dy_ <= 16 && !s.b[x + dx_][y + dy_]) {
                            S = s;
                            S.step = s.step + 1;
                            S.action.first = 0;
                            S.action.second = std::make_pair((x + 2 * dx_) >> 1, (y + 2 * dy_) >> 1);
                            S.s1_index = std::make_pair(x + 2 * dx_, y + 2 * dy_);//映射结果
//                            debug_state(S,s);
                            v.push_back(S);
                        }
                        dx_ = -dx_;
                        dy_ = -dy_;
                    }
                } else {//no board
                    x = x + dx[i];//再走一步
                    y = y + dy[i];
                    S = s;
                    S.step = s.step + 1;
                    S.action.first = 0;
                    S.action.second = std::make_pair(x / 2, y / 2);
                    S.s1_index = std::make_pair(x, y);//映射结果
//                    debug_state(S,s);
                    v.push_back(S);
                }
            } else {
                S = s;
                S.step = s.step + 1;
                S.action.first = 0;
                S.action.second = std::make_pair(x / 2, y / 2);
                S.s1_index = std::make_pair(x, y);//映射结果
//                debug_state(S,s);
                v.push_back(S);
            }

        }
    }
    // for board
    if (!iss0) {//walk s1
        if (s.s1_board_num) {
            //对方身前
            int x = s.s0_index.first;
            int y = s.s0_index.second;
            int dx_=-1;
            if(x+dx_>=1&&x+dx_<=15){
                for(int i = y-3; i <= y+3; i+=2){
                    if(i>=1&&i<=15){
                        if(!s.b[x+dx_][i]&&!s.b[x+dx_][i-1]&&!s.b[x+dx_][i+1]&&
                           bfs(make_pair(2,make_pair((x+dx_-1)>>1,(i-1)>>1)),s)){//horizon
                            S=s;
                            S.step=s.step+1;
                            S.action= make_pair(2,make_pair((x+dx_-1)>>1,(i-1)>>1));
                            S.add_board(make_pair(2,make_pair((x+dx_-1)>>1,(i-1)>>1)));
                            --S.s1_board_num;
                            v.push_back(S);
                        }
                        else if(x+3*dx_>=1&&x+3*dx_<=15&&
                                !s.b[x+3*dx_][i]&&!s.b[x+3*dx_][i-1]&&!s.b[x+3*dx_][i+1]&&
                                bfs(make_pair(2,make_pair((x+3*dx_-1)>>1,(i-1)>>1)),s)){
                            S=s;
                            S.step=s.step+1;
                            S.action=make_pair(2,make_pair((x+3*dx_-1)>>1,(i-1)>>1));
                            S.add_board(make_pair(2,make_pair((x+3*dx_-1)>>1,(i-1)>>1)));
                            --S.s1_board_num;
                            v.push_back(S);
                        }
                        if(x-1>=1&&x-1<=15){//vertical
                            if(!s.b[x-1][i]&&!s.b[x][i]&&!s.b[x-2][i]&&
                               bfs(make_pair(1,make_pair((x-2)>>1,(i-1)>>1)),s)){
                                S=s;
                                S.step=s.step+1;
                                S.action=make_pair(1,make_pair((x-2)>>1,(i-1)>>1));
                                S.add_board(make_pair(1,make_pair((x-2)>>1,(i-1)>>1)));
                                --S.s1_board_num;
                                v.push_back(S);
                            }
                        }
                        if(x+1>=1&&x+1<=15){
                            if(!s.b[x+1][i]&&!s.b[x][i]&&!s.b[x+2][i]&&
                               bfs(make_pair(1,make_pair(x>>1,(i-1)>>1)),s)){
                                S=s;
                                S.step=s.step+1;
                                S.action=make_pair(1,make_pair(x>>1,(i-1)>>1));
                                S.add_board(make_pair(1,make_pair(x>>1,(i-1)>>1)));
                                --S.s1_board_num;
                                v.push_back(S);
                            }
                        }
                    }
                }
            }
            //自己身后
            x=s.s1_index.first;
            y=s.s1_index.second;
            if(x+dx_>=1&&x+dx_<=15){
                for(int i = y-3; i <= y+3; i+=2){
                    if(i>=1&&i<=15){
//                        if(!s.b[x+dx_][i]&&!s.b[x+dx_][i-1]&&!s.b[x+dx_][i+1]&&
//                           bfs(make_pair(2,make_pair((x+dx_-1)>>1,(i-1)>>1)),s)){
//                            S=s;
//                            S.step=s.step+1;
//                            S.action= make_pair(2,make_pair((x+dx_-1)>>1,(i-1)>>1));
//                            S.add_board(make_pair(2,make_pair((x+dx_-1)>>1,(i-1)>>1)));
//                            --S.s1_board_num;
//                            v.push_back(S);
//                        }
//                        else if(x+3*dx_>=1&&x+3*dx_<=15&&
//                                !s.b[x+3*dx_][i]&&!s.b[x+3*dx_][i-1]&&!s.b[x+3*dx_][i+1]&&
//                                bfs(make_pair(2,make_pair((x+3*dx_-1)>>1,(i-1)>>1)),s)){
//                            S=s;
//                            S.step=s.step+1;
//                            S.action=make_pair(2,make_pair((x+3*dx_-1)>>1,(i-1)>>1));
//                            S.add_board(make_pair(2,make_pair((x+3*dx_-1)>>1,(i-1)>>1)));
//                            --S.s1_board_num;
//                            v.push_back(S);
//                        }
                        if(x+2>=1&&x+2<=15){//vertical
                            if(!s.b[x+1][i]&&!s.b[x+2][i]&&!s.b[x+3][i]&&
                               bfs(make_pair(1,make_pair((x+1)>>1,(i-1)>>1)),s)){
                                S=s;
                                S.step=s.step+1;
                                S.action=make_pair(1,make_pair((x+1)>>1,(i-1)>>1));
                                S.add_board(make_pair(1,make_pair((x+1)>>1,(i-1)>>1)));
                                --S.s1_board_num;
                                v.push_back(S);
                            }
                        }
                        if(x+1>=1&&x+1<=15){
                            if(!s.b[x+1][i]&&!s.b[x][i]&&!s.b[x+2][i]&&
                               bfs(make_pair(1,make_pair(x>>1,(i-1)>>1)),s)){
                                S=s;
                                S.step=s.step+1;
                                S.action=make_pair(1,make_pair(x>>1,(i-1)>>1));
                                S.add_board(make_pair(1,make_pair(x>>1,(i-1)>>1)));
                                --S.s1_board_num;
                                v.push_back(S);
                            }
                        }
                    }

                }
            }
        }
    }else {//walk s0
        if (s.s0_board_num) {
            //在对方周围放板
            int x = s.s1_index.first;
            int y = s.s1_index.second;
            int dx_=1;
            if(x+dx_>=1&&x+dx_<=15){
                for(int i = y-3; i <= y+3; i+=2){
                    if(i>=1&&i<=15){
                        if(!s.b[x+dx_][i]&&!s.b[x+dx_][i-1]&&!s.b[x+dx_][i+1]&&
                           bfs(make_pair(2,make_pair((x+dx_-1)>>1,(i-1)>>1)),s)){//horizon
                            S=s;
                            S.step=s.step+1;
                            S.action= make_pair(2,make_pair((x+dx_-1)>>1,(i-1)>>1));
                            S.add_board(make_pair(2,make_pair((x+dx_-1)>>1,(i-1)>>1)));
                            --S.s0_board_num;
                            v.push_back(S);
                        }
                        else if(x+3*dx_>=1&&x+3*dx_<=15&&
                                !s.b[x+3*dx_][i]&&!s.b[x+3*dx_][i-1]&&!s.b[x+3*dx_][i+1]&&
                                bfs(make_pair(2,make_pair((x+3*dx_-1)>>1,(i-1)>>1)),s)){
                            S=s;
                            S.step=s.step+1;
                            S.action=make_pair(2,make_pair((x+3*dx_-1)>>1,(i-1)>>1));
                            S.add_board(make_pair(2,make_pair((x+3*dx_-1)>>1,(i-1)>>1)));
                            --S.s0_board_num;
                            v.push_back(S);
                        }
                        if(x-1>=1&&x-1<=15){//vertical
                            if(!s.b[x-1][i]&&!s.b[x][i]&&!s.b[x-2][i]&&
                               bfs(make_pair(1,make_pair((x-2)>>1,(i-1)>>1)),s)){
                                S=s;
                                S.step=s.step+1;
                                S.action=make_pair(1,make_pair((x-2)>>1,(i-1)>>1));
                                S.add_board(make_pair(1,make_pair((x-2)>>1,(i-1)>>1)));
                                --S.s0_board_num;
                                v.push_back(S);
                            }
                        }
                        if(x+1>=1&&x+1<=15){
                            if(!s.b[x+1][i]&&!s.b[x][i]&&!s.b[x+2][i]&&
                               bfs(make_pair(1,make_pair(x>>1,(i-1)>>1)),s)){
                                S=s;
                                S.step=s.step+1;
                                S.action=make_pair(1,make_pair(x>>1,(i-1)>>1));
                                S.add_board(make_pair(1,make_pair(x>>1,(i-1)>>1)));
                                --S.s0_board_num;
                                v.push_back(S);
                            }
                        }
                    }
                }
            }
            //在自己周围放板
            x=s.s0_index.first;
            y=s.s0_index.second;
            if(x+dx_>=1&&x+dx_<=15){
                for(int i = y-3; i <= y+3; i+=2){
                    if(i>=1&&i<=15){
//                        if(!s.b[x+dx_][i]&&!s.b[x+dx_][i-1]&&!s.b[x+dx_][i+1]&&//horizon
//                           bfs(make_pair(2,make_pair((x+dx_-1)>>1,(i-1)>>1)),s)){
//                            S=s;
//                            S.step=s.step+1;
//                            S.action= make_pair(2,make_pair((x+dx_-1)>>1,(i-1)>>1));
//                            S.add_board(make_pair(2,make_pair((x+dx_-1)>>1,(i-1)>>1)));
//                            --S.s0_board_num;
//                            v.push_back(S);
//                        }
//                        else if(x+3*dx_>=1&&x+3*dx_<=15&&
//                                !s.b[x+3*dx_][i]&&!s.b[x+3*dx_][i-1]&&!s.b[x+3*dx_][i+1]&&
//                                bfs(make_pair(2,make_pair((x+3*dx_-1)>>1,(i-1)>>1)),s)){
//                            S=s;
//                            S.step=s.step+1;
//                            S.action=make_pair(2,make_pair((x+3*dx_-1)>>1,(i-1)>>1));
//                            S.add_board(make_pair(2,make_pair((x+3*dx_-1)>>1,(i-1)>>1)));
//                            --S.s0_board_num;
//                            v.push_back(S);
//                        }
                        if(x-2>=1&&x-2<=15){//vertical
                            if(!s.b[x-1][i]&&!s.b[x-2][i]&&!s.b[x-3][i]&&
                               bfs(make_pair(1,make_pair((x-3)>>1,(i-1)>>1)),s)){
                                S=s;
                                S.step=s.step+1;
                                S.action=make_pair(1,make_pair((x-3)>>1,(i-1)>>1));
                                S.add_board(make_pair(1,make_pair((x-3)>>1,(i-1)>>1)));
                                --S.s0_board_num;
                                v.push_back(S);
                            }
                        }
                        if(x-1>=1&&x-1<=15){
                            if(!s.b[x-1][i]&&!s.b[x][i]&&!s.b[x-2][i]&&
                               bfs(make_pair(1,make_pair((x-2)>>1,(i-1)>>1)),s)){
                                S=s;
                                S.step=s.step+1;
                                S.action=make_pair(1,make_pair((x-2)>>1,(i-1)>>1));
                                S.add_board(make_pair(1,make_pair((x-2)>>1,(i-1)>>1)));
                                --S.s0_board_num;
                                v.push_back(S);
                            }
                        }
                    }
                }
            }
        }
    }
    for(int i = v.size()-1;i>=0;i--){
        int t=mt_rand()%(i+1);
        swap(v[i],v[t]);
    }
//    return v;
}

class Monte_Tree {
public:
    double cal_UCT(double Ni, double Qi, double N) {
        return ((Qi / Ni) + 0.41 * pow(log(N) / Ni, 0.5));
    }

    Ans Selection(const State &s,const bool iss0) {//<win,UCTofson> //if 当前局面走的是s0
        ++tree_depth;
        //TODO 删除不必要的节点
        Ans ans;
        if (!tree[s].ismet) {// first time met add sons
            cerr << "unmet "<<s.step <<endl;
            tree[s].ismet = true;
            next_step(s, iss0,tree[s].son);
            tree[s].head=tree[s].son.begin();
//            cerr << "size " << tree[s].son.size() << endl;
        }
        else if (!tree[s].isexpanded) {
//            cerr << "not expanded! " << s.step << endl;
            auto i=tree[s].head;
            for (; i != tree[s].son.end(); ++i) {
//                cerr << "qwqqq "<<endl;
                if (!tree[*i].ismet) {//add unmet son
//                    cerr << "WUWUWU" <<endl;
                    ans= Simulation(*i,!iss0);
                    double q=iss0? ans.s0win:ans.s1win;
                    tree[*i].ismet = true;
                    next_step(*i, !iss0,tree[*i].son);
                    tree[*i].UCT = cal_UCT(tree[*i].N++, tree[*i].Q += q,
                                           tree[s].N + 1);
                    tree[*i].head=tree[*i].son.begin();
                    tree[s].head=++i;
                    return ans;
                }
            }
//            tree[s].head=i;
            if(i==tree[s].son.end()){
                tree[s].isexpanded=true;
            }
        }
        if(tree[s].isexpanded){
//            cerr << "expanded!" <<s.step<< endl;
            double max_uct = 0;
            int index;
            int i_=0;
            for (auto i = tree[s].son.begin(); i != tree[s].son.end(); ++i) {//update UCT
                if (max_uct < tree[*i].UCT) {
                    index = i_;
                    max_uct = tree[*i].UCT;
                }
                ++i_;
            }
            ans = Selection(tree[s].son[index], !iss0);
            double q=iss0?ans.s0win:ans.s1win;
            tree[tree[s].son[index]].UCT = cal_UCT(tree[tree[s].son[index]].N++,
                                                   tree[tree[s].son[index]].Q += q,
                                                   tree[s].N + 1);
            return ans;
        }
    }

    void print_board(const State &s) const {
        cerr << "-----------------------" << endl;
        cerr << s.s0_index.first/2 << ' ' << s.s0_index.second/2<<' '<<s.s1_index.first/2<<' ' << s.s1_index.second/2<< endl;
        for (int i = 0; i < 17; ++i) {
            for (int j = 0; j < 17; ++j) {
                if (s.s0_index.first == i && s.s0_index.second == j)cerr << 'A' << ' ';
                else if (s.s1_index.first == i && s.s1_index.second == j)cerr << 'B' << ' ';
                else cerr << (s.b[i][j]?'|':'.') << ' ';
            }
            cerr << endl;
        }
        cerr << "------------------------------" << endl;
    }

    Ans judge_state(const State &s,const bool &iss0){
        const int x1=s.s1_index.first;
        const int y1=s.s1_index.second;
        const int x0=s.s0_index.first;
        const int y0=s.s0_index.second;
//        cerr << x1<<' ' << y1 << ' ' << x0 << ' ' << y0 << endl;
        int dist[17][17];
        double mi_s0=100;
        double mi_s1=100;
        bool flag=false;
        for(int v=0;v<2;++v){
//            cerr << "shortest path " <<endl;
            memset(dist,0x3f,sizeof(dist));
            pair<int,int> q[400];
            int h=0,t=-1;
            if(!flag){
                q[++t]= make_pair(x0,y0);
                dist[x0][y0]=1;
            }
            else{
                q[++t]= make_pair(x1,y1);
                dist[x1][y1]=1;
            }
            while(t>=h){
//                cerr << "ququq " << endl;
                pair<int,int> head=q[h++];
                for(int i = 0; i < 4; ++i){
                    const int x=head.first;
                    const int y=head.second;
                    if(x+dx[i]>=0&&x+dx[i]<=16&&y+dy[i]>=0&&y+dy[i]<=16&&!s.b[x+dx[i]][y+dy[i]]){
                        int x_=x+2*dx[i];
                        int y_=y+2*dy[i];
                        if(h==2&&(make_pair(x_,y_)==s.s0_index||make_pair(x_,y_)==s.s1_index)){//TODO 可以跳的情况
                            if(x_+dx[i]>=1&&x_+dx[i]<=15&&y_+dy[i]>=1&&y_+dy[i]<=15&&
                               !s.b[x_+dx[i]][y_+dy[i]]&&
                               dist[x_+2*dx[i]][y_+2*dy[i]]>dist[x][y]+1){
                                dist[x_+2*dx[i]][y_+2*dy[i]]=dist[x][y]+1;
                                q[++t]=make_pair(x_+2*dx[i],y_+2*dy[i]);
                            }
                            else if(dist[x_+2*dx[i]][y_+2*dy[i]]>dist[x][y]+1){
                                int dx_=dy[i];
                                int dy_=dx[i];
                                for(int j = 0; j < 2; ++j){
                                    if(x_+dx_>=1&&x_+dx_<=15&&y_+dy_>=1&&y_+dy_<=15&&
                                       !s.b[x_+dx_][y_+dy_]&&
                                       dist[x_+2*dx_][y_+2*dy_]<dist[x][y]+1){
                                        dist[x_+2*dx_][y_+2*dy_]=dist[x][y]+1;
                                        q[++t]=make_pair(x_+2*dx_,y_+2*dy_);
                                    }
                                    dx_=-dx_;
                                    dy_=-dy_;
                                }
                            }
                        }
                        else if(dist[x_][y_]>dist[x][y]+1){
                            dist[x_][y_]=dist[x][y]+1;
                            q[++t]= make_pair(x_,y_);
                        }
                    }

                }
            }
            if(!flag){
                for(int i = 0;i<17;++i){
                    if(dist[0][i]<mi_s0)mi_s0=dist[0][i];
                }
            }
            else{
                for(int i = 0;i<17;++i){
                    if(dist[16][i]<mi_s1)mi_s1=dist[16][i];
                }
            }
            flag=true;
//            cerr << "----------------" << endl;
//            for(int i = 0; i < 17; i+=2){
//                for(int j = 0; j < 17; j+=2){
//                    if(dist[i][j]==0x3f3f3f3f)cerr << "x ";
//                    else cerr << dist[i][j] <<' ';
//                }
//                cerr << endl;
//            }
        }
//        cerr << "hereee" << endl;
//if(s.b[11][7])cerr << "QWWQ" << endl;
        Ans ans;
        ans.s0win=(mi_s1/(mi_s0));
        ans.s1win=(mi_s0/(mi_s1));
//        cerr << "state_judge " << mi_s0 <<' ' << mi_s1<<' '<<s.s0_index.first/2 <<' '<<s.s0_index.second/2<<' '<<s.s1_index.first/2<<' '<<s.s1_index.second/2 << endl;
//        if(iss0){//run s0
//        cerr <<"s0 win rate "<< s.s0_index.first/2 << ' ' << s.s0_index.second/2 << ' ' <<(mi_s1)/(mi_s0)<<endl;
//            return (mi_s1)/(mi_s0);
//        }
//        else{
//        cerr <<"s1 win rate "<< s.s1_index.first/2 << ' ' << s.s1_index.second/2 << ' ' <<(mi_s0)/(mi_s1)<<endl;
//            return (mi_s0)/(mi_s0);
//        }
        return ans;
    }

    Ans Simulation(const State &s, bool iss0) {
        ++tree_depth;
        ++num_simulate;
        return  judge_state(s ,iss0);
    }//win lose unable

    std::pair<int, std::pair<int, int>> Tree_search() {
        debug = 0;
        num_simulate=0;
        while(double(clock()-time_)/CLOCKS_PER_SEC<1.98){
            tree_depth=0;
            Selection(state, !ai_side);
            tree[state].N++;
//            cerr << "DEPTH " << tree_depth << endl;
        }
        cerr << "SIMULATE " << num_simulate << endl;
        int ma = 0;
        double uct=1e9;
        int index;
        int i_=0;
        for (auto i = tree[state].son.begin(); i != tree[state].son.end(); ++i) {
//            if(tree[*i].ismet)tree[*i].UCT=cal_UCT(tree[*i].N,tree[*i].Q,tree[state].N);
            if (tree[*i].N > ma||tree[*i].N==ma&&tree[*i].UCT>uct) {
                index=i_;
                ma = tree[*i].N;
                uct=tree[*i].UCT;
            }
            ++i_;
        }
        std::pair<int, std::pair<int, int>> loc = tree[state].son[index].action;
        cerr << "-------UCT-------" << tree[state].N << endl;
        for (auto i = tree[state].son.begin(); i != tree[state].son.end(); ++i) {
            cerr << tree[*i].UCT << ' ' << tree[*i].Q << ' ' << tree[*i].N << ' ' << (*i).action.first << ' '
                 << (*i).action.second.first << ' ' << (*i).action.second.second << endl;
        }
//        cerr << "ans " << tree[state].prefer_son.s0_index.first/2 << ' ' <<tree[state].prefer_son.s0_index.second/2 << ' ' << tree[state].prefer_son.s1_index.first/2
//             <<' ' << tree[state].prefer_son.s1_index.second/2 << endl;
        cerr << "action " << loc.first << ' ' << loc.second.first << ' ' << loc.second.second << endl;
        state=tree[state].son[index];
        return loc;
    }
} monte_tree;

std::pair<int, std::pair<int, int> > action(std::pair<int, std::pair<int, int> > loc) {
    time_=clock();
//    cerr << ai_side << "()()" << endl;
//    ++STEP;
    if (ai_side) {
        if (!loc.first)state.s0_index = map_node(loc.second);
        else if (loc.first == 1 || loc.first == 2) {
            bool flag = state.add_board(loc);
            state.s0_board_num--;
        }
    } else {
        if (!loc.first)state.s1_index = map_node(loc.second);
        else if (loc.first == 1 || loc.first == 2) {
            bool flag = state.add_board(loc);
            state.s1_board_num--;
        }
    }
    state.step++;
    std::pair<int, std::pair<int, int>> loc0;
    if (ai_side && !state.s1_board_num || !ai_side && !state.s0_board_num) {
        loc0 = shortest_path(state, !ai_side);
        if (ai_side)state.s1_index = map_node(loc0.second);
        else state.s0_index = map_node(loc0.second);
    } else {
        loc0 = monte_tree.Tree_search();
    }
    cerr<<"clock" << double(clock()-time_)/CLOCKS_PER_SEC << endl;
    return loc0;
}