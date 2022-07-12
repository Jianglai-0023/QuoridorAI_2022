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

//std::mt19937 mt_rand(time(nullptr) ^ 19260817);
std::mt19937 mt_rand(1);
extern int ai_side;
clock_t time_;
int cnt = 0;
int STEP = -1;
int debug_ = 0;
std::string ai_name = "MCTS 2.0";
const int CIRCLE = 1000;
const int WALK_WEIGHT = 50;
const double BOARD_WEIGHT = 0;
int debug = 0;
const int SIMULATION =10;//max is 100 steps
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

    void operator=(const State &s) {
        if(this==&s)return;
        s0_index = s.s0_index;
        s1_index = s.s1_index;
        for (int i = 0; i < 17; ++i) {
            for (int j = 0; j < 17; ++j) {
                b[i][j] = s.b[i][j];
            }
        }
        s0_board_num = s.s0_board_num;
        s1_board_num = s.s1_board_num;
        action = s.action;
        step = s.step;
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
void print_board(const State &s){
    cout << "-----------------------" << endl;
    cout << s.s0_index.first / 2 << ' ' << s.s0_index.second / 2 << ' ' << s.s1_index.first / 2 << ' '
         << s.s1_index.second / 2 << endl;
    for (int i = 0; i < 17; i++) {
        for (int j = 0; j < 17; j++) {
            if (s.s0_index.first == i && s.s0_index.second == j)cout << 'A' << ' ';
            else if (s.s1_index.first == i && s.s1_index.second == j)cout << 'B' << ' ';
            else cout << (s.b[i][j] ? '|' : '.') << ' ';
        }
        cout << endl;
    }
    cout << "------------------------------" << endl;
}
struct Node {
    double Q = 0;
    int N = 0;
    double UCT = 0;
    bool ismet = false;//是否访问过
    bool isexpanded = false;
//    int unexpand_num = 0;
    State prefer_son;
    pair<vector<State>,vector<State>> son;
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
                        !s.b[x_ + dx[i]][y_ + dy[i]] && (dist[x_ + 2 * dx[i]][y_ + 2 * dy[i]] == -1 ||
                                                         dist[x_ + 2 * dx[i]][y_ + 2 * dy[i]] >
                                                         dist[ind.x][ind.y] + 1)) {//对面
                        dist[x_ + 2 * dx[i]][y_ + 2 * dy[i]] = dist[ind.x][ind.y] + 1;
                        path[x_ + 2 * dx[i]][y_ + 2 * dy[i]].x = ind.x;
                        path[x_ + 2 * dx[i]][y_ + 2 * dy[i]].y = ind.y;
                        q[++t] = index(x_ + 2 * dx[i], y_ + 2 * dy[i]);
                    } else {//两边
                        int dx_ = dy[i];
                        int dy_ = dx[i];
                        for (int j = 0; j < 2; ++j) {
                            if (x_ + dx_ >= 0 && x_ + dx_ <= 16 && y_ + dy_ >= 0 && y_ + dy_ <= 16 &&
                                !s.b[x_ + dx_][y_ + dy_] && (dist[x_ + 2 * dx_][y_ + 2 * dy_] == -1 ||
                                                             dist[x_ + 2 * dx_][y_ + 2 * dy_] >
                                                             dist[ind.x][ind.y] + 1)) {
                                dist[x_ + 2 * dx_][y_ + 2 * dy_] = dist[ind.x][ind.y] + 1;
                                path[x_ + 2 * dx_][y_ + 2 * dy_].x = ind.x;
                                path[x_ + 2 * dx_][y_ + 2 * dy_].y = ind.y;
                                q[++t] = index(x_ + 2 * dx_, y_ + 2 * dy_);
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
                        !s.b[x_ + dx[i]][y_ + dy[i]] && (dist[x_ + 2 * dx[i]][y_ + 2 * dy[i]] == -1 ||
                                                         dist[x_ + 2 * dx[i]][y_ + 2 * dy[i]] >
                                                         dist[ind.x][ind.y] + 1)) {//对面
                        dist[x_ + 2 * dx[i]][y_ + 2 * dy[i]] = dist[ind.x][ind.y] + 1;
                        path[x_ + 2 * dx[i]][y_ + 2 * dy[i]].x = ind.x;
                        path[x_ + 2 * dx[i]][y_ + 2 * dy[i]].y = ind.y;
                        q[++t] = index(x_ + 2 * dx[i], y_ + 2 * dy[i]);
                    } else {//两边
                        int dx_ = dy[i];
                        int dy_ = dx[i];
                        for (int j = 0; j < 2; ++j) {
                            if (x_ + dx_ >= 0 && x_ + dx_ <= 16 && y_ + dy_ >= 0 && y_ + dy_ <= 16 &&
                                !s.b[x_ + dx_][y_ + dy_] && (dist[x_ + 2 * dx_][y_ + 2 * dy_] == -1 ||
                                                             dist[x_ + 2 * dx_][y_ + 2 * dy_] >
                                                             dist[ind.x][ind.y] + 1)) {
                                dist[x_ + 2 * dx_][y_ + 2 * dy_] = dist[ind.x][ind.y] + 1;
                                path[x_ + 2 * dx_][y_ + 2 * dy_].x = ind.x;
                                path[x_ + 2 * dx_][y_ + 2 * dy_].y = ind.y;
//                                cerr << "jump " << x_+2*dx_ << ' ' << y_ + 2*dy_ << endl;
                                q[++t] = index(x_ + 2 * dx_, y_ + 2 * dy_);
                            }
                            dx_ = -dx_;
                            dy_ = -dy_;
                        }
                    }
                } else if (dist[x_][y_] == -1 || dist[x_][y_] > dist[ind.x][ind.y] + 1) {
                    dist[x_][y_] = dist[ind.x][ind.y] + 1;
//                    cerr << "???" << dist[x_][y_] << ' ' << x_ / 2 << ' ' << y_ / 2 << endl;
                    path[x_][y_].x = ind.x;
                    path[x_][y_].y = ind.y;
                    q[++t] = index(x_, y_);
                }
            }
        }
//        cerr << "qwqqwq " << endl;
        int mi = 100;
        int index_y = 0;
        for (int i = 0; i < 17; ++i) {
            if (dist[16][i] != -1 && dist[16][i] < mi) {
                mi = dist[16][i];
                index_y = i;
            }
        }
//        cerr << "qwqqqqwq " << endl;
        int point_x = 16, point_y = index_y;
//        cerr << "index_y " << index_y << endl;
//        for (int i = 0; i < 17; i += 2) {
//            for (int j = 0; j < 17; j += 2) {
//                if(dist[i][j]==0x3f3f3f3f)cerr<<"+oo"<<' ';
//                else cerr << setw(3) << dist[i][j] << ' ';
//            }
//            cerr << endl;
//        }
//        for (int i = 0; i < 17; ++i) {
//            for (int j = 0; j < 17; ++j) {
//                if (s.s0_index.first == i && s.s0_index.second == j || s.s1_index.first == i && s.s1_index.second == j)
//                    cerr << "@";
//                else cerr << (s.b[i][j] ? '#' : '.');
//            }
//            cerr << endl;
//        }

//        exit(0);
//        cerr << "WWW" << x << ' ' << y << endl;
        while (path[point_x][point_y].x != x || path[point_x][point_y].y != y) {
//            cerr << "&&%%" << point_x / 2 << ' ' << point_y / 2 << ' ' << path[point_x][point_y].x / 2 << ' '
//                 << path[point_x][point_y].y / 2 << endl;
            int X = path[point_x][point_y].x;
            int Y = path[point_x][point_y].y;
            point_x = X;
            point_y = Y;
        }
//        cerr << "qwwqwqwqqwwqq " << endl;
        return make_pair(0, make_pair(point_x >> 1, point_y >> 1));
    }
}

bool bfs(std::pair<int, std::pair<int, int>> board, const State &s) {//保证加入的板不会重叠交叉
//    return true;
    cerr << "bfs"<<endl;
//    print_board(s);
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

pair<vector<State>,vector<State>> next_step(const State &s, const bool iss0,bool issimulate) {//走s0
    std::vector<State> v_walk;
    std::vector<State> v_board;
    State S;
    cerr <<"chessee" <<iss0<<endl;
    cerr << s.s0_index.first/2 << ' ' << s.s0_index.second/2<<' '<< s.s1_index.first/2<<' '<<s.s1_index.second/2<< endl;
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
                            v_walk.push_back(S);
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
                    v_walk.push_back(S);
                }
            } else {
                S = s;
                S.step = s.step + 1;
                S.action.first = 0;
                S.action.second = std::make_pair(x / 2, y / 2);
                S.s0_index = std::make_pair(x, y);//映射结果
//                debug_state(S,s);
                v_walk.push_back(S);
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
                            v_walk.push_back(S);
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
                    v_walk.push_back(S);
                }
            } else {
                S = s;
                S.step = s.step + 1;
                S.action.first = 0;
                S.action.second = std::make_pair(x / 2, y / 2);
                S.s1_index = std::make_pair(x, y);//映射结果
//                debug_state(S,s);
                v_walk.push_back(S);
            }
        }
    }
    // for board only put beside cheese
    int x[2],y[2];
//    x[0]=s.s0_index.first;
//    y[0]=s.s0_index.second;
//    x[1]=s.s1_index.first;
//    y[1]=s.s1_index.second;
    int dx_=(iss0?1:-1);//放在对手前进方向
    bool add_next_line=true;
//    if(x[iss0]+dx_>=1&&x[iss0]+dx_<=15) {
//        for (int i = 1; i <= 15; i += 2) {//horizon
//            if (!s.b[x[iss0] + dx_][i - 1] && !s.b[x[iss0] + dx_][i] && !s.b[x[iss0] + dx_][i + 1]&&
//            bfs(make_pair(2, make_pair((x[iss0] + dx_ - 1) >> 1, (i - 1) >> 1)),s)) {
//                S = s;
//                if (iss0)S.s0_board_num--;
//                else S.s1_board_num--;
//                S.step++;
//                S.add_board(make_pair(2, make_pair((x[iss0] + dx_ - 1) >> 1, (i - 1) >> 1)));
//                S.action = make_pair(2, make_pair((x[iss0] + dx_ - 1) >> 1, (i - 1) >> 1));
//                cerr << "attack other " << S.action.first << ' ' << S.action.second.first << ' ' << S.action.second.second << endl;
//                v_board.push_back(S);
//                add_next_line = false;
//            }
//        }
//        if (add_next_line) {//本行放满,看下一行
//            if (x[iss0] + 3 * dx_ >= 1 && x[iss0] + 3 * dx_ <= 15) {
//                for (int i = 15; i >= 1; i -= 2) {
//                    if (!s.b[x[iss0] + 3 * dx_][i - 1] && !s.b[x[iss0] + 3 * dx_][i] &&
//                        !s.b[x[iss0] + 3 * dx_][i + 1] &&
//                        bfs(make_pair(2, make_pair((x[iss0] + 3 * dx_ - 1) >> 1, (i - 1) >> 1)), s)) {
//                        S = s;
//                        if (iss0)S.s0_board_num--;
//                        else S.s1_board_num--;
//                        S.step++;
//                        S.add_board(make_pair(2, make_pair((x[iss0] + 3 * dx_ - 1) >> 1, (i - 1) >> 1)));
//                        S.action = make_pair(2, make_pair((x[iss0] + 3 * dx_ - 1) >> 1, (i - 1) >> 1));
//                        v_board.push_back(S);
//                    }
//                }
//            }
//        }
//            dx_=1;
//            int dy_=-1;
//            cerr << "vertical " << endl;
//            for(int i = 0; i < 4; ++i){//vertical
//                if(x[iss0]+dx_>=1&&x[iss0]+dx_<=15&&y[iss0]+dy_>=1&&y[iss0]+dy_<=15){
//                    if(!s.b[x[iss0]+dx_][y[iss0]+dy_]&&!s.b[x[iss0]+dx_-1][y[iss0]+dy_]&&!s.b[x[iss0]+dx_+1][y[iss0]+dy_]&&
//                    bfs(make_pair(1,make_pair((x[iss0]+dx_-1)>>1,(y[iss0]+dy_-1)>>1)),s)){
//                        S=s;
//                        if(iss0)S.s0_board_num--;
//                        else S.s1_board_num--;
//                        S.step++;
//                        S.add_board(make_pair(1,make_pair((x[iss0]+dx_-1)>>1,(y[iss0]+dy_-1)>>1)));
//                        S.action=make_pair(1,make_pair((x[iss0]+dx_-1)>>1,(y[iss0]+dy_-1)>>1));
//                        v_board.push_back(S);
//                    }
//                }
//                if(i==0)dx_=-dx_;
//                else if(i==1) dy_=-dy_;
//                else dx_=-dx_;
//            }
//    }

    //自己身后
//    dx_=(iss0?1:-1);
//    x[0]=s.s1_index.first;
//    x[1]=s.s0_index.first;
//    y[0]=s.s1_index.second;
//    y[1]=s.s0_index.second;
//    add_next_line=true;
//    cerr << "it's me" <<endl;
//    cerr << iss0 << ' ' << x[iss0] << ' ' << dx_ << endl;
//    if(x[iss0]+dx_>=1&&x[iss0]+dx_<=15){
//        for(int i = 1; i <= 15;i+=2){
//            if(!s.b[x[iss0]+dx_][i]&&!s.b[x[iss0]+dx_][i-1]&&!s.b[x[iss0]+dx_][i+1]&&
//            bfs(make_pair(2,make_pair((x[iss0]+dx_-1)>>1,(i-1)>>1)),s)){
//                cerr << "QAQ " << endl;
//                S=s;
//                if(iss0)S.s0_board_num--;
//                else S.s1_board_num--;
//                S.step++;
//                S.add_board(make_pair(2,make_pair((x[iss0]+dx_-1)>>1,(i-1)>>1)));
//                S.action=make_pair(2,make_pair((x[iss0]+dx_-1)>>1,(i-1)>>1));
//                cerr << "me back " << S.action.first << ' ' << S.action.second.first<<' '<<S.action.second.second << endl;
//                v_board.push_back(S);
//                add_next_line=false;
//            }
//        }
//        cerr << "me next " << endl;
//        if(add_next_line){
//           if(x[iss0]+3*dx_>=1&&x[iss0]+3*dx_<=15){
//               for(int i = 1; i <= 15; i+=2){
//                   if(!s.b[x[iss0]+3*dx_][i]&&!s.b[x[iss0]+3*dx_][i-1]&&!s.b[x[iss0]+3*dx_][i+1]&&
//                   bfs(make_pair(2,make_pair((x[iss0]+3*dx_-1)>>1,(i-1)>>1)),s)){
//                       S=s;
//                       if(iss0)S.s0_board_num--;
//                       else S.s1_board_num--;
//                       S.step++;
//                       cerr << "WUWUWU" << endl;
//                       S.add_board(make_pair(2,make_pair((x[iss0]+3*dx_-1)>>1,(i-1)>>1)));
//                       S.action=make_pair(2,make_pair((x[iss0]+3*dx_-1)>>1,(i-1)>>1));
//                       v_board.push_back(S);
//                   }
//               }
//           }
//        }
//        dx_=1;
//        int dy_=-1;
//        for(int i = 0; i < 4; ++i){//vertical
//            if(x[iss0]+dx_>=1&&x[iss0]+dx_<=15&&y[iss0]+dy_>=1&&y[iss0]+dy_<=15){
//                if(!s.b[x[iss0]+dx_][y[iss0]+dy_]&&!s.b[x[iss0]+dx_-1][y[iss0]+dy_]&&!s.b[x[iss0]+dx_+1][y[iss0]+dy_]&&
//                bfs(make_pair(1,make_pair((x[iss0]+dx_-1)>>1,(y[iss0]+dy_-1)>>1)),s)){
//                    S=s;
//                    if(iss0)S.s0_board_num--;
//                    else S.s1_board_num--;
//                    S.step++;
//                    S.add_board(make_pair(1,make_pair((x[iss0]+dx_-1)>>1,(y[iss0]+dy_-1)>>1)));
//                    S.action=make_pair(1,make_pair((x[iss0]+dx_-1)>>1,(y[iss0]+dy_-1)>>1));
//                    v_board.push_back(S);
//                }
//            }
//            if(i==0)dx_=-dx_;
//            else if(i==1) dy_=-dy_;
//            else dx_=-dx_;
//        }
//    }
//    cerr << "return next_step" << endl;
    return make_pair(v_walk,v_board);
}

class Monte_Tree {
public:
    double cal_UCT(double Ni, double Qi, double N) {
        return ((Qi / Ni) + 0.5 * pow(log(N) / Ni, 0.5));
    }

    class Ans {
    public:
        int board0;
        int board1;
        int dist0;
        int dist1;
        Ans() {}
        Ans(int b1, int b0, int d1, int d0) : board0(b0), board1(b1), dist0(d0), dist1(d1) {}
    };

    double backpropagation(const Ans &ans, bool iss0) {
        if(!iss0){//s0 win rate
            return ((ans.dist1) + (ans.board1) * BOARD_WEIGHT) /
                   (ans.dist0 + ans.dist1 + (ans.board0 + ans.board1) * BOARD_WEIGHT+5);
        }
        else{
            return ((ans.dist0) + (ans.board0) * BOARD_WEIGHT) /
                   (ans.dist0 + ans.dist1 + (ans.board0 + ans.board1) * BOARD_WEIGHT+5);
        }
    }

    pair<double,Ans>  Selection(const State &s, bool iss0) {//<win,UCTofson> //if 当前局面走的是s0
        pair<double,Ans> ans;
        if (!tree[s].ismet) {// first time met add sons
            tree[s].ismet = true;
            tree[s].son = next_step(s, iss0,0);
            cerr << "%^%^%"<<endl;
            tree[s].prefer_son = tree[s].son.first[0];
//            cerr << "size " << tree[s].son.size() << endl;
        }
        if (!tree[s].isexpanded) {
            pair<vector<int>,vector<int>> unmet_son;
            cerr << "not expanded! " <<s.step<<' '<< tree[s].son.second.size() << endl;
            int i_=0;
            for (auto i = tree[s].son.first.begin(); i != tree[s].son.first.end(); ++i) {
                if (!tree[*i].ismet) {//add unmet son
                    unmet_son.first.push_back(i_);
                }
                ++i_;
            }
            i_=0;
            for (auto i = tree[s].son.second.begin(); i != tree[s].son.second.end(); ++i) {
                if (!tree[*i].ismet) {//add unmet son
                    unmet_son.second.push_back(i_);
                }
                ++i_;
            }
            if (!unmet_son.first.size()&&!unmet_son.second.size()) {
                tree[s].isexpanded = true;
            } else {
                int iswalk=0;
                if(unmet_son.first.size())iswalk=1;//存在走子的就先走子
                if(iswalk){
                    int size = unmet_son.first.size();
                    int seed = mt_rand() % size;
                    cerr << "&&&&&&&&" << seed<<' '<<size<<endl;
                    ans = Simulation(tree[s].son.first[unmet_son.first[seed]],!iss0);//random expand<simulation's result, simulation's state>
                    cerr << "########" << endl;
                    if (ans.first == -1)return make_pair(-1,ans.second);
                    tree[tree[s].son.first[unmet_son.first[seed]]].ismet = true;
                    tree[tree[s].son.first[unmet_son.first[seed]]].son = next_step(tree[s].son.first[unmet_son.first[seed]],!iss0,0);
                    cerr << "()()()&&&" << endl;
                    tree[tree[s].son.first[unmet_son.first[seed]]].prefer_son = tree[tree[s].son.first[unmet_son.first[seed]]].son.first[0];
                    tree[tree[s].son.first[unmet_son.first[seed]]].UCT = cal_UCT(++tree[tree[s].son.first[unmet_son.first[seed]]].N, tree[tree[s].son.first[unmet_son.first[seed]]].Q += (ans.first),
                                                        tree[s].N + 1);
                }
                else{
                    cerr << "put board " << endl;
                    int size=unmet_son.second.size();
                    int seed = mt_rand()%size;
                    cerr << "%%%%%" << endl;
                    ans = Simulation(tree[s].son.second[unmet_son.second[seed]],!iss0);
                    cerr << "@@@@@@@@@" << endl;
                    if (ans.first == -1)return make_pair(-1,ans.second);
                    tree[tree[s].son.second[unmet_son.second[seed]]].ismet = true;
                    tree[tree[s].son.second[unmet_son.second[seed]]].son = next_step(tree[s].son.second[unmet_son.second[seed]],!iss0,0);
                    cerr << "@@@!!!*(*" << endl;
                    tree[tree[s].son.second[unmet_son.second[seed]]].prefer_son = tree[tree[s].son.second[unmet_son.second[seed]]].son.second[0];
                    tree[tree[s].son.second[unmet_son.second[seed]]].UCT = cal_UCT(++tree[tree[s].son.second[unmet_son.second[seed]]].N, tree[tree[s].son.second[unmet_son.second[seed]]].Q += (ans.first),
                                                                                 tree[s].N + 1);
                }
                return make_pair(backpropagation(ans.second,iss0),ans.second);
            }
        }
        if (tree[s].isexpanded) {
            cerr << "Expanded!" << s.step << endl;
            double max_uct = 0;
            for (auto i = tree[s].son.first.begin(); i != tree[s].son.first.end(); ++i) {//update UCT
                tree[*i].UCT = cal_UCT(tree[*i].N, tree[*i].Q, tree[s].N + 1);
                if (max_uct < tree[*i].UCT) {
                    tree[s].prefer_son = *i;
                    max_uct = tree[*i].UCT;
                }
            }
            for(auto i = tree[s].son.second.begin();i!=tree[s].son.second.end();++i){
                tree[*i].UCT = cal_UCT(tree[*i].N, tree[*i].Q, tree[s].N + 1);
                if (max_uct < tree[*i].UCT) {
                    tree[s].prefer_son = *i;
                    max_uct = tree[*i].UCT;
                }
            }
            ans = Selection(tree[s].prefer_son, !iss0);
            if (ans.first != -1) {
                tree[tree[s].prefer_son].UCT = cal_UCT(++tree[tree[s].prefer_son].N,
                                                       tree[tree[s].prefer_son].Q += (ans.first),
                                                       tree[s].N + 1);
                return make_pair(backpropagation(ans.second,iss0),ans.second);
            }
            return make_pair(-1,ans.second);
        }
    }

    pair<double,Ans> judge_state(const State &s, const bool &iss0) {
        const int x1 = s.s1_index.first;
        const int y1 = s.s1_index.second;
        const int x0 = s.s0_index.first;
        const int y0 = s.s0_index.second;
        cerr<<"judge state " << x1/2 << ' ' << y1/2 << ' ' << x0/2 << ' ' << y0/2 << endl;
        int dist[17][17];
        double mi_s0 = 100;
        double mi_s1 = 100;
        bool flag = false;
        for(int i = 0; i < 17; ++i){
            for(int j = 0; j < 17; ++j){
                if(s.b[i][j])cerr<<'#';
                else cerr << '.';
            }
            cerr << endl;
        }
        for (int v = 0; v < 2; ++v) {
//            cerr << "shortest path " <<endl;
            memset(dist, 0x3f, sizeof(dist));
            pair<int, int> q[400];
            int h = 0, t = -1;
            if (!flag) {
                q[++t] = make_pair(x0, y0);
                dist[x0][y0] = 1;
            } else {
                q[++t] = make_pair(x1, y1);
                dist[x1][y1] = 1;
            }
            while (t >= h) {
                pair<int, int> head = q[h++];
                const int x = head.first;
                const int y = head.second;
//                cerr << "ququq "<<x << ' '<<y << endl;
                for (int i = 0; i < 4; ++i) {
                    if (x + dx[i] >= 0 && x + dx[i] <= 16 && y + dy[i] >= 0 && y + dy[i] <= 16 && !s.b[x+dx[i]][y+dy[i]]) {
                        int x_ = x + 2 * dx[i];
                        int y_ = y + 2 * dy[i];
//                        cerr << "----------------" << endl;
//            for(int i = 0; i < 17; i+=2){
//                for(int j = 0; j < 17; j+=2){
//                    if(dist[i][j]==0x3f3f3f3f)cerr << "x ";
//                    else cerr << dist[i][j] <<' ';
//                }
//                cerr << endl;
//            }
                        if (!flag && make_pair(x_, y_) == s.s1_index || flag && make_pair(x_, y_) == s.s0_index) {//重合对手
                            if (!s.b[x_ + dx[i]][y_ + dy[i]] && dist[x_ + 2 * dx[i]][y_ + 2 * dy[i]] > dist[x][y] + 1) {
                                x_ += 2 * dx[i];
                                y_ += 2 * dy[i];
                                dist[x_][y_] = dist[x][y] + 1;
                                q[++t] = make_pair(x_, y_);
                            } else if(s.b[x_ + dx[i]][y_+dy[i]]){
                                int dx_ = dy[i];
                                int dy_ = dx[i];
                                for (int j = 0; j < 2; ++j) {
                                    if (!s.b[x_ + dx_][y_ + dy_] && dist[x_ + 2 * dx_][y_ + 2 * dy_] > dist[x][y] + 1) {
                                        dist[x_ + 2 * dx_][y_ + 2 * dy_] = dist[x][y] + 1;
                                        q[++t] = make_pair(x_ + 2 * dx_, y_ + 2 * dy_);
                                    }
                                    dx_ = -dx_;
                                    dy_ = -dy_;
                                }
                            }
                        } else if (dist[x_][y_] > dist[x][y] + 1) {
                            dist[x_][y_] = dist[x][y] + 1;
                            q[++t] = make_pair(x_, y_);
                        }
                    }
                }
            }
            if (!flag) {
                for (int i = 0; i < 17; ++i) {
                    if (dist[0][i] < mi_s0)mi_s0 = dist[0][i];
                }
            } else {
                for (int i = 0; i < 17; ++i) {
                    if (dist[16][i] < mi_s1)mi_s1 = dist[16][i];
                }
            }
            flag = true;
//            cerr << "----------------" << endl;
//            for(int i = 0; i < 17; i+=2){
//                for(int j = 0; j < 17; j+=2){
//                    if(dist[i][j]==0x3f3f3f3f)cerr << "x ";
//                    else cerr << dist[i][j] <<' ';
//                }
//                cerr << endl;
//            }
        }
        cerr << "hereee" <<mi_s1<<' '<<mi_s0<< endl;
        pair<double,double> ans;
        if (!iss0) {//run s0
            cerr << "win rate&& " << mi_s1 << ' ' << mi_s0 << ' ' << s.s0_board_num << ' '
                 << ((mi_s1) + (s.s1_board_num) * BOARD_WEIGHT) /
                    (mi_s0 + mi_s1 + (s.s0_board_num + s.s1_board_num) * BOARD_WEIGHT+5) << endl;
            Ans ans(s.s1_board_num,s.s0_board_num,mi_s1,mi_s0);
            return make_pair(((mi_s1) + (s.s1_board_num) * BOARD_WEIGHT) /
                   (mi_s0 + mi_s1 + (s.s0_board_num + s.s1_board_num) * BOARD_WEIGHT+5),ans);
        } else {
            cerr << "win rate " << mi_s1 << ' ' << mi_s0 << ' ' << s.s1_board_num << ' '
                 << ((mi_s0) + (s.s0_board_num) * BOARD_WEIGHT) /
                    (mi_s0 + mi_s1 + (s.s0_board_num + s.s1_board_num) * BOARD_WEIGHT+5)<< endl;
            Ans ans(s.s1_board_num,s.s0_board_num,mi_s1,mi_s0);
            return make_pair(((mi_s0) + (s.s0_board_num) * BOARD_WEIGHT) /
                   (mi_s0 + mi_s1 + (s.s0_board_num + s.s1_board_num) * BOARD_WEIGHT+5),ans);
        }
    }

    pair<double,Ans> Simulation(const State &s, bool iss0) {
        State s0 = s;
        bool win;
        int i;
        bool a = iss0;
        cerr << "SIMU" << endl;
        return judge_state(s,iss0);
        for (i = 0; i < SIMULATION; ++i) {
            ++cnt;
            cerr << i << '&' << endl;
            pair<vector<State>,vector<State>> v = next_step(s0, a,1);
            int iswalk=mt_rand()%2;
            if(iswalk){
                int size = v.first.size();
                int seed = mt_rand() % size;
                s0 = v.first[seed];
            }
            else{
                int size=v.second.size();
                int seed = mt_rand()%size;
                s0=v.second[seed];
            }
//            cerr <<size <<' '<<a<< "simulation " << s0.s0_index.first/2 << ' ' << s0.s0_index.second/2<<' ' << s0.s1_index.first/2<<' '<<s0.s1_index.second/2 <<' '<<s0.s1_board_num<<' '<<s0.s0_board_num<< endl;

//            if (s0.s0_index.first == 0) {
//                if (!iss0)win = false;
//                else win = true;
//                break;
//            } else if (s0.s1_index.first == 16) {
//                if (!iss0)win = true;
//                else win = false;
//                break;
//            }
            a = !a;
        }
//        return judge_state(s0,iss0);
//        if (i == SIMULATION) {
//            cerr << "calculate " << endl;
//            cerr << "board " << s0.s0_board_num << ' ' << s0.s1_board_num << endl;
            return judge_state(s0, iss0);
//        }
//        return win;
    }//win lose unable

    std::pair<int, std::pair<int, int>> Tree_search() {
        debug = 0;
        while (double(clock() - time_) / CLOCKS_PER_SEC < 1.0) {
//            cerr << clock()<<"%%%%" << endl;
            pair<double,Ans> ans = Selection(state, !ai_side);
            if (ans.first != -1)tree[state].N++;
        }
        int ma = 0;
        State ans;
        for (auto i = tree[state].son.first.begin(); i != tree[state].son.first.end(); ++i) {
//            if(tree[*i].ismet)tree[*i].UCT=cal_UCT(tree[*i].N,tree[*i].Q,tree[state].N);
            if (tree[*i].N >= ma) {
                ans = *i;
                ma = tree[*i].N;
            }
        }
        for (auto i = tree[state].son.second.begin(); i != tree[state].son.second.end(); ++i) {
//            if(tree[*i].ismet)tree[*i].UCT=cal_UCT(tree[*i].N,tree[*i].Q,tree[state].N);
            if (tree[*i].N >= ma) {
                ans = *i;
                ma = tree[*i].N;
            }
        }
        std::pair<int, std::pair<int, int>> loc = ans.action;
        cerr << "------UCT--------" << tree[state].N << endl;
        for (auto i = tree[state].son.first.begin(); i != tree[state].son.first.end(); ++i) {
            cerr << tree[*i].UCT << ' ' << tree[*i].Q << ' ' << tree[*i].N << ' ' << i->s0_index.first / 2 << ' '
                 << i->s0_index.second / 2 << ' ' << i->s1_index.first / 2 << ' ' << i->s1_index.second / 2 << endl;
        }
        cerr << "()()()(" << endl;
        for (auto i = tree[state].son.second.begin(); i != tree[state].son.second.end(); ++i) {
            cerr << tree[*i].UCT << ' ' << tree[*i].Q << ' ' << tree[*i].N << ' ' << (*i).action.first << ' '
                 << (*i).action.second.first << ' ' << (*i).action.second.second <<endl;
        }
        cerr << "ans " << tree[state].prefer_son.s0_index.first/2 << ' ' <<tree[state].prefer_son.s0_index.second/2 << ' ' << tree[state].prefer_son.s1_index.first/2
             <<' ' << tree[state].prefer_son.s1_index.second/2 << endl;
        cerr << "action " << loc.first << ' ' << loc.second.first << ' ' << loc.second.second << endl;
        state = ans;
        return loc;
    }
} monte_tree;

std::pair<int, std::pair<int, int> > action(std::pair<int, std::pair<int, int> > loc) {
    cnt = 0;
    time_ = clock();
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
//    print_board(state);
    state.step++;
    std::pair<int, std::pair<int, int>> loc0;
    if (ai_side && !state.s1_board_num || !ai_side && !state.s0_board_num) {
        loc0 = shortest_path(state, !ai_side);
        if (ai_side)state.s1_index = map_node(loc0.second);
        else state.s0_index = map_node(loc0.second);
    } else {
        loc0 = monte_tree.Tree_search();
    }
    cerr << "TIME&&&&& "<<(clock()-time_)<<' ' <<CLOCKS_PER_SEC<< endl;
//    print_board(state);
    return loc0;
}