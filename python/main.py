import argparse
import logging
import os
import sys
import time
import threading

import numpy as np
import pandas
# from score import ScoreboardServer, ScoreboardFake
from maze import *
import bt_terminal
from bt_terminal import BluetoothRemoteController
from tsp import TSP

logging.basicConfig(
    format="%(asctime)s - %(name)s - %(levelname)s - %(message)s", level=logging.INFO
)

log = logging.getLogger(__name__)

# TODO : Fill in the following information
TEAM_NAME = "YOUR_TEAM_NAME"
SERVER_URL = "http://140.112.175.18:5000/"
MAZE_FILE = "data/small_maze.csv"
BT_PORT = ""


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("mode", help="0: treasure-hunting, 1: self-testing", type=str)
    parser.add_argument("--maze-file", default=MAZE_FILE, help="Maze file", type=str)
    parser.add_argument("--bt-port", default=BT_PORT, help="Bluetooth port", type=str)
    parser.add_argument(
        "--team-name", default=TEAM_NAME, help="Your team name", type=str
    )
    parser.add_argument("--server-url", default=SERVER_URL, help="Server URL", type=str)
    return parser.parse_args()

def main(mode: int, bt_port: str, team_name: str, server_url: str, maze_file: str):
    start_time = time.perf_counter()

    # [TODO]: Initialize scoreboard
    # point = ScoreboardServer(team_name, server_url)
    # point = ScoreboardFake("your team name", "data/fakeUID.csv") # for local testing


    if mode == "0":
        log.info("Mode 0: For Midterm treasure-hunting")

        # Initialize Bluetooth
        log.info(f"Connecting to Bluetooth on Port: {bt_port} ...")
        bt = BluetoothRemoteController(bt_port)
        while not bt.is_open():
            pass
        log.info("Bluetooth connected.")

        readThread = threading.Thread(target = bt_terminal.read)
        readThread.daemon = True
        readThread.start()

        # Initialize maze
        maze = Maze(maze_file, start_node=24, start_port=int(Direction.SOUTH), height=6)
        TSP_distance = maze.get_TSP_distance()
        TSP_score = maze.get_score()
        tsp = TSP(TSP_distance, TSP_score)

        current_treasure = 0
        visited_treasures = [0]

        # Get tsp path
        current_time = time.perf_counter() - start_time
        best_score, tsp_path = tsp.solve(70 - current_time, current_treasure, visited_treasures)
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
            elif turn == Turn.BACK:
                bt.back()
        bt.stop()
        bt.write()    
        current_treasure = next_treasure

        while True:
            if bt.need_cmd:
                visited_treasures.append(current_treasure)
                best_score, tsp_path = tsp.solve(70 - current_time, current_treasure, visited_treasures)
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
                    elif turn == Turn.BACK:
                        bt.back()
                bt.stop()
                bt.write()    
                current_treasure = next_treasure


                

    elif mode == "1":
        log.info("Mode 1: Self-testing mode.")
        maze = Maze(maze_file, 1, int(Direction.NORTH), 3)
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
        best_score, tsp_path = tsp.solve(70 - current_time, current_treasure, [0])
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
