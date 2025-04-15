import csv
import logging
import math
from enum import IntEnum
from typing import List
from maze_helper import floyd_warshall

import numpy as np
import pandas

log = logging.getLogger(__name__)


class Turn(IntEnum):
    U_TURN = 0
    TURN_LEFT = 1
    ADVANCE = 2
    TURN_RIGHT = 3

class Direction(IntEnum):
    NORTH = 0
    EAST = 1
    SOUTH = 2
    WEST = 3

def get_turn_direction(from_direction, to_direction):
    turn = (to_direction - from_direction + 4) % 4
    return Turn(turn)

def after_turn_direction(from_direction, turn):
    direction = (from_direction + turn) % 4
    return Direction(direction)

def before_turn_direction(to_direction, turn):
    direction = (to_direction - turn + 4) % 4
    return Direction(direction)

def opposite_direction(direction):
    opposite_direction = (direction + 2) % 4
    return Direction(opposite_direction)

class Maze:
    def __init__(self, filepath: str, start_node: int, start_port: int):
        df = pandas.read_csv("your_file.csv")
        new_order = ["index", "North", "East", "South", "West", "ND", "ED", "SD", "WD"]
        df_swapped = df[new_order]
        df_filled = df_swapped.fillna(-1)
        self.raw_data = df_filled.values
        self.start_node = start_node
        self.start_port = start_port

        # call cpp helper function to get dist and next matrix
        self.dist, self.next = floyd_warshall(self.raw_data.tolist(), start_node, start_port)

        # figure out the treasure nodes and there ports
        self.treasure_nodes = [(start_node, Direction(start_port))]
        self.treasure_node_map = {0: start_node} # map node id to treasure node id for tsp
        for (i, row) in enumerate(self.raw_data):
            if row[0] == start_node:
                continue
            port = []
            for j in range(1, 5):
                if row[j] != -1:
                    port.append(j-1)
            if len(port) == 1:
                self.treasure_node_map[i] = len(self.treasure_nodes)
                self.treasure_nodes.append((i, Direction(port[0])))
    
    def get_distance(self, from_node: int, from_port: int, to_node: int, to_port: int) -> float:
        return self.dist[from_node * 4 + from_port][to_node * 4 + to_port]

    def get_path(self, from_node: int, from_port: int, to_node: int, to_port: int) -> List[Direction]:
        path = []
        current_id = from_node * 4 + from_port
        target_id = to_node * 4 + to_port

        while current_id != target_id:
            next_id = self.next[current_id][target_id]
            next_port = Direction(next_id % 4)
            cur_port = Direction(current_id % 4)
            turn = get_turn_direction(from_port, opposite_direction(next_port))
            path.append(turn)
            current_id = next_id

        return path
    
    def get_TSP_distance(self) -> List[List[float]]:
        n = len(self.treasure_nodes)
        dist = [[0.0] * n for _ in range(n)]
        for i, (node, port) in enumerate(self.treasure_nodes):
            for j, (node2, port2) in enumerate(self.treasure_nodes):
                if node == node2:
                     dist[i][j] = 0
                dist[i][j] = self.get_distance(node, port, node2, port2)

    def get_score(self) -> List[float]:
        score = [0.0] * len(self.treasure_nodes)
        for i, (node, port) in enumerate(self.treasure_nodes):
            score[i] = abs(((node - 1)%6) - ((self.start_node - 1)%6)) + abs(((node - 1)//6) - ((self.start_node - 1)//6))         
        return score
