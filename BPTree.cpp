#include <iostream>
#include "BPTree.h"
#include <chrono>
#include <vector>
#include <queue>
#include <cassert>

using namespace std::chrono;

Node::Node(){
    vector<int> keyVector;
    keys = keyVector;

    vector<void*> ptrVector;
    ptrs = ptrVector;
}

BPTree::BPTree(short int mK, Disk dsk) {
    maxKeys = mK;
    root = nullptr;
    nodeCount = 0;
    disk = dsk;
}

void BPTree::insert(int key, Record* recordPtr){
    bool inserted = false;
    if (this->root == NULL) {
        Node* root = new Node();
        nodeCount++;
        root->size = 1;
        root->isLeaf = true;
        root->keys.push_back(key);
        root->parent = nullptr;

        LLNode* newNode = new LLNode();
        newNode->recordPtr = recordPtr;
        newNode->next = nullptr;

        root->ptrs.push_back(newNode);
        root->ptrs.push_back(nullptr);
        this->root = root;
    } else {
        Node* curr = this->root;

        while (curr->isLeaf == false) {
            for (int i = 0; i < curr->size; i++) {
                if (key < curr->keys[i]) {
                    curr = (Node*) curr->ptrs[i];
                    break;
                }
                if (i == curr->size - 1) {
                    curr = (Node*) curr->ptrs[i + 1];
                    break;
                }
            }
        }

        for (int i = 0; i < curr->size; i++) {
            if (key > curr->keys[i]) {
                continue;
            }

            // Duplicate: insert into LL
            if (key == curr->keys[i]) {
                LLNode* currentNode = (LLNode*) curr->ptrs[i];
                while (currentNode->next) {
                    currentNode = currentNode->next;
                }
                LLNode* newLLNode = new LLNode();
                newLLNode->next = nullptr;
                newLLNode->recordPtr = recordPtr;
                currentNode->next = newLLNode;
                inserted = true;
                break;
            } else { // Make LL of length 1
                LLNode* newLLNode = new LLNode();
                newLLNode->recordPtr = recordPtr;
                newLLNode->next = nullptr;

                curr->keys.insert(curr->keys.begin() + i, key);
                curr->ptrs.insert(curr->ptrs.begin() + i, newLLNode);
                curr->size++;

                inserted = true;
                break;
            }
        }

        if (!inserted) {
            LLNode* newLLNode = new LLNode();
            newLLNode->recordPtr = recordPtr;
            newLLNode->next = nullptr;

            curr->keys.insert(curr->keys.begin() + curr->size, key);
            curr->ptrs.insert(curr->ptrs.begin() + curr->size, newLLNode);
            curr->size++;

            inserted = true;
        }

        if (curr->size > this->maxKeys) {
            int split = (curr->size + 1)/ 2;
            vector<int> leftKeys(curr->keys.begin(), curr->keys.begin() + split);
            vector<int> rightKeys(curr->keys.begin() + split, curr->keys.end());

            int rightSize = rightKeys.size();
            int leftSize = leftKeys.size();

            Node* newLeaf = new Node();
            nodeCount++;
            newLeaf->isLeaf = true;
            newLeaf->size = rightSize;
            newLeaf->keys = rightKeys;
            newLeaf->parent = curr->parent;

            vector<void*> leftPtrs(curr->ptrs.begin(), curr->ptrs.begin() + split);
            vector<void*> rightPtrs(curr->ptrs.begin() + split, curr->ptrs.end());
            leftPtrs.push_back(newLeaf);
            newLeaf->ptrs = rightPtrs;

            curr->ptrs = leftPtrs;
            curr->keys = leftKeys;
            curr->size = leftSize;

            if (curr == this->root) {
                Node* newRoot = new Node();
                nodeCount++;
                newRoot->isLeaf = false;
                newRoot->size = 1;

                vector<int> newRootKeys;
                newRootKeys.push_back(newLeaf->keys.front());
                newRoot->keys = newRootKeys;

                vector<void*> newRootPtrs;
                newRootPtrs.push_back(curr);
                newRootPtrs.push_back(newLeaf);
                newRoot->ptrs = newRootPtrs;
                newRoot->parent = nullptr;
                curr->parent = newRoot;
                newLeaf->parent = newRoot;
                this->root= newRoot;
            } else {
                insertInternal(newLeaf->keys.front(), curr->parent, newLeaf);
            }
        }
    }
}

void BPTree::insertInternal(int key, Node* curr, Node* child) {
    bool inserted = false;
    for (int i = 0; i < curr->size; i++) {
        if (key > curr->keys[i]) {
            continue;
        }
        curr->keys.insert(curr->keys.begin() + i, key);
        curr->ptrs.insert(curr->ptrs.begin() + i + 1, child);
        curr->size++;
        inserted = true;
        break;
    }
    if (!inserted) {
        curr->keys.insert(curr->keys.begin() + curr->size, key);
        curr->ptrs.insert(curr->ptrs.begin() + curr->size + 1, child);
        curr->size++;
        inserted = true;
    }
    if (curr->size > this->maxKeys) {
        Node* newInternal = new Node();
        nodeCount++;
        newInternal->isLeaf = false;
        int split = curr->size / 2;
        int promotedKey = curr->keys[split];

        // promotedKey is removed.
        vector<int> leftKeys(curr->keys.begin(), curr->keys.begin() + split);
        vector<int> rightKeys(curr->keys.begin() + split + 1, curr->keys.end());

        vector<void*> leftPtrs(curr->ptrs.begin(), curr->ptrs.begin() + split + 1);
        vector<void*> rightPtrs(curr->ptrs.begin() + split + 1, curr->ptrs.end());

        newInternal->keys = rightKeys;
        newInternal->ptrs = rightPtrs;
        newInternal->size = rightKeys.size();
        newInternal->parent = curr->parent;

        curr->keys = leftKeys;
        curr->ptrs = leftPtrs;
        curr->size = leftKeys.size();

        for (int i = 0; i < newInternal->size + 1; i++) {
            Node* child = (Node*) newInternal->ptrs[i];
            child->parent = newInternal;
        }

        if (curr == this->root) {
            Node* newRoot = new Node();
            nodeCount++;
            newRoot->isLeaf = false;
            newRoot->size = 1;
            newRoot->keys.push_back(promotedKey);
            newRoot->ptrs.push_back(curr);
            newRoot->ptrs.push_back(newInternal);
            newRoot->parent = nullptr;
            newInternal->parent = newRoot;
            curr->parent = newRoot;
            this->root = newRoot;
        } else {
            insertInternal(promotedKey, curr->parent, newInternal);
        }
    }
}


vector <Record *> BPTree::searchKeyRange(int minNumVotes, int maxNumVotes) { //ToDo Duplicates & dataBlocksAccessed
    vector <Record *> result;

    // start timer && initialize values
    auto start = high_resolution_clock::now();
    int indexNodesAccessed = 0;

    Node* root = getRoot();

    //If root is null, empty tree
    if (root == nullptr){
        std::cout << "ERROR: B+ Tree is Empty!" << std::endl;
        return result; // empty vector
    }

    Node *cursor = root;
    //int dataBlocksAccessed = 0;


    indexNodesAccessed++; // first node - root

    //traverse until we find the leaf node pertaining the minNumVotes
    while (cursor->isLeaf == false){
        for (int i = 0; i < cursor->size; i++) {
            if (minNumVotes < cursor->keys[i]) {
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

    // traverse leaf node to find the keys
    bool allFound = false;
    while (!allFound){
        for (int i = 0; i < cursor->size; i++) {
            int ptrKey = cursor->keys[i];
            if (ptrKey > maxNumVotes){
                allFound = true;
                break;
            }
            if (ptrKey >= minNumVotes && ptrKey <= maxNumVotes) {
                // find the relevant keys and the pointer pointing to the records
                int j = 0;
                LLNode* listNode = (LLNode*) cursor->ptrs[i];
                while (listNode){
                    Record* recordptr = listNode->recordPtr;
                    result.push_back(recordptr);
                    listNode = listNode->next;
                    j++;
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

    //end timer
    auto stop = high_resolution_clock::now();
    auto runningTime = duration_cast<microseconds>(stop - start);


    // print results
    cout << "The number of index nodes the process accesses: " << indexNodesAccessed << endl;
    if (runningTime.count() == 0){
        auto runningTime = duration_cast<nanoseconds>(stop - start);
        cout << "The running time of the retrieval process (B+ Tree): " << runningTime.count() << " nanoseconds" << endl;
    }
    else
        cout << "The running time of the retrieval process (B+ Tree): " << runningTime.count() << " microseconds" << endl;
    return result;

    }

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

    assert (cursor->isLeaf);

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

    // also delete the associated pointer
    cursor->ptrs.erase(cursor->ptrs.begin() + i); // ith key -> ith ptr

    // check if this is both a leaf and a root. if so, then min is 1. if min is < 1 (0), can delete root
    if (!cursor->parent) {
        if (cursor->size == 0) {
            this->root = nullptr;
            nodeCount = 0;
        }
        return;
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
    // try left sibling first
    Node* leftSibling = getLeftSibling(cursor);
    if (leftSibling && leftSibling->size > (this->maxKeys + 1) / 2) { // short circuit evaluation, safe

        // get key and pointer
        int borrowedKey = leftSibling->keys.back(); // last key
        leftSibling->keys.pop_back();
        leftSibling->size--;
        LLNode* borrowedPtr = (LLNode*) leftSibling->ptrs[leftSibling->size - 1]; // 2nd last pointer
        leftSibling->ptrs.erase(leftSibling->ptrs.begin() + leftSibling->size - 1);

        // insert
        cursor->keys.insert(cursor->keys.begin(), borrowedKey); // from left -> smaller than all of cursor's keys
        cursor->size++;
        cursor->ptrs.insert(cursor->ptrs.begin(), borrowedPtr); // 1st pointer

        // propagate new min
        propagateMin(cursor->parent, borrowedKey); // of current node: new min is borrowed key
        // cursor->parent->keys[leftSib] = cursor->keys[0];
        return;

    }

    // try right sibling next
    Node* rightSibling = getRightSibling(cursor);
    if (rightSibling && rightSibling->size > (this->maxKeys + 1) / 2) { // short circuit evaluation, safe

        // get key and pointer
        int borrowedKey = rightSibling->keys.front(); // 1st key
        rightSibling->keys.erase(rightSibling->keys.begin());
        rightSibling->size--;
        LLNode* borrowedPtr = (LLNode*) rightSibling->ptrs[0]; // 1st pointer
        rightSibling->ptrs.erase(rightSibling->ptrs.begin());

        // insert
        cursor->keys.push_back(borrowedKey); // from right -> larger than all of cursor's keys
        cursor->ptrs.insert(cursor->ptrs.begin() + cursor->size, borrowedPtr); // 2nd last pointer (ptr to next node is last)
        cursor->size++;

        // propagate new min
        if (i == 0) { // of current node if leftmost key was deleted
            int newMin = cursor->keys[0];
            propagateMin(cursor->parent, newMin);
        }
        propagateMin(cursor->parent, rightSibling->keys.front()); // of the right sibling borrowed from
        return;

    }

    // if can't borrow from sibling, then merge with sibling. always possible if can't borrow bec sibling would be min.
    // merging with sibling causes 1 key to be deleted in the parent
    // (which one? -> the one in between the 2 pointers: if originalRightMin == key[i], then delete key[i])

    // try left. after merging, the min of merged node is the min of the left node, so no propagation needed
    if (leftSibling) { // cursor is right node in merging

        int rightMin = cursor->keys[0];

        // update min (should stop in direct parent only) for consistency. however key is deleted anyway.
        if (i == 0) {
            propagateMin(cursor->parent, rightMin);
        }

        // shift everything to left child
        leftSibling->keys.insert(leftSibling->keys.end(), cursor->keys.begin(), cursor->keys.end());
        leftSibling->ptrs.pop_back();
        leftSibling->ptrs.insert(leftSibling->ptrs.end(), cursor->ptrs.begin(), cursor->ptrs.end());
        leftSibling->size = leftSibling->keys.size();

        nodeCount--;

        deleteInternal(rightMin, cursor->parent, cursor); // right child ptr is passed
        return;

    }

    // try right. if this is reached, then cursor is a leftmost leaf node.
    if (rightSibling) { // cursor is left node in merging

        int rightMin = rightSibling->keys[0];

        // update min (should go to at least grandparent)
        if (i == 0) { // if leftmost (min) key, then need to update min
            int newMin = cursor->keys[0];
            propagateMin(cursor->parent, newMin);
        }

        // shift everything to left child
        cursor->keys.insert(cursor->keys.end(), rightSibling->keys.begin(), rightSibling->keys.end());
        cursor->ptrs.pop_back(); // remove left node's ptr to next node
        cursor->ptrs.insert(cursor->ptrs.end(), rightSibling->ptrs.begin(), rightSibling->ptrs.end());
        cursor->size = cursor->keys.size();

        nodeCount--;

        deleteInternal(rightMin, cursor->parent, rightSibling); // right child ptr is passed
        return;

    }

    // if fail both sibling checks, then we must be at root
    assert (!cursor->parent);
    return;

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
        return;
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

    nodePtr->keys[index] = min;
    return;

}

// function to get the left sibling of a node if it exists.
// **if it doesn't exist, then returns nullptr**
Node* BPTree::getLeftSibling(Node* nodePtr) {
    Node* parentNode = nodePtr->parent;
    if (!parentNode) { // if no parent node, then no siblings
        return nullptr;
    }
    for (int i = 1; i < parentNode->size+1; i++) {
        if (nodePtr == parentNode->ptrs[i]) {
            return (Node*) parentNode->ptrs[i-1];
        }
    }
    return nullptr; // node is leftmost child, no left siblings
}

// function to get the right sibling of a node if it exists.
// **if it doesn't exist, then returns nullptr**
Node* BPTree::getRightSibling(Node* nodePtr) {
    Node* parentNode = nodePtr->parent;
    if (!parentNode) { // if no parent node, then no siblings
        return nullptr;
    }
    for (int i = 0; i < parentNode->size; i++) {
        if (nodePtr == parentNode->ptrs[i]) {
            return (Node*) parentNode->ptrs[i+1];
        }
    }
    return nullptr; // node is rightmost child, no siblings
}

// this function deletes a key and a pointer from an internal node
void BPTree::deleteInternal(int key, Node* nodePtr, Node* child) {

    // find key
    int i;
    for (i = 0 ; i < nodePtr->size; i++) {
        int currKey = nodePtr->keys[i];
        if (key == currKey) {
            break;
        }
    }

    // if leaf is fully traversed and target key still not found, element not in node: something has gone wrong
    if (nodePtr->keys[i] != key) {
        std::cout << "deleteInternal: wrong key possibly provided: " << key << endl;
        return;
    }

    // delete key and key's right ptr
    nodePtr->keys.erase(nodePtr->keys.begin() + i);
    nodePtr->size--;
    nodePtr->ptrs.erase(nodePtr->ptrs.begin() + i + 1);

    // if this is root, then min is 1. if min is < 1 (0), can delete root and newRoot is child
    if (!nodePtr->parent) {
        if (nodePtr->size == 0) {
            this->root = (Node*) nodePtr->ptrs[0];
            this->root->parent = nullptr;
            nodeCount--;
        }
        return;
    }

    // if at least minimum keys, done
    if (nodePtr->size >= this->maxKeys / 2) {
        return;
    }

    // else, try to borrow from sibling
    Node* leftSibling = getLeftSibling(nodePtr);
    Node* rightSibling = getRightSibling(nodePtr);

    // try left sibling first
    if (leftSibling && leftSibling->size > this->maxKeys / 2) { // short circuit evaluation, safe

        // get key and pointer
        int borrowedKey = leftSibling->keys.back(); // last key
        leftSibling->keys.pop_back();
        void* borrowedPtr = (void*) leftSibling->ptrs[leftSibling->size]; // last pointer
        leftSibling->ptrs.pop_back();

        // insert
        nodePtr->keys.insert(nodePtr->keys.begin(), borrowedKey); // from left -> smaller than all of cursor's keys
        nodePtr->ptrs.insert(nodePtr->ptrs.begin(), borrowedPtr); // 1st pointer
        return;

    }

    // try right sibling next
    if (rightSibling && rightSibling->size > this->maxKeys / 2) { // short circuit evaluation, safe

        // get key and pointer
        int borrowedKey = leftSibling->keys.front(); // first key
        rightSibling->keys.erase(rightSibling->keys.begin());
        void* borrowedPtr = (void*) rightSibling->ptrs[0]; // last pointer
        rightSibling->ptrs.erase(rightSibling->ptrs.begin());

        // insert
        nodePtr->keys.push_back(borrowedKey); // from left -> smaller than all of cursor's keys
        nodePtr->ptrs.push_back(borrowedPtr); // 1st pointer
        return;

    }

    // if can't borrow, merge with sibling // !! in internal node, must pull down 1 key from the parent
    // try left first
    if (leftSibling) { // curr node is right node in merge

        Node* parentPtr = nodePtr->parent;
        int j;
        for (j = 1; j < parentPtr->size+1; j++) {
            if (parentPtr->ptrs[j] == nodePtr) {
                break;
            }
        }
        int demotedKey = parentPtr->keys[j - 1];

        // shift everything to left child
        leftSibling->keys.push_back(demotedKey);
        leftSibling->keys.insert(leftSibling->keys.end(), nodePtr->keys.begin(), nodePtr->keys.end());
        leftSibling->ptrs.insert(leftSibling->ptrs.end(), nodePtr->ptrs.begin(), nodePtr->ptrs.end());
        leftSibling->size = leftSibling->keys.size();

        nodeCount--;

        // update the parent pointers of the right node's children
        for (int i = 0; i < nodePtr->size + 1; i++) {
            ((Node*) nodePtr->ptrs[i])->parent = leftSibling;
        }

        // recurse on parent
        deleteInternal(demotedKey, parentPtr, leftSibling);
        return;

    }

    // try right next
    if (rightSibling) { // curr node is left node in merge
        Node* parentPtr = nodePtr->parent;
        int j;
        for (j = 0; j < parentPtr->size; j++) {
            if (parentPtr->ptrs[j] == nodePtr) {
                break;
            }
        }
        int demotedKey = parentPtr->keys[j];

        // shift everything to left child
        nodePtr->keys.push_back(demotedKey);
        nodePtr->keys.insert(nodePtr->keys.end(), rightSibling->keys.begin(), rightSibling->keys.end());
        nodePtr->ptrs.insert(nodePtr->ptrs.end(), rightSibling->ptrs.begin(), rightSibling->ptrs.end());
        nodePtr->size = nodePtr->keys.size();

        nodeCount--;

        // update the parent pointers of the right node's children
        for (int i = 0; i < nodePtr->size + 1; i++) {
            ((Node*) rightSibling->ptrs[i])->parent = nodePtr;
        }

        // recurse on parent
        deleteInternal(demotedKey, parentPtr, nodePtr);
        return;

    }
}

Node* BPTree::getRoot() {
    return this->root;
}

// Debug function to show root keys
void BPTree::showRoot() {
    Node* root = this->root;
    for (int i = 0; i < root->size; i++) {
        std::cout << root->keys[i] << " ";
    }
    std::cout << endl;
}



int BPTree::getNodeCount() {
    return nodeCount;
}

int BPTree::getLevelCount() {
    if (!this->root) {
        return 0;
    }
    int levels = 1;
    Node* cursor = root;
    while (!cursor->isLeaf) {
        cursor = (Node*) cursor->ptrs[0];
        levels++;
    }
    levels++; // must count leaf level
    return levels;
}

