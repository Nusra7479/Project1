#include <vector>
#include <algorithm>
#include <cstring>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

const int BLOCK_SIZE = 200; // Size of each block in bytes
const int DISK_CAPACITY = 200 *1024 * 1024; // 200 MB
const int MAX_BLOCKS = DISK_CAPACITY / BLOCK_SIZE;

// Record as a struct, containing its attributes
struct Record {
    char tconst[10];
    float averageRating;
    int numVotes;
};

// Class to represent disk storage
class Disk {
public:
    Disk() {
        numBlocks = 0;
        curBlock.size=0;
    }

    void addRecord(Record record) {
        //Check if disk capacity is full
        if (numBlocks >= MAX_BLOCKS) {
            cerr << "Error: Disk capacity is full" << endl;
            return;
        }
        // Check if the current block has sufficient space for another record
        if (curBlock.size + sizeof(record) <= BLOCK_SIZE) {
            // If so, append the record to the current block
            curBlock.records.push_back(record);
            curBlock.size += sizeof(record);
        } else {
            // Else, push the current block to the list of blocks and start a new block
            blocks.push_back(curBlock);
            numBlocks++;
            curBlock.records.clear();
            curBlock.size = sizeof(record);
            curBlock.records.push_back(record);
        }
    }

    void finalizeBlocks() {
        // Pushes the final block (if any) to the list of blocks
        if (curBlock.records.size() > 0) {
            blocks.push_back(curBlock);
            numBlocks++;
        }
    }

    void printRecords() {
        cout<<"Printing records..."<<endl;
        for (int i = 0; i < numBlocks; i++) {
            cout<<"Printing block "<<i+1<<endl;
            Block block = blocks[i];
            for (int j = 0; j < block.records.size(); j++) {
                Record record = block.records[j];
                cout << "Record " << record.tconst << ": " << record.averageRating << " (" << record.numVotes << " votes)" << endl;
            }
        }
        cout<<"Records printed!"<<endl;
    }

    void sortRecords() {
        cout<<"Sorting records..."<<endl;
        // Combine all blocks into a single vector
        vector<Record> records;
        for (int i = 0; i < numBlocks; i++) {
            Block block = blocks[i];
            for (int j = 0; j < block.records.size(); j++) {
                records.push_back(block.records[j]);
            }
        }

        // Sort the records by ascending numVotes
        sort(records.begin(), records.end(), [](Record r1, Record r2) {
            return r1.numVotes < r2.numVotes;
        });

        // Clear the blocks and add the sorted records back
        blocks.clear();
        numBlocks = 0;
        curBlock.records.clear();
        curBlock.size = 0;
        for (int i = 0; i < records.size(); i++) {
            addRecord(records[i]);
        }
        cout<<"Records sorted!"<<endl;
    }
    
    void searchKey(int key) { //Does not work
        ////start search
        int numOfRecords = 0;
         for (int i = 0; i < numBlocks; i++) {
            Block block = blocks[i];
            for (int j = 0; j < block.records.size(); j++) {
                Record record = block.records[j];
                if (record.numVotes == key){
                    numOfRecords++;
                }
                if (record.numVotes > key){
                    //// end search
                    cout << "The number of data blocks that would be accessed by a brute-force linear scan method:" << i << endl;
                    cout << "The number of records" << numOfRecords << endl;
                    return;
                }
            }
        }
    }

    vector<Record> searchRecord(int numVotes) { //returns a vector consisting all records with target numVotes
        vector<Record> result;
        for (int i = 0; i < numBlocks; i++) {
            Block block = blocks[i];
            for (int j = 0; j < block.records.size(); j++) {
                Record record = block.records[j];
                if (record.numVotes == numVotes) {
                    result.push_back(record);
                }
            }
        }
        return result;
    }


private:
    // Struct to represent a block
    struct Block {
        vector<Record> records; // Array of records
        int size; // Size of Block in bytes
    };

    vector<Block> blocks;
    Block curBlock;
    int numBlocks;
};





