#include <vector>
#include <algorithm>
#include <cstring>
#include <fstream>
#include <sstream>
#include <iostream>
#include <chrono>

using namespace std;
using namespace std::chrono;

const int BLOCK_SIZE = 200; // Size of each block in bytes
const int DISK_CAPACITY = 200 *1024 * 1024; // 200 MB
const int MAX_BLOCKS = DISK_CAPACITY / BLOCK_SIZE;

// Record as a struct, containing its attributes
struct Record {
    char tconst[10];
    float averageRating;
    int numVotes;
    bool deleted=false;
};

// Class to represent disk storage
class Disk {
public:
    Disk() {
        numBlocks = 0;
        curBlock.size=0;
    }

    vector<Record*> getAllRecords() {
        vector<Record*> result;
        for (int i = 0; i < numBlocks; i++) {
            Block block = blocks[i];
            for (int j = 0; j < block.records.size(); j++) {
                Record record = block.records[j];
                if (!record.deleted) {
                    result.push_back(&(blocks[i].records[j]));
                }
            }
        }
        return result;
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

    void printRecords() { //only prints undeleted records
        cout<<"Printing records..."<<endl;
        for (int i = 0; i < numBlocks; i++) {
            cout<<"Printing block "<<i+1<<endl;
            Block block = blocks[i];
            for (int j = 0; j < block.records.size(); j++) {
                Record record = block.records[j];
                if(!record.deleted){
                    cout << "Record " << record.tconst << ": " << record.averageRating << " (" << record.numVotes << " votes)" << endl;
                }
            }
        }
        cout<<"Records printed!"<<endl;
    }

    int numberOfRecords(){
        int numRecords=0;
        for (int i = 0; i < numBlocks; i++) {
            Block block = blocks[i];
            for (int j = 0; j < block.records.size(); j++) {
                Record record = block.records[j];
                if(!record.deleted){
                    numRecords++;
                }
            }
        }
        return numRecords;
    }

    int sizeOfRecord(){
        return sizeof(Record);
    }

    int numberOfRecordsPerBlock(){
        return blocks[0].records.size();
    }

    int numberOfBlocksUsed(){
        return blocks.size();
    }

    void sortRecords() { //i left it to sort deleted records as well cause we aren't rlly deleting it anws?
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

    void searchKey(int minKey, int maxKey) { //added to check if record has been deleted
        auto start = high_resolution_clock::now();
        ////start search
        int numOfRecords = 0;
         for (int i = 0; i < numBlocks; i++) {
            Block block = blocks[i];
            for (int j = 0; j < block.records.size(); j++) {
                Record record = block.records[j];
                if (record.numVotes >= minKey && record.numVotes <= maxKey && !record.deleted){
                    numOfRecords++;
                }
                if (record.numVotes > maxKey){
                    //// end search
                    int numDataBlocks = i;
                    if (j == 0)
                        numDataBlocks = i-1;

                    //// stop timer & print results
                    auto stop = high_resolution_clock::now();
                    auto runningTime = duration_cast<microseconds>(stop - start);
                    cout << "The number of data blocks that would be accessed by a brute-force linear scan method: " << numDataBlocks << endl;
                    cout << "The running time of the retrieval process (brute-force linear scan): " << runningTime.count() << " ms" << endl;
                    cout << "The number of records (brute-force linear scan): " << numOfRecords << endl;
                    return;
                }
            }
        }
    }

    vector<Record> searchRecord(int minNumVotes, int maxNumVotes) { //returns a vector consisting all records with target numVotes
        auto start = high_resolution_clock::now();

        int i;
        vector<Record> result;
        for (i = 0; i < numBlocks; i++) {
            Block block = blocks[i];
            for (int j = 0; j < block.records.size(); j++) {
                Record record = block.records[j];
                if (record.numVotes >= minNumVotes && record.numVotes <= maxNumVotes && !record.deleted) {
                    result.push_back(record);
                }
            }
        }

        auto stop = high_resolution_clock::now();
        auto runningTime = duration_cast<microseconds>(stop - start);

        cout << "The number of data blocks that would be accessed by a brute-force linear scan method: " << i << endl;
        cout << "The running time of the retrieval process (brute-force linear scan): " << runningTime.count() << " ms" << endl;
        cout << "The number of records (brute-force linear scan): " << result.size() << endl;
        return result;
    }

    int getDiskIO(vector<Record*> b_targets){ //no need to check if record is deleted because it will be accessed anws
        int dataBlocksAccessed;
        bool terminateLoop = false;
        for (int i = 0; i < numBlocks; i++) {
            for (int j = 0; j < blocks[i].records.size(); j++) {
                for (int k = 0; k < b_targets.size(); k++){
                     if (b_targets[k]->tconst == blocks[i].records[j].tconst){
                        dataBlocksAccessed++;
                        terminateLoop = true;
                        break;
                    }
                }
                if (terminateLoop) break;
            }
            terminateLoop = false;
        }
        cout << "The number of data blocks the process accesses: "<< dataBlocksAccessed << endl;
        return dataBlocksAccessed;
    }

    void deleteRecord(Record* recordPtr) {
        // check if record has already been deleted
        if (recordPtr->deleted==true) {
            cerr << "Error: Record not found" << endl;
            return;
        }

        // Else, we mark the record as deleted
        recordPtr->numVotes = -1;
        recordPtr->deleted = true;

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





