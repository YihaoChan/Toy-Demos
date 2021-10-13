#include "../timestamp/timestamp.h"
#include <iostream>

using std::cout;
using std::endl;

int main() {
    Timestamp timestamp;
    cout << timestamp.timeUntilSec() << endl;
    cout << timestamp.timeUntilDay() << endl;
    return 0;
}