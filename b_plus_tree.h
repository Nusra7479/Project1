#ifndef B_PLUS_TREE_H_INCLUDED
#define B_PLUS_TREE_H_INCLUDED
/*
#include <vector>
class Node {
    bool isLeaf;
    int size;
    int maxKeys; // maximum no of keys in a node
    std::vector<int> keys; // array of keys (numVotes)
    std::vector<void*> ptrs; //non-leaf points to node, leaf points to record
    /////////Programiz
    //int *key, size;
    //Node **ptr;
    /////////Cecilia
    // std::vector<std::shared_ptr<void>> ptrs;     // non-leaf node points to node, leaf node points to vector with shared_ptr of the blocks
    ////////
    friend class BPTree; //allow BPTree to access private and protected members of Node

    public:
        Node(int maxKeys): maxKeys(maxKeys) {}
};


class BPTree {
    Node *root;

    public:
        BPTree();

        void setRoot(Node *root);
        void insertInternal(int key, Node *parent, Node *child);
        Node *findParent(Node *root, Node *child);
        //void insertKey(int key, Record *recordPtr, int blockIndex); //insert record pointer and its key into the BPTree

        //Retrieval
        void searchKey(int key); // For experiment 3
        void searchKeyRange(int keyMin, int keyMax); // For experiment 4


        void display(Node *root);
        Node *getRoot();
};

*/
#endif // B_PLUS_TREE_H_INCLUDED


