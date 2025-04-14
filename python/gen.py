import random
import pprint

def generate_random_test_data(n=16, dist_min=1, dist_max=100, score_min=1, score_max=30):
    """
    產生隨機 n x n 的距離矩陣（對稱，對角線為 0）以及隨機的節點分數列表

    :param n: 節點數量，預設 16
    :param dist_min: 兩點間距離的最小值（非對角線）
    :param dist_max: 兩點間距離的最大值（非對角線）
    :param score_min: 節點分數的最小值
    :param score_max: 節點分數的最大值
    :return: (dist, scores)
             dist 為 n x n 的對稱距離矩陣，scores 為長度 n 的節點分數列表
    """
    # 初始化距離矩陣，先填充 0
    dist = [[0] * n for _ in range(n)]
    for i in range(n):
        for j in range(i+1, n):
            # 產生隨機距離，取值範圍 [dist_min, dist_max]
            d = random.randint(dist_min, dist_max)
            dist[i][j] = d
            dist[j][i] = d  # 保證對稱
    
    # 產生隨機的節點分數列表，每個分數取值範圍 [score_min, score_max]
    scores = [random.randint(score_min, score_max) for _ in range(n)]
    scores[0] = 0  # 假設第一個節點的分數為 0，這是常見的約定
    
    return dist, scores

if __name__ == "__main__":
    random.seed(42)   # 設定隨機種子以保證結果重現
    n = 16
    # 產生距離矩陣與分數
    dist_matrix, scores = generate_random_test_data(n)
    # 定義一個總旅行時間 T，你可依需求調整，這裡僅作範例
    T_limit = 500
    
    print("隨機生成的 16 x 16 距離矩陣:")
    pprint.pprint(dist_matrix)
    print("\n各節點分數:")
    print(scores)
    print("\n總旅行時間限制 T =", T_limit)