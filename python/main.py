import argparse
import logging
import os
import sys
import time
import csv

import numpy as np
import pandas
from score import ScoreboardServer, ScoreboardFake
from maze import *
import bt_terminal
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

    point = ScoreboardServer(team_name, server_url)
    # point = ScoreboardFake("your team name", "data/fakeUID.csv") # for local testing
    # TODO : Initialize necessary variables

    maze = Maze(maze_file, 24, Direction.SOUTH)
    TSP_distance = maze.get_TSP_distance()
    TSP_score = maze.get_score()
    tsp = TSP(TSP_distance, TSP_score)

    if mode == "0":
        log.info("Mode 0: For treasure-hunting")
        # [todo] Initialize Bluetooth

        current_treasure = 0

        # Get tsp path
        current_time = time.perf_counter() - start_time
        tsp_path = tsp.solve(70 - current_time, current_treasure, [0])
        print([maze.treasure_nodes[nd] for nd in tsp_path])

        # get path to first treasure
        current_treasure = tsp_path[0]
        current_treasure_node, current_treasure_port = maze.treasure_nodes[current_treasure]
        next_treasure = tsp_path[1]
        next_treasure_node, next_treasure_port = maze.treasure_nodes[next_treasure]
       
        # [todo] send path to car

        while True:
            time.sleep(0.05)
            # [todo] wait for car signal

            # [todo] case 1: get idle state
            if "idle" is True:
                # get next tsp path
                tsp_path = tsp.solve(70 - current_time, current_treasure, [0])
                print([maze.treasure_nodes[nd] for nd in tsp_path])

                # get path to next treasure
                current_treasure = tsp_path[0]
                current_treasure_node, current_treasure_port = maze.treasure_nodes[current_treasure]
                next_treasure = tsp_path[1]
                next_treasure_node, next_treasure_port = maze.treasure_nodes[next_treasure]

                path = maze.get_path(current_treasure_node, current_treasure_port, next_treasure_node, next_treasure_port)
                print(path)

                # [todo] send path to car
            
            # [todo] case 2: get RFID
            elif "treasure" is True:
                # [todo] send to server
                print("UID")


    elif mode == "1":
        log.info("Mode 1: Self-testing mode.")

    else:
        log.error("Invalid mode")
        sys.exit(1)


if __name__ == "__main__":
    args = parse_args()
    main(**vars(args))
