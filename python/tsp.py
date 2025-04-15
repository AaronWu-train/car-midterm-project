import math
import time
from gen import generate_random_test_data

class TSP:
    """
    TSP 類別，解決旅行商問題。

    :param dist: 二維列表，dist[i][j] 為從節點 i 到 j 的旅行時間，只包含寶藏點
    :param scores: 一維列表，scores[i] 為節點 i 的分數（起點分數可視需求設定，如設為 0）
    """
    
    def __init__(self, dist, scores):
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

    def solve(self, T, start=0, previsited=0):
        """
        解決 TSP 問題：在總旅行時間 T 限制下，
        從指定起點出發，並考慮已有部分點已訪問（不再重複拜訪且不計算其分數）的情況下，
        選擇其餘節點使得獲得的分數最大。

        :param T: 整數，旅行總時間限制
        :param start: 整數，起點節點編號
        :param previsited: int 或 list，表示已訪問過的點（不再計分）。
                           可傳入 bitmask 或節點列表，預設為 0 表示無預訪問點。
        :return: (max_score, best_path)
                 max_score 為新增獲得分數（不包含 previsited 的分數），
                 best_path 為最佳路徑（從起點出發拜訪的節點順序，不包含預先訪問的部分）。
        """
        n = len(self.dist)
        INF = math.inf

        # 若 previsited 是 list，則將其轉換為 bitmask
        if previsited is None:
            previsited = 0
        elif isinstance(previsited, list):
            pv = 0
            for node in previsited:
                pv |= (1 << node)
            previsited = pv

        # 確保起點包含在初始狀態中
        if not (previsited & (1 << start)):
            initial_mask = previsited | (1 << start)
        else:
            initial_mask = previsited

        # dp[mask][v] 表示從起點出發，
        # 拜訪 mask 中的點（其中已包含 previsited）後，
        # 最後停在節點 v 的最小花費時間。
        dp = [[INF] * n for _ in range(1 << n)]
        parent = [[None] * n for _ in range(1 << n)]
        dp[initial_mask][start] = 0

        # 狀態轉移：從每個已訪問集合 mask 與當前節點 v，
        # 嘗試擴展到尚未訪問的節點 u。
        for mask in range(1 << n):
            for v in range(n):
                if dp[mask][v] == INF:
                    continue
                for u in range(n):
                    # 若 u 已在 mask 中，或已預先訪問則跳過
                    if mask & (1 << u):
                        continue
                    next_mask = mask | (1 << u)
                    new_time = dp[mask][v] + self.dist[v][u]
                    # 超過時間限制則跳過此次轉移
                    if new_time > T:
                        continue
                    if new_time < dp[next_mask][u]:
                        dp[next_mask][u] = new_time
                        parent[next_mask][u] = (mask, v)

        # 搜尋所有狀態中，在時間限制內 (dp[mask][v] <= T) 的最佳狀態
        # 計算新增分數 = 已拜訪節點分數總和 - 預先訪問的分數總和
        max_score = -1
        best_mask = None
        best_last = None
        for mask in range(1 << n):
            for v in range(n):
                if dp[mask][v] <= T:
                    added_score = self.score_sum[mask] - self.score_sum[previsited]
                    if added_score > max_score:
                        max_score = added_score
                        best_mask = mask
                        best_last = v

        # 若無可行解則回傳 None
        if best_mask is None:
            return None, None

        # 利用 parent 陣列重建最佳路徑（僅包含從起點新拜訪的節點序列）
        path = []
        mask = best_mask
        v = best_last
        while True:
            path.append(v)
            prev = parent[mask][v]
            if prev is None:
                break
            mask, v = prev
        path.reverse()
        return max_score, path

# 測試範例
if __name__ == "__main__":

    # 使用隨機產生的 16 x 16 測資
    dist_matrix, scores = generate_random_test_data(n=16)
    T_limit = 500  # 總旅行時間限制

    # 假設預先已拜訪的點，例如：點 1、點 4、點 7（這些點不會再次拜訪，亦不計入分數）
    previsited = [3, 4]
    
    # 設定起點 (可自訂，這裡假設起點為 3)
    start_point = 3

    start_time = time.perf_counter()
    tsp = TSP(dist_matrix, scores)
    max_score, best_path = tsp.solve(T_limit, start=start_point, previsited=previsited)
    
    if best_path is None:
        print("在時間限制內無可行解")
    else:
        print("最大獲得分數（不含預先拜訪點） =", max_score)
        print("最佳路徑 =", best_path)

    end_time = time.perf_counter()
    print("執行時間：{:.6f} 秒".format(end_time - start_time))
