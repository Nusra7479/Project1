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
    int nodeCount;

    public:
        BPTree();
        BPTree(short int mK);


        void setRoot(Node *root);
        void insertInternal(int key, Node *parent, Node *child);
        Node *findParent(Node *root, Node *child);
        void insert(int key, Record *recordPtr); //insert record pointer and its key into the BPTree
        void llInsert(int key);
        void showRoot();
        void showChildren();
        int getNodeCount();
        int getLevelCount();


        //Retrieval for experiment 3 & 4
        //vector<void *> searchKey(int key);
        vector<Record *> searchKeyRange(int keyMin, int keyMax);

        void display(Node *root);
        Node *getRoot();
};

class LLNode {
    Record* recordPtr;
    LLNode* next;
    friend class BPTree;

    public:
        // LLNode();
};





