#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <dirent.h> // Standard C/C++ header for scanning directory folders

using namespace std;

struct ComparisonResult {
    string fileName;
    double similarityScore;
};

bool compareScores(const ComparisonResult &a, const ComparisonResult &b) {
    return a.similarityScore > b.similarityScore;
}

// Read text file contents into a string
string loadFile(const string &filePath) {
    ifstream file(filePath.c_str());
    if (!file.is_open()) return "";
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Convert string to lower case and strip special characters
string preprocessText(const string &rawText) {
    string cleaned = "";
    for (size_t i = 0; i < rawText.length(); ++i) {
        char ch = rawText[i];
        if (isalnum(ch) || isspace(ch)) {
            cleaned += tolower(ch);
        } else {
            cleaned += ' ';
        }
    }
    return cleaned;
}

// Tokenize cleaned text into words
vector<string> tokenize(const string &text) {
    vector<string> tokens;
    stringstream ss(text);
    string word;
    while (ss >> word) {
        tokens.push_back(word);
    }
    return tokens;
}

// Trie Data Structure
struct TrieNode {
    map<char, TrieNode*> children;
    bool isEndOfWord;
    TrieNode() : isEndOfWord(false) {}
};

class Trie {
private:
    TrieNode* root;

    void clearTree(TrieNode* node) {
        if (!node) return;
        for (map<char, TrieNode*>::iterator it = node->children.begin(); it != node->children.end(); ++it) {
            clearTree(it->second);
        }
        delete node;
    }

public:
    Trie() { root = new TrieNode(); }
    ~Trie() { clearTree(root); }

    void insert(const string &word) {
        TrieNode* curr = root;
        for (size_t i = 0; i < word.length(); ++i) {
            char ch = word[i];
            if (curr->children.find(ch) == curr->children.end()) {
                curr->children[ch] = new TrieNode();
            }
            curr = curr->children[ch];
        }
        curr->isEndOfWord = true;
    }
};

// Generate contiguous N-Grams
vector<string> generateNGrams(const vector<string> &tokens, int n) {
    vector<string> ngrams;
    if (tokens.size() < static_cast<size_t>(n)) return ngrams;

    for (size_t i = 0; i <= tokens.size() - n; ++i) {
        string gram = "";
        for (int j = 0; j < n; ++j) {
            gram += tokens[i + j] + (j == n - 1 ? "" : " ");
        }
        ngrams.push_back(gram);
    }
    return ngrams;
}

// Calculate Jaccard Similarity Score
double calculateJaccardSimilarity(const vector<string> &grams1, const vector<string> &grams2) {
    if (grams1.empty() || grams2.empty()) return 0.0;

    set<string> set1(grams1.begin(), grams1.end());
    set<string> set2(grams2.begin(), grams2.end());

    int intersectionSize = 0;
    for (set<string>::iterator it = set1.begin(); it != set1.end(); ++it) {
        if (set2.count(*it)) {
            intersectionSize++;
        }
    }

    set<string> unionSet = set1;
    unionSet.insert(set2.begin(), set2.end());

    return (static_cast<double>(intersectionSize) / unionSet.size()) * 100.0;
}

// Automatically fetch all .txt files from a folder path
vector<string> getFilesInDirectory(const string &folderPath) {
    vector<string> files;
    DIR *dir = opendir(folderPath.c_str());
    if (dir == NULL) return files;

    struct dirent *entity;
    while ((entity = readdir(dir)) != NULL) {
        string fname = entity->d_name;
        // Check if file ends with .txt
        if (fname.length() > 4 && fname.substr(fname.length() - 4) == ".txt") {
            files.push_back(fname);
        }
    }
    closedir(dir);
    return files;
}

int main() {
    cout << "==========================================" << endl;
    cout << "    INTELLIPLAG AUTO-FOLDER ENGINE       " << endl;
    cout << "==========================================" << endl;

    // STEP 1: Get Target File Name
    string targetFileName;
    cout << "\n[Step 1] Enter target file name (e.g. submission.txt): ";
    getline(cin, targetFileName);

    string targetText = loadFile(targetFileName);
    if (targetText.empty()) {
        cout << "\n[Error] Could not open target file '" << targetFileName << "'." << endl;
        cout << "Press Enter to exit...";
        cin.get();
        return 0;
    }

    // STEP 2: Get Library Folder Name
    string folderPath;
    cout << "[Step 2] Enter library folder name (e.g. library): ";
    getline(cin, folderPath);

    vector<string> libraryFiles = getFilesInDirectory(folderPath);

    if (libraryFiles.empty()) {
        cout << "\n[Error] No .txt files found in directory '" << folderPath << "'." << endl;
        cout << "Press Enter to exit...";
        cin.get();
        return 0;
    }

    // Processing Target Document
    vector<string> targetTokens = tokenize(preprocessText(targetText));
    
    Trie targetTrie;
    for (size_t i = 0; i < targetTokens.size(); ++i) {
        targetTrie.insert(targetTokens[i]);
    }

    int nGramSize = 3;
    vector<string> targetNGrams = generateNGrams(targetTokens, nGramSize);

    vector<ComparisonResult> results;

    // Automatically scan & compare every file in folder
    for (size_t i = 0; i < libraryFiles.size(); ++i) {
        string fullPath = folderPath + "/" + libraryFiles[i];
        string libText = loadFile(fullPath);
        
        if (libText.empty()) continue;

        vector<string> libTokens = tokenize(preprocessText(libText));
        vector<string> libNGrams = generateNGrams(libTokens, nGramSize);

        double score = calculateJaccardSimilarity(targetNGrams, libNGrams);

        ComparisonResult res;
        res.fileName = libraryFiles[i];
        res.similarityScore = score;
        results.push_back(res);
    }

    // Sort by highest plagiarism match
    sort(results.begin(), results.end(), compareScores);

    // Final Report Output
    cout << "\n==================================================" << endl;
    cout << "         INTELLIPLAG MATCH RANKING REPORT         " << endl;
    cout << "==================================================" << endl;
    cout << " Target File Analyzed : " << targetFileName << endl;
    cout << " Library Folder       : " << folderPath << endl;
    cout << " Files Found & Tested : " << results.size() << endl;
    cout << "--------------------------------------------------" << endl;
    cout << left << setw(28) << "Library Document" << " | " << "Similarity Score" << endl;
    cout << "--------------------------------------------------" << endl;

    cout << fixed << setprecision(2);
    for (size_t i = 0; i < results.size(); ++i) {
        cout << left << setw(28) << results[i].fileName << " | " 
             << results[i].similarityScore << "%";

        if (results[i].similarityScore >= 60.0) {
            cout << " [HIGH MATCH]";
        } else if (results[i].similarityScore >= 20.0) {
            cout << " [MODERATE OVERLAP]";
        } else {
            cout << " [LOW / UNIQUE]";
        }
        cout << endl;
    }
    cout << "==================================================\n" << endl;

    cout << "Press Enter to exit...";
    cin.get();

    return 0;
}
