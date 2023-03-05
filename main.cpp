#include <iostream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <fstream>
#include <sstream>
#include "BPTree.h"

using namespace std;

// Reads TSV file and saves it to Disk
void readTSVFile(string filename, Disk& disk, BPTree& bptree) {
    ifstream infile(filename);
    if (!infile) {
        cerr << "Error: unable to open file " << filename << endl;
        return;
    }
    string line;
    getline(infile, line); // Skip header
    while (getline(infile, line)) {
        stringstream ss(line);
        string tconst;
        float averageRating;
        int numVotes;
        ss >> tconst >> averageRating >> numVotes;
        Record record;
        strncpy(record.tconst, tconst.c_str(), sizeof(record.tconst));
        record.averageRating = averageRating;
        record.numVotes = numVotes;
        disk.addRecord(record);
    }
    disk.finalizeBlocks();
    infile.close();
}


void printRecords(const vector<Record>& records) {
    cout << "Printing records..." << endl;
    for (const Record& record : records) {
        cout << "Record " << record.tconst << ": " << record.averageRating << " (" << record.numVotes << " votes)" << endl;
    }
    cout << "Records printed!" << endl;
}

float getAvgRating (vector<Record*> b_targets){
    float totalRating;
    for (int i=0; i< b_targets.size(); i++){
        totalRating = totalRating + b_targets[i]->averageRating;
    }
    float average = totalRating/b_targets.size();
    cout << "The average of averageRatings of the records that are returned: " << average << endl;
    cout << "The number of records (B+ Tree): " << b_targets.size() << endl;
    return average;
}

int main()
{
    short int n = 15;
    bool sorted = true; // sorted is true since we are implementing the storage sequentially

    cout << "------------------------ Storage aspects and testing ------------------------" <<endl;
    Disk disk;
    BPTree bpTree = BPTree(n, disk);

    // Read TSV to disk
    string filename = "data.tsv";
    readTSVFile(filename, disk, bpTree);
    disk.countNumOfRecordsIncludingDeleted();

    // Sort records
    if (sorted)
         disk.sortRecords();
    disk.countNumOfRecordsIncludingDeleted();

    // Get all records on disk for insertion in BPTree
    vector<Record*> allRecordPointers = disk.getAllRecords();
    cout<<"Number of records in the vector: "<<allRecordPointers.size()<<endl; //just to show that indeed all record has been added
    for (int i = 0; i < allRecordPointers.size(); i++) {
        Record* recordPtr = allRecordPointers[i];
        bpTree.insert(recordPtr->numVotes, recordPtr);
    }

    cout << "------------------------ Experiment 1 ------------------------" <<endl;
    cout<<"Total number of records: "<<disk.numberOfRecords()<<endl;
    cout<<"Size of a record: "<<disk.sizeOfRecord()<<endl;
    cout<<"Number of records per block: "<<disk.numberOfRecordsPerBlock()<<endl;
    cout<<"Number of blocks for storing the data: "<<disk.numberOfBlocksUsed()<<endl;
    cout << endl;

    cout << "------------------------ Experiment 2 ------------------------" <<endl;
    cout << "Parameter n of the B+ tree: " << n << endl;
    cout << "Number of nodes of the B+ tree: " << bpTree.getNodeCount() << endl;
    cout << "Number of levels of the B+ tree: " << bpTree.getLevelCount() << endl;
    cout << "Content of the root node: ";
    bpTree.showRoot();
    cout << endl;

    cout << "------------------------ Experiment 3 ------------------------" <<endl; //Requires Testing
    //Search in B+ Tree
    vector<Record*> b_targets3;
    b_targets3 = bpTree.searchKeyRange(500, 500);
    disk.getDiskIO(b_targets3);
    getAvgRating(b_targets3);
    cout << endl;
    //Search in Storage
    if (sorted)
        disk.searchKey(500, 500);
    else{
        vector<Record> targets3;
        targets3=disk.searchRecord(500, 500);
        //printRecords(targets3);
    }
    cout << endl;
    cout << "------------------------ Experiment 4 ------------------------" <<endl; //Requires Testing
    //Search in B+ Tree
    vector<Record*> b_targets4;
    b_targets4 = bpTree.searchKeyRange(30000, 40000);
    disk.getDiskIO(b_targets4);
    getAvgRating(b_targets4);
    cout << endl;
    //Search in Storage
    if (sorted)
        disk.searchKey(30000, 40000);
    else{
        vector<Record> targets4;
        targets4=disk.searchRecord(30000, 40000);
        //printRecords(targets4);
    }
    cout << endl;
    cout << "------------------------ Experiment 5 ------------------------" <<endl;
    //Delete in Storage
    vector<Record> targets5;
    if (sorted){
        targets5=disk.searchKey(1000, 1000);
    }
    else{

        targets5=disk.searchRecord(1000, 1000);
        //printRecords(targets5);
    }
    // Do not delete records yet, delete using bpTree (running Time and data blocks accessed for deletion ver similar to that of retirval)
    // Comment out below portion to delete the keys by brute-force linear scan
    //
    //for (auto& r : targets5)
    //   disk.deleteRecord(&r);
    //
    cout << endl;

    // Delete in B+ Tree
    auto start = high_resolution_clock::now();
    bpTree.deleteKey(1000); // real experiment
    auto stop = high_resolution_clock::now();
    auto runningTime = duration_cast<microseconds>(stop - start);
    cout << "Number of nodes of the B+ tree: " << bpTree.getNodeCount() << endl;
    cout << "Number of levels of the B+ tree: " << bpTree.getLevelCount() << endl;
    cout << "Content of the root node: ";
    bpTree.showRoot();
    if (runningTime.count() == 0){
        auto runningTime = duration_cast<nanoseconds>(stop - start);
        cout << "The running time of the deletion process (B+ Tree): " << runningTime.count() << " nanoseconds" << endl;
    }
     else
        cout << "The running time of the deletion process (B+ Tree): " << runningTime.count() << " microseconds" << endl;
    cout << endl;



    return 0;
}
