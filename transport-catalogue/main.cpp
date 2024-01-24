#include <iostream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"
//#include "t_c_tester.h"

using namespace std;

int main() {
    
    //t_c_tester::Test3();

    transport::Catalogue catalogue;
    
    input::Reader input_reader;
    input_reader.ReadData(cin,catalogue);

    request::Reader request_reader(cout,catalogue);
    request_reader.ReadAndPrintRequests(cin);
}