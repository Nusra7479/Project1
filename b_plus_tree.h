#ifndef B_PLUS_TREE_H_INCLUDED
#define B_PLUS_TREE_H_INCLUDED

class Node {
    bool isLeaf;
    int size;
    int maxKeys; // maximum no of keys in a node
    std::vector<float> keys; // array of keys (numVotes)
    std::vector<void*> ptrs; //non-leaf points to node, leaf points to record
    /////////Programiz
    //int *key, size;
    //Node **ptr;
    /////////Cecilia
    // std::vector<std::shared_ptr<void>> ptrs;     // non-leaf node points to node, leaf node points to vector with shared_ptr of the blocks
    ////////
    friend class BPTree; //allow BPTree to access private and protected members of Node

    public:
        Node(maxKeys): maxKeys(maxKeys) {}
};


class BPTree {
    Node *root;

    public:
        BPTree();

        void setRoot(Node *root);
        void insertInternal(float key, Node *parent, Node *child);
        Node *findParent(Node *root, Node *child);

        void insertKey(float key, Record *recordPtr); //insert record pointer and its key into the BPTree
        void searchKey(float key);
        void display(Node *root);
        Node *getRoot();
};


#endif // B_PLUS_TREE_H_INCLUDED
