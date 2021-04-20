#include <string>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <cstring>
#include <optional>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <vector>

int main() {


    char *tmp = new char[0];
    if (tmp == nullptr) std::cout << "no";
    delete[] tmp;

    return 0;
}