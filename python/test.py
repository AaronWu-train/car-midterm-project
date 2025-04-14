import math

def prize_collecting_tsp_without_return(dist, scores, T):
    """
    解決不返回起點的加權 TSP 問題：在總時間 T 限制下，選擇部分節點（各帶有獎勵分數），
    使得最終得到的分數最大。路徑固定從起點 0 出發，不要求返回起點。

    同時利用 parent 陣列記錄父節點以重建最佳路徑。

    :param dist: 二維列表，dist[i][j] 為從節點 i 到 j 的旅行時間（或距離）
    :param scores: 一維列表，scores[i] 為節點 i 的分數（起點分數可視需求設定，如設為 0）
    :param T: 整數，旅行總時間限制
    :return: (max_score, best_path)
             max_score 為在時間限制內能取得的最大總分，
             best_path 為最佳路徑（包含起點 0 到最後一個節點的拜訪序列）
    """
    n = len(dist)
    INF = math.inf
    # dp[mask][v] 表示從起點 0 出發，拜訪 mask 中的節點後，位於節點 v 的最小花費時間
    dp = [[INF] * n for _ in range(1 << n)]
    # parent[mask][v] 用以記錄父狀態 (prev_mask, prev_vertex) 以便重建路徑
    parent = [[None] * n for _ in range(1 << n)]
    
    dp[1][0] = 0  # 初始狀態：僅拜訪起點 0
    
    # 狀態轉移：對每一個訪問集合 mask 與當前節點 v，嘗試擴展至未訪問的節點 u
    for mask in range(1 << n):
        for v in range(n):
            if dp[mask][v] == INF:
                continue
            for u in range(n):
                if mask & (1 << u):
                    continue  # 節點 u 已經在訪問集合中
                next_mask = mask | (1 << u)
                new_time = dp[mask][v] + dist[v][u]
                # 若新時間超過限制則不進行轉移
                if new_time > T:
                    continue
                if new_time < dp[next_mask][u]:
                    dp[next_mask][u] = new_time
                    parent[next_mask][u] = (mask, v)
                    
    # 預先計算各 mask 下的總分數
    score_sum = [0] * (1 << n)
    for mask in range(1 << n):
        s = 0
        for i in range(n):
            if mask & (1 << i):
                s += scores[i]
        score_sum[mask] = s
    
    # 搜尋所有狀態中，時間消耗 dp[mask][v] <= T 的情況下，獲得最大分數的狀態
    max_score = -1
    best_mask = None
    best_last = None
    for mask in range(1 << n):
        for v in range(n):
            if dp[mask][v] <= T and score_sum[mask] > max_score:
                max_score = score_sum[mask]
                best_mask = mask
                best_last = v
    
    # 若找不到滿足時間限制的路徑，則回傳 None
    if best_mask is None:
        return None, None
    
    # 利用 parent 陣列重建從起點到 best_last 的最佳路徑
    path = []
    mask = best_mask
    v = best_last
    while True:
        path.append(v)
        prev = parent[mask][v]
        if prev is None:
            break
        mask, v = prev
    path.reverse()  # 得到從起點到 best_last 的順序
    
    return max_score, path

# 測試範例
if __name__ == "__main__":
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
    T_limit = 15  # 總時間限制

    max_score, best_path = prize_collecting_tsp_without_return(dist, scores, T_limit)
    if best_path is None:
        print("在時間限制內無可行解")
    else:
        print("最大獲得分數 =", max_score)
        print("最佳路徑 =", best_path)