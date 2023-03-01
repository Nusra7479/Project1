#include <iostream>
#include "b_plus_tree.h"
#include <chrono>
#include "storage.cpp"
using namespace std::chrono;




vector<void *> BPTree::searchKeyRange(int minNumVotes, int maxNumVotes){ //ToDo Duplicates & dataBlocksAccessed
    vector<void *> result;

    //// start timer && initialize values
    auto start = high_resolution_clock::now();
    int indexNodesAccessed = 0;

    ////If root is null, empty tree
    if (root == nullptr){
        std::cout << "ERROR: B+ Tree is Empty!" << std::endl;
        return result;
    }

    //int dataBlocksAccessed = 0;

    Node *cursor = root;
    indexNodesAccessed++; // first node - root

    ////traverse until we find the leaf node pertaining the minNumVotes
    while (cursor->isLeaf == false){
        for (int i = 0; i < cursor->size; i++) {
            if (minNumVotes < cursor->key[i]) {
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

    //// traverse leaf node to find the keys
    bool allFound = false;
    while (!allFound){
        for (int i = 0; i < cursor->size; i++) {
            int ptrKey = cursor->key[i];
            if (ptrKey > maxNumVotes){
                allFound = true;
                break;
            }
            if (ptrKey >= minNumVotes && ptrKey <= maxNumVotes) {
                void *recordptr = cursor->ptrs[i];
                result.push_back(recordptr);
                //dataBlocksAccessed++;
                //totalRecords++;
                //totalRating = totalRating + recordptr->averageRating;

                //// handle duplicates TODO
                // here
            }
        }
        if (!allFound){
            //// Move to the next leaf node
            cursor = cursor->ptrs[cursor->size];
            indexNodesAccessed++;
        }
    }

    //float avgRating = totalRating/totalRecords;

    ////end timer
    auto stop = high_resolution_clock::now();
    auto runningTime = duration_cast<microseconds>(stop - start);

    //// print results
    //cout << "------------------------ Experiment 3 ------------------------" <<endl;
    cout << "The number of index nodes the process accesses: " << indexNodesAccessed << endl;
    //cout << "The number of data blocks the process accesses: "<< dataBlocksAccessed << endl;
    //cout << "The average of “averageRating’s” of the records that are returned: " << avgRating << endl;
    cout << "The running time of the retrieval process: " << runningTime.count() << endl;
    return result;



    }

}


