#include "request_handler.h"
#include <iostream>
int main() {
    transport::Catalogue catalogue;
    request::Handler handler(catalogue);

    handler.ReadInput(std::cin);
    handler.PrintJson(std::cout);
}