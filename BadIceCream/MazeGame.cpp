#include <graphics.h>
#include <conio.h>
#include <vector>
#include <iostream>
#include <ctime> 
#include <thread> // 引入线程库
#include <windows.h> // 引入Windows API库
#include <mmsystem.h> // 引入MCI库头文件
#include <fstream> // 添加文件操作头文件
#include <string> // 添加字符串操作头文件
// A星
#include <queue>
#include <unordered_map>
#include <cmath>
#include <algorithm>
//// 可选：SDL 音频（用于音效）
//#include <SDL.h>
//#include <SDL_mixer.h>




// 链接MCI库
#pragma comment(lib, "winmm.lib") // 添加MCI库链接库

using namespace std;

// 游戏常量
const int BLOCK_SIZE = 40;       // 每个方块的大小
const int MAZE_WIDTH = 15;       // 迷宫宽度(方块数)
const int MAZE_HEIGHT = 15;      // 迷宫高度(方块数)
const int WINDOW_WIDTH = BLOCK_SIZE * MAZE_WIDTH;  // 窗口宽度 600
const int WINDOW_HEIGHT = BLOCK_SIZE * MAZE_HEIGHT;// 窗口高度 600

// 方向枚举
enum Direction { UP, DOWN, LEFT, RIGHT };

// 位置结构体
// struct Point 定义了一个"点"类型，包含两个成员变量
struct Point {
    int x;
    int y;
    // 如果创建 Point 时没有指定参数，则 x 和 y 都默认为 0。
    Point(int x = 0, int y = 0) : x(x), y(y) {} 
};

enum class GameQuitChoice {
    RESTART,   // 重新开始
    NEXT_LEVEL,// 下一关
    BACK_MENU  // 返回主菜单
};

// 游戏状态全局变量
int map[MAZE_HEIGHT][MAZE_WIDTH];  // 迷宫地图
Point player;                      // 玩家位置
vector<Point> fruits;              // 水果位置
vector<Point> npcs;                // NPC位置
int totalFruits = 0;               // 总水果数
int collectedFruits = 0;           // 已收集水果数
bool gameOver = false;             // 游戏是否结束
bool gameWin = false;              // 是否胜利

bool ifShowGameStartMenuGUI = true;  // 是否显示开始菜单
bool ifShowChooseCharacterGUI = false;  // 是否显示选择角色菜单
bool ifShowGameGuideGUI = false;  // 游戏玩法说明界面
bool ifShowGameGUI = false;  // 是否显示游戏界面
int currentPoint = 0;      // 当前选择的菜单项

IMAGE characterSelectImg[3][24];  // 角色选择背景图
IMAGE bricksImg;               // 墙壁图片
IMAGE bricks_3dImg;               // 墙壁图片
IMAGE golden_apple_maskImg;    // 水果掩码图
IMAGE golden_appleImg;         // 水果图片
IMAGE iceImg;                  // 冰块图片
IMAGE ice_fruitImg;             
IMAGE characterImgs[3][4];        // 三个角色图片
IMAGE npcImg;        // NPC图片
IMAGE chooseLevelImg;           // 选择关卡背景图
IMAGE chooseLevelImgs[12];           // 选择12个关卡背景图
IMAGE startMenuImg;           // 开始菜单背景图
IMAGE startMenuImgs[5];           // 5个选项开始菜单背景图
IMAGE gameWinImg;              // 胜利界面背景图
IMAGE gameLoseImg;              // 失败界面背景图
IMAGE gameWinImgs[3];
IMAGE gameLoseImgs[3];
IMAGE loginImg;
IMAGE registerImg;
IMAGE registerImgs[5];
IMAGE loginImgs[2];

int selectedCharacter = -1; // 当前选中的角色索引（-1表示未选择）

// 在全局变量区域添加
//thread* pGameThread = nullptr;           // 游戏监听线程指针
thread* pNPCThread = nullptr;            // NPC移动线程指针
thread* pICEThread = nullptr;            // 冰块发射/破坏线程指针

// 全局变量：记录音量（0-100）
int musicVolume = 50;  // 默认音量50%
Direction characterDirection = DOWN; // 角色当前方向
Direction npcDirection = RIGHT; // NPC当前方向

const int ICE_COOLDOWN = 500; // 冷却冷却时间（毫秒）
DWORD last_launch_time = 0; // 记录上一次发射时间
DWORD current_time = 0; // 记录当前系统时间（毫秒）

// A* 路径缓存会话编号（每次进入新关卡递增，用于重置静态缓存）
int gPathSessionId = 0;

// 记录上一帧的玩家与NPC位置，用于"交换位置"碰撞检测
Point gLastPlayerPos(-1, -1);
Point gLastNPCPos(-1, -1);

// 吃水果音效初始化标记
bool gEatSoundOpened = false;
bool gGameWinSoundOpened = false;
bool gGameLoseSoundOpened = false;



// 添加用户认证相关的全局变量
bool ifShowLoginGUI = true;  // 是否显示登录界面
bool ifShowRegisterGUI = false;  // 是否显示注册界面
bool isLoggedIn = false;  // 是否已登录
string currentUsername = "";  // 当前登录的用户名
const string USER_DATA_FILE = "users.txt";  // 用户数据文件

// 添加用户结构体
struct User {
    string username;
    string password;
    User(string u = "", string p = "") : username(u), password(p) {}
};

vector<User> users;  // 存储所有用户数据


void movePlayer(Direction dir);  
void showGameGUI();
void showGameStartMenuGUI();
void enterDifferentMenu(int currentPoint);
GameQuitChoice gameLoop(int selectedLevel);
void lauchBreakIce();
void showChooseLevelGUI();
void levelManager(int selectedLevel);
Direction chooseNewDirection(int deltaX, int deltaY);
void updateNPC();
void updateNPC3();
void playEatFruitSound();


// 在现有函数声明后添加
void showLoginGUI();
void showRegisterGUI();
bool loginUser(const string& username, const string& password);
bool registerUser(const string& username, const string& password);
bool userExists(const string& username);
void saveUserData();
void loadUserData();


    // 监听
// 后续加入退出这个GUI的功能ifShowGameGUI == false;

//void listenGameGUI() {
//    // 如果是游戏界面，监听键盘事件
//    //在进入游戏时清空键盘缓冲区
//   /* gameOver = false;
//    gameWin = false;*/
//    while (_kbhit()) {
//        _getch();
//    }
//    while (ifShowGameGUI == true) {
//        if (_kbhit()) {
//            int ch = _getch();
//            switch (ch) {
//            case 72: case 'w': case 'W': 
//                movePlayer(UP);
//                break;
//            case 80: case 's': case 'S':
//                movePlayer(DOWN);
//                break;
//            case 75: case 'a': case 'A':
//                movePlayer(LEFT);
//                break;
//            case 77: case 'd': case 'D':
//                movePlayer(RIGHT);
//                break;
//            case ' ': // 空格键
//                // 发射/破坏冰块
//                  // 计算当前时间与上次发射的时间差
//                current_time = GetTickCount(); // 获取当前系统时间（毫秒）
//                // 检查是否超过冷却时间
//                if (current_time - last_launch_time >= ICE_COOLDOWN) {
//                    lauchBreakIce(); // 执行发射/破坏操作
//                    last_launch_time = current_time; // 更新上次发射时间
//                }
//                // 如果没有写这个brak会怎么样：
//                // 按下空格键时，不仅会触发 lauchBreakIce();（发射 / 破坏冰块功能）
//                // 还会 同时执行 case 27 的逻辑：将 gameOver 设为 true，gameWin 设为 false，导致游戏直接结束
//                break;
//            case 27
// ESC键退出游戏
//                gameOver = true;
//                gameWin = false;
//                break;
//            }
//        }
//        Sleep(10); // 降低CPU占用
//    }
//}





// 初始化图片资源
// 初始化图片资源
void initImages() {
    // 加载墙壁图片
    loadimage(&bricksImg, _T("res/bricks.png"), 40, 40);
    loadimage(&bricks_3dImg, _T("res/bricks_3d.png"), 40, 40);
    // 加载水果图片
    loadimage(&golden_apple_maskImg, _T("res/golden_apple_mask.png"), 32, 32);
    loadimage(&golden_appleImg, _T("res/golden_apple.png"), 32, 32);
    // 加载冰块图片
    loadimage(&iceImg, _T("res/ice.png"), 40, 40);
    loadimage(&ice_fruitImg, _T("res/ice_fruit.png"), 40, 40);
    // 加载角色图片
    loadimage(&characterImgs[0][0], _T("res/Steve.png"), 32, 32);
    loadimage(&characterImgs[1][0], _T("res/MaoMao.png"), 32, 32);
    loadimage(&characterImgs[2][0], _T("res/Dylan.png"), 32, 32);

    loadimage(&characterImgs[0][1], _T("res/Steve_left.png"), 32, 32);
    loadimage(&characterImgs[1][1], _T("res/MaoMao_left.png"), 32, 32);
    loadimage(&characterImgs[2][1], _T("res/Dylan_left.png"), 32, 32);

    loadimage(&characterImgs[0][2], _T("res/Steve_up.png"), 32, 32);
    loadimage(&characterImgs[1][2], _T("res/MaoMao_up.png"), 32, 32);
    loadimage(&characterImgs[2][2], _T("res/Dylan_up.png"), 32, 32);

    loadimage(&characterImgs[0][3], _T("res/Steve_right.png"), 32, 32);
    loadimage(&characterImgs[1][3], _T("res/MaoMao_right.png"), 32, 32);
    loadimage(&characterImgs[2][3], _T("res/Dylan_right.png"), 32, 32);
    // 加载NPC图片
    loadimage(&npcImg, _T("res/NPC.png"), 32, 32);
    // 加载关卡选择背景图
    loadimage(&chooseLevelImg, _T("res/chooseLevel.png"), 600, 600);
    // 加载12个关卡选择背景图
    loadimage(&chooseLevelImgs[0], _T("res/chooseLevel_1.png"), 600, 600);
    loadimage(&chooseLevelImgs[1], _T("res/chooseLevel_2.png"), 600, 600);
    loadimage(&chooseLevelImgs[2], _T("res/chooseLevel_3.png"), 600, 600);
    loadimage(&chooseLevelImgs[3], _T("res/chooseLevel_4.png"), 600, 600);
    loadimage(&chooseLevelImgs[4], _T("res/chooseLevel_5.png"), 600, 600);
    loadimage(&chooseLevelImgs[5], _T("res/chooseLevel_6.png"), 600, 600);
    loadimage(&chooseLevelImgs[6], _T("res/chooseLevel_7.png"), 600, 600);
    loadimage(&chooseLevelImgs[7], _T("res/chooseLevel_8.png"), 600, 600);
    loadimage(&chooseLevelImgs[8], _T("res/chooseLevel_9.png"), 600, 600);
    loadimage(&chooseLevelImgs[9], _T("res/chooseLevel_10.png"), 600, 600);
    loadimage(&chooseLevelImgs[10], _T("res/chooseLevel_11.png"), 600, 600);
    loadimage(&chooseLevelImgs[11], _T("res/chooseLevel_12.png"), 600, 600);
    // 加载开始菜单背景图
    loadimage(&startMenuImg, _T("res/startMenu.png"), 600, 600);
    // 加载五个选型的开始菜单
    loadimage(&startMenuImgs[0], _T("res/startMenu_1.png"), 600, 600);
    loadimage(&startMenuImgs[1], _T("res/startMenu_2.png"), 600, 600);
    loadimage(&startMenuImgs[2], _T("res/startMenu_3.png"), 600, 600);
    loadimage(&startMenuImgs[3], _T("res/startMenu_4.png"), 600, 600);
    loadimage(&startMenuImgs[4], _T("res/startMenu_5.png"), 600, 600);
    // 加载游戏结束界面
    loadimage(&gameWinImg, _T("res/gameWin.png"), 600, 600);
    loadimage(&gameLoseImg, _T("res/gameLose.png"), 600, 600);
    // 加载游戏结束界面不同选项
    loadimage(&gameWinImgs[0], _T("res/gameWin_1.png"), 600, 600);
    loadimage(&gameWinImgs[1], _T("res/gameWin_2.png"), 600, 600);
    loadimage(&gameWinImgs[2], _T("res/gameWin_3.png"), 600, 600);
    loadimage(&gameLoseImgs[0], _T("res/gameLose_1.png"), 600, 600);
    loadimage(&gameLoseImgs[1], _T("res/gameLose_2.png"), 600, 600);
    loadimage(&gameLoseImgs[2], _T("res/gameLose_3.png"), 600, 600);
    // 登录/注册界面
    loadimage(&loginImg, _T("res/login.png"), 600, 600);
    loadimage(&registerImg, _T("res/register.png"), 600, 600);
    // 登录/注册的选项
    loadimage(&registerImgs[0], _T("res/register_1.png"), 600, 600);
    loadimage(&registerImgs[1], _T("res/register_2.png"), 600, 600);
    loadimage(&registerImgs[2], _T("res/register_3.png"), 600, 600);
    loadimage(&registerImgs[3], _T("res/register_4.png"), 600, 600);
    loadimage(&registerImgs[4], _T("res/register_5.png"), 600, 600);
    loadimage(&loginImgs[0], _T("res/login_1.png"), 600, 600);
    loadimage(&loginImgs[1], _T("res/login_2.png"), 600, 600);
    // 加载角色选择背景图（图片需包含3个角色贴图）
    loadimage(&characterSelectImg[0][0], _T("res/character1_0.png"), 135, 300);
    loadimage(&characterSelectImg[1][0], _T("res/character2_0.png"), 135, 300);
    loadimage(&characterSelectImg[2][0], _T("res/character3_0.png"), 135, 300);

    loadimage(&characterSelectImg[0][1], _T("res/character1_1.png"), 135, 300);
    loadimage(&characterSelectImg[1][1], _T("res/character2_1.png"), 135, 300);
    loadimage(&characterSelectImg[2][1], _T("res/character3_1.png"), 135, 300);

    loadimage(&characterSelectImg[0][2], _T("res/character1_2.png"), 135, 300);
    loadimage(&characterSelectImg[1][2], _T("res/character2_2.png"), 135, 300);
    loadimage(&characterSelectImg[2][2], _T("res/character3_2.png"), 135, 300);

    loadimage(&characterSelectImg[0][3], _T("res/character1_3.png"), 135, 300);
    loadimage(&characterSelectImg[1][3], _T("res/character2_3.png"), 135, 300);
    loadimage(&characterSelectImg[2][3], _T("res/character3_3.png"), 135, 300);

    loadimage(&characterSelectImg[0][4], _T("res/character1_4.png"), 135, 300);
    loadimage(&characterSelectImg[1][4], _T("res/character2_4.png"), 135, 300);
    loadimage(&characterSelectImg[2][4], _T("res/character3_4.png"), 135, 300);

    loadimage(&characterSelectImg[0][5], _T("res/character1_5.png"), 135, 300);
    loadimage(&characterSelectImg[1][5], _T("res/character2_5.png"), 135, 300);
    loadimage(&characterSelectImg[2][5], _T("res/character3_5.png"), 135, 300);

    loadimage(&characterSelectImg[0][6], _T("res/character1_6.png"), 135, 300);
    loadimage(&characterSelectImg[1][6], _T("res/character2_6.png"), 135, 300);
    loadimage(&characterSelectImg[2][6], _T("res/character3_6.png"), 135, 300);

    loadimage(&characterSelectImg[0][7], _T("res/character1_7.png"), 135, 300);
    loadimage(&characterSelectImg[1][7], _T("res/character2_7.png"), 135, 300);
    loadimage(&characterSelectImg[2][7], _T("res/character3_7.png"), 135, 300);

    loadimage(&characterSelectImg[0][8], _T("res/character1_8.png"), 135, 300);
    loadimage(&characterSelectImg[1][8], _T("res/character2_8.png"), 135, 300);
    loadimage(&characterSelectImg[2][8], _T("res/character3_8.png"), 135, 300);

    loadimage(&characterSelectImg[0][9], _T("res/character1_9.png"), 135, 300);
    loadimage(&characterSelectImg[1][9], _T("res/character2_9.png"), 135, 300);
    loadimage(&characterSelectImg[2][9], _T("res/character3_9.png"), 135, 300);

    loadimage(&characterSelectImg[0][10], _T("res/character1_10.png"), 135, 300);
    loadimage(&characterSelectImg[1][10], _T("res/character2_10.png"), 135, 300);
    loadimage(&characterSelectImg[2][10], _T("res/character3_10.png"), 135, 300);

    loadimage(&characterSelectImg[0][11], _T("res/character1_11.png"), 135, 300);
    loadimage(&characterSelectImg[1][11], _T("res/character2_11.png"), 135, 300);
    loadimage(&characterSelectImg[2][11], _T("res/character3_11.png"), 135, 300);

    loadimage(&characterSelectImg[0][12], _T("res/character1_12.png"), 135, 300);
    loadimage(&characterSelectImg[1][12], _T("res/character2_12.png"), 135, 300);
    loadimage(&characterSelectImg[2][12], _T("res/character3_12.png"), 135, 300);

    loadimage(&characterSelectImg[0][13], _T("res/character1_13.png"), 135, 300);
    loadimage(&characterSelectImg[1][13], _T("res/character2_13.png"), 135, 300);
    loadimage(&characterSelectImg[2][13], _T("res/character3_13.png"), 135, 300);

    loadimage(&characterSelectImg[0][14], _T("res/character1_14.png"), 135, 300);
    loadimage(&characterSelectImg[1][14], _T("res/character2_14.png"), 135, 300);
    loadimage(&characterSelectImg[2][14], _T("res/character3_14.png"), 135, 300);

    loadimage(&characterSelectImg[0][15], _T("res/character1_15.png"), 135, 300);
    loadimage(&characterSelectImg[1][15], _T("res/character2_15.png"), 135, 300);
    loadimage(&characterSelectImg[2][15], _T("res/character3_15.png"), 135, 300);

    loadimage(&characterSelectImg[0][16], _T("res/character1_16.png"), 135, 300);
    loadimage(&characterSelectImg[1][16], _T("res/character2_16.png"), 135, 300);
    loadimage(&characterSelectImg[2][16], _T("res/character3_16.png"), 135, 300);

    loadimage(&characterSelectImg[0][17], _T("res/character1_17.png"), 135, 300);
    loadimage(&characterSelectImg[1][17], _T("res/character2_17.png"), 135, 300);
    loadimage(&characterSelectImg[2][17], _T("res/character3_17.png"), 135, 300);

    loadimage(&characterSelectImg[0][18], _T("res/character1_18.png"), 135, 300);
    loadimage(&characterSelectImg[1][18], _T("res/character2_18.png"), 135, 300);
    loadimage(&characterSelectImg[2][18], _T("res/character3_18.png"), 135, 300);

    loadimage(&characterSelectImg[0][19], _T("res/character1_19.png"), 135, 300);
    loadimage(&characterSelectImg[1][19], _T("res/character2_19.png"), 135, 300);
    loadimage(&characterSelectImg[2][19], _T("res/character3_19.png"), 135, 300);

    loadimage(&characterSelectImg[0][20], _T("res/character1_20.png"), 135, 300);
    loadimage(&characterSelectImg[1][20], _T("res/character2_20.png"), 135, 300);
    loadimage(&characterSelectImg[2][20], _T("res/character3_20.png"), 135, 300);

    loadimage(&characterSelectImg[0][21], _T("res/character1_21.png"), 135, 300);
    loadimage(&characterSelectImg[1][21], _T("res/character2_21.png"), 135, 300);
    loadimage(&characterSelectImg[2][21], _T("res/character3_21.png"), 135, 300);

    loadimage(&characterSelectImg[0][22], _T("res/character1_22.png"), 135, 300);
    loadimage(&characterSelectImg[1][22], _T("res/character2_22.png"), 135, 300);
    loadimage(&characterSelectImg[2][22], _T("res/character3_22.png"), 135, 300);

    loadimage(&characterSelectImg[0][23], _T("res/character1_23.png"), 135, 300);
    loadimage(&characterSelectImg[1][23], _T("res/character2_23.png"), 135, 300);
    loadimage(&characterSelectImg[2][23], _T("res/character3_23.png"), 135, 300);
}

// 初始化迷宫地图
void initMap(int selectedLevel) {

	
    // 初始化全部为墙壁
    for (int i = 0; i < MAZE_HEIGHT; i++) {
        for (int j = 0; j < MAZE_WIDTH; j++) {
            map[i][j] = 1;  // 1表示墙壁
        }
    }
    map[1][1] = 0;  

    // 创建一个简单的迷宫
    switch (selectedLevel) {
    case 0: {
        int level1Map[MAZE_HEIGHT][MAZE_WIDTH] = {
       {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},  // 外围墙壁
       {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},  // 第一行空地
       {1,0,1,1,1,0,0,0,0,0,1,1,1,0,1},  // 左侧和右侧对称墙壁
       {1,0,1,0,1,0,0,0,0,0,1,0,1,0,1},  // 对称的单块墙壁
       {1,0,1,0,1,0,0,0,0,0,1,0,1,0,1},  // 与上一行对称
       {1,0,1,0,1,0,0,0,0,0,1,0,1,0,1},  // 保持对称结构
       {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},  // 中间全空地行
       {1,0,0,0,0,0,0,1,0,0,0,0,0,0,1},  // 中心十字墙壁
       {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},  // 中间全空地行
       {1,0,1,0,1,0,0,0,0,0,1,0,1,0,1},  // 对称结构（与上行呼应）
       {1,0,1,0,1,0,0,0,0,0,1,0,1,0,1},  // 保持对称
       {1,0,1,0,1,0,0,0,0,0,1,0,1,0,1},  // 与上方对应行对称
       {1,0,1,1,1,0,0,0,0,0,1,1,1,0,1},  // 左右对称墙壁（与第三行呼应）
       {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},  // 最后一行空地
       {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}   // 外围墙壁
        };
        // 复制到实际地图
        for (int i = 0; i < MAZE_HEIGHT; i++) {
            for (int j = 0; j < MAZE_WIDTH; j++) {
                map[i][j] = level1Map[i][j];
            }
        }
        break;
    }
    case 1: {
        int level2Map[MAZE_HEIGHT][MAZE_WIDTH] = {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},  // 外围墙壁
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},  // 第一行空地
        {1,0,1,1,0,0,1,0,1,0,0,1,1,0,1},  // 左侧和右侧对称墙壁结构
        {1,0,1,0,0,0,1,0,1,0,0,0,1,0,1},  // 对称的单块墙壁
        {1,0,0,0,1,0,0,0,0,0,1,0,0,0,1},  // 中间对称障碍物
        {1,0,0,0,1,1,1,1,1,1,1,0,0,0,1},  // 中间横向墙壁
        {1,0,0,0,0,0,0,1,0,0,0,0,0,0,1},  // 中心位置单墙
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},  // 中心区域全空
        {1,0,0,0,0,0,0,1,0,0,0,0,0,0,1},  // 与第六行对称
        {1,0,0,0,1,1,1,1,1,1,1,0,0,0,1},  // 与第五行对称
        {1,0,0,0,1,0,0,0,0,0,1,0,0,0,1},  // 与第四行对称
        {1,0,1,0,0,0,1,0,1,0,0,0,1,0,1},  // 与第三行对称
        {1,0,1,1,0,0,1,0,1,0,0,1,1,0,1},  // 与第二行对称
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},  // 最后一行空地
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}   // 外围墙壁
        };
        // 复制到实际地图
        for (int i = 0; i < MAZE_HEIGHT; i++) {
            for (int j = 0; j < MAZE_WIDTH; j++) {
                map[i][j] = level2Map[i][j];
            }
        }
        break;
    }
    case 2: {
        int level3Map[MAZE_HEIGHT][MAZE_WIDTH] = {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},  // 外围墙壁
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},  // 顶部空地
        {1,0,1,1,0,1,0,1,0,1,0,1,1,0,1},  // 对称墙壁
        {1,0,1,0,0,0,0,0,0,0,0,0,1,0,1},  // 通道设计
        {1,0,0,0,1,0,1,1,1,0,1,0,0,0,1},  // 向中心引导
        {1,0,1,0,0,0,0,0,0,0,0,0,1,0,1},  // 对称路径
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},  // 开阔区域
        {1,0,1,0,0,0,1,0,1,0,0,0,1,0,1},  // 中心十字
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},  // 开阔区域
        {1,0,1,0,0,0,0,0,0,0,0,0,1,0,1},  // 对称路径
        {1,0,0,0,1,0,1,1,1,0,1,0,0,0,1},  // 向中心引导
        {1,0,1,0,0,0,0,0,0,0,0,0,1,0,1},  // 通道设计
        {1,0,1,1,0,1,0,1,0,1,0,1,1,0,1},  // 对称墙壁
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},  // 底部空地
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}   // 外围墙壁
        };
        // 复制到实际地图
        for (int i = 0; i < MAZE_HEIGHT; i++) {
            for (int j = 0; j < MAZE_WIDTH; j++) {
                map[i][j] = level3Map[i][j];
            }
        }
        break;
    }
    case 3: {
        int level4Map[MAZE_HEIGHT][MAZE_WIDTH] = {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},  // 外围墙壁
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},  // 顶部入口
        {1,1,1,1,1,1,0,1,1,1,1,1,1,0,1},  // 右侧通道
        {1,0,0,0,0,1,0,1,0,0,0,0,1,0,1},  // 螺旋内侧
        {1,0,1,1,0,1,0,1,0,1,1,0,1,0,1},  // 螺旋墙壁
        {1,0,1,0,0,0,0,0,0,0,0,0,1,0,1},  // 螺旋路径
        {1,0,1,0,1,1,0,1,0,1,0,1,1,0,1},  // 螺旋结构
        {1,0,1,0,0,0,0,0,0,1,0,0,0,0,1},  // 中心区域
        {1,0,1,1,1,1,0,1,1,1,0,1,1,0,1},  // 螺旋结构
        {1,0,0,0,0,1,0,1,0,0,0,0,1,0,1},  // 螺旋路径
        {1,1,1,1,0,1,0,1,0,1,1,0,1,0,1},  // 螺旋墙壁
        {1,0,0,0,0,1,0,1,0,0,0,0,1,0,1},  // 螺旋内侧
        {1,0,1,1,1,1,0,1,1,1,1,1,1,0,1},  // 左侧通道
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},  // 底部出口
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}   // 外围墙壁
        };
        // 复制到实际地图
        for (int i = 0; i < MAZE_HEIGHT; i++) {
            for (int j = 0; j < MAZE_WIDTH; j++) {
                map[i][j] = level4Map[i][j];
            }
        }
        break;
    }
    case 4: {
        int level5Map[MAZE_HEIGHT][MAZE_WIDTH] = {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},  // 外围墙壁
        {1,0,0,0,1,0,0,0,0,0,0,1,0,0,1},  // 顶部岛屿群
        {1,0,1,0,1,0,1,1,1,1,0,1,0,1,1},  // 岛屿分隔
        {1,0,1,0,0,0,1,0,0,1,0,0,0,1,1},  // 桥梁连接
        {1,0,1,1,1,0,1,0,0,1,0,1,1,1,1},  // 水域分隔
        {1,0,0,0,1,0,1,0,0,1,0,1,0,0,1},  // 通道设计
        {1,1,1,0,1,0,1,0,0,1,0,1,0,1,1},  // 岛屿边界
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},  // 中心大岛
        {1,1,1,0,1,0,1,0,0,1,0,1,0,1,1},  // 岛屿边界
        {1,0,0,0,1,0,1,0,0,1,0,1,0,0,1},  // 通道设计
        {1,0,1,1,1,0,1,0,0,1,0,1,1,1,1},  // 水域分隔
        {1,0,1,0,0,0,1,0,0,1,0,0,0,1,1},  // 桥梁连接
        {1,0,1,0,1,0,1,1,1,1,0,1,0,1,1},  // 岛屿分隔
        {1,0,0,0,1,0,0,0,0,0,0,1,0,0,1},  // 底部岛屿群
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}   // 外围墙壁
        };
        // 复制到实际地图
        for (int i = 0; i < MAZE_HEIGHT; i++) {
            for (int j = 0; j < MAZE_WIDTH; j++) {
                map[i][j] = level5Map[i][j];
            }
        }
        break;
    }
    case 5: {
        int level6Map[MAZE_HEIGHT][MAZE_WIDTH] = {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},  // 外围墙壁
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},  // 顶部入口
        {1,1,1,1,1,1,0,1,0,1,1,1,1,0,1},  // 右侧通道
        {1,0,0,0,0,1,0,1,0,1,0,0,1,0,1},  // 螺旋内侧
        {1,0,0,1,0,1,0,0,0,1,0,0,1,0,1},  // 螺旋墙壁
        {1,0,1,0,0,1,0,0,0,1,0,0,1,0,1},  // 螺旋路径
        {1,0,1,0,1,1,0,1,0,1,0,1,1,0,1},  // 螺旋结构
        {1,0,0,0,0,0,0,0,0,1,0,1,1,0,1},  // 中心区域
        {1,0,1,0,1,1,0,1,1,1,0,1,1,0,1},  // 螺旋结构
        {1,0,0,0,0,1,0,1,0,0,0,0,1,0,1},  // 螺旋路径
        {1,0,1,1,0,0,0,0,0,1,1,1,1,0,1},  // 螺旋墙壁
        {1,0,1,0,0,1,0,1,0,0,0,0,1,0,1},  // 螺旋内侧
        {1,0,1,1,1,1,0,1,1,1,1,1,1,0,1},  // 左侧通道
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},  // 底部出口
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}   // 外围墙壁
        };
        // 复制到实际地图
        for (int i = 0; i < MAZE_HEIGHT; i++) {
            for (int j = 0; j < MAZE_WIDTH; j++) {
                map[i][j] = level6Map[i][j];
            }
        }
        break;
    }
    case 6: {
        int level7Map[MAZE_HEIGHT][MAZE_WIDTH] = {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},  // 外围墙壁
        {1,0,1,0,0,0,0,0,0,0,0,0,0,0,1},  // 顶部入口
        {1,0,1,0,1,1,0,0,1,1,1,1,0,0,1},  // 右侧通道
        {1,0,0,0,1,0,0,0,1,0,0,1,0,0,1},  // 螺旋内侧
        {1,0,1,0,1,0,1,1,1,0,1,1,1,0,1},  // 螺旋墙壁
        {1,0,1,0,1,0,0,0,0,0,0,0,0,0,1},  // 螺旋路径
        {1,0,1,0,1,0,1,0,1,0,1,0,1,0,1},  // 螺旋结构
        {1,0,1,0,1,0,0,0,1,1,1,1,1,0,1},  // 中心区域
        {1,0,1,0,1,1,1,0,0,0,0,0,1,0,1},  // 螺旋结构
        {1,0,1,0,1,0,0,0,0,0,1,1,1,0,1},  // 螺旋路径
        {1,0,1,0,1,0,0,1,0,0,1,0,0,0,1},  // 螺旋墙壁
        {1,0,1,0,1,0,0,1,1,1,1,1,1,0,1},  // 螺旋内侧
        {1,0,1,0,1,0,0,1,0,0,0,0,1,0,1},  // 左侧通道
        {1,0,0,0,1,0,0,0,0,1,1,0,0,0,1},  // 底部出口
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}   // 外围墙壁
        };
        // 复制到实际地图
        for (int i = 0; i < MAZE_HEIGHT; i++) {
            for (int j = 0; j < MAZE_WIDTH; j++) {
                map[i][j] = level7Map[i][j];
            }
        }
        break;
    }
    case 7: {
        int level8Map[MAZE_HEIGHT][MAZE_WIDTH] = {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},  // 外围墙壁
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},  // 顶部入口
        {1,1,1,1,1,1,0,1,0,1,1,1,1,0,1},  // 右侧通道
        {1,0,0,0,0,1,0,1,0,1,0,0,0,0,1},  // 螺旋内侧
        {1,0,1,0,0,1,0,0,0,1,0,0,1,0,1},  // 螺旋墙壁
        {1,0,1,1,1,1,0,1,0,1,1,1,1,0,1},  // 螺旋路径
        {1,0,0,0,0,0,0,1,0,0,0,0,0,0,1},  // 螺旋结构
        {1,0,1,0,1,0,0,1,0,1,0,0,1,0,1},  // 中心区域
        {1,0,1,1,1,1,1,1,1,1,1,1,0,0,1},  // 螺旋结构
        {1,0,1,0,0,0,0,0,0,0,0,0,0,0,1},  // 螺旋路径
        {1,0,1,0,1,0,1,0,0,1,0,1,0,0,1},  // 螺旋墙壁
        {1,0,1,0,1,1,1,1,1,1,1,1,0,0,1},  // 螺旋内侧
        {1,0,1,0,0,0,0,1,0,0,0,0,0,0,1},  // 左侧通道
        {1,0,0,0,1,1,0,0,0,0,1,1,1,0,1},  // 底部出口
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}   // 外围墙壁
        };
        // 复制到实际地图
        for (int i = 0; i < MAZE_HEIGHT; i++) {
            for (int j = 0; j < MAZE_WIDTH; j++) {
                map[i][j] = level8Map[i][j];
            }
        }
        break;
    }
    case 8: {
        int level9Map[MAZE_HEIGHT][MAZE_WIDTH] = {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},  // 外围墙壁
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},  // 顶部入口
        {1,0,1,1,1,0,1,0,0,1,0,0,0,0,1},  // 右侧通道
        {1,0,1,0,0,0,0,0,1,0,0,1,0,0,1},  // 螺旋内侧
        {1,0,1,0,1,0,0,1,0,0,1,0,0,0,1},  // 螺旋墙壁
        {1,0,0,0,0,0,1,0,0,1,0,0,1,0,1},  // 螺旋路径
        {1,0,1,0,0,1,0,0,1,0,0,1,0,0,1},  // 螺旋结构
        {1,0,0,0,1,0,0,1,0,0,0,0,0,0,1},  // 中心区域
        {1,0,0,1,0,0,1,0,0,1,0,0,1,0,1},  // 螺旋结构
        {1,0,1,0,0,1,0,0,0,0,0,0,0,0,1},  // 螺旋路径
        {1,0,0,0,1,0,0,1,0,0,1,0,0,0,1},  // 螺旋墙壁
        {1,0,0,1,0,0,1,0,0,1,0,0,1,0,1},  // 螺旋内侧
        {1,0,1,0,0,1,0,0,1,0,0,1,1,0,1},  // 左侧通道
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},  // 底部出口
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}   // 外围墙壁
        };
        // 复制到实际地图
        for (int i = 0; i < MAZE_HEIGHT; i++) {
            for (int j = 0; j < MAZE_WIDTH; j++) {
                map[i][j] = level9Map[i][j];
            }
        }
        break;
    }
    case 9: {
        int level10Map[MAZE_HEIGHT][MAZE_WIDTH] = {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},  // 外围墙壁
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},  // 顶部入口
        {1,0,1,1,1,1,1,0,1,1,1,1,1,0,1},  // 右侧通道
        {1,0,1,0,0,0,1,0,0,0,0,0,1,0,1},  // 螺旋内侧
        {1,0,1,0,1,1,1,0,1,1,0,1,1,0,1},  // 螺旋墙壁
        {1,0,1,0,0,1,1,1,1,1,0,0,1,0,1},  // 螺旋路径
        {1,0,1,0,0,0,0,0,1,0,0,0,1,0,1},  // 螺旋结构
        {1,0,1,1,1,1,0,1,1,0,1,0,1,0,1},  // 中心区域
        {1,0,0,0,0,1,0,0,1,1,1,0,1,0,1},  // 螺旋结构
        {1,1,1,1,0,1,0,0,0,1,0,0,1,0,1},  // 螺旋路径
        {1,0,0,1,0,1,0,1,1,1,0,0,1,0,1},  // 螺旋墙壁
        {1,0,0,0,0,0,0,1,0,0,0,1,1,0,1},  // 螺旋内侧
        {1,0,1,1,1,1,1,1,0,1,1,1,1,0,1},  // 左侧通道
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},  // 底部出口
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}   // 外围墙壁
        };
        // 复制到实际地图
        for (int i = 0; i < MAZE_HEIGHT; i++) {
            for (int j = 0; j < MAZE_WIDTH; j++) {
                map[i][j] = level10Map[i][j];
            }
        }
        break;
    }
    case 10: {
        int level11Map[MAZE_HEIGHT][MAZE_WIDTH] = {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},  // 外围墙壁
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},  // 顶部入口
        {1,0,1,1,1,1,1,1,1,1,1,1,1,0,1},  // 右侧通道
        {1,0,0,0,0,0,0,1,0,0,0,0,0,0,1},  // 螺旋内侧
        {1,0,1,1,1,1,1,1,1,1,1,1,1,0,1},  // 螺旋墙壁
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},  // 螺旋路径
        {1,0,1,1,1,1,1,1,1,1,1,1,1,0,1},  // 螺旋结构
        {1,0,0,0,0,0,0,1,0,0,0,0,0,0,1},  // 中心区域
        {1,0,1,1,1,1,1,1,1,1,1,1,1,0,1},  // 螺旋结构
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},  // 螺旋路径
        {1,0,1,1,1,1,1,1,1,1,1,1,1,0,1},  // 螺旋墙壁
        {1,0,0,0,0,0,0,1,0,0,0,0,0,0,1},  // 螺旋内侧
        {1,0,1,1,1,1,1,1,1,1,1,1,1,0,1},  // 左侧通道
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},  // 底部出口
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}   // 外围墙壁
        };
        // 复制到实际地图
        for (int i = 0; i < MAZE_HEIGHT; i++) {
            for (int j = 0; j < MAZE_WIDTH; j++) {
                map[i][j] = level11Map[i][j];
            }
        }
        break;
    }
    case 11: {
        int level12Map[MAZE_HEIGHT][MAZE_WIDTH] = {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},  // 外围墙壁
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},  // 顶部入口
        {1,0,1,1,1,1,1,1,0,1,1,1,1,0,1},  // 右侧通道
        {1,0,1,0,0,0,0,0,0,0,0,0,1,0,1},  // 螺旋内侧
        {1,0,1,0,1,1,1,1,1,1,1,0,1,0,1},  // 螺旋墙壁
        {1,0,1,0,1,0,0,0,0,0,1,0,1,0,1},  // 螺旋路径
        {1,1,1,0,1,0,1,1,1,0,0,0,1,0,1},  // 螺旋结构
        {1,0,0,0,1,0,1,0,0,0,1,0,1,0,1},  // 中心区域
        {1,0,1,0,1,0,1,0,1,0,1,0,1,0,1},  // 螺旋结构
        {1,0,1,0,1,0,0,0,1,0,1,0,1,0,1},  // 螺旋路径
        {1,0,1,0,1,1,1,1,1,0,1,0,1,0,1},  // 螺旋墙壁
        {1,0,1,0,0,0,0,0,0,0,1,0,1,0,1},  // 螺旋内侧
        {1,0,1,1,1,1,1,1,1,1,1,0,1,0,1},  // 左侧通道
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},  // 底部出口
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}   // 外围墙壁
        };
        // 复制到实际地图
        for (int i = 0; i < MAZE_HEIGHT; i++) {
            for (int j = 0; j < MAZE_WIDTH; j++) {
                map[i][j] = level12Map[i][j];
            }
        }
        break;
    }
    }
}

// 初始化玩家位置
void initPlayer() {
    player = Point(1, 1);  // 玩家从左上角开始
}

// 初始化水果
void initFruits() {
    // 初始化水果收集个数和水果总数为0
    collectedFruits = 0;
    totalFruits = 0;
    // 在空地上随机放置水果
    for (int i = 0; i < MAZE_HEIGHT; i++) {
        for (int j = 0; j < MAZE_WIDTH; j++) {
            if (map[i][j] == 0) {  // 如果是空地
                // 随机决定是否放置水果
                if (rand() % 10 < 1) {
                    map[i][j] = 2;  // 2表示水果
                    // i是行号,j是列号 因此i代表y坐标,j代表x坐标
                    // 将新创建的 Point 实例（表示水果的位置）添加到 fruits 向量的末尾
                    fruits.push_back(Point(j, i));
                    totalFruits++;
                }
            }
        }
    }
}

// 初始化NPC
void initNPCs() {
	// 为游戏添加NPC
    map[6][8] = 3;  // 3表示NPC
    // 将NPC的位置添加到npcs向量中
    npcs.push_back(Point(8, 6));

}

// 移动玩家
void movePlayer(Direction dir) {
	// 如果游戏结束，返回
	if (gameOver) return;

	int newX = player.x;
	int newY = player.y;

	// 根据方向计算新位置
	switch (dir) {
	case UP:
		newY--;
		characterDirection = UP;
		break;
	case DOWN:
		newY++;
		characterDirection = DOWN;
		break;
	case LEFT:
		newX--;
		characterDirection = LEFT;
		break;
	case RIGHT:
		newX++;
		characterDirection = RIGHT;
		break;
	}


	int oldPlayerX = player.x;
	int oldPlayerY = player.y;

	// 检查新位置是否是墙壁
	if (map[newY][newX] != 1 && map[newY][newX] != 6 && map[newY][newX] != 9) {
		// 检查是否吃到水果
		if (map[newY][newX] == 2) {
			collectedFruits++;
            // 播放吃水果音效
            playEatFruitSound();
            // 检查是否收集完所有水果
			map[newY][newX] = 0;  // 移除水果
			// 检查是否收集完所有水果
			if (collectedFruits == totalFruits) {
				gameOver = true;
				gameWin = true;
                // 播放游戏胜利音效
                TCHAR cmd[256];

                _stprintf_s(cmd, _T("open sound/gamewin.mp3 alias gamewin"));
                mciSendString(cmd, NULL, 0, NULL);
                gGameWinSoundOpened = true;

                // 重新从头播放一次
                _stprintf_s(cmd, _T("stop gamewin"));
                mciSendString(cmd, NULL, 0, NULL);
                _stprintf_s(cmd, _T("play gamewin from 0"));
                mciSendString(cmd, NULL, 0, NULL);
              
			}
		}

		// 地图值判定是否与NPC发生碰撞
		if (map[newY][newX] == 3 || map[newY][newX] == 4) {
            // 播放游戏失败音效
            //TCHAR cmd2[256];

            //_stprintf_s(cmd2, _T("open sound/gamelose.mp3 alias gamelose"));
            //mciSendString(cmd2, NULL, 0, NULL);
            //gGameLoseSoundOpened = true;

            //// 重新从头播放一次
            //_stprintf_s(cmd2, _T("stop gamelose"));
            //mciSendString(cmd2, NULL, 0, NULL);
            //_stprintf_s(cmd2, _T("play gamelose from 0"));
            //mciSendString(cmd2, NULL, 0, NULL);

			gameOver = true;
			gameWin = false;
            
		}

		// 更新玩家位置
		player.x = newX;
		player.y = newY;
		gLastPlayerPos = Point(oldPlayerX, oldPlayerY);

		// 坐标直接重合碰撞
		if (!npcs.empty() && player.x == npcs[0].x && player.y == npcs[0].y) {
			gameOver = true;
			gameWin = false;
		}
		// 交换位置碰撞（玩家新==NPC旧 且 NPC新==玩家旧）
		if (!npcs.empty() && player.x == gLastNPCPos.x && player.y == gLastNPCPos.y
			&& npcs[0].x == oldPlayerX && npcs[0].y == oldPlayerY) {
			gameOver = true;
			gameWin = false;
		}
	}
}

// 绘制游戏
void showGameGUI() {

	ifShowGameGUI = true;  // 进入游戏界面

    // 开始双缓冲（在内存中绘制）
    BeginBatchDraw();

    cleardevice();  // 清空屏幕

    // 绘制迷宫
    for (int i = 0; i < MAZE_HEIGHT; i++) {
        for (int j = 0; j < MAZE_WIDTH; j++) {
            int x = j * BLOCK_SIZE;
            int y = i * BLOCK_SIZE;

            // 绘制不同的元素
            if (map[i][j] == 1) {  // 墙壁
				//putimage(x, y, &bricksImg);
				putimage(x, y, &bricksImg);
			}
            else if (map[i][j] == 2) {  // 水果             
                putimage(x + 4, y + 4, &golden_apple_maskImg, NOTSRCERASE);
                putimage(x + 4, y + 4, &golden_appleImg, SRCERASE);
            }
            else if (map[i][j] == 3 || map[i][j] == 4) {  // NPC
                // 绘制NPC（以格子坐标为准，确保与地图状态一致）
                putimage(x + 4, y + 4, &npcImg);
            }
            else if (map[i][j] == 9) {  // 冰块
                putimage(x, y, &iceImg);
            }
            else if (map[i][j] == 6) {  // 藏了水果的冰块
                putimage(x, y, &ice_fruitImg);
            }
        }
    }

    int dirIndex;
    switch (characterDirection) {
    case DOWN: dirIndex = 0; break;
    case LEFT: dirIndex = 1; break;
    case UP:   dirIndex = 2; break;
    case RIGHT:dirIndex = 3; break;
    }

    // 绘制角色
    int playerX = player.x * BLOCK_SIZE;
    int playerY = player.y * BLOCK_SIZE;
    putimage(playerX + 4, playerY +4, &characterImgs[selectedCharacter][dirIndex]);

    // 显示分数
    settextstyle(20, 0, _T("Arial"));
    settextcolor(WHITE);
    TCHAR scoreText[256];
    _stprintf_s(scoreText, _T("已收集: %d/%d"), collectedFruits, totalFruits);
    outtextxy(10, WINDOW_HEIGHT - 30, scoreText);

    EndBatchDraw();
}

// 更新NPC的位置
//666
//void updateNPC() {
//
//    int newX = npcs[0].x;
//    int newY = npcs[0].y;
//
//    if (map[npcs[0].y][npcs[0].x] == 4) {
//		map[npcs[0].y][npcs[0].x] = 2; // 如果NPC原来藏了水果，现在这个位置变成水果
//	}
//	else {
//		map[npcs[0].y][npcs[0].x] = 0; // 否则这个位置变成空地
//    }
//
//    // 根据方向计算新位置
//    switch (npcDirection) {
//
//    case UP:
//        newY--;
//        if (map[newY][newX] == 1 || map[newY][newX] == 6 || map[newY][newX] == 9) {
//            newY++;
//            //npcDirection = RIGHT;
//             // 生成随机数，让RIGHT方向概率最大
//            int randNum = rand() % 6; // 生成0-5的随机数
//            switch (randNum) {
//            case 0:
//            case 1:
//            case 2:
//                npcDirection = RIGHT;  // 3/6的概率，50%
//                break;
//            case 3:
//                npcDirection = LEFT;   // 1/6的概率，约16.7%
//                break;
//            case 4:
//                npcDirection = UP;     // 1/6的概率，约16.7%
//                break;
//            case 5:
//                npcDirection = DOWN;   // 1/6的概率，约16.7%
//                break;
//            }
//        }
//
//        break;
//    case DOWN:
//        newY++;
//        if (map[newY][newX] == 1 || map[newY][newX] == 6 || map[newY][newX] == 9) {
//            newY--;
//            //npcDirection = LEFT;
//            int randNum = rand() % 6; // 生成0-5的随机数
//            switch (randNum) {
//            case 0:
//            case 1:
//            case 2:
//                npcDirection = LEFT;  // 3/6的概率，50%
//                break;
//            case 3:
//                npcDirection = RIGHT;   // 1/6的概率，约16.7%
//                break;
//            case 4:
//                npcDirection = UP;     // 1/6的概率，约16.7%
//                break;
//            case 5:
//                npcDirection = DOWN;   // 1/6的概率，约16.7%
//                break;
//            }
//        }
//
//        break;
//    case LEFT:
//        newX--;
//        if (map[newY][newX] == 1 || map[newY][newX] == 6 || map[newY][newX] == 9) {
//            newX++;
//            //npcDirection = UP;
//            int randNum = rand() % 6; // 生成0-5的随机数
//            switch (randNum) {
//            case 0:
//            case 1:
//            case 2:
//                npcDirection = UP;  // 3/6的概率，50%
//                break;
//            case 3:
//                npcDirection = LEFT;   // 1/6的概率，约16.7%
//                break;
//            case 4:
//                npcDirection = RIGHT;     // 1/6的概率，约16.7%
//                break;
//            case 5:
//                npcDirection = DOWN;   // 1/6的概率，约16.7%
//                break;
//            }
//        }
//
//        break;
//    case RIGHT:
//        newX++;
//        if (map[newY][newX] == 1 || map[newY][newX] == 6 || map[newY][newX] == 9) {
//            newX--;
//            //npcDirection = DOWN;
//            int randNum = rand() % 6; // 生成0-5的随机数
//            switch (randNum) {
//            case 0:
//            case 1:
//            case 2:
//                npcDirection = DOWN;  // 3/6的概率，50%
//                break;
//            case 3:
//                npcDirection = LEFT;   // 1/6的概率，约16.7%
//                break;
//            case 4:
//                npcDirection = UP;     // 1/6的概率，约16.7%
//                break;
//            case 5:
//                npcDirection = RIGHT;   // 1/6的概率，约16.7%
//                break;
//            }
//        }
//
//        break;
//    }
//    // 更新NPC位置
//    npcs[0] = Point(newX, newY);
//    // 如果NPC要去的地方是空地
//    if (map[newY][newX] == 0) { 
//        // 这个地方变成NPC
//        map[npcs[0].y][npcs[0].x] = 3;
//    }
//    // 如果NPC要去的地方是水果
//    else if (map[newY][newX] == 2) {
//        // 这个地方变成藏了水果的NPC
//        /*玩家碰到4会死，4的贴图是3*/
//        map[npcs[0].y][npcs[0].x] = 4;
//    }
//    
//
//    Sleep(500);  
//    
//    
//} 

// 更新NPC的位置
void updateNPC2() {
    // 获取玩家位置和NPC当前位置
    int playerX = player.x;
    int playerY = player.y;
    int npcX = npcs[0].x;
    int npcY = npcs[0].y;

    // 记录NPC原来的位置用于地图更新
    int oldX = npcX;
    int oldY = npcY;

    // 更新地图上原来的NPC位置
    if (map[oldY][oldX] == 4) {
        map[oldY][oldX] = 2; // 如果NPC原来藏了水果，恢复为水果
    }
    else {
        map[oldY][oldX] = 0; // 否则恢复为空地
    }

    int newX = npcX;
    int newY = npcY;

    // 计算NPC与玩家的位置差
    int deltaX = playerX - npcX;
    int deltaY = playerY - npcY;

    npcDirection = chooseNewDirection(deltaX, deltaY);
    // 根据方向计算新位置
    switch (npcDirection) {
    case UP:
        newY--;
        if (map[newY][newX] == 1 || map[newY][newX] == 6 || map[newY][newX] == 9) {
            newY++;
            // 需要改变方向时，优先向玩家方向移动
            /*chooseNewDirection(deltaX, deltaY);*/
            // 在updateNPC函数中这样调用
            npcDirection = chooseNewDirection(deltaX, deltaY);

        }
        break;
    case DOWN:
        newY++;
        if (map[newY][newX] == 1 || map[newY][newX] == 6 || map[newY][newX] == 9) {
            newY--;
            npcDirection = chooseNewDirection(deltaX, deltaY);
        }
        break;
    case LEFT:
        newX--;
        if (map[newY][newX] == 1 || map[newY][newX] == 6 || map[newY][newX] == 9) {
            newX++;
            npcDirection = chooseNewDirection(deltaX, deltaY);
        }
        break;
    case RIGHT:
        newX++;
        if (map[newY][newX] == 1 || map[newY][newX] == 6 || map[newY][newX] == 9) {
            newX--;
            npcDirection = chooseNewDirection(deltaX, deltaY);
        }
        break;
    }

    // 更新NPC位置
    npcs[0] = Point(newX, newY);

    // 更新地图上的NPC新位置
    if (map[newY][newX] == 0) {
        map[newY][newX] = 3; // 空地变为NPC
    }
    else if (map[newY][newX] == 2) {
        map[newY][newX] = 4; // 水果位置变为藏有水果的NPC
    }

    Sleep(50);
}

// 根据玩家位置选择新方向，让NPC更倾向于追踪玩家
Direction chooseNewDirection(int deltaX, int deltaY) {
    // 确定优先方向（朝向玩家的方向）
    Direction preferredDirection = npcDirection; // 默认保持当前方向

    // 根据位置差确定优先追踪方向
    if (abs(deltaX) > abs(deltaY)) {
        // X方向差距更大，优先水平移动
        preferredDirection = (deltaX > 0) ? RIGHT : LEFT;
    }
    else {
        // Y方向差距更大，优先垂直移动
        preferredDirection = (deltaY > 0) ? DOWN : UP;
    }

    // 生成随机数，让优先方向（追踪方向）有最高概率
    int randNum = rand() % 6; // 0-5的随机数
    switch (randNum) {
    case 0:
    case 1:
    case 2:
        return preferredDirection;  // 3/6的概率，50%

    case 3:
        return (preferredDirection == RIGHT) ? LEFT : RIGHT;  // 相反水平方向
    case 4:
        return UP;
    case 5:
        return DOWN;   // 下方向;
    }
    return preferredDirection;
}


// 检查游戏是否结束
bool isGameOver() {
    return gameOver;
}


// 移动开始菜单的光标
//void movePoint(Direction dir) {
//    switch (dir) {
//    case UP:
//        currentPoint = (currentPoint - 1 + 5) % 5;
//        break;
//    case DOWN:
//        currentPoint = (currentPoint + 1 + 5) % 5;
//        break;
//    }
//}

// 发射/破坏冰块
void lauchBreakIce() {
    // 启动冰块发射监听线程（处理按键）
    if (pICEThread == nullptr || !pICEThread->joinable()) {
        if (pICEThread != nullptr) {
            delete pICEThread;
        }
        pICEThread = new thread([=] {
            // 判断玩家面前是否有冰块
            int DX, DY;
            switch (characterDirection) {
            case UP:
                DX = 0;
                DY = -1;
                break;
            case DOWN:
                DX = 0;
                DY = 1;
                break;
            case LEFT:
                DX = -1;
                DY = 0;
                break;
            case RIGHT:
                DX = 1;
                DY = 0;
                break;
            }
            int faceX = player.x + DX;
            int faceY = player.y + DY;
            int faceX_first = faceX;
            int faceY_first = faceY;

            if (map[faceY_first][faceX_first] == 0 || map[faceY_first][faceX_first] == 2) {
                while (map[faceY][faceX] == 0 || map[faceY][faceX] == 2) {
                    switch (map[faceY][faceX]) {
                    case 0:
                        map[faceY][faceX] = 9;
                        break;
                    case 2:
                        map[faceY][faceX] = 6;
                        break;
                    }
                    faceX += DX;
                    faceY += DY;
                    Sleep(400);
                }
            }

            else if (map[faceY_first][faceX_first] == 6 || map[faceY_first][faceX_first] == 9) {
                while (map[faceY][faceX] == 6 || map[faceY][faceX] == 9) {
                    switch (map[faceY][faceX]) {
                    case 6:
                        map[faceY][faceX] = 2;
                        break;
                    case 9:
                        map[faceY][faceX] = 0;
                        break;
                    }
                    faceX += DX;
                    faceY += DY;
                    Sleep(400);
                }   
            }

            });
    }

 
    // 关闭线程
    if (pICEThread != nullptr) {
        pICEThread->detach();
        delete pICEThread;
        pICEThread = nullptr;
    }
}




// 初始化迷宫
void InitMaze(int selectedLevel) {
    initMap(selectedLevel);  // 给迷宫赋值
    initPlayer(); // 给玩家的位置赋值
    initNPCs();
    initFruits(); // 给水果的位置赋值
	// 新关卡，递增路径会话编号以触发 updateNPC3 的静态缓存重置
	gPathSessionId++;
}


// 绘制开始菜单
void showGameStartMenuGUI() {
    BeginBatchDraw();
    cleardevice();
    putimage(0, 0, &startMenuImg);
    EndBatchDraw();

    MOUSEMSG msg;
    bool isHovered[5] = { false };
    int selectedLevel;
    bool ifShowGameStartMenuGUI = true;


    while (ifShowGameStartMenuGUI == true) {
        /*BeginBatchDraw();*/
        //  定义角色选择区域（每个角色100x100像素，间隔50像素）
       
        RECT charRects[5] = {
        {WINDOW_WIDTH / 2 - 100, 200, WINDOW_WIDTH / 2 + 100, 250},  // 开始游戏
        {WINDOW_WIDTH / 2 - 100, 270, WINDOW_WIDTH / 2 + 100, 320},  // 游戏说明
        {WINDOW_WIDTH / 2 - 100, 340-1, WINDOW_WIDTH / 2 + 100, 390-1},  // 游戏设置
        {WINDOW_WIDTH / 2 - 100, 410-6, WINDOW_WIDTH / 2 + 100, 460-6},   // 关于我们
        {WINDOW_WIDTH / 2 - 100, 480-10, WINDOW_WIDTH / 2 + 100, 530-10}   // 退出游戏
        };

        // 处理鼠标事件
        if (MouseHit()) {
            msg = GetMouseMsg();
            // 遍历5个区域，更新各自的悬停状态（isHovered）
            for (int i = 0; i < 5; i++) {
                // 遍历5个区域哪个区域被鼠标悬停
                isHovered[i] = (msg.x >= charRects[i].left && msg.x <= charRects[i].right
                    && msg.y >= charRects[i].top && msg.y <= charRects[i].bottom);
            }
            // 处理鼠标左键点击（选择角色）
            if (msg.uMsg == WM_LBUTTONDOWN) {
                for (int i = 0; i < 5; i++) {
                    // 如果被点击的位置是上面5个区域中悬停的某一个
                    if (isHovered[i]) {
                        currentPoint = i;
                        // 进入其他页面 
                        ifShowGameStartMenuGUI = false;
                        enterDifferentMenu(currentPoint);
                        return;      // 退出函数，避免后续绘制
                    }
                }
            }
        }

        BeginBatchDraw();

        // 绘制矩形框，参数为：左上角x, 左上角y, 右下角x, 右下角y
        for (int i = 0; i < 5; i++) {
            if (isHovered[i]) {   
                putimage(0, 0, &startMenuImgs[i]);
                break;
            }
            else {
                putimage(0, 0, &startMenuImg);
            }

        }

        EndBatchDraw();

        Sleep(5);  // 降低CPU占用，避免循环过快
    }

}

// 选择不同的角色（支持鼠标悬停逐帧动画）
void showChooseCharacterGUI() {
    // 1. 初始化界面状态
    ifShowChooseCharacterGUI = true;
    setbkcolor(BLACK);
    cleardevice();

    // 角色选择区域（保持原尺寸：130*300/135*300）
    RECT charRects[3] = {
        {50, 200, 180, 500},   // 角色1区域（左：50，右：180 → 宽130；上：200，下：500 → 高300）
        {235, 200, 370, 500},  // 角色2区域（左：235，右：370 → 宽135；高300）
        {420, 200, 550, 500}   // 角色3区域（左：420，右：550 → 宽130；高300）
    };

    // 动画控制变量（局部变量，仅作用于当前界面）
    const int TOTAL_FRAMES = 24;    // 每个角色的总动画帧数（0-23）
    int currentFrame[3] = { 0, 0, 0 };// 3个角色各自的当前帧索引（初始第0帧）
    bool isHovered[3] = { false };    // 3个角色是否被鼠标悬停（初始未悬停）
    DWORD lastFrameTime[3] = { 0 };   // 上一帧播放时间（控制动画速度）
    const int FRAME_DELAY = 40;     // 每帧间隔（毫秒），40ms=25帧/秒（可调整速度）

    // 提示文字（初始绘制一次，后续双缓冲需重绘）
    settextstyle(24, 0, _T("微软雅黑"));
    settextcolor(WHITE);
    RECT tipRect = { 0, 100, WINDOW_WIDTH, 150 };  // 文字居中显示
    drawtext(_T("请选择角色"), &tipRect, DT_CENTER | DT_VCENTER);

    MOUSEMSG msg = { 0 };
    // 2. 界面主循环（持续处理输入和绘制）
    while (ifShowChooseCharacterGUI == true) {
        // -------------------------- 步骤1：处理鼠标事件（更新悬停状态） --------------------------
        if (MouseHit()) {
            msg = GetMouseMsg();
            // 遍历3个角色，更新各自的悬停状态（isHovered）
            for (int i = 0; i < 3; i++) {
                isHovered[i] = (msg.x >= charRects[i].left && msg.x <= charRects[i].right
                    && msg.y >= charRects[i].top && msg.y <= charRects[i].bottom);
            }
            // 处理鼠标左键点击（选择角色）
            if (msg.uMsg == WM_LBUTTONDOWN) {
                for (int i = 0; i < 3; i++) {
                    if (isHovered[i]) {  // 点击了悬停的角色
                        selectedCharacter = i;
                        ifShowChooseCharacterGUI = false;  // 关闭当前界面
						// 进入选择关卡界面
                        showChooseLevelGUI();    
                        return;      // 退出函数，避免后续绘制
                    }
                }
            }
        }

        // -------------------------- 步骤2：更新动画帧（控制播放速度） --------------------------
        DWORD currentTime = GetTickCount();  // 获取当前系统时间（毫秒）
        for (int i = 0; i < 3; i++) {
            // 仅当角色被悬停，且距离上一帧超过FRAME_DELAY，才更新到下一帧
            if (isHovered[i] && (currentTime - lastFrameTime[i] >= FRAME_DELAY)) {
                currentFrame[i] = (currentFrame[i] + 1) % TOTAL_FRAMES;  // 循环播放（0→23→0）
                lastFrameTime[i] = currentTime;  // 记录当前帧时间，避免过快
            }
        }

        // -------------------------- 步骤3：双缓冲绘制（避免动画闪烁） --------------------------
        BeginBatchDraw();  // 开始双缓冲：所有绘制先在内存中完成
        cleardevice();     // 清空内存画布（避免上一帧残留）

        // 重绘提示文字（双缓冲需重新绘制所有内容）
        drawtext(_T("请选择角色"), &tipRect, DT_CENTER | DT_VCENTER);

        // 绘制3个角色（按当前帧绘制，悬停播放动画，未悬停显示第0帧）
        for (int i = 0; i < 3; i++) {
            // 绘制角色图片：悬停时用当前帧，未悬停时固定显示第0帧
            if (isHovered[i]) {
                putimage(charRects[i].left, charRects[i].top, &characterSelectImg[i][currentFrame[i]]);
            }
            else {
                putimage(charRects[i].left, charRects[i].top, &characterSelectImg[i][0]);
            }
            // 绘制选择框：悬停时黄色高亮，未悬停时灰色
            //setlinecolor(isHovered[i] ? YELLOW : LIGHTGRAY);
            //setlinestyle(PS_SOLID, 3);  // 3像素粗实线
            //rectangle(charRects[i].left, charRects[i].top, charRects[i].right, charRects[i].bottom);
        }

        EndBatchDraw();  // 结束双缓冲：将内存中的画面一次性显示到屏幕

        Sleep(5);  // 降低CPU占用，避免循环过快
    }
}

// 显示游戏说明页面
void showGameGuideGUI() {
    ifShowGameGuideGUI = true;

    setbkcolor(WHITE);
    cleardevice();
    settextstyle(24, 0, _T("微软雅黑"));
    settextcolor(BLACK);

    // 绘制游戏说明界面
    IMAGE GameInstructionImg;
    loadimage(&GameInstructionImg, _T("res/GameInstruction.png"), 600, 600);
	putimage(0,0, &GameInstructionImg);  // 绘制背景图);

    // 如果是游戏玩法说明界面，监听键盘事件}
    while (_kbhit()) {
		_getch(); // 清空按键缓冲区，避免误触发
    }
    while (ifShowGameGuideGUI == true) {
        if (_kbhit()) {
            int ch = _getch();
            switch (ch) {
            case 27: // ESC键退出游戏 
                ifShowGameGuideGUI = false;
                showGameStartMenuGUI();
                break;
            }
        }
        Sleep(10); // 降低CPU占用
    }

   
}

// 显示游戏设置页面
void showGameSettingsGUI() {
    // 标记进入设置界面
    bool ifShowSettings = true;

    // 滑条区域定义（位置和尺寸）
    const int SLIDER_X = WINDOW_WIDTH / 4;         // 滑条左上角X坐标
    const int SLIDER_Y = WINDOW_HEIGHT / 2;        // 滑条左上角Y坐标
    const int SLIDER_WIDTH = WINDOW_WIDTH / 2;     // 滑条总宽度
    const int SLIDER_HEIGHT = 20;                  // 滑条高度
    const int THUMB_RADIUS = 15;                   // 滑块半径

    while (_kbhit()) {
        _getch(); // 清空按键缓冲区，避免误触发
    }

    // 绘制设置界面
    while (ifShowSettings) {
        // 开始双缓冲绘制（避免闪烁）
        BeginBatchDraw();

        // 1. 清空屏幕并绘制背景
        setbkcolor(BLACK);
        cleardevice();

        // 2. 绘制标题
        settextstyle(40, 0, _T("Arial"));
        settextcolor(WHITE);
        RECT titleRect = { 0, 50, WINDOW_WIDTH, 100 };
        drawtext(_T("游戏设置"), &titleRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        // 3. 绘制音量文字说明
        settextstyle(24, 0, _T("Arial"));
        TCHAR volumeText[256];
          
        _stprintf_s(volumeText, _T("背景音乐音量: %d%%"), musicVolume);
        RECT textRect = { SLIDER_X, SLIDER_Y - 40, SLIDER_X + SLIDER_WIDTH, SLIDER_Y - 10 };
        drawtext(volumeText, &textRect, DT_LEFT | DT_VCENTER);

        // 4. 绘制滑条背景（灰色长条）
        setfillcolor(DARKGRAY);
        solidrectangle(SLIDER_X, SLIDER_Y, SLIDER_X + SLIDER_WIDTH, SLIDER_Y + SLIDER_HEIGHT);

        // 5. 绘制滑条已选中部分（绿色长条）
        int progressWidth = (SLIDER_WIDTH * musicVolume) / 100;  // 根据音量计算长度
        setfillcolor(GREEN);
        solidrectangle(SLIDER_X, SLIDER_Y, SLIDER_X + progressWidth, SLIDER_Y + SLIDER_HEIGHT);

        // 6. 绘制滑块（白色圆形）
        int thumbX = SLIDER_X + progressWidth;  // 滑块X坐标（随音量变化）
        setfillcolor(WHITE);
        solidcircle(thumbX, SLIDER_Y + SLIDER_HEIGHT / 2, THUMB_RADIUS);

        // 7. 绘制返回提示
        settextstyle(20, 0, _T("Arial"));
        settextcolor(LIGHTGRAY);
        RECT backRect = { 0, WINDOW_HEIGHT - 40, WINDOW_WIDTH, WINDOW_HEIGHT };
        drawtext(_T("按ESC键返回主菜单"), &backRect, DT_CENTER | DT_VCENTER);

        // 结束双缓冲绘制（一次性显示）
        EndBatchDraw();

        // 8. 处理鼠标和键盘输入
        if (_kbhit()) {
            int key = _getch();
            if (key == 27) {  // ESC键返回主菜单
                ifShowSettings = false;
                showGameStartMenuGUI();  // 回到主菜单
                break;
            }
        }

        // 处理鼠标事件（拖动滑条）
        MOUSEMSG msg;
        if (MouseHit()) {
            msg = GetMouseMsg();

            // 鼠标左键按下或拖动时更新音量
            if (msg.uMsg == WM_LBUTTONDOWN || msg.uMsg == WM_MOUSEMOVE && msg.mkLButton) {
                // 检查鼠标是否在滑条区域内
                if (msg.y >= SLIDER_Y - THUMB_RADIUS &&
                    msg.y <= SLIDER_Y + SLIDER_HEIGHT + THUMB_RADIUS) {

                    // 计算新音量（限制在0-100之间）
                    int newVolume = ((msg.x - SLIDER_X) * 100) / SLIDER_WIDTH;
                    if (newVolume < 0) newVolume = 0;
                    if (newVolume > 100) newVolume = 100;

                    // 更新音量值（实际项目中可在此处调用音频API设置音量）
                   // musicVolume = newVolume;
                    // 只有音量发生变化时才更新（避免重复发送命令）
                   
                        musicVolume = newVolume;

                        // 调整系统音量
                        // showGameSettingsGUI函数中使用waveOutSetVolume来调整系统的音量。这个函数影响的是整个系统的音量，而不是单独的音频文件。
                        DWORD newVolumeSetting = (DWORD)((musicVolume * 0xFFFF / 100) | ((musicVolume * 0xFFFF / 100) << 16));
                        waveOutSetVolume(NULL, newVolumeSetting);
                    
                }
            }
        }

        Sleep(2);  // 降低CPU占用
    }
}

// 显示"关于我们"页面（包含自定义图片）
void showAboutUSGUI() {
    // 1. 初始化页面背景（白色背景，清空之前的内容）
    setbkcolor(WHITE);
    cleardevice();

    // 2. 定义图片变量（EasyX 用 IMAGE 类型存储图片）
    IMAGE aboutImage;

    // 3. 加载自定义图片（关键步骤！需替换为你的图片路径和尺寸）
    // 参数说明：
    // &aboutImage：图片存储的变量
    // _T("res/about_us.jpg")：图片文件路径（支持 jpg/png/bmp 等格式）
    // 800, 500：图片显示的宽度和高度（可根据窗口尺寸调整）
    // 注意：路径分为"相对路径"和"绝对路径"，推荐用相对路径（避免移植报错）
    loadimage(&aboutImage, _T("res/about_us.png"), 600, 600);

    // 4. 检查图片是否加载成功（避免路径错误导致黑屏）
    //if (!isLoadSuccess) {
    //    // 加载失败时显示提示文字
    //    settextstyle(30, 0, _T("Arial"));
    //    settextcolor(RED);
    //    RECT tipRect = { 0, 200, WINDOW_WIDTH, 250 }; // 提示文字区域
    //    drawtext(_T("图片加载失败！请检查图片路径是否正确"), &tipRect, DT_CENTER | DT_VCENTER);
    //    return; // 加载失败，直接返回，不执行后续绘制
    //}

    // 5. 绘制图片到窗口（居中显示，避免图片偏移）
    // 计算图片居中的坐标：(窗口宽度-图片宽度)/2，(窗口高度-图片高度)/2
    int imgX = (WINDOW_WIDTH - 600) / 2;
    int imgY = (WINDOW_HEIGHT - 600) / 2;
    putimage(imgX, imgY, &aboutImage); // 绘制图片到指定位置

    // 6. （可选）添加"返回菜单"提示文字
    settextstyle(24, 0, _T("Arial"));
    settextcolor(BLACK);
    RECT backTipRect = { 0, WINDOW_HEIGHT - 50, WINDOW_WIDTH, WINDOW_HEIGHT };
    drawtext(_T("按 ESC 键返回主菜单"), &backTipRect, DT_CENTER | DT_VCENTER);

    while (_kbhit()) {
        _getch(); // 清空按键缓冲区，避免误触发
    }

    while (true) { 
        if (_kbhit()) {
            int key = _getch();
            if (key == 27) {  // ESC键返回主菜单
                showGameStartMenuGUI();  // 回到主菜单
                break;
            }
        }
        Sleep(2);
    }
}

// 显示游戏关卡选择页面
void showChooseLevelGUI() {
   /* BeginBatchDraw();*/
    //setbkcolor(WHITE);
    cleardevice();
    putimage(0, 0, &chooseLevelImg);

    MOUSEMSG msg;
    bool isHovered[12] = { false };
    int selectedLevel;
    bool ifShowChooseLevelGUI = true;


    while (ifShowChooseLevelGUI == true) {
        BeginBatchDraw();
        //  定义角色选择区域（每个角色100x100像素，间隔50像素）
        RECT charRects[12] = {
            {167, 84, 225, 140},   
            {267, 84, 324, 140},   
            {368, 84, 429, 140}, 

            {167, 206, 225, 267},   
            {267, 206, 332, 267},   
            {368, 206, 432, 267},   

            {167, 327, 225, 385},   
            {267, 327, 332, 385},
            {368, 327, 432, 385},

            {167, 450, 225, 506},
            {267, 450, 332, 506},
            {368, 450, 432, 506}
              
        };
        // 处理鼠标事件
        if (MouseHit()) {
            msg = GetMouseMsg();
            // 遍历12个关卡，更新各自的悬停状态（isHovered）
            for (int i = 0; i < 12; i++) {
                // 遍历12个区域哪个区域被鼠标悬停
                isHovered[i] = (msg.x >= charRects[i].left && msg.x <= charRects[i].right
                    && msg.y >= charRects[i].top && msg.y <= charRects[i].bottom);
            }
            // 处理鼠标左键点击（选择角色）
            if (msg.uMsg == WM_LBUTTONDOWN) {
                for (int i = 0; i < 12; i++) {
                    // 如果被点击的位置是上面12个区域中悬停的某一个
                    if (isHovered[i]) {  
                        selectedLevel = i; 
                        // 进入选择关卡界面 
                        ifShowChooseLevelGUI = false;
                        /*gameLoop(selectedLevel);*/
						levelManager(selectedLevel); 
                        return;      // 退出函数，避免后续绘制
                    }
                }
            }
        }

        // 设置画笔颜色为青色
        setlinecolor(RGB(0, 249, 176));

        // 设置画笔宽度为3像素
        setlinestyle(PS_SOLID, 3);

        // 绘制矩形框，参数为：左上角x, 左上角y, 右下角x, 右下角y
        for (int i = 0; i < 12; i++) {
            if (isHovered[i]) {
                /*setlinecolor(RGB(0, 249, 176));
                rectangle(charRects[i].left, charRects[i].top, charRects[i].right, charRects[i].bottom);*/
                putimage(0, 0, &chooseLevelImgs[i]);
            }
            else{
                /*setlinecolor(RGB(255, 255, 255));
                rectangle(charRects[i].left, charRects[i].top, charRects[i].right, charRects[i].bottom);*/
                //putimage(charRects[i].left, charRects[i].top, &chooseLevelImgs[i]);
            }
           
        }
        
         EndBatchDraw();

        Sleep(5);  // 降低CPU占用，避免循环过快
    }





    
   
}

// 进入不同的页面
void enterDifferentMenu(int currentPoint) {
    switch (currentPoint) {
    case 0:
        //开始游戏
        showChooseCharacterGUI();
        break;
    case 1:
        //游戏说明
        showGameGuideGUI();
        break;
    case 2:
        //游戏设置
        showGameSettingsGUI();
        break;
    case 3:
        //关于我们
        showAboutUSGUI();
         break;
    case 4:
        //退出游戏
        exit(0);
        break;
    }
}

// 关卡管理器：负责启动关卡、处理关卡切换（无嵌套）
//666
void levelManager(int startLevel) {
    int currentLevel = startLevel;
    while (true) { // 循环：根据玩家选择启动对应关卡
        GameQuitChoice choice = gameLoop(currentLevel); // 调用gameLoop，等待其正常返回
        switch (choice) {
        case GameQuitChoice::RESTART:
            // 重新开始当前关卡（currentLevel不变）
            break;
        case GameQuitChoice::NEXT_LEVEL:
            // 下一关（currentLevel+1）
            currentLevel++;
            // 可选：判断是否超过最大关卡（如5关）
            if (currentLevel > 11) {
                MessageBox(NULL, _T("已通关所有关卡！"), _T("恭喜"), MB_OK);
                currentLevel = 0; // 重置为第一关
            }
            break;
        case GameQuitChoice::BACK_MENU:
            // 返回主菜单（退出关卡管理器）
            showGameStartMenuGUI();
            return;
        }
    }
}

// 游戏主循环：输入→更新→绘制
// 1.初始化地图（地图，水果，npc）
// 2.创建监听游戏的线程（监听玩家的按键输入w,a,s,d,esc）和更新npc的线程(定时更新npc的位置和方向)
// 3.如果游戏没有结束，刷新游戏画面
// 4.游戏结束后显示胜利/失败界面，并且监听鼠标点击事件来进入不同界面
GameQuitChoice gameLoop(int selectedLevel) {
    ifShowGameGUI = true;  // 标记进入游戏界面
    InitMaze(selectedLevel);            // 重新初始化迷宫（避免多次启动游戏时状态残留）
    
    // 启动NPC移动监听线程
    if (pNPCThread == nullptr || !pNPCThread->joinable()) {
        if (pNPCThread != nullptr) {
            delete pNPCThread;
        }
        pNPCThread = new std::thread([=] {
            while (ifShowGameGUI == true) {
                updateNPC3();  // A* 移动
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
            });
    }


    while (_kbhit()) {
        _getch();
    }
    while (ifShowGameGUI == true) {
        showGameGUI();
        if (_kbhit()) {
            int ch = _getch();
            switch (ch) {
            case 72: case 'w': case 'W':
                movePlayer(UP);
                break;
            case 80: case 's': case 'S':
                movePlayer(DOWN);
                break;
            case 75: case 'a': case 'A':
                movePlayer(LEFT);
                break;
            case 77: case 'd': case 'D':
                movePlayer(RIGHT);
                break;
            case ' ': // 空格键
                // 发射/破坏冰块
                  // 计算当前时间与上次发射的时间差
                current_time = GetTickCount(); // 获取当前系统时间（毫秒）
                // 检查是否超过冷却时间
                if (current_time - last_launch_time >= ICE_COOLDOWN) {
                    lauchBreakIce(); // 执行发射/破坏操作
                    last_launch_time = current_time; // 更新上次发射时间
                }
                // 如果没有写这个brak会怎么样：
                // 按下空格键时，不仅会触发 lauchBreakIce();（发射 / 破坏冰块功能）
                // 还会 同时执行 case 27 的逻辑：将 gameOver 设为 true，gameWin 设为 false，导致游戏直接结束
                break;
            case 27: // ESC键退出游戏
                gameOver = true;
                gameWin = false;
                break;
            }
        }

        if (gameOver) {
            ifShowGameGUI = false;
            // 游戏结束时清理线程
            if (pNPCThread != nullptr && pNPCThread->joinable()) {
                pNPCThread->join(); // 等待NPC线程结束
                delete pNPCThread;
                pNPCThread = nullptr;
            }
        }
        Sleep(10); // 降低CPU占用
     
    }

    


    

    // 游戏持续循环：直到游戏结束
    //while (!gameOver && ifShowGameGUI) { 
    //    showGameGUI();  // 持续绘制游戏画面（确保画面实时更新）
    //    Sleep(10);      // 控制帧率，降低CPU占用
    //}
     
    // 当游戏结束后
    // 游戏结束后：记录玩家选择
    GameQuitChoice userChoice = GameQuitChoice::BACK_MENU; // 默认返回主菜单
    while (gameOver == true) {
        BeginBatchDraw();
        if (gameWin == true) {
            cleardevice();
            putimage(0, 0, &gameWinImg);          
        }
        else {
            cleardevice();
            putimage(0, 0, &gameLoseImg);
        }   

        // 处理鼠标事件
        MOUSEMSG msg;
		RECT charRects[3] = {
		{148, 200, 450, 268},   // 重新开始
		{150, 300, 450, 375},   // 返回主菜单
		{150, 405, 450, 475}    // 退出游戏
		};
        bool isHovered[3] = { false };
        int selectedChoice;

      

        if (MouseHit()) {
            msg = GetMouseMsg();
            // 遍历3个选项，更新各自的悬停状态（isHovered）
            for (int i = 0; i < 3; i++) {
                isHovered[i] = (msg.x >= charRects[i].left && msg.x <= charRects[i].right
                    && msg.y >= charRects[i].top && msg.y <= charRects[i].bottom);
            }
            // 处理鼠标左键点击
            if (msg.uMsg == WM_LBUTTONDOWN) {
                for (int i = 0; i < 3; i++) {
                    if (isHovered[i]) {  // 点击了悬停的角色

                        switch (i) {
                        case 0: userChoice = GameQuitChoice::RESTART;
                            break;
                        case 1: userChoice = GameQuitChoice::NEXT_LEVEL;
                            break; 
                        case 2: userChoice = GameQuitChoice::BACK_MENU;
                            break;
                        }
                        gameOver = false;
                        break;
                    }
                }
            }

            for (int i = 0; i < 3; i++) {   
				if (isHovered[i]) {
                    if (gameWin == true) {
                        putimage(0, 0, &gameWinImgs[i]);
                    }
                    else {
                        putimage(0, 0, &gameLoseImgs[i]);
                    }
					
				}
            }

            EndBatchDraw();
        }

        Sleep(5);
    }




    // 游戏结束后，关闭线程
    ifShowGameGUI = false;
    // 关闭游戏监听线程
    /*if (pGameThread != nullptr) {
        pGameThread->detach();
        delete pGameThread;
        pGameThread = nullptr;
    }*/
    // 关闭NPC移动监听线程
    if (pNPCThread != nullptr) {
        pNPCThread->detach();
        delete pNPCThread;
        pNPCThread = nullptr;
    }


    // 游戏结束后返回主菜单（可选）
    gameOver = false;  // 重置游戏状态
    gameWin = false;
    collectedFruits = 0;
    totalFruits = 0;
    fruits.clear();
    npcs.clear();
    
    return userChoice;
}


// -----A星寻路算法-----
// 地图格子状态检查（需要根据实际地图实现）
bool isWalkable(int x, int y) {
	// 边界检查
	if (x < 0 || x >= MAZE_WIDTH || y < 0 || y >= MAZE_HEIGHT) {
		return false;
	}
	// 障碍物：1 墙，6/9 冰块
	if (map[y][x] == 1 || map[y][x] == 6 || map[y][x] == 9) {
		return false;
	}
	return true;
}

// A*算法中的节点结构体，用于表示网格中的一个位置及相关路径信息
struct Node {
	int x, y;           // 坐标
	int gCost;          // 起点到当前节点的代价
	int hCost;          // 当前节点到终点的预估代价
	Node* parent;       // 指向父节点的指针，用于回溯完整路径

	// 节点构造函数
	Node(int x_, int y_, Node* parent_)
		// 初始化x,y坐标，父节点指针，gCost和hCost初始为0
		: x(x_), y(y_), parent(parent_), gCost(0), hCost(0) {
	}

	// 计算总代价fCost = gCost + hCost，用于节点优先级排序
	int fCost() const { return gCost + hCost; }
};

// A*开放列表比较器（小顶堆按fCost）
struct NodeCompare {
	bool operator()(const Node* a, const Node* b) const {
		if (a->fCost() == b->fCost()) {
			// fCost 相同，hCost 小者优先（更接近目标）
			return a->hCost > b->hCost;
		}
		return a->fCost() > b->fCost();
	}
};

// 计算曼哈顿距离（启发函数）
int manhattanDistance(int x1, int y1, int x2, int y2) {
    return abs(x1 - x2) + abs(y1 - y2);
}

// 查找路径
vector<Point> findPath(int startX, int startY, int targetX, int targetY) {
	priority_queue<Node*, vector<Node*>, NodeCompare> openList;
	unordered_map<int, unordered_map<int, Node*>> nodes;

	Node* startNode = new Node(startX, startY, nullptr);
	startNode->hCost = manhattanDistance(startX, startY, targetX, targetY);
	openList.push(startNode);
	nodes[startX][startY] = startNode;

	// 4方向移动（上下左右）
	int dirs[4][2] = { {0, 1}, {0, -1}, {1, 0}, {-1, 0} };

	while (!openList.empty()) {
		Node* currentNode = openList.top();
		openList.pop();

		if (currentNode->x == targetX && currentNode->y == targetY) {
			vector<Point> path;
			while (currentNode != nullptr) {
				path.emplace_back(currentNode->x, currentNode->y);
				currentNode = currentNode->parent;
			}
			reverse(path.begin(), path.end());

			for (auto& xEntry : nodes) {
				for (auto& yEntry : xEntry.second) {
					delete yEntry.second;
				}
			}
			return path;
		}

		for (auto& dir : dirs) {
			int newX = currentNode->x + dir[0];
			int newY = currentNode->y + dir[1];

			if (!isWalkable(newX, newY)) {
				continue;
			}

			int newGCost = currentNode->gCost + 10; // 直线代价

			if (nodes.find(newX) == nodes.end() || nodes[newX].find(newY) == nodes[newX].end()) {
				Node* newNode = new Node(newX, newY, currentNode);
				newNode->gCost = newGCost;
				newNode->hCost = manhattanDistance(newX, newY, targetX, targetY);
				openList.push(newNode);
				nodes[newX][newY] = newNode;
			}
			else {
				Node* existingNode = nodes[newX][newY];
				if (newGCost < existingNode->gCost) {
					existingNode->parent = currentNode;
					existingNode->gCost = newGCost;
					openList.push(existingNode);
				}
			}
		}
	}

	for (auto& xEntry : nodes) {
		for (auto& yEntry : xEntry.second) {
			delete yEntry.second;
		}
	}

	return {};
}

// NPC移动更新函数
void updateNPC() {
    // 检查NPC列表是否为空
    if (npcs.empty()) {
        return;
    }

    // 获取玩家位置和NPC当前位置
    int playerX = player.x;
    int playerY = player.y;
    int npcX = npcs[0].x;
    int npcY = npcs[0].y;

    // 如果已经到达玩家位置，不需要移动
    if (npcX == playerX && npcY == playerY) {
        return;
    }

    // 静态变量用于跟踪路径状态，避免每帧重新计算
    static Point lastPlayerPos;    // 记录上一帧玩家位置
    static vector<Point> path;     // 当前路径缓存
    static int currentStep = 0;    // 当前移动到的步骤索引

    // 玩家位置变化或路径已走完，需要重新计算路径
    if (playerX != lastPlayerPos.x || playerY != lastPlayerPos.y || currentStep >= path.size() - 1) {
        path = findPath(npcX, npcY, playerX, playerY);
        currentStep = 0;
        lastPlayerPos = player;  // 更新玩家位置记录
    }

    // 如果找到有效路径，移动NPC到下一步
    if (path.size() > currentStep + 1) {
        currentStep++;
        npcs[0].x = path[currentStep].x;
        npcs[0].y = path[currentStep].y;
    }
}

void updateNPC3() {
	if (npcs.empty()) {
		return;
	}

	int playerX = player.x;
	int playerY = player.y;
	int npcX = npcs[0].x;
	int npcY = npcs[0].y;

	// 同格即失败（防止极端情况下重叠未被判定）
	if (npcX == playerX && npcY == playerY) {
		gameOver = true;
		gameWin = false;
		return;
	}

	// 缓存路径，玩家移动或走完路径时重算；并在会话变化时重置
	static Point lastPlayerPos(-1, -1);
	static vector<Point> path;
	static size_t currentStep = 0;
	static int localSessionId = -1;
	if (localSessionId != gPathSessionId) {
		path.clear();
		currentStep = 0;
		lastPlayerPos = Point(-1, -1);
		localSessionId = gPathSessionId;
	}

	bool needRepath = (playerX != lastPlayerPos.x || playerY != lastPlayerPos.y || currentStep >= path.size() - 1);
	if (needRepath) {
		path = findPath(npcX, npcY, playerX, playerY);
		currentStep = 0;
		lastPlayerPos = Point(playerX, playerY);
	}

	// 目标下一步
	bool moved = false;
	int nextX = npcX;
	int nextY = npcY;
	if (path.size() > currentStep + 1) {
		currentStep++;
		nextX = path[currentStep].x;
		nextY = path[currentStep].y;
		moved = true;
	}

	// 如果A*未找到路径或没有下一步，采用贪心4邻作为兜底
	if (!moved) {
		int dirs[4][2] = { {0,1},{0,-1},{1,0},{-1,0} };
		int bestDX = 0, bestDY = 0;
		int bestH = INT_MAX;
		for (auto& d : dirs) {
			int tx = npcX + d[0];
			int ty = npcY + d[1];
			if (isWalkable(tx, ty)) {
				int h = manhattanDistance(tx, ty, playerX, playerY);
				if (h < bestH) {
					bestH = h;
					bestDX = d[0];
					bestDY = d[1];
				}
			}
		}
		if (bestH != INT_MAX) {
			nextX = npcX + bestDX;
			nextY = npcY + bestDY;
			moved = true;
		}
	}

	if (moved) {
		// 旧位置清理
		int oldX = npcs[0].x;
		int oldY = npcs[0].y;
		gLastNPCPos = Point(oldX, oldY);
		if (map[oldY][oldX] == 4) {
			map[oldY][oldX] = 2;
		}
		else if (map[oldY][oldX] == 3) {
			map[oldY][oldX] = 0;
		}

		// 碰到玩家则直接判负
		if (nextX == player.x && nextY == player.y) {
			gameOver = true;
			gameWin = false;
			return;
		}
		// 交换位置碰撞：NPC新==玩家旧 且 玩家新==NPC旧
		if (nextX == gLastPlayerPos.x && nextY == gLastPlayerPos.y
			&& player.x == gLastNPCPos.x && player.y == gLastNPCPos.y) {
			gameOver = true;
			gameWin = false;
			return;
		}

		// 写新位置
		npcs[0].x = nextX;
		npcs[0].y = nextY;
		if (map[nextY][nextX] == 2) {
			map[nextY][nextX] = 4; // 藏有水果
		}
		else if (map[nextY][nextX] == 0) {
			map[nextY][nextX] = 3;
		}

	}
}

// 播放吃水果音效（首次调用时打开，之后复用别名）
void playEatFruitSound() {
    TCHAR cmd[256];
 
        _stprintf_s(cmd, _T("open sound/eat_fruit.mp3 alias eatfruit"));
        mciSendString(cmd, NULL, 0, NULL);
        gEatSoundOpened = true;
    
    // 重新从头播放一次
    _stprintf_s(cmd, _T("stop eatfruit"));
    mciSendString(cmd, NULL, 0, NULL);
    _stprintf_s(cmd, _T("play eatfruit from 0"));
    mciSendString(cmd, NULL, 0, NULL);
}


// 加载玩家数据
void loadUserData() {
    users.clear();
    // ifstream：C++标准库中的“输入文件流”类，用于从文件读取数据
    ifstream file(USER_DATA_FILE);
    if (file.is_open()) {
        // 定义string变量line，用于暂存每次读取的“一行文件内容”
        string line;
        // getline(file, line)：从file流中读取一整行数据，存入line变量
        while (getline(file, line)) {
            // 判断是否找到逗号（排除格式错误的行）
            // size_t：无符号整数类型，专门用于存储“长度/位置”（避免负数）
            size_t pos = line.find(',');
            //   - 成立：当前行有逗号，格式正确，可拆分用户名和密码
            //   - 不成立：当前行无逗号（格式错误），跳过该行不处理
            // string::npos 是 C++ 标准库 string 类里定义的 静态无符号整数常量
            // 它的核心作用只有一个 ——标记 “查找操作失败”（比如在字符串里找某个字符 / 子串，没找到时就返回这个值）
            if (pos != string::npos) {
                string username = line.substr(0, pos);
                string password = line.substr(pos + 1);
                users.push_back(User(username, password));
            }
        }
        file.close();
    }
}

void saveUserData() {
    // ofstream：C++ 标准库的“输出文件流”类，用于向文件写入数据
    ofstream file(USER_DATA_FILE);
    if (file.is_open()) {
        for (const auto& user : users) {
            file << user.username << "," << user.password << endl;
        }
        file.close();
    }
}

bool userExists(const string& username) {
    for (const auto& user : users) {
        if (user.username == username) {
            return true;
        }
    }
    return false;
}

bool registerUser(const string& username, const string& password) {
    if (username.empty() || password.empty()) {
        return false;
    }

    if (userExists(username)) {
        MessageBox(NULL, _T("用户名已存在"), _T("错误"), MB_OK);
        return false;  // 用户名已存在
    }

    users.push_back(User(username, password));
    saveUserData();
    return true;
}

// const& 表示只读引用，避免拷贝，提高效率
bool loginUser(const string& username, const string& password) {
    for (const auto& user : users) {
        if (user.username == username && user.password == password) {
            currentUsername = username;
            isLoggedIn = true;
            return true;
        }
    }
    return false;
}

// 显示登录界面
void showLoginGUI() {
    ifShowLoginGUI = true;
    ifShowRegisterGUI = false;

    string username = "";
    string password = "";
    bool isUsernameInput = true;  // 当前是否在输入用户名
    bool showPassword = false;    // 是否显示密码（用于输入时的视觉反馈）
    int currentInput = 0;

    setbkcolor(WHITE);
    cleardevice();
        
    if (_kbhit()) {
         _getch();
    }

    while (ifShowLoginGUI) {
        BeginBatchDraw();
        cleardevice();
        putimage(0, 0, &loginImg);

        switch (currentInput) {
        case 0:
            putimage(0, 0, &loginImgs[0]);
            break;
        case 1:
            putimage(0, 0, &loginImgs[1]);
            break;
        default:
            break;
        }

        // 绘制用户名输入框背景   
        RECT userInputRect = { 215, 165, 480, 220 };

        // 绘制用户名文本
        settextcolor(GREEN);
        settextstyle(40, 0, _T("微软雅黑")); 
        TCHAR usernameText[256];
        _stprintf_s(usernameText, _T("%S"), username.c_str());
        drawtext(usernameText, &userInputRect, DT_LEFT | DT_VCENTER);

      
        // 绘制密码文本（显示*号）
        RECT passInputRect = { 215, 275, 480, 330 };
        TCHAR passwordText[256];
        settextstyle(40, 0, _T("微软雅黑"));  // 30为高度，0表示宽度自适应
        string displayPassword = showPassword ? password : string(password.length(), '*');
        _stprintf_s(passwordText, _T("%S"), displayPassword.c_str());
        drawtext(passwordText, &passInputRect, DT_LEFT | DT_VCENTER);

        // 登录/注册
        RECT loginRect = { 145, 370, 275, 460 };
        //solidrectangle(loginRect.left, loginRect.top, loginRect.right, loginRect.bottom); 
        RECT registerRect = { 335, 370, 465, 460 };
        //solidrectangle(registerRect.left, registerRect.top, registerRect.right, registerRect.bottom);

        // 绘制提示信息
        settextstyle(30, 0, _T("微软雅黑"));
        setbkcolor(RGB(254, 232, 227));
        settextcolor(BLACK);
        RECT tipRect = { 0, 115, WINDOW_WIDTH, 160 };
        drawtext(_T("按Tab键切换输入框，Enter键确认"), &tipRect, DT_CENTER | DT_VCENTER);
        setbkcolor(WHITE);

        EndBatchDraw();

        // 处理键盘输入
        if (_kbhit()) {
            int ch = _getch();
            switch (ch) {
            case 9: // Tab键切换输入框
                isUsernameInput = !isUsernameInput;
                currentInput = (currentInput + 1) % 2;
               // currentInput = isUsernameInput ? 0 : 1;
                break;
            case 13: // Enter键
                if (isUsernameInput) {
                    currentInput = (currentInput + 1) % 2;
                    isUsernameInput = false;
                }
                else {
                    // 尝试登录
                    if (loginUser(username, password)) {
                        ifShowLoginGUI = false;
                        showGameStartMenuGUI();
                        return;
                    }
                    else {
                        int ch = _getch();

                        MessageBox(NULL, _T("用户名或密码错误！"), _T("登录失败"), MB_OK);
                    }
                }
                break;
            case 8: // Backspace键
                if (isUsernameInput && !username.empty()) {
                    username.pop_back();
                }
                else if (!isUsernameInput && !password.empty()) {
                    password.pop_back();
                }
                break;
            case 27: // ESC键退出
                exit(0);
                break;
            // 方便测试
            case 0x12:
                showGameStartMenuGUI();
                return;
            default:
                if (ch >= 32 && ch <= 126) { // 可打印字符
                    if (isUsernameInput && username.length() < 20) {
                        username += (char)ch;
                    }
                    else if (!isUsernameInput && password.length() < 20) {
                        password += (char)ch;
                    }
                }
                break;
            }
        }

        // 处理鼠标点击
        MOUSEMSG msg;
        if (MouseHit()) {
            msg = GetMouseMsg();
            if (msg.uMsg == WM_LBUTTONDOWN) {
                // 检查是否点击了登录按钮
                if (msg.x >= loginRect.left && msg.x <= loginRect.right && msg.y >= loginRect.top && msg.y <= loginRect.bottom) {
                    if (loginUser(username, password)) {
                        ifShowLoginGUI = false;
                        showGameStartMenuGUI();
                        return;
                    }
                    else {
                        MessageBox(NULL, _T("用户名或密码错误！"), _T("登录失败"), MB_OK);
                    }
                }
                // 检查是否点击了注册按钮
                else if (msg.x >= registerRect.left && msg.x <= registerRect.right && msg.y >= registerRect.top && msg.y <= registerRect.bottom) {
                    ifShowLoginGUI = false;
                    showRegisterGUI();
                    return;
                }
            }
        }

        Sleep(10);
    }
}

// 显示注册界面
void showRegisterGUI() {
    ifShowRegisterGUI = true;
    ifShowLoginGUI = false;

    string username = "";
    string password = "";
    string confirmPassword = "";
    int currentInput = 0;  // 0: 用户名, 1: 密码, 2: 确认密码
    // 注册/返回
    RECT registerRect = { 145, 450, 275, 540 };
    RECT backRect = { 335, 450, 465, 540 };

    BeginBatchDraw();
    setbkcolor(BLACK);
    cleardevice();
    putimage(0, 0, &registerImg);
    EndBatchDraw();

    while (ifShowRegisterGUI) {
        BeginBatchDraw();
        cleardevice();
        switch (currentInput) {
        case 0: 
            putimage(0, 0, &registerImgs[0]);
            break;
        case 1:
            putimage(0, 0, &registerImgs[1]);
            break;
        case 2:
            putimage(0, 0, &registerImgs[2]);
            break;
        default:
            break;
        }

        // 处理鼠标点击
        MOUSEMSG msg;
        if (MouseHit()) {
            msg = GetMouseMsg();
            /*if (msg.x >= registerRect.left && msg.x <= registerRect.right && msg.y >= registerRect.top && msg.y <= registerRect.bottom) {
                putimage(0, 0, &registerImgs[3]);
            }
            else if (msg.x >= backRect.left && msg.x <= backRect.right && msg.y >= backRect.top && msg.y <= backRect.bottom) {
                putimage(0, 0, &registerImgs[4]);
            }*/


            if (msg.uMsg == WM_LBUTTONDOWN) {
                // 检查是否点击了注册按钮
                if (msg.x >= registerRect.left && msg.x <= registerRect.right && msg.y >= registerRect.top && msg.y <= registerRect.bottom) {
                    if (username.empty() || password.empty() || confirmPassword.empty()) {
                        MessageBox(NULL, _T("请填写完整信息！"), _T("注册失败"), MB_OK);
                    }
                    else if (password != confirmPassword) {
                        MessageBox(NULL, _T("两次输入的密码不一致！"), _T("注册失败"), MB_OK);
                    }
                    else if (userExists(username)) {
                        MessageBox(NULL, _T("用户名已存在！"), _T("注册失败"), MB_OK);
                    }
                    else if (registerUser(username, password)) {
                        MessageBox(NULL, _T("注册成功！"), _T("注册成功"), MB_OK);
                        ifShowRegisterGUI = false;
                        showLoginGUI();
                        return;
                    }
                    else {
                        MessageBox(NULL, _T("注册失败！"), _T("注册失败"), MB_OK);
                    }
                }
                // 检查是否点击了返回按钮
                else if (msg.x >= backRect.left && msg.x <= backRect.right && msg.y >= backRect.top && msg.y <= backRect.bottom) {
                    ifShowRegisterGUI = false;
                    showLoginGUI();
                    return;
                }
            }
        }
        
        // 绘制用户名输入框
        RECT userInputRect = { 215, 165, 485, 225 };
        settextstyle(40, 0, _T("微软雅黑"));
        TCHAR userText[256];
        _stprintf_s(userText, _T("%S"), username.c_str());
        drawtext(userText, &userInputRect, DT_LEFT | DT_VCENTER);

        // 绘制密码输入框
        RECT passInputRect = { 220, 260, 485, 317 };
        settextstyle(40, 0, _T("微软雅黑"));
        TCHAR passwordText[256];
        string displayPassword = string(password.length(), '*');
        _stprintf_s(passwordText, _T("%S"), displayPassword.c_str());
        drawtext(passwordText, &passInputRect, DT_LEFT | DT_VCENTER);

        // 绘制确认密码输入框
        RECT comfirmInputRect = { 220, 355, 485, 410 };
        settextstyle(40, 0, _T("微软雅黑"));
        TCHAR confirmPasswordText[256];
        string displayConfirmPassword = string(confirmPassword.length(), '*');
        _stprintf_s(confirmPasswordText, _T("%S"), displayConfirmPassword.c_str());
        drawtext(confirmPasswordText, &comfirmInputRect, DT_LEFT | DT_VCENTER);

        


      
        // 绘制提示信息
        settextstyle(30, 0, _T("微软雅黑"));
        setbkcolor(RGB(254, 232, 227));
        settextcolor(BLACK);
        RECT tipRect = { 0, 115, WINDOW_WIDTH, 160 };
        drawtext(_T("按Tab键切换输入框，Enter键确认"), &tipRect, DT_CENTER | DT_VCENTER);
        setbkcolor(WHITE);


        

        EndBatchDraw();

        // 处理键盘输入
        if (_kbhit()) {
            int ch = _getch();
            switch (ch) {
            case 9: // Tab键切换输入框
                currentInput = (currentInput + 1) % 3;
                break;
            case 13: // Enter键
                if (username.empty() || password.empty() || confirmPassword.empty()) {
                    MessageBox(NULL, _T("请填写完整信息！"), _T("注册失败"), MB_OK);
                }
                else if (password != confirmPassword) {
                    MessageBox(NULL, _T("两次输入的密码不一致！"), _T("注册失败"), MB_OK);
                }
                else if (userExists(username)) {
                    MessageBox(NULL, _T("用户名已存在！"), _T("注册失败"), MB_OK);
                }
                else if (registerUser(username, password)) {
                    MessageBox(NULL, _T("注册成功！"), _T("注册成功"), MB_OK);
                    ifShowRegisterGUI = false;
                    showLoginGUI();
                    return;
                }
                else {
                    MessageBox(NULL, _T("注册失败！"), _T("注册失败"), MB_OK);
                }
                break;
            case 8: // Backspace键
                if (currentInput == 0 && !username.empty()) {
                    username.pop_back();
                }
                else if (currentInput == 1 && !password.empty()) {
                    password.pop_back();
                }
                else if (currentInput == 2 && !confirmPassword.empty()) {
                    confirmPassword.pop_back();
                }
                break;
            case 27: // ESC键返回登录
                ifShowRegisterGUI = false;
                showLoginGUI();
                return;
            default:
                if (ch >= 32 && ch <= 126) { // 可打印字符
                    if (currentInput == 0 && username.length() < 20) {
                        username += (char)ch;
                    }
                    else if (currentInput == 1 && password.length() < 20) {
                        password += (char)ch;
                    }
                    else if (currentInput == 2 && confirmPassword.length() < 20) {
                        confirmPassword += (char)ch;
                    }
                }
                break;
            }
        }

        Sleep(10);
    }
}



// 主函数
int main() {
    // 将当前时间作为随机数种子
    srand((unsigned int)time(0));

    // 初始化图形窗口
    initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);
    setbkcolor(BLUE);
    cleardevice();
    initImages();

    // 加载用户数据
    loadUserData();

    // 播放背景音乐
    TCHAR bgmCommand[256];
    _stprintf_s(bgmCommand, _T("open res/bgm.mp3 alias bgm"));
    mciSendString(bgmCommand, NULL, 0, NULL);
    _stprintf_s(bgmCommand, _T("play bgm repeat"));
    mciSendString(bgmCommand, NULL, 0, NULL);

    // 首先显示登录界面
    showLoginGUI();

    // 主线程阻塞，避免直接退出（等待所有界面线程执行）
    while (true) {
        Sleep(10000);  // 持续阻塞，直到用户主动关闭窗口
    }

    closegraph();  // 理论上不会执行，但若用户关闭窗口，释放资源

    // 关闭背景音乐
    _stprintf_s(bgmCommand, _T("stop bgm"));
    mciSendString(bgmCommand, NULL, 0, NULL);
    _stprintf_s(bgmCommand, _T("close bgm"));
    mciSendString(bgmCommand, NULL, 0, NULL);

    return 0;
}








