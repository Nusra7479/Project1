#include <iostream>
#include "b_plus_tree.h"

using namespace std;

int main()
{
    int n = 5;
    cout << "Hello world!" << n << endl;
    BPTree bpTree;


    cout << "------------------------ Experiment 3 ------------------------" <<endl;
    bpTree.searchKey(500);
    //storage.searchKey(500);
    cout << "------------------------ Experiment 4 ------------------------" <<endl;
    //bpTree.searchKeyRange(30000,40000);
    //storage.searchKeyRange(30000, 40000);



    return 0;
}
