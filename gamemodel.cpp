#include <utility>
#include <stdlib.h>
#include <time.h>
#include "gamemodel.h"
#include <limits>

GameModel::GameModel()
{

}
std::vector<std::vector<int>> gameMapVec2;
void GameModel::startGame(GameType type)
{
    gameType = type;
    // 初始棋盘
    gameMapVec.clear();
    for (int i = 0; i < kBoardSizeNum; i++)
    {
        std::vector<int> lineBoard;
        for (int j = 0; j < kBoardSizeNum; j++)
            lineBoard.push_back(0);
        gameMapVec.push_back(lineBoard);
    }

    // 如果是AI模式，需要初始化评分数组
    if (gameType == BOT)
    {
        scoreMapVec.clear();
        for (int i = 0; i < kBoardSizeNum; i++)
        {
            std::vector<int> lineScores;
            for (int j = 0; j < kBoardSizeNum; j++)
                lineScores.push_back(0);
            scoreMapVec.push_back(lineScores);
        }
    }

    // 己方下为true,对方下位false
    playerFlag = true;
    Simuflag=false;
    gameMapVec2=gameMapVec;


}

void GameModel::updateGameMap(int row, int col)
{
    if (playerFlag){
        gameMapVec[row][col] = 1;
        gameMapVec2=gameMapVec;
    }
    else{
        gameMapVec[row][col] = -1;
        gameMapVec2=gameMapVec;
    }

    // 换手
    playerFlag = !playerFlag;
}

void GameModel::tree::Sincvec(std::vector<std::vector<int>> gameMapvec, std::vector<std::vector<int> > treeMapvec){
    treeMapvec=gameMapvec;
}

void GameModel::actionByPerson(int row, int col)
{
    updateGameMap(row, col);
}

void GameModel::actionByAI(int &clickRow, int &clickCol)
{
    int alpha=INT_MAX;
    int beta=INT_MIN;
    tree t=BuildTree(gameMapVec);

    int sis=alphaBeta(t.root,1,alpha,beta,Simuflag);

    // 从评分中找出最大分数的位置
    int maxScore = 0;
    std::vector<std::pair<int, int>> maxPoints;

    for (int row = 1; row < kBoardSizeNum; row++)
        for (int col = 1; col < kBoardSizeNum; col++)
        {
            // 前提是这个坐标是空的
            if (gameMapVec[row][col] == 0)
            {
                if (scoreMapVec[row][col] > maxScore)          // 找最大的数和坐标
                {
                    maxPoints.clear();
                    maxScore = scoreMapVec[row][col];
                    maxPoints.push_back(std::make_pair(row, col));
                }
                else if (scoreMapVec[row][col] == maxScore)     // 如果有多个最大的数，都存起来
                    maxPoints.push_back(std::make_pair(row, col));
            }
        }

    // 随机落子，如果有多个点的话
    srand((unsigned)time(0));
    int index = rand() % maxPoints.size();
    //int index=1;

    std::pair<int, int> pointPair = maxPoints.at(index);
    clickRow = pointPair.first; // 记录落子点
    clickCol = pointPair.second;
    updateGameMap(clickRow, clickCol);
}

// 实现tree的BuildTree函数
GameModel::tree GameModel::BuildTree(const std::vector<std::vector<int>>&treeMapvec){
    tree t;
    t.root = new node(treeMapvec); // 使用初始棋盘状态创建根节点
    tree::buildTreeRecursive(t.root,4,kBoardSizeNum);
    return t;
}


void GameModel::tree::buildTreeRecursive(node* current, int depth, int kmapnum){
    if (depth == 0) return; // 基本情况：达到深度限制
    Sincvec(gameMapVec2,current->treeMapVec);

    for (int row = 0; row < kmapnum; ++row) {
        for (int col = 0; col < kmapnum; ++col) {
            // 检查位置是否可用
            if (row > 0 && col > 0 && current->treeMapVec[row][col] == 0) {
                // 创建新的棋盘状态
                std::vector<std::vector<int>> newMap = current->treeMapVec;
                newMap[row][col] = -1; // 假设1代表当前玩家的棋子
                current->cood[0][0]=row;
                current->cood[0][1]=col;
                node* child = new node(newMap);
                current->children.push_back(child);
                child->treeMapVec=newMap;
                // 递归构建子树
                buildTreeRecursive(child, depth - 1, kmapnum);
            }
        }
    }
}


// Alpha-Beta剪枝辅助函数
int GameModel::alphaBeta(node* n, int depth, int alpha, int beta, bool maximizingPlayer) {
    if (depth == 0 || n->children.empty()) {
        n->score=abScore(n->cood[0][0],n->cood[0][1]);
        scoreMapVec[n->cood[0][0]][n->cood[0][1]]=abScore(n->cood[0][0],n->cood[0][1]);
        return n->score; // 假设叶节点的分数已经被计算
    }

    if (maximizingPlayer) {
        int maxEval = std::numeric_limits<int>::min();
        for (node* child : n->children) {
            int eval = alphaBeta(child, depth - 1, alpha, beta, false);
            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);
            if (beta <= alpha)
                break; // Beta剪枝
        }
        return maxEval;
    } else {
        int minEval = std::numeric_limits<int>::max();
        for (node* child : n->children) {
            int eval = alphaBeta(child, depth - 1, alpha, beta, true);
            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);
            if (beta <= alpha)
                break; // Alpha剪枝
        }
        return minEval;
    }
}

int GameModel::abScore(int row,int col){
    int personNum = 0; // 玩家连成子的个数
    int botNum = 0; // AI连成子的个数
    int emptyNum = 0; // 各方向空白位的个数
    if (row > 0 && col > 0 &&
        gameMapVec[row][col] == 0)
    {
        // 遍历周围八个方向
        for (int y = -1; y <= 1; y++)
            for (int x = -1; x <= 1; x++)
            {
                // 重置
                personNum = 0;
                botNum = 0;
                emptyNum = 0;

                // 原坐标不算
                if (!(y == 0 && x == 0))
                {
                    // 每个方向延伸4个子

                    // 对玩家白子评分（正反两个方向）
                    for (int i = 1; i <= 4; i++)
                    {
                        if (row + i * y > 0 && row + i * y < kBoardSizeNum &&
                            col + i * x > 0 && col + i * x < kBoardSizeNum &&
                            gameMapVec[row + i * y][col + i * x] == 1) // 玩家的子
                        {
                            personNum++;
                        }
                        else if (row + i * y > 0 && row + i * y < kBoardSizeNum &&
                                 col + i * x > 0 && col + i * x < kBoardSizeNum &&
                                 gameMapVec[row + i * y][col + i * x] == 0) // 空白位
                        {
                            emptyNum++;
                            break;
                        }
                        else            // 出边界
                            break;
                    }

                    for (int i = 1; i <= 4; i++)
                    {
                        if (row - i * y > 0 && row - i * y < kBoardSizeNum &&
                            col - i * x > 0 && col - i * x < kBoardSizeNum &&
                            gameMapVec[row - i * y][col - i * x] == 1) // 玩家的子
                        {
                            personNum++;
                        }
                        else if (row - i * y > 0 && row - i * y < kBoardSizeNum &&
                                 col - i * x > 0 && col - i * x < kBoardSizeNum &&
                                 gameMapVec[row - i * y][col - i * x] == 0) // 空白位
                        {
                            emptyNum++;
                            break;
                        }
                        else            // 出边界
                            break;
                    }

                    if (personNum == 1)                      // 杀二
                        scoreMapVec[row][col] += 10;
                    else if (personNum == 2)                 // 杀三
                    {
                        if (emptyNum == 1)
                            scoreMapVec[row][col] += 30;
                        else if (emptyNum == 2)
                            scoreMapVec[row][col] += 40;
                    }
                    else if (personNum == 3)                 // 杀四
                    {
                        // 量变空位不一样，优先级不一样
                        if (emptyNum == 1)
                            scoreMapVec[row][col] += 60;
                        else if (emptyNum == 2)
                            scoreMapVec[row][col] += 110;
                    }
                    else if (personNum == 4)                 // 杀五
                        scoreMapVec[row][col] += 10100;

                    // 进行一次清空
                    emptyNum = 0;

                    // 对AI黑子评分
                    for (int i = 1; i <= 4; i++)
                    {
                        if (row + i * y > 0 && row + i * y < kBoardSizeNum &&
                            col + i * x > 0 && col + i * x < kBoardSizeNum &&
                            gameMapVec[row + i * y][col + i * x] == 1) // 玩家的子
                        {
                            botNum++;
                        }
                        else if (row + i * y > 0 && row + i * y < kBoardSizeNum &&
                                 col + i * x > 0 && col + i * x < kBoardSizeNum &&
                                 gameMapVec[row +i * y][col + i * x] == 0) // 空白位
                        {
                            emptyNum++;
                            break;
                        }
                        else            // 出边界
                            break;
                    }

                    for (int i = 1; i <= 4; i++)
                    {
                        if (row - i * y > 0 && row - i * y < kBoardSizeNum &&
                            col - i * x > 0 && col - i * x < kBoardSizeNum &&
                            gameMapVec[row - i * y][col - i * x] == -1) // AI的子
                        {
                            botNum++;
                        }
                        else if (row - i * y > 0 && row - i * y < kBoardSizeNum &&
                                 col - i * x > 0 && col - i * x < kBoardSizeNum &&
                                 gameMapVec[row - i * y][col - i * x] == 0) // 空白位
                        {
                            emptyNum++;
                            break;
                        }
                        else            // 出边界
                            break;
                    }

                    if (botNum == 0)                      // 普通下子
                        scoreMapVec[row][col] += 5;
                    else if (botNum == 1)                 // 活二
                        scoreMapVec[row][col] += 10;
                    else if (botNum == 2)
                    {
                        if (emptyNum == 1)                // 死三
                            scoreMapVec[row][col] += 25;
                        else if (emptyNum == 2)
                            scoreMapVec[row][col] += 50;  // 活三
                    }
                    else if (botNum == 3)
                    {
                        if (emptyNum == 1)                // 死四
                            scoreMapVec[row][col] += 55;
                        else if (emptyNum == 2)
                            scoreMapVec[row][col] += 100; // 活四
                    }
                    else if (botNum >= 4)
                        scoreMapVec[row][col] += 10000;   // 活五

                }
            }
    }

}

// 返回第二层中分数最大节点的坐标
std::vector<int> GameModel::getBestMove(tree& t) {
    int maxScore = std::numeric_limits<int>::min();
    std::vector<int> bestMove = {-1, -1}; // 存储最佳移动的坐标

    // 假设根节点是最大化玩家
    for (node* child : t.root->children) {
        int score = alphaBeta(child, 2, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), false);
        if (score > maxScore) {
            maxScore = score;
            bestMove[0] = child->cood[0][0];
            bestMove[1] = child->cood[0][1];
        }
    }

    return bestMove;
}


/*int GameModel::alphaBetaSearch(int depth, int alpha, int beta) {


    // 如果达到搜索深度限制或者游戏结束，则返回当前局面的评分


    // 获取所有可能的落子位置
    for (int row = 0; row < kBoardSizeNum; row++){
        for (int col = 0; col < kBoardSizeNum; col++)

        {    if (depth == 0 || isDeadGame()||isWin(row,col)) {
                Simuflag=false;
                return abScore();
            }

             // 空白点就算
             if (row > 0 && col > 0 &&
                gameMapVec[row][col] == 0)
             {

                // 如果是AI方，进行Max层搜索
               if (!Simuflag)
                {

                  // 模拟下子
                  gameMapVec[row][col]=-1;
                  Simuflag=!Simuflag;

                  // 调用 minValue 函数
                  alpha = std::max(alpha, alphaBetaSearch(depth - 1, alpha, beta));

                  // 恢复原来的状态
                  gameMapVec[row][col]=0;

                  // 进行 alpha-beta 剪枝
                  if (alpha >= beta) {
                    break;
                  }

                  return alpha;
               }
              // 如果是对手方，进行Min层搜索
               else {

               // 模拟下子
               gameMapVec[row][col]=1;

               // 调用 maxValue 函数
               beta = std::min(beta, alphaBetaSearch(depth - 1, alpha, beta));

               // 恢复原来的状态
               gameMapVec[row][col]=0;

               // 进行 alpha-beta 剪枝
               if (alpha >= beta) {
                   break;
               }

               return beta;
        }
      }
    }
    }
}

int GameModel::abScore(){
    int personNum = 0; // 玩家连成子的个数
    int botNum = 0; // AI连成子的个数
    int emptyNum = 0; // 各方向空白位的个数
    for (int row = 0; row < kBoardSizeNum; row++){
        for (int col = 0; col < kBoardSizeNum; col++)
        {
            // 空白点就算
            if (row > 0 && col > 0 &&
                gameMapVec[row][col] == 0)
            {
                // 遍历周围八个方向
                for (int y = -1; y <= 1; y++)
                    for (int x = -1; x <= 1; x++)
                    {
                        // 重置
                        personNum = 0;
                        botNum = 0;
                        emptyNum = 0;

                        // 原坐标不算
                        if (!(y == 0 && x == 0))
                        {
                            // 每个方向延伸4个子

                            // 对玩家白子评分（正反两个方向）
                            for (int i = 1; i <= 4; i++)
                            {
                                if (row + i * y > 0 && row + i * y < kBoardSizeNum &&
                                    col + i * x > 0 && col + i * x < kBoardSizeNum &&
                                    gameMapVec[row + i * y][col + i * x] == 1) // 玩家的子
                                {
                                    personNum++;
                                }
                                else if (row + i * y > 0 && row + i * y < kBoardSizeNum &&
                                         col + i * x > 0 && col + i * x < kBoardSizeNum &&
                                         gameMapVec[row + i * y][col + i * x] == 0) // 空白位
                                {
                                    emptyNum++;
                                    break;
                                }
                                else            // 出边界
                                    break;
                            }

                            for (int i = 1; i <= 4; i++)
                            {
                                if (row - i * y > 0 && row - i * y < kBoardSizeNum &&
                                    col - i * x > 0 && col - i * x < kBoardSizeNum &&
                                    gameMapVec[row - i * y][col - i * x] == 1) // 玩家的子
                                {
                                    personNum++;
                                }
                                else if (row - i * y > 0 && row - i * y < kBoardSizeNum &&
                                         col - i * x > 0 && col - i * x < kBoardSizeNum &&
                                         gameMapVec[row - i * y][col - i * x] == 0) // 空白位
                                {
                                    emptyNum++;
                                    break;
                                }
                                else            // 出边界
                                    break;
                            }

                            if (personNum == 1)                      // 杀二
                                scoreMapVec[row][col] += 10;
                            else if (personNum == 2)                 // 杀三
                            {
                                if (emptyNum == 1)
                                    scoreMapVec[row][col] += 30;
                                else if (emptyNum == 2)
                                    scoreMapVec[row][col] += 40;
                            }
                            else if (personNum == 3)                 // 杀四
                            {
                                // 量变空位不一样，优先级不一样
                                if (emptyNum == 1)
                                    scoreMapVec[row][col] += 60;
                                else if (emptyNum == 2)
                                    scoreMapVec[row][col] += 110;
                            }
                            else if (personNum == 4)                 // 杀五
                                scoreMapVec[row][col] += 10100;

                            // 进行一次清空
                            emptyNum = 0;

                            // 对AI黑子评分
                            for (int i = 1; i <= 4; i++)
                            {
                                if (row + i * y > 0 && row + i * y < kBoardSizeNum &&
                                    col + i * x > 0 && col + i * x < kBoardSizeNum &&
                                    gameMapVec[row + i * y][col + i * x] == 1) // 玩家的子
                                {
                                    botNum++;
                                }
                                else if (row + i * y > 0 && row + i * y < kBoardSizeNum &&
                                         col + i * x > 0 && col + i * x < kBoardSizeNum &&
                                         gameMapVec[row +i * y][col + i * x] == 0) // 空白位
                                {
                                    emptyNum++;
                                    break;
                                }
                                else            // 出边界
                                    break;
                            }

                            for (int i = 1; i <= 4; i++)
                            {
                                if (row - i * y > 0 && row - i * y < kBoardSizeNum &&
                                    col - i * x > 0 && col - i * x < kBoardSizeNum &&
                                    gameMapVec[row - i * y][col - i * x] == -1) // AI的子
                                {
                                    botNum++;
                                }
                                else if (row - i * y > 0 && row - i * y < kBoardSizeNum &&
                                         col - i * x > 0 && col - i * x < kBoardSizeNum &&
                                         gameMapVec[row - i * y][col - i * x] == 0) // 空白位
                                {
                                    emptyNum++;
                                    break;
                                }
                                else            // 出边界
                                    break;
                            }

                            if (botNum == 0)                      // 普通下子
                                scoreMapVec[row][col] += 5;
                            else if (botNum == 1)                 // 活二
                                scoreMapVec[row][col] += 10;
                            else if (botNum == 2)
                            {
                                if (emptyNum == 1)                // 死三
                                    scoreMapVec[row][col] += 25;
                                else if (emptyNum == 2)
                                    scoreMapVec[row][col] += 50;  // 活三
                            }
                            else if (botNum == 3)
                            {
                                if (emptyNum == 1)                // 死四
                                    scoreMapVec[row][col] += 55;
                                else if (emptyNum == 2)
                                    scoreMapVec[row][col] += 100; // 活四
                            }
                            else if (botNum >= 4)
                                scoreMapVec[row][col] += 10000;   // 活五

                        }
                    }

            }
            return scoreMapVec[row][col];
        }
    }
}*/


// 计算评分函数
void GameModel::calculateScore(tree t)
{
    // 统计玩家或者电脑连成的子
    //int personNum = 0; // 玩家连成子的个数
    //int botNum = 0; // AI连成子的个数
    //int emptyNum = 0; // 各方向空白位的个数
    int alpha = INT_MIN; // 初始化 alpha 为负无穷
    int beta = INT_MAX; // 初始化 beta 为正无穷

    // 清空评分数组
    scoreMapVec.clear();
    for (int i = 0; i < kBoardSizeNum; i++)
    {
        std::vector<int> lineScores;
        for (int j = 0; j < kBoardSizeNum; j++)
            lineScores.push_back(0);
        scoreMapVec.push_back(lineScores);
    }


    /* 计分（此处是完全遍历，其实可以用bfs或者dfs加减枝降低复杂度，通过调整权重值，调整AI智能程度以及攻守风格）
    for (int row = 0; row < kBoardSizeNum; row++)
        for (int col = 0; col < kBoardSizeNum; col++)
        {
            // 空白点就算
            if (row > 0 && col > 0 &&
                gameMapVec[row][col] == 0)
            {
                // 遍历周围八个方向
                for (int y = -1; y <= 1; y++)
                    for (int x = -1; x <= 1; x++)
                    {
                        // 重置
                        personNum = 0;
                        botNum = 0;
                        emptyNum = 0;

                        // 原坐标不算
                        if (!(y == 0 && x == 0))
                        {
                            // 每个方向延伸4个子

                            // 对玩家白子评分（正反两个方向）
                            for (int i = 1; i <= 4; i++)
                            {
                                if (row + i * y > 0 && row + i * y < kBoardSizeNum &&
                                    col + i * x > 0 && col + i * x < kBoardSizeNum &&
                                    gameMapVec[row + i * y][col + i * x] == 1) // 玩家的子
                                {
                                    personNum++;
                                }
                                else if (row + i * y > 0 && row + i * y < kBoardSizeNum &&
                                         col + i * x > 0 && col + i * x < kBoardSizeNum &&
                                         gameMapVec[row + i * y][col + i * x] == 0) // 空白位
                                {
                                    emptyNum++;
                                    break;
                                }
                                else            // 出边界
                                    break;
                            }

                            for (int i = 1; i <= 4; i++)
                            {
                                if (row - i * y > 0 && row - i * y < kBoardSizeNum &&
                                    col - i * x > 0 && col - i * x < kBoardSizeNum &&
                                    gameMapVec[row - i * y][col - i * x] == 1) // 玩家的子
                                {
                                    personNum++;
                                }
                                else if (row - i * y > 0 && row - i * y < kBoardSizeNum &&
                                         col - i * x > 0 && col - i * x < kBoardSizeNum &&
                                         gameMapVec[row - i * y][col - i * x] == 0) // 空白位
                                {
                                    emptyNum++;
                                    break;
                                }
                                else            // 出边界
                                    break;
                            }

                            if (personNum == 1)                      // 杀二
                                scoreMapVec[row][col] += 10;
                            else if (personNum == 2)                 // 杀三
                            {
                                if (emptyNum == 1)
                                    scoreMapVec[row][col] += 30;
                                else if (emptyNum == 2)
                                    scoreMapVec[row][col] += 40;
                            }
                            else if (personNum == 3)                 // 杀四
                            {
                                // 量变空位不一样，优先级不一样
                                if (emptyNum == 1)
                                    scoreMapVec[row][col] += 60;
                                else if (emptyNum == 2)
                                    scoreMapVec[row][col] += 110;
                            }
                            else if (personNum == 4)                 // 杀五
                                scoreMapVec[row][col] += 10100;

                            // 进行一次清空
                            emptyNum = 0;

                            // 对AI黑子评分
                            for (int i = 1; i <= 4; i++)
                            {
                                if (row + i * y > 0 && row + i * y < kBoardSizeNum &&
                                    col + i * x > 0 && col + i * x < kBoardSizeNum &&
                                    gameMapVec[row + i * y][col + i * x] == 1) // 玩家的子
                                {
                                    botNum++;
                                }
                                else if (row + i * y > 0 && row + i * y < kBoardSizeNum &&
                                         col + i * x > 0 && col + i * x < kBoardSizeNum &&
                                         gameMapVec[row +i * y][col + i * x] == 0) // 空白位
                                {
                                    emptyNum++;
                                    break;
                                }
                                else            // 出边界
                                    break;
                            }

                            for (int i = 1; i <= 4; i++)
                            {
                                if (row - i * y > 0 && row - i * y < kBoardSizeNum &&
                                    col - i * x > 0 && col - i * x < kBoardSizeNum &&
                                    gameMapVec[row - i * y][col - i * x] == -1) // AI的子
                                {
                                    botNum++;
                                }
                                else if (row - i * y > 0 && row - i * y < kBoardSizeNum &&
                                         col - i * x > 0 && col - i * x < kBoardSizeNum &&
                                         gameMapVec[row - i * y][col - i * x] == 0) // 空白位
                                {
                                    emptyNum++;
                                    break;
                                }
                                else            // 出边界
                                    break;
                            }

                            if (botNum == 0)                      // 普通下子
                                scoreMapVec[row][col] += 5;
                            else if (botNum == 1)                 // 活二
                                scoreMapVec[row][col] += 10;
                            else if (botNum == 2)
                            {
                                if (emptyNum == 1)                // 死三
                                    scoreMapVec[row][col] += 25;
                                else if (emptyNum == 2)
                                    scoreMapVec[row][col] += 50;  // 活三
                            }
                            else if (botNum == 3)
                            {
                                if (emptyNum == 1)                // 死四
                                    scoreMapVec[row][col] += 55;
                                else if (emptyNum == 2)
                                    scoreMapVec[row][col] += 100; // 活四
                            }
                            else if (botNum >= 4)
                                scoreMapVec[row][col] += 10000;   // 活五

                        }
                    }

            }
        }*/
}

bool GameModel::isWin(int row, int col)
{
    // 横竖斜四种大情况，每种情况都根据当前落子往后遍历5个棋子，有一种符合就算赢
    // 水平方向
    for (int i = 0; i < 5; i++)
    {
        // 往左5个，往右匹配4个子，20种情况
        if (col - i > 0 &&
            col - i + 4 < kBoardSizeNum &&
            gameMapVec[row][col - i] == gameMapVec[row][col - i + 1] &&
            gameMapVec[row][col - i] == gameMapVec[row][col - i + 2] &&
            gameMapVec[row][col - i] == gameMapVec[row][col - i + 3] &&
            gameMapVec[row][col - i] == gameMapVec[row][col - i + 4])
            return true;
    }

    // 竖直方向(上下延伸4个)
    for (int i = 0; i < 5; i++)
    {
        if (row - i > 0 &&
            row - i + 4 < kBoardSizeNum &&
            gameMapVec[row - i][col] == gameMapVec[row - i + 1][col] &&
            gameMapVec[row - i][col] == gameMapVec[row - i + 2][col] &&
            gameMapVec[row - i][col] == gameMapVec[row - i + 3][col] &&
            gameMapVec[row - i][col] == gameMapVec[row - i + 4][col])
            return true;
    }

    // 左斜方向
    for (int i = 0; i < 5; i++)
    {
        if (row + i < kBoardSizeNum &&
            row + i - 4 > 0 &&
            col - i > 0 &&
            col - i + 4 < kBoardSizeNum &&
            gameMapVec[row + i][col - i] == gameMapVec[row + i - 1][col - i + 1] &&
            gameMapVec[row + i][col - i] == gameMapVec[row + i - 2][col - i + 2] &&
            gameMapVec[row + i][col - i] == gameMapVec[row + i - 3][col - i + 3] &&
            gameMapVec[row + i][col - i] == gameMapVec[row + i - 4][col - i + 4])
            return true;
    }

    // 右斜方向
    for (int i = 0; i < 5; i++)
    {
        if (row - i > 0 &&
            row - i + 4 < kBoardSizeNum &&
            col - i > 0 &&
            col - i + 4 < kBoardSizeNum &&
            gameMapVec[row - i][col - i] == gameMapVec[row - i + 1][col - i + 1] &&
            gameMapVec[row - i][col - i] == gameMapVec[row - i + 2][col - i + 2] &&
            gameMapVec[row - i][col - i] == gameMapVec[row - i + 3][col - i + 3] &&
            gameMapVec[row - i][col - i] == gameMapVec[row - i + 4][col - i + 4])
            return true;
    }

    return false;
}

bool GameModel::isDeadGame()
{
    // 所有空格全部填满
    for (int i = 1; i < kBoardSizeNum; i++)
        for (int j = 1; j < kBoardSizeNum; j++)
        {
            if (!(gameMapVec[i][j] == 1 || gameMapVec[i][j] == -1))
                return false;
        }
    return true;
}

