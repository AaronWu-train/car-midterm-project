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
    BACKWARD = 0
    LEFT = 1
    FORWARD = 2
    RIGHT = 3

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
    def __init__(self, filepath: str, start_node: int, start_port: int, height: int = 6):
        self.height = height
        df = pandas.read_csv(filepath)
        new_order = ["index", "North", "East", "South", "West", "ND", "ED", "SD", "WD"]
        df_swapped = df[new_order]
        df_filled = df_swapped.fillna(-1)
        self.raw_data = df_filled.values.astype(int).tolist()
        self.start_node = start_node
        self.start_port = start_port

        # call cpp helper function to get dist and next matrix
        self.dist, self.next = floyd_warshall(self.raw_data, start_node, start_port)

        # figure out the treasure nodes and there ports
        self.treasure_nodes = [(start_node, Direction(start_port))]
        self.treasure_node_map = {start_node: 0} # map node id to treasure node id for tsp
        for (i, row) in enumerate(self.raw_data):
            if row[0] == start_node:
                continue
            port = []
            for j in range(1, 5):
                if row[j] != -1:
                    port.append(j-1)
            if len(port) == 1:
                self.treasure_node_map[row[0]] = len(self.treasure_nodes)
                self.treasure_nodes.append((row[0], Direction(port[0])))
        print(f"Treasure nodes: {self.treasure_nodes}")
        print(f"Treasure node map: {self.treasure_node_map}")
    
    def get_distance(self, from_node: int, from_port: int, to_node: int, to_port: int) -> float:
        return self.dist[from_node * 4 + from_port][to_node * 4 + to_port]

    def get_path(self, from_node: int, from_port: int, to_node: int, to_port: int) -> List[Direction]:
        path = []
        current_id = from_node * 4 + from_port
        target_id = to_node * 4 + to_port

        while current_id != target_id:
            path.append(target_id)
            if self.next[current_id][target_id] == target_id:
                break
            target_id = self.next[current_id][target_id]
            if target_id == -1:
                break
        path.append(current_id)
        path.reverse()
        print(f"Path from {from_node} to {to_node}: {path}")
        # convert path to directions
        directions = []
        for i in range(len(path)-1):
            from_node = path[i] // 4
            from_port = path[i] % 4
            to_node = path[i+1] // 4
            to_port = path[i+1] % 4
            turn = get_turn_direction(from_port, opposite_direction(to_port))
            directions.append(turn)

        return directions
    
    def get_TSP_distance(self) -> List[List[float]]:
        n = len(self.treasure_nodes)
        dist = [[0.0] * n for _ in range(n)]
        for i, (node, port) in enumerate(self.treasure_nodes):
            for j, (node2, port2) in enumerate(self.treasure_nodes):
                if node == node2:
                     dist[i][j] = 0
                dist[i][j] = self.get_distance(node, port, node2, port2)
        return dist

    def get_score(self) -> list[int]:
        score = [0] * len(self.treasure_nodes)
        for i, (node, port) in enumerate(self.treasure_nodes):
            score[i] = abs(((node - 1) % self.height) - ((self.start_node - 1)%self.height)) + abs(((node - 1)//self.height) - ((self.start_node - 1)//self.height))         
        return score
