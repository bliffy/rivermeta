#include <iostream>

#include "mylib.h"

int main(){
    std::cout<< "hello world\n";

    int my_val = 5, new_val;
    new_val = my_function(my_val);

    std::cout<<"vals "<<my_val<<" to "<<new_val<<"\n";

system("pause");
    return 0;
}

