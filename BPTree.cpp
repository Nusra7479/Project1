#include <iostream>
#include "BPTree.h"
#include <chrono>
#include <vector>

using namespace std::chrono;

Node::Node(){
    vector<int> keyVector;
    keys = keyVector;

    vector<void*> ptrVector;
    ptrs = ptrVector;
}

LLNode::LLNode(){}

BPTree::BPTree(short int mK) {
    maxKeys = mK;
    root = nullptr;
}

void BPTree::insert(int key, Record* recordPtr){
    // std::cout << "Key: " << endl;
    // std::cout << key << endl;

    if (root == nullptr){
        root = new Node;
        root->size=1;
        root->isLeaf = true;
        root->keys.push_back(key);
        root->parent = nullptr;

        LLNode* newLLNode = new LLNode();
        newLLNode->recordPtr = recordPtr;
        newLLNode->next = nullptr;

        root->ptrs.push_back(newLLNode);
        root->ptrs.push_back(nullptr);
    }

    else {
        Node* cursor = root;

        while (cursor->isLeaf == false) {

            // std::cout << "Cursor:" << endl;
            // std::cout << cursor << endl;

            for (int i = 0; i < cursor->size; i++) {
                if (key < cursor->keys[i]) {
                    cursor = (Node*) cursor->ptrs[i];
                    break;
                }
                if (i == cursor->size - 1) {
                    // std::cout << "SPECIAL:" << endl;
                    // std::cout << cursor << endl;
                    cursor = (Node*) cursor->ptrs[i + 1];
                    break;
                }
            }
        }

        // Reached leaf

        for (int i = 0; i < cursor->size + 1; i++) { // TODO: VERY UGLY

                if (i == cursor->size) {
                    LLNode* newLLNode = new LLNode();
                    newLLNode->recordPtr = recordPtr;
                    newLLNode->next = nullptr;

                    cursor->keys.insert(cursor->keys.begin() + i, key);
                    cursor->ptrs.insert(cursor->ptrs.begin() + i, newLLNode);
                    cursor->size++;

                    break;
                }

                if (key > cursor->keys[i]) {
                    continue;
                }
                // Duplicate: insert into LL
                if (key == cursor->keys[i]) {
                    LLNode* existingPtr = (LLNode*) cursor->ptrs[i]; // Safe cast because key exists already.
                    LLNode listNode = *existingPtr;
                    listNode.insert(recordPtr);
                    break;
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

            // std::cout << "isLeaf:" << endl;
            // std::cout << cursor->isLeaf << endl;

            // std::cout << "parent?:" << endl;
            // std::cout << (cursor->parent) << endl;

            // std::cout << "key?:" << endl;
            // std::cout << key << endl;

            // std::cout << "size?:" << endl;
            // std::cout << (cursor->size) << endl;

            // std::cout << " " << endl;

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

            // std::cout << "isLeaf:" << endl;
            // std::cout << cursor->isLeaf << endl;

            // std::cout << "parent?:" << endl;
            // std::cout << (cursor->parent) << endl;

            if (cursor->isLeaf && !(cursor->parent)) { // Should only happen once?
                // std::cout << "once?" << endl;

                Node* newRoot = new Node();
                

                newRoot->isLeaf = false;
                newRoot->size = 1;

                vector<int> newRootKeys; // TODO: make this less ugly
                int promotedKey = right->keys.front();

                // std::cout << "promotedKey:" << endl;
                // std::cout << promotedKey << endl;

                newRootKeys.push_back(promotedKey);
                newRoot->keys = newRootKeys;

                vector<void*> newRootPtrs;
                newRootPtrs.push_back(cursor);
                newRootPtrs.push_back(right);
                newRoot->ptrs = newRootPtrs;
                newRoot->parent = nullptr;
                cursor->parent = newRoot;
                right->parent = newRoot;

                this->root = newRoot;
                return;
            }

            right->parent = cursor->parent;

            // Parent insertion / update
            int promotedKey = right->keys.front();
            insertInternal(promotedKey, cursor->parent, right);
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
            parent->parent = newRoot;

            this->root = newRoot;
        } else { // current node is internal, recurse on promoted key
            insertInternal(promotedKey, parent->parent, parent);
        }
    }
}

vector <Record *> BPTree::searchKeyRange(int minNumVotes, int maxNumVotes) { //ToDo Duplicates & dataBlocksAccessed
    vector <Record *> result;

    //// start timer && initialize values
    auto start = high_resolution_clock::now();
    int indexNodesAccessed = 0;

    Node* root = getRoot();

    ////If root is null, empty tree
    if (root == nullptr){
        std::cout << "ERROR: B+ Tree is Empty!" << std::endl;
        return result; // empty vector
    }

    Node *cursor = root;
    //int dataBlocksAccessed = 0;


    indexNodesAccessed++; // first node - root

    ////traverse until we find the leaf node pertaining the minNumVotes
    while (cursor->isLeaf == false){
        for (int i = 0; i < cursor->size; i++) {
            // std::cout << "HELLO" << endl;
            if (minNumVotes < cursor->keys[i]) {
                cursor = (Node*) cursor->ptrs[i]; // if searchKey < NodeKey, go to LHS Node
                indexNodesAccessed++;
                break;
            }

            if (i == cursor->size - 1) {
                // std::cout << "SPECIAL:" << endl;
                // std::cout << cursor << endl;
                cursor = (Node*) cursor->ptrs[i + 1];
                indexNodesAccessed++;
                break;
            }
        }
    }

    //// traverse leaf node to find the keys
    bool allFound = false;
    while (!allFound){
        for (int i = 0; i < cursor->size; i++) {
            int ptrKey = cursor->keys[i];
            if (ptrKey > maxNumVotes){
                allFound = true;
                break;
            }
            if (ptrKey >= minNumVotes && ptrKey <= maxNumVotes) {
                //void *recordptr = cursor->ptrs[i];
                //result.push_back(recordptr);

                //// handle duplicates TODO
                // here
                LLNode* listNode = (LLNode*) cursor->ptrs[i];
                while (listNode){
                    Record* recordptr = listNode->recordPtr;
                    result.push_back(recordptr);
                    listNode = listNode->next;
                }

            }
        }
        if (!allFound){
            //// Move to the next leaf node
            cursor = (Node*) cursor->ptrs[cursor->size];
            if (!cursor) { // cursor is null, reached last leaf node
                break; // out of while loop
            }
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
    //cout << "The average of �averageRating�s� of the records that are returned: " << avgRating << endl;
    cout << "The running time of the retrieval process: " << runningTime.count() << endl;
    return result;

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

Node* BPTree::getRoot() {
    return this->root;
}

void BPTree::showRoot() {
    Node* root = this->root;
    for (int i = 0; i < root->size; i++) {
        std::cout << root->keys[i] << endl;
    }
}

void BPTree::showChildren() {
    Node* root = this->root;

    for (int i = 0; i < root->ptrs.size(); i++) {
        Node* child = (Node*) root->ptrs[i];
        for (int j = 0; j < child->keys.size(); j++) {
            std::cout << i << endl;
            std::cout << child->keys[j] << endl;
        }
    }
}
