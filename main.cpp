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
	const int BOX_X = 32;
	const int BOX_Y = 32;
	int array[ROW][COL] = { 0 };
}

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
void createMaze();
void Dijkstra(const Graph& graph, std::vector<long long>& distances, int startIndex);
void Dijkstra(const Graph& graph, std::vector<long long>& distances, int startIndex, int targetIndex, std::vector<int>& path);

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

	SetDrawScreen(DX_SCREEN_BACK);
	srand((unsigned int)time(NULL));
	createMaze();
	array[1][1] = 10;
	array[ROW - 2][COL - 2] = 11;

	while (true)
	{
		ClearDrawScreen();

		for (int i = 0; i < ROW; i++)
		{
			for (int j = 0; j < COL; j++)
			{
				if (array[i][j] == 1)//壁
				{
					DrawBox(i * BOX_X, j * BOX_Y, BOX_X * (i + 1), BOX_Y * (j + 1), GetColor(255, 255, 255), TRUE);
				}
				if (array[i][j] == 2)//柱
				{
					DrawBox(i * BOX_X, j * BOX_Y, BOX_X * (i + 1), BOX_Y * (j + 1), GetColor(255, 0, 0), TRUE);
				}
				if (array[i][j] == 3)//柱から伸びた壁
				{
					DrawBox(i * BOX_X, j * BOX_Y, BOX_X * (i + 1), BOX_Y * (j + 1), GetColor(255, 255, 0), TRUE);
				}
				if (array[i][j] == 10)//スタート地点
				{
					DrawBox(i * BOX_X, j * BOX_Y, BOX_X * (i + 1), BOX_Y * (j + 1), GetColor(0, 0, 255), TRUE);
				}
				if (array[i][j] == 11)//ゴール地点
				{
					DrawBox(i * BOX_X, j * BOX_Y, BOX_X * (i + 1), BOX_Y * (j + 1), GetColor(0, 255, 255), TRUE);
				}
			}
		}

		if (CheckHitKey(KEY_INPUT_SPACE))
		{

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
				array[i][j] = 1;
			}
		}
	}

	for (int i = 0;i < ROW;i++)
	{
		for (int j = 0;j < COL;j++)
		{
			if (array[i][j] == 1)
			{
				continue;
			}
			int tx = i % 2;
			int ty = j % 2;
			if (tx == 0 && ty == 0)
			{
				array[i][j] = 2;
			}
		}
	}

	for (int i = 2; i < ROW; i++)
	{
		for (int j = 2; j < COL; j++)
		{
			if (array[i][j] == 2)
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
		array[r][c - 1] = 3;
	}
	if (rdm == 1)
	{
		array[r][c + 1] = 3;
	}
	if (rdm == 2)
	{
		array[r + 1][c] = 3;
	}
	if (rdm == 3)
	{
		array[r - 1][c] = 3;
	}
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