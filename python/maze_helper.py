# maze_helper.py

from enum import IntEnum
from typing import List, Tuple

INF = 10**9 + 7

class DelayTime:
    FORWARD = 0.5
    BACKWARD = 2.0
    LEFT = 1.0
    RIGHT = 1.0

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

def get_turn(frm: Direction, to: Direction) -> Turn:
    return Turn((to - frm) % 4)

def opposite_direction(d: Direction) -> Direction:
    return Direction((d + 2) % 4)

def get_graph_id(node_id: int, d: Direction) -> int:
    return node_id * 4 + int(d)

def build_graph(
    raw_data: List[List[int]],
    start_node: int,
    start_dir: Direction
) -> List[List[float]]:
    n = len(raw_data)
    size = (n + 1) * 4
    # 初始化距離矩陣
    graph = [[INF] * size for _ in range(size)]
    for i in range(size):
        graph[i][i] = 0.0
    # 起點方向連向起點自身
    graph[int(start_dir)][get_graph_id(start_node, start_dir)] = 0.0

    for row in raw_data:
        curr = row[0]
        # row[1..4]: North, East, South, West 的鄰接節點
        # row[5..8]: ND, ED, SD, WD 的距離
        for j in range(1, 5):
            nxt = row[j]
            if nxt == -1:
                continue
            base_dist = row[j + 4] * DelayTime.FORWARD
            for frm in Direction:
                to   = Direction(j - 1)
                turn = get_turn(frm, to)
                w = base_dist
                if turn == Turn.BACKWARD:
                    w += DelayTime.BACKWARD
                elif turn == Turn.LEFT:
                    w += DelayTime.LEFT
                elif turn == Turn.RIGHT:
                    w += DelayTime.RIGHT
                u = get_graph_id(curr, frm)
                v = get_graph_id(nxt, opposite_direction(to))
                graph[u][v] = w
    return graph

def floyd_warshall(
    raw_data: List[List[int]],
    start_node: int = 24,
    start_direction: Direction = Direction.SOUTH
) -> Tuple[List[List[float]], List[List[int]]]:
    """
    回傳 (dist, next)：
      dist[i][j] = 最短距離
      next[i][j] = 在 i→j 最短路徑中，從 i 走向 j 前的最後一個中繼節點
    """
    dist = build_graph(raw_data, start_node, start_direction)
    size = len(dist)
    nxt = [[-1] * size for _ in range(size)]

    # 初始化 next 矩陣
    for i in range(size):
        for j in range(size):
            if dist[i][j] < INF:
                nxt[i][j] = i
        nxt[i][i] = i

    # Floyd–Warshall 三重迴圈
    for k in range(size):
        for i in range(size):
            if dist[i][k] >= INF:
                continue
            for j in range(size):
                via = dist[i][k] + dist[k][j]
                if via < dist[i][j]:
                    dist[i][j] = via
                    nxt[i][j]  = nxt[k][j]
    return dist, nxt

# 範例用法
if __name__ == "__main__":
    raw = [
        [1, -1, -1,  2, -1, -1, -1,  1, -1],
        [2,  1, -1,  3,  5,  1, -1,  1,  1],
        [3,  2, -1, -1, -1,  1, -1, -1, -1],
        [4, -1, -1,  5, -1, -1, -1,  1, -1],
        [5,  4,  2,  6, -1,  1,  1,  1, -1],
        [6,  5, -1, -1, -1,  1, -1, -1, -1],
    ]
    dist, nxt = floyd_warshall(raw, start_node=1, start_direction=Direction.SOUTH)
    print(dist)
    # 印出從節點 1 向節點 6 的最短距離
    u = get_graph_id(1, Direction.NORTH)
    v = get_graph_id(6, Direction.NORTH)
    print(f"Distance: {dist[u][v]}")