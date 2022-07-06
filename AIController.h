#include <iostream>
#include <string>
#include <utility>

extern std::string ai_name;
int ai_side;

void init();
std::pair<int, std::pair<int, int> > action(std::pair<int, std::pair<int, int> >);

void SubmitInit() {
    std::cin >> ai_side;
    init();
    std::cout << ai_name << std::endl;
    std::cout.flush();
}

std::pair<int, std::pair<int, int> > Get() {
    int a, b, c;
    std::cin >> a >> b >> c;
    return std::make_pair(a, std::make_pair(b, c));
}

void Post(std::pair<int, std::pair<int, int> > loc) {
    std::cout << loc.first << ' ' << loc.second.first << ' ' << loc.second.second << std::endl;
    std::cout.flush();
}

int main() {
    SubmitInit();
    while (true) {
        Post(action(Get()));
    }
    return 0;
}
