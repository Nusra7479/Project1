#include <iostream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <fstream>
#include <sstream>
#include "storage.cpp"

using namespace std;

void readTSVFile(string filename, Disk& disk) {
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
    infile.close();
}

void printRecords(const vector<Record>& records) {
    cout << "Printing records..." << endl;
    for (const Record& record : records) {
        cout << "Record " << record.tconst << ": " << record.averageRating << " (" << record.numVotes << " votes)" << endl;
    }
    cout << "Records printed!" << endl;
}

int getDiskIO(Disk disk, vector<void*> b_targets){
    int dataBlocksAccessed;

    for (int i = 0; i < disk.numBlocks; i++) {
        Block block = blocks[i];
        for (int j = 0; j < block.records.size(); j++) {
            Record record = block.records[j];
            for (int k = 0; k < b_targets.size(); k++){
                 if (b_targets[k]->tconst == record->tconst){
                    dataBlocksAccessed++;
                    break;
                }
            }
        }
    }
    cout << "The number of data blocks the process accesses: "<< dataBlocksAccessed << endl;
    return dataBlocksAccessed;
}

float getAvgRating (vector<void*> b_targets){
    float totalRating;
    for (int i; i< b_targets.size(); i++){
        totalRating = totalRating + recordptr->averageRating;
    }
    float average = totalRating/b_targets.size();
    cout << "The average of “averageRating’s” of the records that are returned: " << average<< endl;
    cout << "The number of records (B+ Tree): " << b_targets.size() << endl;
    return average
}

int main()
{
    int n = 5;
    bool sorted = false;
    cout << "Hello world!" << n << endl;
    //BPTree bpTree;
    cout << "------------------------ Storage aspects and testing ------------------------" <<endl;
    Disk disk;
    // Read TSV
    string filename= "data.tsv";
    readTSVFile(filename, disk);
    int records;
    //disk.printRecords();
    // sort em
    if (sorted)
        disk.sortRecords();
    // Print to test again
    //disk.printRecords();
    cout << "------------------------ Experiment 1 ------------------------" <<endl;
    cout << endl;

    cout << "------------------------ Experiment 2 ------------------------" <<endl;
    cout << endl;

    cout << "------------------------ Experiment 3 ------------------------" <<endl; //Requires Testing, Handling Duplicates
    ////Search in B+ Tree
    vector<void*> b_targets3;
    b_targets3 = bpTree.searchKeyRange(500, 500);
    getDiskIO(disk, b_targets3);
    getAvgRating(b_targets3);

    ////Search in Storage
    if (sorted)
        disk.searchKey(500, 500);
    else{
        vector<Record> targets3;
        targets3=disk.searchRecord(500, 500);
        //printRecords(targets3);
    }
    cout << "------------------------ Experiment 4 ------------------------" <<endl; //Requires Testing, Handling Duplicates
    ////Search in B+ Tree
    vector<void*> b_targets4;
    b_targets4 = bpTree.searchKeyRange(30000, 40000);
    getDiskIO(disk, b_targets4);
    getAvgRating(b_targets4);

    ////Search in Storage
    if (sorted)
        disk.searchKey(30000, 40000);
    else{
        vector<Record> targets4;
        targets4=disk.searchRecord(30000, 40000);
        //printRecords(targets4);
    }


    return 0;
}
