// maze.cpp
// This program receives a maze from the python, 
// then use Floyd-Warshall algorithm to find the shortest path between every two nodes
// and return the distance matrix with 4*n nodes

#include <utility>
#include <iostream>
#include <vector>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "delay_time.h"

using namespace std;
namespace py = pybind11;

const int INF = 1e9 + 7;

enum class Direction {
    NORTH = 0,
    EAST = 1,
    SOUTH = 2,
    WEST = 3
};

enum class Turn {
    BACKWARD = 0,
    LEFT = 1,
    FORWARD = 2,
    RIGHT = 3
};

Turn get_turn(Direction from, Direction to) {
    // @param from: the direction of in port of one node
    // @param to: the direction of out port of one node
    // @return: the turn direction
    int turn = (static_cast<int>(to) - static_cast<int>(from) + 4) % 4;
    return static_cast<Turn>(turn);
}

Direction after_turn_direction(Direction from, Turn turn) {
    // @param from: the direction of in port of one node
    // @param turn: the turn direction
    // @return: the direction of out port after the turn
    int direction = (static_cast<int>(from) + static_cast<int>(turn)) % 4;
    return static_cast<Direction>(direction);
}

Direction before_turn_direction(Direction to, Turn turn) {
    // @param from: the direction of in port of one node
    // @param turn: the turn direction
    // @return: the direction of in port before the turn
    int direction = (static_cast<int>(to) - static_cast<int>(turn) + 4) % 4;
    return static_cast<Direction>(direction);
}

Direction opposite_direction(Direction direction) {
    // @param direction: the direction of in port of one node
    // @return: the opposite direction
    int opposite_direction = (static_cast<int>(direction) + 2) % 4;
    return static_cast<Direction>(opposite_direction);
}

// the graph node id of a port is calulated by the following formula:
// graph_id = (node_id * 4) + direction
int get_graph_id(int node_id, Direction direction) {
    return (node_id * 4) +  static_cast<int>(direction);
}

int get_node_id(int graph_id) {
    return graph_id / 4;
}

Direction get_direction(int graph_id) {
    return static_cast<Direction>(graph_id % 4);
}

vector<vector<double>> build_graph(vector<vector<int>> raw_data, int start_node, Direction start_direction) {
    // @param raw_data: adjacency array of the maze
    // the columns of the content array are NOT as same as maze.csv:
    // index, North, East, South, West, ND, ED, SD, WD,
    int n = raw_data.size();
    vector<vector<double>> graph(n * 4 + 4, vector<double>(n * 4 + 4, INF));
    for (int i = 0; i < n * 4 + 4; i++) {
        graph[i][i] = 0;
    }
    graph[static_cast<int>(start_direction)][start_node * 4 + static_cast<int>(start_direction)] = 0;

    for (int i = 0; i < n; i++) {
        int current_node = raw_data[i][0];
        for (int j = 1; j <= 4; j++) {
            if (raw_data[i][j] != -1) {
                int next_node = raw_data[i][j];
                // add four edges for each direction to the next_node
                for (int k = 0; k < 4; k++) {
                    Direction from = static_cast<Direction>(k);
                    Direction to = static_cast<Direction>(j - 1);
                    Turn turn = get_turn(from, to);
                    int from_graph_id = get_graph_id(current_node, from);
                    int to_graph_id = get_graph_id(next_node, opposite_direction(to));
                    double weight = raw_data[i][j + 4] * DelayTime::FORWARD;
                    if (turn == Turn::BACKWARD) {
                        weight += DelayTime::BACKWARD;
                    } else if (turn == Turn::LEFT) {
                        weight += DelayTime::LEFT;
                    } else if (turn == Turn::RIGHT) {
                        weight += DelayTime::RIGHT;
                    } 
                    graph[from_graph_id][to_graph_id] = weight;
                }
            }
        }
    }
    return graph;
}

pair<vector<vector<double>>, vector<vector<int>>> floyd_warshall (vector<vector<int>> raw_data, int start_node = 24, Direction start_direction = Direction::SOUTH) {
    // @param raw_data: adjacency array of the maze
    // the columns of the content array are NOT as same as maze.csv:
    // index, North, East, South, West, ND, ED, SD, WD,
    // @param start_node: the start node id, 
    // @param start_direction: the start direction id
    // @return: the distance matrix and the next matrix

    int n = raw_data.size();
    vector<vector<double>> dist = build_graph(raw_data, start_node, start_direction);

    const int graph_size = (n + 1) * 4;
    
    vector<vector<int>> next(graph_size, vector<int>(graph_size, -1));
    for (int i = 0; i < graph_size; i++) {
        for (int j = 0; j < graph_size; j++) {
            if (dist[i][j] < INF) {
                next[i][j] = j;
            }
        }
        next[i][i] = i;
    }

    for (int k = 0; k < graph_size; k++) {
        for (int i = 0; i < graph_size; i++) {
            for (int j = 0; j < graph_size; j++) {
                if (dist[i][k] < INF && dist[k][j] < INF) {
                    if (dist[i][j] > dist[i][k] + dist[k][j]) {
                        dist[i][j] = dist[i][k] + dist[k][j];
                        next[i][j] = next[k][j];
                    }
                }
            }
        }
    }
    return make_pair(dist, next);
}

PYBIND11_MODULE(maze_helper, m) {
    m.doc() = "Maze Floyd-Warshall Module, return two 2D array: dist and next"; 
    m.def("floyd_warshall", &floyd_warshall, "A function that return distance matrix using Floyd-Warshall algorithm", py::arg("raw_data"), py::arg("start_node"), py::arg("start_direction"));
}

// int main() {
//     vector<vector<int>> raw_data = {
//         {1, -1, -1,  2, -1, -1, -1,  1, -1},
//         {2,  1, -1,  3,  5,  1, -1,  1,  1},
//         {3,  2, -1, -1, -1,  1, -1, -1, -1},
//         {4, -1, -1,  5, -1, -1, -1,  1, -1},
//         {5,  4,  2,  6, -1,  1,  1,  1, -1},
//         {6,  5, -1, -1, -1,  1, -1, -1, -1}
//     };
//     auto result = floyd_warshall(raw_data);
//     auto dist = result.first;
//     auto next = result.second;
//     cout << dist[1 * 4 + 0][6 * 4 + 0] << endl; // Example to print distance from node 1 to node 2
//     return 0;
// }