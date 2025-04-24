import argparse
import logging
import os
import sys
import time
import threading

import numpy as np
import pandas
from score import ScoreboardServer, ScoreboardFake
from maze import *
import bt_terminal
from bt_terminal import BluetoothRemoteController
from tsp import TSP

logging.basicConfig(
    format="%(asctime)s - %(name)s - %(levelname)s - %(message)s", level=logging.INFO
)

log = logging.getLogger(__name__)

# Default information
TEAM_NAME = "nEEil"
SERVER_URL = "http://140.112.175.18:5000"
BT_PORT = "COM4"

MAZE_FILE = "data/big_maze_113.csv"
HEIGHT = 6
START_NODE = 24
START_PORT=int(Direction.SOUTH)

# MAZE_FILE = "data/medium_maze.csv"
# HEIGHT = 3
# START_NODE = 1
# START_PORT=int(Direction.SOUTH)

# MAZE_FILE = "data/maze_cross.csv"
# HEIGHT = 3
# START_NODE = 6
# START_PORT=int(Direction.SOUTH)

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("mode", default=0, help="0: treasure-hunting, 1: self-testing", type=str)
    parser.add_argument("--maze-file", default=MAZE_FILE, help="Maze file", type=str)
    parser.add_argument("--height", default=HEIGHT, help="Height of map", type=int)
    parser.add_argument("--start-node", default=START_NODE, help="Start node", type=int)
    parser.add_argument("--start-port", default=START_PORT, help="Start port", type=int)
    parser.add_argument("--bt-port", default=BT_PORT, help="Bluetooth port", type=str)
    parser.add_argument("--team-name", default=TEAM_NAME, help="Your team name", type=str)
    parser.add_argument("--server-url", default=SERVER_URL, help="Server URL", type=str)
    return parser.parse_args()

def main(mode: int, bt_port: str, team_name: str, server_url: str, maze_file: str, height: int, start_node: int, start_port: int):

    if mode == "0":
        log.info("Mode 0: For Midterm treasure-hunting")

        # Initialize Bluetooth
        log.info(f"Connecting to Bluetooth on Port: {bt_port} ...")
        bt = BluetoothRemoteController(bt_port)
        while not bt.is_open():
            pass
        log.info("Bluetooth connected.")


        # Initialize scoreboard
        point = ScoreboardServer(team_name, server_url)
        start_time = time.perf_counter()
        # point = ScoreboardFake("your team name", "data/fakeUID.csv") # for local testing

        # Initialize maze
        maze = Maze(maze_file, start_node=start_node, start_port=start_port, height=height)
        TSP_distance = maze.get_TSP_distance()
        TSP_score = maze.get_score()
        tsp = TSP(TSP_distance, TSP_score)

        current_treasure = 0
        visited_treasures = [0]

        # Get tsp path
        current_time = time.perf_counter() - start_time
        best_score, tsp_path = tsp.solve(73 - current_time, current_treasure, visited_treasures)
        print("TSP path:")
        print([maze.treasure_nodes[nd] for nd in tsp_path])

        # get path to first treasure
        current_treasure = tsp_path[0]
        current_treasure_node, current_treasure_port = maze.treasure_nodes[current_treasure]
        next_treasure = tsp_path[1]
        next_treasure_node, next_treasure_port = maze.treasure_nodes[next_treasure]
       
        # send path to car
        path = maze.get_path(current_treasure_node, current_treasure_port, next_treasure_node, next_treasure_port)
        print(path)
        for turn in path:
            if turn == Turn.LEFT:
                bt.left()
            elif turn == Turn.RIGHT:
                bt.right()
            elif turn == Turn.FORWARD:
                bt.forward()
            elif turn == Turn.BACKWARD:
                bt.back()
        bt.stop()
        bt.write()    
        current_treasure = next_treasure

        while len(tsp_path) > 1:
            bt.readStat(scoreboard=point)
            if bt.need_cmd:
                visited_treasures.append(current_treasure)
                current_time = time.perf_counter() - start_time
                uidlist = bt.get_uid_list()
                if len(uidlist) == 0:
                    uidlist.append("00000000")
                current_score, time_remaining = point.add_UID(uidlist[-1])
                print(f"Current score: {current_score}, time remaining: {time_remaining}")
                best_score, tsp_path = tsp.solve(73 - current_time, current_treasure, visited_treasures)
                # best_score, tsp_path = tsp.solve(time_remaining, current_treasure, visited_treasures)

                if len(tsp_path) <= 1:
                    print("No more treasures to visit.")
                    break

                current_treasure = tsp_path[0]
                current_treasure_node, current_treasure_port = maze.treasure_nodes[current_treasure]
                next_treasure = tsp_path[1]
                next_treasure_node, next_treasure_port = maze.treasure_nodes[next_treasure]
            
                # send path to car
                path = maze.get_path(current_treasure_node, current_treasure_port, next_treasure_node, next_treasure_port)
                print(path)
                for turn in path:
                    if turn == Turn.LEFT:
                        bt.left()
                    elif turn == Turn.RIGHT:
                        bt.right()
                    elif turn == Turn.FORWARD:
                        bt.forward()
                    elif turn == Turn.BACKWARD:
                        bt.back()
                bt.stop()
                bt.write()    
                current_treasure = next_treasure
        
        uidlist = bt.get_uid_list()
        for uid in uidlist:
            score, time_remaining = point.add_UID(uid)
            print(f"Current score: {score}, time remaining: {time_remaining}")
            log.info(f"Current score: {score}, time remaining: {time_remaining}")

    elif mode == "1":
        start_time = time.perf_counter()
        log.info("Mode 1: Self-testing mode.")
        maze = Maze(maze_file, 24, int(Direction.SOUTH), height=6)
        TSP_distance = maze.get_TSP_distance()
        TSP_score = maze.get_score()
        tsp = TSP(TSP_distance, TSP_score)

        print("TSP distance matrix:")
        print(TSP_distance)
        print("TSP score matrix:")
        print(TSP_score)

        current_treasure = 0

        # Get tsp path
        current_time = time.perf_counter() - start_time
        best_score, tsp_path = tsp.solve(70, current_treasure, [0])
        print(f"Best score: {best_score}")
        print("TSP path:")
        print(tsp_path)

        # get path to first treasure
        for i in range(len(tsp_path) - 1):
            current_treasure = tsp_path[i]
            current_treasure_node, current_treasure_port = maze.treasure_nodes[current_treasure]
            next_treasure = tsp_path[i+1]
            next_treasure_node, next_treasure_port = maze.treasure_nodes[next_treasure]
        
            path = maze.get_path(current_treasure_node, current_treasure_port, next_treasure_node, next_treasure_port)
            print(path)

    else:
        log.error("Invalid mode")
        sys.exit(1)


if __name__ == "__main__":
    args = parse_args()
    main(**vars(args))
