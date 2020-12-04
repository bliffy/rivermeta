#include <vector>

extern "C" {

int my_function(int a) {
    std::vector<int> k; k.push_back(a);
    a = k[0] * 3;
    return a;
}

}
