#include <iostream>
#include <map>

using namespace std;


int main()
{
    cout << "Hello World!" << endl;
    std::map<int, int> my_map;
    
    my_map[1] = 1; 
    my_map[4] = 4; 
    my_map[6] = 6; 
    my_map[8] = 8; 
    my_map[12] = 12; 


    std::cout << (--my_map.lower_bound(10))->first;
    std::cout << my_map.lower_bound(10)->first;
    return 0;
}
