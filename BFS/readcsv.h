#include <iostream>
#include <fstream>   // ifstream
#include <sstream>   // istringstream
#include <vector>
#include <string>
using namespace std;

vector<vector<string>> readCsv(const string& filename) {
    // 用來存放整個 CSV 檔案的所有資料
    vector<vector<string>> data;

    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "無法打開檔案: " << filename << endl;
        return data; // 回傳空的 data
    }

    string line;
    // 逐行讀取
    while (getline(file, line)) {
        // 這裡用來存放當前「那一行」拆解後的每個欄位
        vector<string> row;
        // 用 istringstream 將整行文字拆成多個欄位
        istringstream iss(line);
        string cell;
        // 以逗號 ',' 為分隔符
        while (getline(iss, cell, ',')) {
            row.push_back(cell);
        }
        // 將一整行的欄位放入 data
        data.push_back(row);
    }

    file.close();
    return data;
}
