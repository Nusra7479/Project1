#include <iostream>
#include "b_plus_tree.h"
#include "storage.cpp"
#include <chrono>
using namespace std::chrono;

BPTree::BPTree(short int mK) {
    maxKeys = mK;
}

void BPTree::insert(int key, Record* recordPtr){
    if (root == nullptr){
        root = new Node;
        root->size=1;
        root->isLeaf = true;
        root->keys[0] = key;
        root->parent = nullptr;
    } 

    else {
        Node* cursor = root;
        Node* parent;
 
        while (cursor->isLeaf == false) {
            parent = cursor;
 
            for (int i = 0; i < cursor->size; i++) {
                if (key < cursor->keys[i]) {
                    cursor = (Node*) cursor->ptrs[i];
                    break;
                }
 
                if (i == cursor->size - 1) {
                    cursor = (Node*) cursor->ptrs[i + 1];
                    break;
                }
            }
        }

        // Reached leaf
        
        for (int i = 0; i < cursor->size; i++) {
                if (key > cursor->keys[i]) {
                    continue;
                }
                // Duplicate: insert into LL
                if (key == cursor->keys[i]) {
                    LLNode* existingPtr = (LLNode*) cursor->ptrs[i]; // Safe cast because key exists already.
                    LLNode listNode = *existingPtr;
                    listNode.insert(recordPtr);
                }
                // Make LL of length 1
                LLNode* newLLNode = new LLNode();
                newLLNode->recordPtr = recordPtr;
                newLLNode->next = nullptr;

                cursor->keys.insert(cursor->keys.begin() + i, key);
                cursor->ptrs.insert(cursor->ptrs.begin() + i, newLLNode);
                cursor->size++;

                break;
        }

        // Split if needed
        if (cursor->size > this->maxKeys) {
            int halfSize = cursor->size / 2; // floor
            vector<int> leftKeys(cursor->keys.begin(), cursor->keys.begin() + (cursor->size - halfSize));
            vector<int> rightKeys(cursor->keys.begin() + (cursor->size - halfSize), cursor->keys.end());

            int rightSize = rightKeys.size();
            int leftSize = leftKeys.size();

            Node* right = new Node();
            right->isLeaf = true;
            right->size = rightSize;
            right->parent = cursor->parent;
            right->keys = rightKeys;

            // Handling pointers

            int ptrHalfSize = cursor->ptrs.size() / 2;
            vector<void*> leftPtrs(cursor->ptrs.begin(), cursor->ptrs.begin() + ptrHalfSize);
            vector<void*> rightPtrs(cursor->ptrs.begin() + ptrHalfSize, cursor->ptrs.end());

            right->ptrs = rightPtrs;

            // Update left leaf
            cursor->keys = leftKeys;
            cursor->size = leftKeys.size();
            cursor->ptrs = leftPtrs;
            cursor->ptrs.push_back(&right);
            
            // Parent insertion / update
            int toInsert = right->keys.front();
            insertInternal(toInsert, cursor->parent, right);
        } 
    }
}

void BPTree::insertInternal(int key, Node* parent, Node* child) {

    // insert key and pointer
    for (int i = 0; i < parent->size; i++) {
        if (key > parent->keys[i]) {
            continue;
        }
        parent->keys.insert(parent->keys.begin() + i, key);
        parent->ptrs.insert(parent->ptrs.begin() + i + 1, child);
        parent->size++;
        break;
    }

    // split if needed
    if (parent->size > this->maxKeys) {

        // remove the key to be inserted to parent
        int promotedKeyIndex = (parent->keys.size() - 1) / 2;
        int promotedKey = parent->keys[promotedKeyIndex];
        parent->keys.erase(parent->keys.begin() + promotedKeyIndex);

        // remaining keys: distribute between left and right, with more in left
        int leftSize = parent->keys.size() - parent->keys.size() / 2; // floor
        vector<int> leftKeys(parent->keys.begin(), parent->keys.begin() + leftSize);
        vector<int> rightKeys(parent->keys.begin() + leftSize, parent->keys.end()); 
        int rightSize = rightKeys.size();

        // distribute pointers
        vector<void*> leftPtrs(parent->ptrs.begin(), parent->ptrs.begin() + leftSize + 1);
        vector<void*> rightPtrs(parent->ptrs.begin() + leftSize + 1, parent->ptrs.end());
        
        // create right node
        Node* right = new Node();
        right->isLeaf = false;
        right->size = rightSize;
        right->parent = parent->parent;
        right->keys = rightKeys;
        right->ptrs = rightPtrs;

        // Update left node
        parent->size = leftSize;
        parent->keys = leftKeys;
        parent->ptrs = leftPtrs;

        // update parent pointers of right node's children
        for (int i = 0; i < right->ptrs.size(); i++) {
            Node* currChild = (Node*) right->ptrs[i];
            currChild->parent = right;
        }

        // two cases: root or internal
        if (!parent->parent) { // current node is the root, we need to make a new root
            Node* newRoot = new Node();
            
            newRoot->isLeaf = false;
            newRoot->size = 1;

            vector<int> newRootKeys; // TODO: make this less ugly
            newRootKeys.push_back(promotedKey);
            newRoot->keys = newRootKeys;
            
            vector<void*> newRootPtrs;
            newRootPtrs.push_back(parent);
            newRootPtrs.push_back(right);
            newRoot->ptrs = newRootPtrs;
            newRoot->parent = nullptr;

            this->root = newRoot;
        } else { // current node is internal, recurse on promoted key
            insertInternal(promotedKey, parent->parent, parent);
        }
    }
}

void BPTree::searchKey(int key) { //ToDo Duplicates & dataBlocksAccessed
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
            if (key < cursor->keys[i]) {
                cursor = (Node*) cursor->ptrs[i]; // if searchKey < NodeKey, go to LHS Node
                indexNodesAccessed++;
                break;
            }

            if (i == cursor->size - 1) {
                cursor = (Node*) cursor->ptrs[i + 1];
                indexNodesAccessed++;
                break;
            }
        }
    }

    //// traverse leaf node to find the key
    
    for (int i = 0; i < cursor->size; i++) {
      // if not duplicate
        if (cursor->keys[i] == key) {
            Record *recordptr = (Record*) cursor->ptrs[i];
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
            std::cout << "The number of index nodes the process accesses: " << indexNodesAccessed << std::endl;
            std::cout << "The number of data blocks the process accesses: "<< dataBlocksAccessed << std::endl;
            std::cout << "The average of �averageRating�s� of the records that are returned: " << avgRating << std::endl;
            std::cout << "The running time of the retrieval process: " << runningTime.count() << std::endl;
            return;
        }
    }
    std::cout << "Not found\n";

    }


void LLNode::insert(Record* recordPtr) {
    LLNode* currentNode = this;
    while (currentNode->next) {
        currentNode = currentNode->next;
    }
    LLNode* newLLNode = new LLNode();
    newLLNode->next = nullptr;
    newLLNode->recordPtr = recordPtr;
    currentNode->next = newLLNode;
}


