from enum import IntEnum, Enum
from typing import List, Tuple

INF = 10**9 + 7

class DelayTime(float, Enum):
    FORWARD = 1
    BACKWARD = 2
    LEFT = 1
    RIGHT = 1

class Direction(IntEnum):
    NORTH = 0
    EAST  = 1
    SOUTH = 2
    WEST  = 3

class Turn(IntEnum):
    BACKWARD = 0
    LEFT     = 1
    FORWARD  = 2
    RIGHT    = 3

def get_graph_id(node_id: int, d: Direction) -> int:
    return node_id * 4 + int(d)

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
