#include "request_handler.h"
#include <iostream>
int main() {

    transport::Catalogue catalogue;
    request::Handler handler(catalogue);

    handler.ReadJson(std::cin);
    handler.FillCatalogue();
    handler.PrintJson(std::cout);
}