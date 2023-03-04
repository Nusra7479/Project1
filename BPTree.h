#pragma once
#include "storage.cpp"

#include <vector>
class Node {
    bool isLeaf;
    short int size;
    std::vector<int> keys; // array of keys (numVotes)
    std::vector<void*> ptrs; //non-leaf points to node, leaf points to record (can be linked list!)
    Node* parent; // parent ptr
    friend class BPTree; //allow BPTree to access private and protected members of Node
    // Record ptrs and linked list of Record ptrs

    public:
        Node();
};


class BPTree {
    Node *root;
    int maxKeys; // maximum no of keys in a node
    int nodeCount = 0;
    Disk disk; // the associated disk

    public:
        BPTree(short int mK, Disk dsk);

        void insertInternal(int key, Node *parent, Node *child);
        void insert(int key, Record *recordPtr); //insert record pointer and its key into the BPTree
        void showRoot();
        int getNodeCount();
        int getLevelCount();

        // Retrieval for experiment 3 & 4
        vector<Record *> searchKeyRange(int keyMin, int keyMax);

        // Deletion for experiment 5
        void deleteKey(int key);
        void propagateMin(Node* nodePtr, int min);
        Node* getLeftSibling(Node* nodePtr);
        Node* getRightSibling(Node* nodePtr);
        void deleteInternal(int key, Node* nodePtr, Node* child);

        Node *getRoot();
};

class LLNode {
    Record* recordPtr;
    LLNode* next;
    friend class BPTree;

    public:
        // LLNode();
};





