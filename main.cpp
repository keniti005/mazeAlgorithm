#include "DxLib.h"
#include <time.h>
#include <algorithm>
#include <vector>
#include <utility>
#include <queue>

namespace
{
	const int WIN_WIDTH = 1024;
	const int WIN_HEIGHT = 768;
	const int ROW = 11;
	const int COL = 11;
	const int BOX_X = 64;
	const int BOX_Y = 64;
	const int DX[4] = { 1,0,-1,0 };
	const int DY[4] = { 0,1,0,-1 };
	int sx, sy, gx, gy = 0;
	//std::vector<std::vector<long>> array;
	//int array[ROW][COL] = {};
	//int root[ROW][COL] = {};
	int pathIdxNow = 0;
	std::vector<std::pair<int, int>> route;//移動経路の保存
	std::vector<std::vector<int>>array(COL, std::vector<int>(ROW, -1));
	//std::vector<std::vector<int>>root(COL, std::vector<int>(ROW, -1));
	std::vector<std::vector<int>> dist(COL, std::vector<int>(ROW, -1));//-1に初期化（未探索）
	std::vector<std::vector<int>>prev_x(COL, std::vector<int>(ROW, -1));
	std::vector<std::vector<int>>prev_y(COL, std::vector<int>(ROW, -1));
}

//100がスタート地点
//101がゴール地点
/*
array[11][11] =
{
	{101,101,101,101,101,101,101,101,101,101,101},
	{101,200,  0,  0,  0,  0,  0,  0,  0,  0,101},
	{101,  0,  0,  0,  0,  0,  0,  0,  0,  0,101},
	{101,  0,  0,  0,  0,  0,  0,  0,  0,  0,101},
	{101,  0,  0,  0,  0,  0,  0,  0,  0,  0,101},
	{101,  0,  0,  0,  0,  0,  0,  0,  0,  0,101},
	{101,  0,  0,  0,  0,  0,  0,  0,  0,  0,101},
	{101,  0,  0,  0,  0,  0,  0,  0,  0,  0,101},
	{101,  0,  0,  0,  0,  0,  0,  0,  0,  0,101},
	{101,  0,  0,  0,  0,  0,  0,  0,  0,201,101},
	{101,101,101,101,101,101,101,101,101,101,101}
};
*/

constexpr long long INF = (1LL << 60);

struct Edge
{
	//行先
	int to;

	//コスト
	int cost;
};

using Graph = std::vector<std::vector<Edge>>;
using Pair = std::pair<long long, int>;//{distance,form}
void setWall(int r, int c, bool a);
void createMaze();//棒倒し法
void Dijkstra(const Graph& graph, std::vector<long long>& distances, int startIndex);
void Dijkstra(const Graph& graph, std::vector<long long>& distances, int startIndex, int targetIndex, std::vector<int>& path);
void BFS();

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	ChangeWindowMode(true);
	SetWindowSizeChangeEnableFlag(false, false);
	SetMainWindowText("TITLE");
	SetGraphMode(WIN_WIDTH, WIN_HEIGHT, 32);
	SetWindowSizeExtendRate(1.0);
	SetBackgroundColor(0,0,0);

	// ＤＸライブラリ初期化処理
	if (DxLib_Init() == -1)
	{
		DxLib_End();
	}
	array[1][1] = 0;//スタート地点

	SetDrawScreen(DX_SCREEN_BACK);
	srand((unsigned int)time(NULL));
	createMaze();//迷路作成(棒倒し法)
	array[ROW - 2][COL - 2] = 201;//ゴール地点
	bool isKey = false;
	bool isBFS = false;
	bool isGoal = false;
	//歩数
	int runX = 1;
	int runY = 1;

	//外壁作成
	for (int i = 0;i < ROW; i++)
	{
		for (int j = 0;j < COL; j++)
		{
			if (i == 0 || j == 0 || i == ROW - 1 || j == COL - 1)
			{
				array[i][j] = 101;
			}
		}
	}


	while (true)
	{
		ClearDrawScreen();

		//押したら探索
		if (CheckHitKey(KEY_INPUT_S))
		{
			isBFS = true;
		}

		if (isBFS == true)
		{
			BFS();
		}
		if (CheckHitKey(KEY_INPUT_SPACE) && isBFS == true)
		{
			if (!(isKey))
			{
				if (!(isGoal))
				{
					pathIdxNow++;
					runX = route[pathIdxNow].first;
					runY = route[pathIdxNow].second;
				}
			}
			isKey = true;
		}
		else
		{
			isKey = false;
		}

		int searchX = 0, searchY = 0;
		for (int i = 0; i < ROW; i++)
		{
			for (int j = 0; j < COL; j++)
			{
				if (array[j][i] == 101)//壁
				{
					DrawBox(i * BOX_X, j * BOX_Y, BOX_X * (i + 1), BOX_Y * (j + 1), GetColor(255, 255, 255), TRUE);
				}
				if (array[j][i] == 102)//柱
				{
					DrawBox(i * BOX_X, j * BOX_Y, BOX_X * (i + 1), BOX_Y * (j + 1), GetColor(255, 0, 0), TRUE);
				}
				if (array[j][i] == 103)//柱から伸びた壁
				{
					DrawBox(i * BOX_X, j * BOX_Y, BOX_X * (i + 1), BOX_Y * (j + 1), GetColor(255, 255, 0), TRUE);
				}
				if (array[j][i] == 0)//スタート地点
				{
					DrawBox(i * BOX_X, j * BOX_Y, BOX_X * (i + 1), BOX_Y * (j + 1), GetColor(0, 0, 255), TRUE);
				}
				if (array[j][i] == 201)//ゴール地点
				{
					DrawBox(i * BOX_X, j * BOX_Y, BOX_X * (i + 1), BOX_Y * (j + 1), GetColor(0, 255, 255), TRUE);
				}
				if (array[j][i] == 105)//探索した後の床
				{
					DrawBox(i * BOX_X, j * BOX_Y, BOX_X * (i + 1), BOX_Y * (j + 1), GetColor(128, 128, 128), TRUE);
				}
				if (j == runX && i == runY)
				{
					LoadGraphScreen(i * BOX_X, j * BOX_Y, "sun.png", TRUE);
				}
			}
		}
		if (array[runY][runX] == array[gx][gy])
		{
			isGoal = true;
		}

		ScreenFlip();
		WaitTimer(16);
		if (ProcessMessage() == -1)
			break;
		if (CheckHitKey(KEY_INPUT_ESCAPE) == 1)
			break;
	}

	DxLib_End();
	return 0;
}

void createMaze()
{
	for (int i = 0;i < ROW; i++)
	{
		for (int j = 0;j < COL; j++)
		{
			if (i == 0 || j == 0 || i == ROW - 1 || j == COL - 1)
			{
				array[i][j] = 101;
			}
		}
	}

	for (int i = 0;i < ROW;i++)
	{
		for (int j = 0;j < COL;j++)
		{
			if (array[i][j] == 101)
			{
				continue;
			}
			int tx = i % 2;
			int ty = j % 2;
			if (tx == 0 && ty == 0)
			{
				array[i][j] = 102;
			}
		}
	}

	for (int i = 2; i < ROW; i++)
	{
		for (int j = 2; j < COL; j++)
		{
			if (array[i][j] == 102)
			{
				if (i == 2)
				{
					setWall(i, j, true);// 1段目の柱からは上下左右に
				}
				else
				{
					setWall(i, j, false);// 2段目の柱からは下左右のみ
				}
			}
		}
	}
}

void setWall(int r, int c, bool a)
{
	int max = 0;
	if (a == true)
	{
		max = 4;
	}
	else
	{
		max = 3;
	}
	int rdm = (rand() % max);
	if (rdm == 0)
	{
		array[r][c - 1] = 103;
	}
	if (rdm == 1)
	{
		array[r][c + 1] = 103;
	}
	if (rdm == 2)
	{
		array[r + 1][c] = 103;
	}
	if (rdm == 3)
	{
		array[r - 1][c] = 103;
	}
}

void BFS()
{
	for (int i = 0;i < ROW; i++)
	{
		for (int j = 0; j < COL; j++)
		{
			if (array[i][j] == 0)
			{
				sx = i;
				sy = j;
			}
			if (array[i][j] == 201)
			{
				gx = i;
				gy = j;
			}
		}
	}
	std::vector<std::vector<int>> dist(COL, std::vector<int>(ROW, -1));//-1に初期化（未探索）

	dist[sx][sy] = 0;//スタート地点
	//dist[ROW - 2][COL - 2] = 201;//ゴール地点


	std::vector<std::vector<int>>prev_x(COL, std::vector<int>(ROW, -1));
	std::vector<std::vector<int>>prev_y(COL, std::vector<int>(ROW, -1));

	std::queue<std::pair<int, int>> que;//一度見た頂点の撃ちまだ訪れていない頂点を表すキュー
	que.push(std::make_pair(sx, sy));//スタート地点

	while (!que.empty())
	{
		std::pair<int, int>current_pos = que.front();//キューの先頭を見る
		int x = current_pos.first;
		int y = current_pos.second;
		que.pop();

		//隣接頂点を探索
		for (int direction = 0; direction < 4; direction++)
		{
			int next_x = x + DX[direction];
			int next_y = y + DY[direction];
			if (next_x < 0 || next_x >= COL || next_y < 0 || next_y >= ROW)
			{
				continue;
			}
			if (array[next_x][next_y] == 101 || array[next_x][next_y] == 102 || array[next_x][next_y] == 103)//道として通れない地点を判断
			{
				continue;
			}
			if (dist[next_x][next_y] == -1)
			{
				que.push(std::make_pair(next_x, next_y));
				dist[next_x][next_y] = dist[x][y] + 1;
				prev_x[next_x][next_y] = x;
				prev_y[next_x][next_y] = y;
			}
		}
	}
	//ゴールにいたるまでの最短経路を作る
	int x = gx, y = gy;
	while (x != -1 && y != -1)
	{
		array[x][y] = 105;
		route.push_back({x,y});

		int px = prev_x[x][y];
		int py = prev_y[x][y];
		x = px, y = py;
	}

	//スタート地点から進むように反転
	std::reverse(route.begin(), route.end());

	array[sx][sy] = 0;
	array[gx][gy] = 201;
}

void Dijkstra(const Graph& graph, std::vector<long long>& distances, int startIndex)
{
	// 「現時点での最短距離, 頂点」の順に取り出す priority_queue
	// デフォルトの priority_queue は降順に取り出すため std::greater を使う
	std::priority_queue<Pair, std::vector<Pair>, std::greater<Pair>> q;
	q.emplace((distances[startIndex] = 0), startIndex);

	while (!q.empty())
	{
		const long long distance = q.top().first;
		const int from = q.top().second;
		q.pop();

		// 最短距離でなければ処理しない
		if (distances[from] < distance)
		{
			continue;
		}

		// 現在の頂点からの各辺について
		for (const auto& edge : graph[from])
		{
			// to までの新しい距離
			const long long d = (distances[from] + edge.cost);

			// d が現在の記録より小さければ更新
			if (d < distances[edge.to])
			{
				q.emplace((distances[edge.to] = d), edge.to);
			}
		}
	}
}

// distances は頂点数と同じサイズ, 全要素 INF で初期化しておく
void Dijkstra(const Graph& graph, std::vector<long long>& distances, int startIndex, int targetIndex, std::vector<int>& path)
{
	// 直前の頂点を記録する配列
	std::vector<int> p(distances.size(), -1);

	// 「現時点での最短距離, 頂点」の順に取り出す priority_queue
	// デフォルトの priority_queue は降順に取り出すため std::greater を使う
	std::priority_queue<Pair, std::vector<Pair>, std::greater<Pair>> q;
	q.emplace((distances[startIndex] = 0), startIndex);

	while (!q.empty())
	{
		const long long distance = q.top().first;
		const int from = q.top().second;
		q.pop();

		// 最短距離でなければ処理しない
		if (distances[from] < distance)
		{
			continue;
		}

		// 現在の頂点からの各辺について
		for (const auto& edge : graph[from])
		{
			// to までの新しい距離
			const long long d = (distances[from] + edge.cost);

			// d が現在の記録より小さければ更新
			if (d < distances[edge.to])
			{
				// 直前の頂点を記録
				p[edge.to] = from;

				q.emplace((distances[edge.to] = d), edge.to);
			}
		}
	}

	// targetIndex に到達できれば最短経路を再構築する
	if (distances[targetIndex] != INF)
	{
		for (int i = targetIndex; i != -1; i = p[i])
		{
			path.push_back(i);
		}

		std::reverse(path.begin(), path.end());
	}

}