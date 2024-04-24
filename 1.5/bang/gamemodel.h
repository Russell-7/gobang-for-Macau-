#ifndef GAMEMODEL_H
#define GAMEMODEL_H

// ---- 五子棋游戏模型类 ---- //
#include <vector>

// 游戏类型，双人还是AI（目前固定让AI下黑子）
enum GameType
{
    PERSON,
    BOT
};

// 游戏状态
enum GameStatus
{
    PLAYING,
    WIN,
    DEAD
};



// 棋盘尺寸
const int kBoardSizeNum = 15;
extern std::vector<std::vector<int>> gameMapVec2;

class GameModel
{
public:
    GameModel();

public:
    std::vector<std::vector<int>> gameMapVec;// 存储当前游戏棋盘和棋子的情况,空白为0，白子1，黑子-1
     //treeMapVec; //存储预测的状态
    std::vector<std::vector<int>> scoreMapVec; // 存储各个点位的评分情况，作为AI下棋依据
    bool playerFlag; // 标示下棋方
    bool Simuflag;
    GameType gameType; // 游戏模式
    GameStatus gameStatus; // 游戏状态

    void startGame(GameType type); // 开始游戏

    void actionByPerson(int row, int col); // 人执行下棋
    void actionByAI(int &clickRow, int &clickCol); // 机器执行下棋
    void updateGameMap(int row, int col); // 每次落子后更新游戏棋盘
    bool isWin(int row,int col); // 判断游戏是否胜利
    bool isDeadGame(); // 判断是否和棋
    int abScore(int row,int col);


    class node {
    public:
        std::vector<std::vector<int>> treeMapVec; // 使之公开，方便操作
        std::vector<node*> children; // 存储子节点的指针
        int cood[1][2]={{-1,-1}};
        int score; // 存储评分

        // 构造函数
        node(const std::vector<std::vector<int>>& mapVec) : treeMapVec(mapVec) {}

        // 析构函数
        ~node() {
            for (auto child : children) {
                delete child; // 释放子节点占用的内存
            }
        }

        // 添加子节点
        void addChild(node* child) {
            children.push_back(child);
        }



    };

    class tree {
    public:
        node* root; // 树的根节点

        // 构造函数
        tree() : root(nullptr) {}

        // 析构函数
        ~tree() {
            delete root; // 释放根节点及其子树占用的内存
        }

        static void buildTreeRecursive(node* current, int depth, int kmapnum);

        static void Sincvec( std::vector<std::vector<int>> gameMapvec,std::vector<std::vector<int>>treeMapvec);


        // 建树函数，需要具体实现

    };
        static tree BuildTree(const std::vector<std::vector<int>>&mapvec);
        void calculateScore(tree t); // 计算评分
        int alphaBeta(node* n, int depth, int alpha, int beta, bool turn);
        // 同步棋盘状态

        std::vector<int> getBestMove(tree& t);
};



#endif // GAMEMODEL_H
