import math
from gen import generate_random_test_data
import time

class TSP:
    """
    TSP 類別，實現了旅行商問題的解決方案。
    :param dist: 二維列表，dist[i][j] 為從節點 i 到 j 的旅行時間，0 based，只包含寶藏點
    :param scores: 一維列表，scores[i] 為節點 i 的分數 (起點分數可視需求設定，如設為 0)
    """

    def __init__(self, dist, scores, T):
        self.dist = dist
        self.scores = scores
        n = len(dist)

        # Pre-calculate total scores for each mask
        self.score_sum = [0] * (1 << n)
        for mask in range(1 << n):
            s = 0
            for i in range(n):
                if mask & (1 << i):
                    s += scores[i]
            self.score_sum[mask] = s

    def solve(self, T, start=0):
        """
        解決 TSP 問題：在總時間 T 限制下，選擇部分節點（各帶有獎勵分數），
        :param T: 整數，旅行總時間限制
        :return: (max_score, best_path)
                max_score 為在時間限制內能取得的最大總分，
                best_path 為最佳路徑（包含起點 0 到最後一個節點的拜訪序列）
        """
        n = len(self.dist)
        INF = math.inf

        # dp[mask][v] 表示從起點 0 出發，拜訪 mask 中的節點後，位於節點 v 的最小花費時間
        dp = [[INF] * n for _ in range(1 << n)]
        # parent[mask][v] 用以記錄父狀態 (prev_mask, prev_vertex) 以便重建路徑
        parent = [[None] * n for _ in range(1 << n)]
        
        # initial state
        init_mask = 1 << start
        dp[init_mask][start] = 0

        # State transition: for each visited set mask and current node v, try to extend to unvisited node u
        for mask in range(1 << n):
            for v in range(n):
                if dp[mask][v] == INF:
                    continue
                for u in range(n):
                    if mask & (1 << u):
                        continue  # node u already in visited set
                    next_mask = mask | (1 << u)
                    new_time = dp[mask][v] + dist[v][u]

                    # if new time exceeds limit, skip transition
                    if new_time > T:
                        continue
                    if new_time < dp[next_mask][u]:
                        dp[next_mask][u] = new_time
                        parent[next_mask][u] = (mask, v)

        # Search for the maximum score under the time limit T
        max_score = -1
        best_mask = None
        best_last = None
        for mask in range(1 << n):
            for v in range(n):
                if dp[mask][v] <= T and self.score_sum[mask] > max_score:
                    max_score = self.score_sum[mask]
                    best_mask = mask
                    best_last = v
        
        # if no path found within time limit, return None
        if best_mask is None:
            return None, None

        # Reconstruct the best path from start to best_last using parent array
        path = []
        mask = best_mask
        v = best_last
        while True:
            path.append(v)
            prev = parent[mask][v]
            if prev is None:
                break
            mask, v = prev
        path.reverse()  # get the order from start to best_last
        return max_score, path
    

# 測試範例
if __name__ == "__main__":
    start_time = time.perf_counter()

    # 範例：定義 5 個節點之間的距離矩陣與各節點獎勵分數
    dist = [
        [0, 2, 9, 10, 7],
        [2, 0, 6, 4, 3],
        [9, 6, 0, 8, 5],
        [10, 4, 8, 0, 6],
        [7, 3, 5, 6, 0]
    ]
    # 各節點分數，假設起點 0 分數設為 0
    scores = [0, 10, 15, 20, 12]
    T_limit = 500  # 總時間限制

    dist, scores = generate_random_test_data()

    tsp = TSP(dist, scores, T_limit)

    max_score, best_path = tsp.solve(T_limit, start=3)
    if best_path is None:
        print("在時間限制內無可行解")
    else:
        print("最大獲得分數 =", max_score)
        print("最佳路徑 =", best_path)

    
    end_time = time.perf_counter()
    print("執行時間：{:.6f} 秒".format(end_time - start_time))