/*
 * Author: Carter Lewis
 * Assignment Title: Program 2
 * Assignment Description: Huffman Coding
 * Due Date: 2-2-2025
 * Date Created: 1-28-2025
 * Date Last Modified: 2-1-2025
 */

#include <iostream>
#include <map>
#include <fstream>
#include <queue>
#include <cassert>

using namespace std;

class huffNode;
class huffer;
class unHuffer;
struct CompareHuffNode;

void      frequencyDistribution(string fileName, map<char, int>& m);
huffNode* huffTreeBuilder(const map<char,int> freq);
void      writeMetaData(const map<char, int>& table, ofstream& out);
void      readMetaData(map<char, int>& table, ifstream& in);
void      huff(string in, string out);
void      unHuff(string in, string out);


int main(int argc, char ** argv) {
    string mode = argv[1];
    if(mode == "-huff") {
        huff(argv[2], argv[3]);
    }
    else if(mode == "-unhuff") {
        unHuff(argv[2], argv[3]);
    }
    return 0;
}


class huffNode {
    char val;
    int frequency;
public:
    huffNode* left;
    huffNode* right;
    huffNode(char v = '\0', int f = 0, huffNode* l = nullptr, huffNode* r = nullptr)
            : val(v), frequency(f), left(l), right(r) {}

    ~huffNode() {
        delete left;
        delete right;
    }
    void print(ostream& out) {
        out << val << ": " << frequency << endl;
    }

    int getFreq() const {
        return frequency;
    }

    char getVal() const {
        return val;
    }

    bool operator <(const huffNode& other) const {
        return frequency > other.frequency;
    }

    void generateTable (map<char, string>& m, string str) {
        if(val != '\0') {
            m[val] = str;
            return;
        }
        left ->generateTable(m, str+'0');
        right->generateTable(m, str+'1');
    }
};
struct CompareHuffNode {
    bool operator()(huffNode* n1, huffNode* n2) {
        return *n1 < *n2;
    }
};
class huffer {
    char buffer;
    int index;
    map<char, string> huffTable;
    ofstream& out;
    void write() {
        out.write(&buffer, sizeof(char));
        buffer = '\0';
        index = 0;
    }
public:
    huffer(map<char, string>& t, ofstream& out): buffer('\0'), index(0), huffTable(t), out(out) {}

    void writeCharacter(char c) {
        string str = huffTable[c];
        for(int i = 0; i < str.length(); i++) {
            buffer <<= 1;
            if(str.at(i) == '1') {
                buffer = buffer | 0b00000001;
            }
            index++;
            if(index == 8) {
                write();
            }
        }
    }
    void clear() {
        if(index > 0) {
            buffer <<= (8-index);
            write();
        }
    }
};
class unHuffer {
    char buffer;
    int index;
    huffNode* root;
    huffNode* tmp;

public:
    unHuffer(huffNode*& n): buffer('\0'), index(0), root(n), tmp(n) {}

    void writeCharacter(char c, ofstream& out) {
        for(int i = 0; i < 8; i++) {
            if(tmp->left == nullptr && tmp->right == nullptr) {
                out << tmp->getVal();
                tmp = root;
            }
            if(c & 0b10000000) {
                tmp = tmp->right;
            }
            else {
                tmp = tmp->left;
            }
            c <<= 1;
        }
    }
};

void huff(string in, string out) {
    map<char,int> freq;
    map<char,string> code;
    ifstream inFile;
    ofstream outFile;
    outFile.open(out, ios::out | ios::binary);
    char c;

    frequencyDistribution(in, freq);
    huffNode* treeRoot = huffTreeBuilder(freq);

    treeRoot->generateTable(code, "");

    huffer huffObj(code, outFile);
    inFile.open(in);

    writeMetaData(freq, outFile);
    while(inFile.get(c)) {
        huffObj.writeCharacter(c);
    }
    huffObj.clear();
}

void unHuff(string in, string out) {
    map<char, int> freq;
    ifstream inFile;
    ofstream outFile;
    char c;
    outFile.open(out);
    inFile.open(in, ios::in | ios::binary);

    readMetaData(freq, inFile);
    huffNode* treeRoot = huffTreeBuilder(freq);

    unHuffer obj(treeRoot);
    while(inFile.read(reinterpret_cast<char*>(&c), sizeof(char))) {
        obj.writeCharacter(c, outFile);
    }
}

void frequencyDistribution(string fileName, map<char, int>& m) {
    ifstream inFile;
    inFile.open(fileName);
    assert(inFile);
    char c;
    while(inFile.get(c)) {
        m[c]++;
    }
    m[24]++;
}
huffNode* huffTreeBuilder(const map<char,int> freq) {
    priority_queue<huffNode*, vector<huffNode*>, CompareHuffNode> q;
    for(auto i: freq) {
        q.push(new huffNode(i.first, i.second));
    }
    huffNode* tmp1;
    huffNode* tmp2;
    while(q.size() > 1) {
        tmp1 = q.top();
        q.pop();
        tmp2 = q.top();
        q.pop();
        q.push(new huffNode('\0', tmp1->getFreq()+tmp2->getFreq(), tmp1, tmp2));
    }
    return q.top();
}
void writeMetaData(const map<char, int>& table, ofstream& out) {
    int size = table.size();
    out.write(reinterpret_cast<const char *>(&size), sizeof(int));
    for(auto i: table) {
        char n = i.first;
        int  f = i.second;
        out.write(reinterpret_cast<char*>(&n), sizeof(char));
        out.write(reinterpret_cast<char*>(&f), sizeof(int));
    }
}
void readMetaData(map<char, int>& table, ifstream& in) {
    int size;
    in.read(reinterpret_cast<char*>(&size), sizeof(int));
    for(int i = 0; i < size; i++) {
        char c;
        int n;
        in.read(reinterpret_cast<char*>(&c), sizeof(char));
        in.read(reinterpret_cast<char*>(&n), sizeof(int));
        table[c] = n;
    }
}

