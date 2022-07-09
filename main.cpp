#include "AIController.h"
#include <utility>
#include <vector>
#include<cstdlib>
#include<ctime>
#include<unordered_map>
#include<cmath>
#include<cstring>
#include<random>
std::mt19937 mt_rand(time(nullptr));
extern int ai_side;
int STEP=-1;
std::string ai_name = "MCTS 1.0";
const int CIRCLE = 2000;
const int WALK_WEIGHT = 50;
int debug = 0;
const int SIMULATION = 2000;//max is 100 steps
using namespace std;
const int dx[4]{0, 1, 0, -1};
const int dy[4]{1, 0, -1, 0};
struct poi {
    int x;
    int y;
};

class State {
public:
    std::pair<int, int> s0_index;
    std::pair<int, int> s1_index;
    bool b[17][17];
    int s0_board_num = 10;
    int s1_board_num = 10;
    int step=0;
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
        if(step!=opt.step)return false;
        return true;
    }

    bool operator!=(const State &opt) const {
        return !(*this == opt);
    }

//    void operator=(const State &s) {
//        for (int i = 0; i < 17; ++i) {
//            for (int j = 0; j < 17; ++j) {
//                b[i][j] = s.b[i][j];
//            }
//        }
//        s0_index = s.s0_index;
//        s1_index = s.s1_index;
//        board_num = s.board_num;
//        action = s.action;
//    }

    State() {
        s0_index = std::make_pair(16, 8);
        s1_index = std::make_pair(0, 8);
        memset(b, 0, sizeof(b));
        action = make_pair(1, make_pair(-1, -1));
    }

    bool add_board(std::pair<int, std::pair<int, int> > loc) {//保证这里放入的board一定合法
        int x = 2 * loc.second.first + 1;
        int y = 2 * loc.second.second + 1;
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
    int Q = 0;
    int N = 0;
    double UCT = 0;
    bool ismet = false;//是否访问过
    bool isexpanded = false;
//    int unexpand_num = 0;
    State prefer_son;
    std::vector<State> son;
};

struct Hash {
    std::size_t operator()(const State &t) const {
        std::size_t hash_result =
                (((233ull * (t.s0_index.first + 1) + (t.s0_index.second + 1)) * 233ull + t.s1_index.first + 1) * 233ull +
                t.s1_index.second + 1)*233ul + t.step;
        for (int i = 0; i < 17; ++i)
            for (int j = 0; j < 17; ++j)
                hash_result = (hash_result * 233ull + t.b[i][j] + 1);
        return hash_result;
    }
};

struct Equal {
    bool operator()(const State &a, const State &b) const {
        return a == b;
    }
};

std::unordered_map<State, Node, Hash> tree;

//init function is called once at the beginning
bool bfs(std::pair<int, std::pair<int, int>> board) {//保证加入的板不会重叠交叉
    int x = 2 * board.second.first + 1;
    int y = 2 * board.second.second + 1;
    int dist[17][17];
    bool map[17][17];
    memset(dist, -1, sizeof(dist));
    for (int i = 0; i < 17; ++i) {
        for (int j = 0; j < 17; ++j) {
            map[i][j] = state.b[i][j];
        }
    }
    if (board.first == 1) {//vertical
        map[x][y] = map[x + 1][y] = map[x - 1][y] = 1;
    } else {//horizon
        map[x][y] = map[x][y + 1] = map[x][y - 1] = 1;
    }
    poi q[300];
    int h = 0, t = -1;
    poi p;
    p.x = state.s0_index.first;
    p.y = state.s0_index.second;
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
    p.x = state.s1_index.first;
    p.y = state.s1_index.second;
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

void init() {
//    std::mt19937 mt_rand(time(nullptr));
    srand(time(nullptr) ^ 19260817);
    state.step=0;
}

std::pair<int, int> map_node(std::pair<int, int> loc) {
    return std::make_pair(loc.first * 2, loc.second * 2);
}

std::vector<State> next_step(const State &s, bool iss0) {//走s0
    std::vector<State> v;
    State S;
    for (int i = 0; i < 4; i++) {//for cheese
        // TODO 遇见对方跳过
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
//                if (x < 0 || x > 16 || y < 0 || y > 16)continue;
                if (s.b[x][y]||x < 0 || x > 16 || y < 0 || y > 16) {//with board or out of bound
//                    cerr << "herhe" << endl;
                    x -= dx[i];//back to another cheese
                    y -= dy[i];
                    int dx_ = dy[i];//exchange
                    int dy_ = dx[i];
//                    cerr << x+dx_ << "*1*" << y+dy_ << endl;
                    if (x + dx_ >= 0 && x + dx_ <= 16 && y + dy_ >= 0 && y + dy_ <= 16 && !s.b[x + dx_][y + dy_]) {
                        S = s;
                        S.step=s.step+1;
                        S.action.first = 0;
                        S.action.second = std::make_pair((x + 2 * dx_) >> 1, (y + 2 * dy_) >> 1);
                        S.s0_index = std::make_pair(x + 2 * dx_, y + 2 * dy_);//映射结果
                        v.push_back(S);
                    }
                    dx_ = -dx_;
                    dy_ = -dy_;
//                    cerr << x+dx_ << "*2*" << y+dy_ << endl;
                    if (x + dx_ >= 0 && x + dx_ <= 16 && y + dy_ >= 0 && y + dy_ <= 16 && !s.b[x + dx_][y + dy_]) {
                        S = s;
                        S.step=s.step+1;
                        S.action.first = 0;
                        S.action.second = std::make_pair((x + 2 * dx_) >> 1, (y + 2 * dy_) >> 1);
                        S.s0_index = std::make_pair(x + 2 * dx_, y + 2 * dy_);//映射结果
                        v.push_back(S);
                    }
                } else {//no board
                    x = x + dx[i];//可以跳过
                    y = y + dy[i];
                    S = s;
                    S.step=s.step+1;
                    S.action.first = 0;
                    S.action.second = std::make_pair(x / 2, y / 2);
                    S.s0_index = std::make_pair(x, y);//映射结果
                    v.push_back(S);
                }
            } else {
                S = s;
                S.step=s.step+1;
                S.action.first = 0;
                S.action.second = std::make_pair(x / 2, y / 2);
                S.s0_index = std::make_pair(x, y);//映射结果
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
//                if (x < 0 || x > 16 || y < 0 || y > 16)continue;
                if (s.b[x][y]||x < 0 || x > 16 || y < 0 || y > 16) {//with board or out of bound
                    x -= dx[i];//back to another cheese
                    y -= dy[i];
                    int dx_ = dy[i];//exchange
                    int dy_ = dx[i];
                    if (x + dx_ >= 0 && x + dx_ <= 16 && y + dy_ >= 0 && y + dy_ <= 16 && !s.b[x + dx_][y + dy_]) {
                        S = s;
                        S.step=s.step+1;
                        S.action.first = 0;
                        S.action.second = std::make_pair((x + 2 * dx_) >> 1, (y + 2 * dy_) >> 1);
                        S.s1_index = std::make_pair(x + 2 * dx_, y + 2 * dy_);//映射结果
                        v.push_back(S);
                    }
                    dx_ = -dx_;
                    dy_ = -dy_;
                    if (x + dx_ >= 0 && x + dx_ <= 16 && y + dy_ >= 0 && y + dy_ <= 16 && !s.b[x + dx_][y + dy_]) {
                        S = s;
                        S.step=s.step+1;
                        S.step=s.step+1;
                        S.action.first = 0;
                        S.action.second = std::make_pair((x + 2 * dx_) >> 1, (y + 2 * dy_) >> 1);
                        S.s1_index = std::make_pair(x + 2 * dx_, y + 2 * dy_);//映射结果
                        v.push_back(S);
                    }
                } else {//no board
                    x = x + dx[i];//再走一步
                    y = y + dy[i];
                    S = s;
                    S.step=s.step+1;
                    S.action.first = 0;
                    S.action.second = std::make_pair(x / 2, y / 2);
                    S.s1_index = std::make_pair(x, y);//映射结果
                    v.push_back(S);
                }
            } else {
                S = s;
                S.step=s.step+1;
                S.action.first = 0;
                S.action.second = std::make_pair(x / 2, y / 2);
                S.s1_index = std::make_pair(x, y);//映射结果
                v.push_back(S);
            }

        }
//        cerr << "cheese " << i << endl;
//for(int j = 0; j < WALK_WEIGHT; ++j){

//}
    }
    // for board
    /*策略：放在自己的身后；对方的身前；放在已有的板子周围
     *横竖板随机
     */
    //放在对方周围
//        if (!iss0) {//walk s1
//            if(s.s1_board_num){
//                int x_ = s.s0_index.first;
//                int y_ = s.s0_index.second;
//                srand((unsigned) time(NULL));
//                int vertical = rand() % 2;
//                if (vertical) {//vertical
//                    int x = x_ - 1;
//                    int y = y_ - 1;
//                    if (x > 0 && y > 0 && !s.b[x][y] && !s.b[x - 1][y] && !s.b[x + 1][y] &&
//                        bfs(std::make_pair(1, std::make_pair((x - 1) >> 1, (y - 1) >> 1)))) {//满足放板子条件
//                        S = s;
//                        S.action = std::make_pair(1, std::make_pair((x - 1) >> 1, (y - 1) >> 1));
//                        S.add_board(std::make_pair(1, std::make_pair((x - 1) >> 1, (y - 1) >> 1)));
//                        --S.s1_board_num;
////                    cerr << "other"<<endl;
//                        v.push_back(S);
//                    }
//                    x = x_ - 1;
//                    y = y_ + 1;
//                    if (x > 0 && y < 16 && !s.b[x][y] && !s.b[x - 1][y] && !s.b[x + 1][y] &&
//                        bfs(std::make_pair(1, std::make_pair((x - 1) >> 1, (y - 1) >> 1)))) {
//                        S = s;
//                        S.action = std::make_pair(1, std::make_pair((x - 1) >> 1, (y - 1) >> 1));
//                        S.add_board(std::make_pair(1, std::make_pair((x - 1) >> 1, (y - 1) >> 1)));
//                        --S.s1_board_num;
////                    cerr << "other"<<endl;
//                        v.push_back(S);
//                    }
//                }
//                else {//horizon
//                    int x = x_ - 1;
//                    int y = y_ - 1;
//                    if (y > 0 && y < 16 && !s.b[x][y - 1] && !s.b[x][y] && !s.b[x][y + 1] &&
//                        bfs(std::make_pair(2, std::make_pair((x - 1) >> 1, (y - 1) >> 1)))) {
//                        S = s;
//                        S.action = std::make_pair(2, std::make_pair((x - 1) >> 1, (y - 1) >> 1));
//                        S.add_board(std::make_pair(2, std::make_pair((x - 1) >> 1, (y - 1) >> 1)));
//                        --S.s1_board_num;
////                    cerr << "other"<<endl;
//                        v.push_back(S);
//                    }
//                    x = x_ - 1;
//                    y = y_ + 1;
//                    if (y > 0 && y < 16 && !s.b[x][y - 1] && !s.b[x][y + 1] && !s.b[x][y] &&
//                        bfs(std::make_pair(2, std::make_pair((x - 1) >> 1, (y - 1) >> 1)))) {
//                        S = s;
//                        S.action = std::make_pair(2, std::make_pair((x - 1) >> 1, (y - 1) >> 1));
//                        S.add_board(std::make_pair(2, std::make_pair((x - 1) >> 1, (y - 1) >> 1)));
//                        --S.s1_board_num;
////                    cerr << "other"<<endl;
//                        v.push_back(S);
//                    }
//                }
//            }
//        } else {//
//            if(s.s0_board_num){
//                int x_ = s.s1_index.first;
//                int y_ = s.s1_index.second;
//                srand((unsigned) time(NULL));
//                int vertical = rand() % 2;
//                if (vertical) {//vertical
//                    int x = x_ + 1;
//                    int y = y_ - 1;
//                    if (x < 16 && y > 0 && !s.b[x - 1][y] && !s.b[x + 1][y] && !s.b[x][y] &&
//                        bfs(std::make_pair(1, std::make_pair((x - 1) >> 1, (y - 1) >> 1)))) {//满足放板子条件
//                        S = s;
//                        S.action = std::make_pair(1, std::make_pair((x - 1) >> 1, (y - 1) >> 1));
//                        S.add_board(std::make_pair(1, std::make_pair((x - 1) >> 1, (y - 1) >> 1)));
//                        --S.s0_board_num;
////                    cerr << "other"<<endl;
//                        v.push_back(S);
//                    }
//                    x = x_ + 1;
//                    y = y_ + 1;
//                    if (x < 16 && y < 16 && !s.b[x - 1][y] && !s.b[x][y] && !s.b[x + 1][y] &&
//                        bfs(std::make_pair(1, std::make_pair((x - 1) >> 1, (y - 1) >> 1)))) {
//                        S = s;
//                        S.action = std::make_pair(1, std::make_pair((x - 1) >> 1, (y - 1) >> 1));
//                        S.add_board(std::make_pair(1, std::make_pair((x - 1) >> 1, (y - 1) >> 1)));
//                        --S.s0_board_num;
////                    cerr << "other"<<endl;
//                        v.push_back(S);
//                    }
//                }
//                else {//horizon
//                    int x = x_ + 1;
//                    int y = y_ - 1;
//                    if (y > 0 && y < 16 && !s.b[x][y - 1] && !s.b[x][y] && !s.b[x][y + 1] &&
//                        bfs(std::make_pair(2, std::make_pair((x - 1) >> 1, (y - 1) >> 1)))) {
//                        S = s;
//                        S.action = std::make_pair(2, std::make_pair((x - 1) >> 1, (y - 1) >> 1));
//                        S.add_board(std::make_pair(2, std::make_pair((x - 1) >> 1, (y - 1) >> 1)));
//                        --S.s0_board_num;
////                    cerr << "other"<<endl;
//                        v.push_back(S);
//                    }
//                    x = x_ + 1;
//                    y = y_ + 1;
//                    if (y > 0 && y < 16 && !s.b[x][y - 1] && !s.b[x][y] && !s.b[x][y + 1] &&
//                        bfs(std::make_pair(2, std::make_pair((x - 1) >> 1, (y - 1) >> 1)))) {
//                        S = s;
//                        S.action = std::make_pair(2, std::make_pair((x - 1) >> 1, (y - 1) >> 1));
//                        S.add_board(std::make_pair(2, std::make_pair((x - 1) >> 1, (y - 1) >> 1)));
//                        --S.s0_board_num;
////                    cerr << "other"<<endl;
//                        v.push_back(S);
//                    }
//                }
//            }
//
//            //put board next to other boards
//        }
//        //放在已有的板子周围
//        if(iss0&&s.s0_board_num){
//            for (int i = 0; i < 17; ++i) {
//                for (int j = 0; j < 17; ++j) {
//                    if (s.b[i][j]) {
//                        srand((unsigned) time(NULL));
//                        int vertical = rand() % 2;
//                        if (vertical) {
//                            for (int t = 0; t < 4; ++t) {
//                                int x = i + dx[t];
//                                int y = j + dy[t];
//                                if (!(x % 2) || !(y % 2))continue;//必须是奇数
//                                if (y <= 0 || y >= 16 || x >= 16 || x <= 0 || s.b[x][y] || s.b[x + 1][y] || s.b[x - 1][y] ||
//                                    !bfs(std::make_pair(1, std::make_pair((x - 1) >> 1, (y - 1) >> 1))))
//                                    continue;
//                                S = s;
//                                S.action = std::make_pair(1, std::make_pair((x - 1) >> 1, (y - 1) >> 1));
//                                S.add_board(std::make_pair(1, std::make_pair((x - 1) >> 1, (y - 1) >> 1)));
//                                S.s0_board_num--;
////                            cerr <<"board "<< i << ' ' << j <<endl;
//                                v.push_back(S);
//                            }
//                        } else {
//                            for (int t = 0; t < 4; ++t) {
//                                int x = i + dx[t];
//                                int y = j + dy[t];
//                                if (!(x % 2) || !(y % 2))continue;
//                                if (y <= 0 || y >= 16 || x <= 0 || x >= 16 || s.b[x][y] || s.b[x][y + 1] || s.b[x][y - 1] ||
//                                    !bfs(std::make_pair(2, std::make_pair((x - 1) >> 1, (y - 1) >> 1))))
//                                    continue;
//                                S = s;
//                                S.action = std::make_pair(2, std::make_pair((x - 1) >> 1, (y - 1) >> 1));
//                                S.add_board(std::make_pair(2, std::make_pair((x - 1) >> 1, (y - 1) >> 1)));
//                                S.s0_board_num--;
////                            cerr <<"board "<< i << ' ' << j <<endl;
//                                v.push_back(S);
//                            }
//                        }
//                    }
//                }
//            }
//        }
//        else if(!iss0&&s.s1_board_num){
//            for (int i = 0; i < 17; ++i) {
//                for (int j = 0; j < 17; ++j) {
//                    if (s.b[i][j]) {
//                        srand((unsigned) time(NULL));
//                        int vertical = rand() % 2;
//                        if (vertical) {
//                            for (int t = 0; t < 4; ++t) {
//                                int x = i + dx[t];
//                                int y = j + dy[t];
//                                if (!(x % 2) || !(y % 2))continue;//必须是奇数
//                                if (y <= 0 || y >= 16 || x >= 16 || x <= 0 || s.b[x][y] || s.b[x + 1][y] || s.b[x - 1][y] ||
//                                    !bfs(std::make_pair(1, std::make_pair((x - 1) >> 1, (y - 1) >> 1))))
//                                    continue;
//                                S = s;
//                                S.action = std::make_pair(1, std::make_pair((x - 1) >> 1, (y - 1) >> 1));
//                                S.add_board(std::make_pair(1, std::make_pair((x - 1) >> 1, (y - 1) >> 1)));
//                                S.s1_board_num--;
////                            cerr <<"board "<< i << ' ' << j <<endl;
//                                v.push_back(S);
//                            }
//                        } else {
//                            for (int t = 0; t < 4; ++t) {
//                                int x = i + dx[t];
//                                int y = j + dy[t];
//                                if (!(x % 2) || !(y % 2))continue;
//                                if (y <= 0 || y >= 16 || x <= 0 || x >= 16 || s.b[x][y] || s.b[x][y + 1] || s.b[x][y - 1] ||
//                                    !bfs(std::make_pair(2, std::make_pair((x - 1) >> 1, (y - 1) >> 1))))
//                                    continue;
//                                S = s;
//                                S.action = std::make_pair(2, std::make_pair((x - 1) >> 1, (y - 1) >> 1));
//                                S.add_board(std::make_pair(2, std::make_pair((x - 1) >> 1, (y - 1) >> 1)));
//                                S.s1_board_num--;
////                            cerr <<"board "<< i << ' ' << j <<endl;
//                                v.push_back(S);
//                            }
//                        }
//                    }
//                }
//            }
//        }

//    cerr << "#####" << v.size() << endl;
    return v;
}

//std::pair<int, std::pair<int, int> > changew_state(std::pair<int, std::pair<int, int> > loc) {
////    tree.erase(state);
//    if (!loc.first) {
//        if (!ai_side) {
//            state.s0_index.first = loc.second.first * 2;
//            state.s0_index.second = loc.second.second * 2;
//        } else {
//            state.s1_index.first = loc.second.first * 2;
//            state.s1_index.second = loc.second.second * 2;
//        }
//    } else {
//        state.add_board(loc);
////        --state.board_num;
//    }
//    return loc;
//}

class Monte_Tree {
public:
    double cal_UCT(double Ni, double Qi, double N) {
        return ((Qi / Ni) + 2.41 * pow(log(N) / Ni, 0.5));
    }

    int Selection(const State &s, bool iss0) {//<win,UCTofson> //if 当前局面走的是s0
        //TODO 删除不必要的节点
        int ans;
        if (!tree[s].ismet) {// first time met add sons
            tree[s].ismet = true;
            tree[s].son = next_step(s, iss0);
            tree[s].prefer_son = State();
        }
        double max_uct = -1;
        if (!tree[s].isexpanded) {
//            cerr << "not expanded! " << s.step << endl;
            for (auto i = tree[s].son.begin(); i != tree[s].son.end(); ++i) {
                if (!tree[*i].ismet) {//updateUCT
                    ans = Simulation(*i, !iss0);//
                    cerr << "%%%" << ans << endl;
                    if (ans == -1)return -1;//没有修改uct 有可能全都是-1 导致没有儿子的uct被修改
                    tree[*i].son = next_step(*i, !iss0);
//                    tree[*i].prefer_son = tree[*i].son[0];//for the biggest uct
                    tree[*i].ismet = true;
                    int n = ++tree[*i].N;
//                    if(!(ai_side^(!iss0)))ans=1-ans;
                    int q = tree[*i].Q += ans;
                    tree[*i].UCT = cal_UCT(double(n), double(q), tree[s].N+1);
//                    cerr << "UCT " << tree[*i].UCT<<' '<<i->s0_index.first/2<<' '<<i->s0_index.second/2<<' '<<i->s1_index.first/2<<' '<<i->s1_index.second/2 << endl;
                    double ma_=0;
                    for (auto j = tree[s].son.begin(); j != tree[s].son.end(); ++j) {//update UCT
                       if(tree[*j].ismet) tree[*j].UCT= cal_UCT(tree[*j].N,tree[*j].Q,tree[s].N+1);
                        if (ma_ < tree[*j].UCT){
                            tree[s].prefer_son = *j;
                            ma_=tree[*j].UCT;
                        }
                    }
                    return ans;
                }
            }
            double ma_=0;
            for (auto j = tree[s].son.begin(); j != tree[s].son.end(); ++j) {//update UCT
                tree[*j].UCT= cal_UCT(tree[*j].N,tree[*j].Q,tree[s].N+1);
                if (ma_ < tree[*j].UCT){
                    tree[s].prefer_son = *j;
                    ma_=tree[*j].UCT;
                }
            }
            tree[s].isexpanded = true;
            ans = Selection(tree[s].prefer_son,!iss0);
            if (ans != -1) {
                tree[tree[s].prefer_son].UCT = cal_UCT(tree[tree[s].prefer_son].N++, tree[tree[s].prefer_son].Q += ans,
                                                       tree[s].N + 1);
                return ans;
            }
            return -1;

        } else {
//            cerr << "expanded!" <<s.step<< endl;
            max_uct = 0;
            for (auto i = tree[s].son.begin(); i != tree[s].son.end(); ++i) {//update UCT
                tree[*i].UCT= cal_UCT(tree[*i].N,tree[*i].Q,tree[s].N+1);
                if (max_uct < tree[*i].UCT) {
                    tree[s].prefer_son = *i;
                    max_uct = tree[*i].UCT;
                }
            }
            ans = Selection(tree[s].prefer_son, !iss0);
            if (ans != -1) {
                tree[tree[s].prefer_son].UCT = cal_UCT(tree[tree[s].prefer_son].N++, tree[tree[s].prefer_son].Q += ans,
                                                       tree[s].N + 1);
//                cerr<<"UCT "<<tree[tree[s].prefer_son].UCT<<endl;
                return  ans;
            }
            return -1;
        }
    }

    int Simulation(const State &s, bool iss0) {
        State s0 = s;
        bool win;
        int i;
        bool a = iss0;
        for (i = 0; i < SIMULATION; ++i) {//最多走1000步
            std::vector<State> v = next_step(s0, a);
            int size = v.size();
//            cerr <<s0.step<<' '<<a<< "simulation " << s0.s0_index.first/2 << ' ' << s0.s0_index.second/2<<' ' << s0.s1_index.first/2<<' '<<s0.s1_index.second/2 << endl;
            int seed = mt_rand() % size;
            s0 = v[seed];
            if(s0.s0_index.first == 0){
                if(ai_side)win= false;
                else win=true;
                break;
            }
            else if(s0.s1_index.first==16){
                if(ai_side)win=true;
                else win=false;
                break;
            }
            a = !a;
        }
        if (i == SIMULATION)return -1;
        return win;
    }//win lose unable

    std::pair<int, std::pair<int, int>> Tree_search() {
        debug = 0;
        for (int i = 0; i < CIRCLE; ++i) {//TODO 用时间作为循环变量
//            cerr << "%&&&&" << !ai_side << endl;
            int ans=Selection(state,!ai_side);
            if(ans!=-1)tree[state].N++;
//            cerr << '#' << i << endl;
        }
        int ma=0;
        State ans;
        for(auto i = tree[state].son.begin();i!=tree[state].son.end();++i){
//            if(tree[*i].ismet)tree[*i].UCT=cal_UCT(tree[*i].N,tree[*i].Q,tree[state].N);
            if(tree[*i].N>=ma){
                ans=*i;
                ma=tree[*i].N;
            }
        }
        std::pair<int, std::pair<int, int>> loc = ans.action;
        cerr << "--------------" <<tree[state].N<< endl;
        for(auto i = tree[state].son.begin();i!=tree[state].son.end();++i){
            cerr << tree[*i].UCT<<' '<<tree[*i].Q<<' '<<tree[*i].N << ' ' << i->s0_index.first/2 << ' ' << i->s0_index.second/2 << ' ' << i->s1_index.first/2 << ' ' << i->s1_index.second/2 << endl;
        }
//        cerr << "ans " << tree[state].prefer_son.s0_index.first/2 << ' ' <<tree[state].prefer_son.s0_index.second/2 << ' ' << tree[state].prefer_son.s1_index.first/2
//             <<' ' << tree[state].prefer_son.s1_index.second/2 << endl;
        cerr << "action " << loc.first << ' ' <<loc.second.first << ' ' << loc.second.second << endl;
        state = ans;
        return loc;
    }
} monte_tree;

std::pair<int, std::pair<int, int> > action(std::pair<int, std::pair<int, int> > loc) {
    cerr << ai_side<<"()()" << endl;
//    ++STEP;
    if (ai_side) {
        if (!loc.first)state.s0_index = map_node(loc.second);
        else if (loc.first == 1 || loc.first == 2) {
            bool flag = state.add_board(loc);
            state.s0_board_num--;
            if (!flag)std::cerr << "boad false";
        }
    } else {
        if (!loc.first)state.s1_index = map_node(loc.second);
        else if (loc.first == 1 || loc.first == 2) {
            bool flag = state.add_board(loc);
            state.s1_board_num--;
            if (!flag)std::cerr << "boad false";
        }
    }
    state.step++;
    std::pair<int, std::pair<int, int> > loc0 = monte_tree.Tree_search();
//    cerr << "--------------------" << endl;
//    std::cerr<<loc0.first<<' '<<loc0.second.first<<' '<<loc0.second.second << "&&";
//    return loc0;
//    std::vector<State> v = next_step(state);
//    int size = v.size();
//    srand((unsigned) time(NULL));
//    int seed = rand() % size;
//    std::pair<int, std::pair<int, int> > loc0 = v[seed].action;
//    if (loc0.first == 2) {
//        int x = loc0.second.first * 2 + 1;
//        int y = loc0.second.second * 2 + 1;
//    }
cerr << loc0.first << ' ' << loc0.second.first << ' ' << loc0.second.second << endl;
    return loc0;
}