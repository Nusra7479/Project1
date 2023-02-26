#include <iostream>
#include "b_plus_tree.h"
#include <chrono>
using namespace std::chrono;




void BPTree::searchKey(int key){ //ToDo Duplicates & dataBlocksAccessed
    //// start timer
    auto start = high_resolution_clock::now();

    ////If root is null, empty tree
    if (root == nullptr){
        std::cout << "ERROR: B+ Tree is Empty!" << std::endl;
        return;
    }

    ////initialize values
    int indexNodesAccessed = 0;
    int dataBlocksAccessed = 0;
    float totalRating = 0.0;
    int totalRecords = 0;

    Node *cursor = root;
    indexNodesAccessed++; // first node - root

    ////traverse until we find the leaf node
    while (cursor->isLeaf == false){
        for (int i = 0; i < cursor->size; i++) {
            if (x < cursor->key[i]) {
                cursor = cursor->ptr[i]; // if searchKey < NodeKey, go to LHS Node
                indexNodesAccessed++;
                break;
            }

            if (i == cursor->size - 1) {
                cursor = cursor->ptr[i + 1];
                indexNodesAccessed++;
                break;
            }
        }
    }

    //// traverse leaf node to find the key
    for (int i = 0; i < cursor->size; i++) {
      if (cursor->key[i] == x) {
        Record *recordptr = cursor->ptrs[i];
        dataBlocksAccessed++;
        totalRecords++;
        totalRating = totalRating + recordptr->averageRating;
        //// handle duplicates TODO
        // here



        //cout << "Found\n";
        float avgRating = totalRating/totalRecords;

        ////end timer
        auto stop = high_resolution_clock::now();
        auto runningTime = duration_cast<microseconds>(stop - start);

        //// print results
        //cout << "------------------------ Experiment 3 ------------------------" <<endl;
        cout << "The number of index nodes the process accesses: " << indexNodesAccessed << endl;
        cout << "The number of data blocks the process accesses: "<< dataBlocksAccessed << endl;
        cout << "The average of “averageRating’s” of the records that are returned: " << avgRating << endl;
        cout << "The running time of the retrieval process: " << runningTime.count() << endl;
        return;
      }
    }
    cout << "Not found\n";


    }

}


