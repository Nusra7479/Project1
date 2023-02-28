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

int main()
{
    int n = 5;
    cout << "Hello world!" << n << endl;
    BPTree bpTree;
    cout << "------------------------ Storage aspects and testing ------------------------" <<endl;
    Disk disk;
    // Read TSV
    string filename= "data.tsv";
    readTSVFile(filename, disk);
    int records;
    disk.printRecords();
    // sort em
    disk.sortRecords();
    // Print to test again
    disk.printRecords();

    
    cout << "------------------------ Experiment 3 ------------------------" <<endl;
    bpTree.searchKey(500);
    vector<Record> targets;
    targets=disk.searchRecord(500);
    printRecords(targets);
    //storage.searchKey(500);
    cout << "------------------------ Experiment 4 ------------------------" <<endl;
    //bpTree.searchKeyRange(30000,40000);
    //storage.searchKeyRange(30000, 40000);



    return 0;
}
