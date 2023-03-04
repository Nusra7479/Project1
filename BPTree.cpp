#include <iostream>
#include "BPTree.h"
#include <chrono>
#include <vector>
#include <cassert>

using namespace std::chrono;

// issues:
// 1. exception in searchKeyRange (while traversing leaves?). could be due to insertion -> number of ptrs != number of keys + 1
// 2. duplicate keys in internal nodes in large cases (with whole data.tsv). small cases seem to be ok, so might be issue with insertInternal

Node::Node(){
    vector<int> keyVector;
    keys = keyVector;

    vector<void*> ptrVector;
    ptrs = ptrVector;
}

BPTree::BPTree(short int mK, Disk dsk) {
    maxKeys = mK;
    disk = dsk;
    root = nullptr;
}

void BPTree::insert(int key, Record* recordPtr){
    // std::cout << "Key: " << endl;
    // std::cout << key << endl;

    if (root == nullptr){
        root = new Node();
        root->size=1;
        root->isLeaf = true;
        root->keys.push_back(key);
        root->parent = nullptr;
        nodeCount++;

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
                    LLNode* existingPtr = (LLNode*) cursor->ptrs[i];
                    LLNode* newNode = new LLNode();
                    newNode->next = existingPtr;
                    newNode->recordPtr = recordPtr;
                    cursor->ptrs[i] = newNode;
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
            nodeCount++;

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
                nodeCount++;
                return;
            }

            right->parent = cursor->parent;
            
            assert (cursor->keys.size() + 1 == cursor->ptrs.size());

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

        nodeCount++;

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

            nodeCount++;

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
                LLNode* listNode = (LLNode*) cursor->ptrs[i];
                while (listNode) {
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

// TODO: check edge cases: maybe doesnt matter though
// 1. deletion from the root, esp deleting all elements
void BPTree::deleteKey(int key) {

    // initialise cursor
    Node* cursor = this->root;

    // traverse down to leaf
    while (cursor->isLeaf == false) {
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

    // save oldMin in leaf
    int oldMin = cursor->keys[0];

    // find element to delete
    int i;
    for (i = 0 ; i < cursor->size; i++) {
        int currKey = cursor->keys[i];
        if (key == currKey) {
            break;
        }
    }

    // if leaf is fully traversed and target key still not found, element not in tree: return
    if (cursor->keys[i] != key) {
        return;
    }

    // otherwise, the element is found: delete the element
    cursor->keys.erase(cursor->keys.begin() + i);
    cursor->size--;

    // mark the associated records as deleted
    LLNode* head = (LLNode *) cursor->ptrs[i];
    while (head) {
        Record* currRecord = head->recordPtr;
        this->disk.deleteRecord(currRecord);
        head = head->next;
    }

    // if at least minimum keys, then just need to update parents if leftmost (min) key was deleted
    if (cursor->size >= (this->maxKeys + 1) / 2) {  
        if (i == 0) {
            int newMin = cursor->keys[0];
            propagateMin(cursor->parent, newMin);
        }    
        return;
    }

    // else, the node has less than the minimum keys. try to borrow from siblings. 
    // try left
    Node* leftSibling = getLeftSibling(cursor, oldMin);
    if (leftSibling && leftSibling->size > (this->maxKeys + 1) / 2) { // short circuit evaluation, safe
        
        // get key and pointer
        int borrowedKey = leftSibling->keys.back(); // last key
        leftSibling->keys.pop_back();
        LLNode* borrowedPtr = (LLNode*) leftSibling->ptrs[leftSibling->size - 1]; // 2nd last pointer
        leftSibling->ptrs.erase(leftSibling->ptrs.begin() + leftSibling->size - 1);

        // insert
        cursor->keys.insert(cursor->keys.begin(), borrowedKey); // from left -> smaller than all of cursor's keys
        cursor->ptrs.insert(cursor->ptrs.begin(), borrowedPtr); // 1st pointer

        // propagate new min
        propagateMin(cursor->parent, borrowedKey);
        return;

    }

    // try right
    Node* rightSibling = getRightSibling(cursor, oldMin);
    if (rightSibling && rightSibling->size > (this->maxKeys + 1) / 2) { // short circuit evaluation, safe
        
        // get key and pointer
        int borrowedKey = rightSibling->keys.front(); // 1st key
        rightSibling->keys.erase(rightSibling->keys.begin());
        LLNode* borrowedPtr = (LLNode*) rightSibling->ptrs[0]; // 1st pointer
        rightSibling->ptrs.erase(rightSibling->ptrs.begin());

        // insert
        cursor->keys.push_back(borrowedKey); // from right -> larger than all of cursor's keys
        cursor->ptrs.insert(cursor->ptrs.end() - 1, borrowedPtr); // 2nd last pointer (ptr to next node is last)

        // propagate new min
        propagateMin(rightSibling->parent, rightSibling->keys.front());
        return;

    }

    // if can't borrow from sibling, then merge with sibling. always possible if can't borrow bec sibling would be min. 
    // i guess u can choose either sibling then
    // merging with sibling causes 1 key to be deleted in the parent (which one?) deleteInternal?
    // kind of same procedure: check siblings for same parent and > min, borrow if can
    // if can't, then merge, recurse up
    // TODO

    // final suffering: if delete the last key in the root, then reassign root to its only child (will be only child for sure)
    // TODO

}

// updates minimum values upwards in internal nodes after deletion.
// this function takes in min, which is the new minimum value of some subtree pointed to by *nodePtr's ptrs vector
// the function either updates the appropriate key in the current node or calls itself to update the current node's parent
void BPTree::propagateMin(Node* nodePtr, int min) {

    // if pointer is null, then previous iteration was on root and no changes need to be made
    if (!nodePtr) {
        return;
    }

    // if min is less than the first key, then the current node's keys do not need to be updated.
    // the min of the entire subtree rooted at the current node has increased, and the parent must be updated. 
    if (min < nodePtr->keys[0]) {
        propagateMin(nodePtr->parent, min);
    }

    // iterate through parent keys. if min < currKey, then the element to the left of currKey should be oldMin. 
    int i;
    for (i = 1; i < nodePtr->size + 1; i++) {
        if (i == nodePtr->size) { // reached the end of keys, index is left of end of keys i.e. last element
            break;
        }
        int currKey = nodePtr->keys[i];
        if (min < currKey) {
            break;
        }
    }
    int index = i - 1;

    // if min is different from old value, update. if not, then no change made. // should always be different
    nodePtr->keys[index] = min; 
    return;

}

// function to get the left sibling of a node if it exists. min should be old min before deletion.
// **if it doesn't exist, then returns nullptr**
Node* BPTree::getLeftSibling(Node* nodePtr, int min) {
    Node* parentNode = nodePtr->parent;
    if (!parentNode) { // if no parent node, then no siblings 
        return nullptr;
    }
    if (min < parentNode->keys[0]) { // leftmost child, no left sibling
        return nullptr;        
    }
    for (int i = 0; i < parentNode->size; i++) {
        if (min == parentNode->keys[i]) {
            return (Node*) parentNode->ptrs[i]; // nodePtr is (i+1)th
        }
    }
    // something has gone wrong
    std::cout << "getLeftSibling: wrong min possibly provided" << endl;
    return nullptr;
}

// function to get the right sibling of a node if it exists. min should be old min before deletion.
// **if it doesn't exist, then returns nullptr**
Node* BPTree::getRightSibling(Node* nodePtr, int min) {
    Node* parentNode = nodePtr->parent;
    if (!parentNode) { // if no parent node, then no siblings 
        return nullptr;
    }
    if (min == parentNode->keys.back()) { // rightmost chlid, no right sibling
        return nullptr;
    }
    for (int i = 0; i < parentNode->size-1; i++) {
        if (min == parentNode->keys[i]) {
            return (Node*) parentNode->ptrs[i+2]; // nodePtr is (i+1)th
        }
    }
    // something has gone wrong
    std::cout << "getRightSibling: wrong min possibly provided" << endl;
    return nullptr;
}

Node* BPTree::getRoot() {
    return this->root;
}

// Debug function to show root keys
void BPTree::showRoot() {
    Node* root = this->root;
    for (int i = 0; i < root->size; i++) {
        std::cout << root->keys[i] << ", ";
    }
    std::cout << endl;
}

// Debug function to show root's children keys
void BPTree::showChildren() {
    Node* root = this->root;
    for (int i = 0; i < root->ptrs.size(); i++) {
        std::cout << "Child " << i << ": ";
        Node* child = (Node*) root->ptrs[i];
        for (int j = 0; j < child->keys.size(); j++) {
            std::cout << child->keys[j] << ", ";
        }
        std::cout << endl;
    }
    std::cout << endl;
}

int BPTree::getNodeCount() {
    return nodeCount;
}

int BPTree::getLevelCount() {
    int levels = 1;
    Node* cursor = root;
    while (!cursor->isLeaf) {
        cursor = (Node*) cursor->ptrs[0];
        levels++;
    }
    return levels;
}
