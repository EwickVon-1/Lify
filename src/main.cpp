#include <iostream>

int main() {
    std::cout << "Hello World!\n";

    int a{};
    int b{};

    std::cout << "Input a number: ";
    std::cin >> a;

    std::cout << "Input another number: ";
    std::cin >> b;

    return 0;
}

int add(int a, int b) {
    return a + b;
}
