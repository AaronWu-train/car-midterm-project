#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <stack>
#include "readcsv.h"
using namespace std;

struct Edge {
    int to;
    int cost;
};

void Dijkstra(vector<vector<Edge>>, int, int);
vector<int> StrToVec(const vector<string>&);
vector<vector<Edge>> buildGraph(vector<vector<int>>);
int getDir(int, int, int);
vector<vector<int>> table;

int main(){
    vector<vector<string>> data = readCsv("maze.csv");
    for (size_t i=1; i<data.size(); i++)
    {
        vector<int> row = StrToVec(data[i]);
        table.push_back(row);
    }
    
    int start, end;
    cout << "Input start node: ";
    cin >> start;
    cout << "Input end node: ";
    cin >> end;

    vector<vector<Edge>> graph = buildGraph(table);

    Dijkstra(graph, start, end);
    
    return 0;
}

void Dijkstra(vector<vector<Edge>> graph, int start, int end){
    int INF = 10000;
    vector<int> dist(100, INF);
    vector<bool> visited(100, false);
    vector<int> prev(100, -1);
    
    dist[start] = 0;

    for (int i=0; i<graph.size(); i++)
    {
        int u = -1;
        for (int j=0; j<graph.size(); j++)
        {
            if (!visited[j] && dist[j]<INF && (u==-1 || dist[j]<dist[u]))
                u = j;
        }
        if (u==end) break;
        visited[u] = true;
        
        for (Edge& e: graph[u])
        {
            if (dist[e.to] > dist[u] + e.cost)
            {
                dist[e.to] = dist[u] + e.cost;
                prev[e.to] = u;
            }
            
        }
    }

    stack<int> path;
    int node = end;
    path.push(node);
    while (node!=start)
    {
        path.push(prev[node]);
        node = prev[node];
    }

    int currOrient = -1;
    while (path.size()>1)
    {
        int curr = path.top();
        path.pop();
        currOrient = getDir(curr, path.top(), currOrient);        
    }

}

vector<int> StrToVec(const vector<string>& rowStr){
    vector<int> row;
    for (size_t i = 0; i < rowStr.size(); i++) {
        string cell = rowStr[i];
        // 移除前後空白
        while (!cell.empty() && isspace(cell.front()))
            cell.erase(cell.begin());
        while (!cell.empty() && isspace(cell.back()))
            cell.pop_back();
        if (cell.empty())
            row.push_back(-1);
        else
            row.push_back(stoi(cell));
    }
    return row;
}

vector<vector<Edge>> buildGraph(vector<vector<int>> table) {
    vector<vector<Edge>> graph(100);
    for(int i=0; i<table.size(); i++)
    {
        int node = table[i][0];
        vector<int> dir = {table[i][1], table[i][2], table[i][3], table[i][4]};
        vector<int> distance = {table[i][5], table[i][6], table[i][7], table[i][8]};
        for (int j=0; j<4; j++)
        {
            if (dir[j]!=-1 && distance[j]!=-1)
                graph[node].push_back({dir[j], distance[j]});
        }
    }
    return graph;
}

int getDir(int from, int to, int currOrient){
    //orient: 0:N, 1:E, 2:S, 3:W
    int orient;
    if (table[from-1][1] == to) orient = 0;
    else if (table[from-1][2] == to) orient = 2;
    else if (table[from-1][3] == to) orient = 3;
    else if (table[from-1][4] == to) orient = 1;
    if (currOrient == -1)
    {
        cout << 'f';
        return orient;
    }
    else
    {
        orient = ((orient-currOrient)+4)%4;
        if (orient==0) cout << 'f';
        else if (orient==1) cout << 'r';
        else if (orient==2) cout << 'b';
        else cout << 'l';
        return orient;
    }
}
/*
class Node {
    public:
        Node(){
            N=-1;
            S=-1;
            W=-1;
            E=-1;
            nDis=-1;
            sDis=-1;
            wDis=-1;
            eDis=-1;
        }
        Node(int _N, int _S, int _W, int _E, int _nDis, int _sDis, int _wDis, int _eDis){
            N=_N;
            S=_S;
            W=_W;
            E=_E;
            nDis=_nDis;
            sDis=_sDis;
            wDis=_wDis;
            eDis=_eDis;
        }
    private:
        int N, S, W, E;
        int nDis, sDis, wDis, eDis;
};
*/