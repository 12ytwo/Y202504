#include<easyx.h>
#include<vector>
#include<cmath>


IMAGE img_background;
IMAGE img_shadow;

#pragma comment(lib,"MSIMG32.LIB")
#pragma comment(lib,"Winmm.lib")


bool running = true;
bool is_game_started = false;
bool is_character_selection = false;  // 角色选择状态
int selected_character = 0;           // 选中的角色索引

// 角色选择按钮布局参数
const int CHAR_BTN_WIDTH = 160;      // 单个按钮宽度
const int CHAR_BTN_HEIGHT = 200;     // 单个按钮高度
const int CHAR_BTN_SPACING = 40;     // 按钮间距
const int CHAR_BTN_TOP = 300;        // 距离顶部的固定位置

//绘制透明底图片的自定义函数
inline void putimage_alpha(int x, int y, IMAGE* img) {
	int w = img->getwidth();
	int h = img->getheight();
	AlphaBlend(GetImageHDC(NULL), x, y, w, h,
		GetImageHDC(img), 0, 0, w, h, { AC_SRC_OVER,0,255,AC_SRC_ALPHA });
}

class Atlas {
public:
	Atlas(LPCTSTR path, int num) {
		TCHAR path_file[256];
		for (size_t i = 0; i < num; i++) {
			_stprintf_s(path_file, path, i);

			IMAGE* frame = new IMAGE();
			loadimage(frame, path_file);
			frame_list.push_back(frame);//利用pushback函数把图片对象的指针添加到容器里面
		}
	}
	~Atlas() {
		for (size_t i = 0; i < frame_list.size(); i++) {
			delete frame_list[i];
		}
	}

public:
	std::vector<IMAGE*>frame_list;
};

// 存储不同角色的动画图集
Atlas* atlas_player_left[3];  // 3个角色的左移动画
Atlas* atlas_player_right[3]; // 3个角色的右移动画
Atlas* atlas_enemy_left;
Atlas* atlas_enemy_right;

class Animation {
public:
	Animation(Atlas* atlas, int interval) {
		anim_atlas = atlas;
		interval_ms = interval;
	}

	~Animation() = default;

	//动画播放
	void Play(int x, int y, int delta) {
		timer += delta;

		if (timer >= interval_ms) {
			idx_frame = (idx_frame + 1) % anim_atlas->frame_list.size();
			timer = 0;
		}

		putimage_alpha(x, y, anim_atlas->frame_list[idx_frame]);
	}


private:
	int timer = 0;//动画计时器
	int idx_frame = 0;//动画帧索引
	int interval_ms = 0;
	Atlas* anim_atlas;
};

const int WIDTH0 = 1280;//页面宽度
const int HEIGHT0 = 720;//页面高度

const int BUTTON_WIDTH = 190;
const int BUTTON_HEIGHT = 75;

class Player {
public:
	const int PLAYER_WIDTH = 80;//玩家宽度
	const int PLAYER_HEIGHT = 80;//玩家高度
	POINT position = { 300,300 };//初始化玩家位置
public:
	Player( ) {
		loadimage(&img_shadow, _T("img/shadow_player.png"));
		// 使用选中的角色图集
		anim_left = new Animation(atlas_player_left[selected_character], 45);
		anim_right = new Animation(atlas_player_right[selected_character], 45);
	};

	~Player() {
		delete anim_left;
		delete anim_right;
	}

	void ProcessEvent(const ExMessage& msg) {
		//通过按键来实现角色移动
		if (msg.message == WM_KEYDOWN) {
			switch (msg.vkcode) {
			case VK_UP:
				is_move_up = true;
				break;
			case VK_DOWN:
				is_move_down = true;
				break;
			case VK_LEFT:
				is_move_left = true;
				break;
			case VK_RIGHT:
				is_move_right = true;
				break;
			}
		}
		else if (msg.message == WM_KEYUP) {
			switch (msg.vkcode) {
			case VK_UP:
				is_move_up = false;
				break;
			case VK_DOWN:
				is_move_down = false;
				break;
			case VK_LEFT:
				is_move_left = false;
				break;
			case VK_RIGHT:
				is_move_right = false;
				break;
			}
		}
	}

	void Move() {
		//利用向量知识来解决斜方向移动速度特别快的问题
		int dir_x = is_move_right - is_move_left;
		int dir_y = is_move_down - is_move_up;;
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
		if (len_dir != 0) {
			double normalized_x = dir_x / len_dir;
			double normalized_y = dir_y / len_dir;
			position.x += (int)(SPEED * normalized_x);
			position.y += (int)(SPEED * normalized_y);
		}

		//对玩家位置进行校准，防止玩家超出游戏界面
		if (position.x < 0)
			position.x = 0;
		if (position.y < 0)
			position.y = 0;
		if (position.x + PLAYER_WIDTH > WIDTH0)
			position.x = WIDTH0 - PLAYER_WIDTH;
		if (position.y + PLAYER_HEIGHT > HEIGHT0)
			position.y = HEIGHT0 - PLAYER_HEIGHT;
	}

	void Draw(int delta) {
		int pos_shadow_x = position.x + (PLAYER_WIDTH / 2 - SHADOW_WIDTH / 2);
		int pos_shadow_y = position.y + PLAYER_HEIGHT - 8;//偏移一点点
		putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow);

		static bool facing_left = false;
		int dir_x = is_move_right - is_move_left;
		if (dir_x < 0)
			facing_left = true;
		else if (dir_x > 0)
			facing_left = false;

		if (facing_left)
			anim_left->Play(position.x, position.y, delta);
		else
			anim_right->Play(position.x, position.y, delta);
	}

	const POINT& GetPosition() const {
		return position;
	};

private:
	const int SHADOW_WIDTH = 32;//影子大小
	const int SPEED = 5;//定义速度

private:
	IMAGE img_shadow;
	Animation* anim_left;
	Animation* anim_right;
	bool is_move_up = false;
	bool is_move_down = false;
	bool is_move_left = false;
	bool is_move_right = false;
};

class Bullet {
public:
	POINT position = { 0,0 };

public:
	Bullet() = default;
	~Bullet() = default;

	void Draw() const {
		// 根据选择的角色改变子弹颜色
		switch (selected_character) {
		case 0:
			setfillcolor(RGB(252, 225, 235));
			setlinecolor(RGB(215, 108, 124));
			break;
		case 1:
			setlinecolor(RGB(134, 218, 227));
			setfillcolor(RGB(51, 166, 223));
			break;
		case 2:
			setlinecolor(RGB(250, 210, 89));
			setfillcolor(RGB(255, 183, 43));
			break;
		default:
			setlinecolor(RED);
			setfillcolor(RED);
		}
		fillcircle(position.x, position.y, R);
	}

	int GetR()const {
		return R;
	}
private:
	const int R = 8;
};


class Enemy {
public:
	//敌人生成边界
	enum class SpawnEdge {
		Up = 0,
		Down,
		Left,
		Right
	};

	Enemy() {
		loadimage(&img_shadow, _T("img/shadow_enemy.png"));
		anim_left = new Animation(atlas_enemy_left, 45);
		anim_right = new Animation(atlas_enemy_right, 45);

		SpawnEdge edge = (SpawnEdge)(rand() % 4);
		switch (edge) {
		case SpawnEdge::Up:
			position.x = rand() % WIDTH0;
			position.y = -FRAME_HEIGHT;
			break;
		case SpawnEdge::Down:
			position.x = rand() % WIDTH0;
			position.y = HEIGHT0;
			break;
		case SpawnEdge::Left:
			position.x = -FRAME_WIDTH;
			position.y = rand() % HEIGHT0;
			break;
		case SpawnEdge::Right:
			position.x = WIDTH0;
			position.y = rand() % HEIGHT0;
			break;
		default:
			break;
		}
	}

	bool CheckBulletCollision(const Bullet& bullet) {
		//将子弹等效为点，判断是否在敌人矩形内
		bool is_overlap_x = bullet.position.x >= position.x && bullet.position.x <= position.x + FRAME_WIDTH;
		bool is_overlap_y = bullet.position.y >= position.y && bullet.position.y <= position.y + FRAME_HEIGHT;
		return is_overlap_x && is_overlap_y;
	}

	bool CheckPlayerCollision(const Player& player) {
		//将敌人中心位置等效为点，判断是否在玩家矩形内
		POINT check_position = { position.x + FRAME_WIDTH / 2,position.y + FRAME_HEIGHT / 2 };
		bool is_overlap_x = player.position.x >= position.x && player.position.x <= position.x + FRAME_WIDTH;
		bool is_overlap_y = player.position.y >= position.y && player.position.y <= position.y + FRAME_HEIGHT;
		return is_overlap_x && is_overlap_y;
	}

	void Move(const Player& player) {
		const POINT& player_position = player.GetPosition();
		int dir_x = player_position.x - position.x;
		facing_left = (dir_x < 0);
		int dir_y = player_position.y - position.y;
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
		if (len_dir != 0) {
			double normalized_x = dir_x / len_dir;
			double normalized_y = dir_y / len_dir;
			position.x += (int)(SPEED * normalized_x);
			position.y += (int)(SPEED * normalized_y);
		}
	}

	void Draw(int delta) {
		int pos_shadow_x = position.x + (FRAME_WIDTH / 2 - SHADOW_WIDTH / 2);
		int pos_shadow_y = position.y + FRAME_HEIGHT - 31;//偏移一点点
		putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow);

		if (facing_left)
			anim_left->Play(position.x, position.y, delta);
		else
			anim_right->Play(position.x, position.y, delta);
	}

	const POINT& GetPosition() const {
		return position;
	}

	~Enemy() {
		delete anim_left;
		delete anim_right;
	}

	void Hurt() {
		alive = false;
	}

	bool CheckAlive() {
		return alive;
	}

private:
	const int FRAME_WIDTH = 60;//敌人宽度
	const int FRAME_HEIGHT = 60;//敌人高度
	const int SHADOW_WIDTH = 48;//影子大小
	const int SPEED = 4;//定义速度

private:
	IMAGE img_shadow;
	Animation* anim_left;
	Animation* anim_right;
	POINT position = { 0,0 };//初始化敌人位置;
	bool facing_left = false;
	bool alive = true;
	bool is_move_up = false;
	bool is_move_down = false;
	bool is_move_left = false;
	bool is_move_right = false;
};



//按钮类
class Button {
public:
	Button(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed) {
		region = rect;
		loadimage(&img_idle, path_img_idle);
		loadimage(&img_hovered, path_img_hovered);
		loadimage(&img_pushed, path_img_pushed);
	}

	~Button() = default;

	void ProcessEvent(const ExMessage& msg) {
		switch (msg.message) {
		case WM_MOUSEMOVE:
			if (status == Status::Idle && CheckCursorHit(msg.x, msg.y))
				status = Status::Hovered;
			else if (status == Status::Hovered && !CheckCursorHit(msg.x, msg.y))
				status = Status::Idle;
			break;
		case WM_LBUTTONDOWN:
			if (CheckCursorHit(msg.x, msg.y))
				status = Status::Pushed;
			break;
		case WM_LBUTTONUP:
			if (status == Status::Pushed && CheckCursorHit(msg.x, msg.y))
				OnClick();
			break;
		default:
			break;
		}
	}

	void Draw() {
		switch (status) {
		case Status::Idle:
			putimage_alpha(region.left, region.top, &img_idle);
			break;
		case Status::Hovered:
			putimage_alpha(region.left, region.top, &img_hovered);
			break;
		case Status::Pushed:
			putimage_alpha(region.left, region.top, &img_pushed);
			break;
		}
	}

protected:
	virtual void OnClick() = 0;

private:
	enum class Status {
		Idle = 0,
		Hovered,
		Pushed
	};

private:
	//检测鼠标点击
	bool CheckCursorHit(int x, int y) {
		return x >= region.left && x <= region.right && y >= region.top && y <= region.bottom;
	}

private:
	RECT region;//描述位置和大小
	IMAGE img_idle;
	IMAGE img_hovered;
	IMAGE img_pushed;
	Status status = Status::Idle;
};


//开始游戏按钮
class StartGameButton :public Button {
public:
	StartGameButton(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
		:Button(rect, path_img_idle, path_img_hovered, path_img_pushed) {}
	~StartGameButton() = default;

protected:
	void OnClick() {
		is_character_selection = true;  // 进入角色选择界面
	}
};

//退出游戏按钮
class QuitGameButton :public Button {
public:
	QuitGameButton(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
		:Button(rect, path_img_idle, path_img_hovered, path_img_pushed) {}
	~QuitGameButton() = default;

protected:
	void OnClick() {
		running = false;
	}
};

// 角色选择按钮
class CharacterButton :public Button {
public:
	CharacterButton(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed, int character_index)
		:Button(rect, path_img_idle, path_img_hovered, path_img_pushed), character_index(character_index) {}
	~CharacterButton() = default;

protected:
	void OnClick() {
		selected_character = character_index;
		is_character_selection = false;
		is_game_started = true;
		mciSendString(_T("play bgm repeat from 0"), NULL, 0, NULL);
	}

private:
	int character_index;
	RECT region;//描述位置和大小
};




//生成新的敌人
void TryGenerateEnemy(std::vector<Enemy*>& enemy_list) {
	const int INTERVAL = 100;
	static int  counter = 0;
	if ((++counter) % INTERVAL == 0) {
		enemy_list.push_back(new Enemy());
	}
};

//更新子弹的位置
void UpdateBullets(std::vector<Bullet>& bullet_list, const Player& player) {
	const double R_SPEED = 0.004;
	const double T_SPEED = 0.004;
	double radian_interval = 2 * 3.1415926 / bullet_list.size();
	POINT player_position = player.GetPosition();
	double radius = 100 + 25 * sin(GetTickCount() * R_SPEED);
	for (size_t i = 0; i < bullet_list.size(); i++) {
		double radian = GetTickCount() * T_SPEED + radian_interval * i;
		bullet_list[i].position.x = player_position.x + player.PLAYER_WIDTH / 2 + (int)(radius * sin(radian));
		bullet_list[i].position.y = player_position.y + player.PLAYER_HEIGHT / 2 + (int)(radius * cos(radian));
	}
}

//绘制当前玩家得分
void DrawPlayerScore(int score) {
	static TCHAR text[64];
	_stprintf_s(text, _T("当前玩家得分为：%d"), score);
	settextstyle(22, 0, _T("微软雅黑"));
	setbkmode(TRANSPARENT);
	settextcolor(RGB(80, 134, 85));
	outtextxy(10, 10, text);
}



// 绘制角色选择界面
void DrawCharacterSelection() {
	static IMAGE img_character_bg;
	static CharacterButton* btn_character[3] = { nullptr }; // 使用数组管理按钮

	// 首次加载时初始化
	if (!btn_character[0]) {
		loadimage(&img_character_bg, _T("img/character_selection_bg.png"), WIDTH0, HEIGHT0);

		// 计算起始X坐标（整体居中）
		int total_width = CHAR_BTN_WIDTH * 3 + CHAR_BTN_SPACING * 2;
		int start_x = (WIDTH0 - total_width) / 2;

		// 初始化三个按钮
		for (int i = 0; i < 3; i++) {
			RECT rect = {
				start_x + i * (CHAR_BTN_WIDTH + CHAR_BTN_SPACING),
				CHAR_BTN_TOP,
				start_x + (i + 1) * CHAR_BTN_WIDTH + i * CHAR_BTN_SPACING,
				CHAR_BTN_TOP + CHAR_BTN_HEIGHT
			};

			TCHAR path_idle[64], path_hovered[64], path_pushed[64];
			_stprintf_s(path_idle, _T("img/character%d_idle.png"), i);
			_stprintf_s(path_hovered, _T("img/character%d_hovered.png"), i);
			_stprintf_s(path_pushed, _T("img/character%d_pushed.png"), i);

			btn_character[i] = new CharacterButton(rect, path_idle, path_hovered, path_pushed, i);
		}
	}

	// 绘制背景
	putimage(0, 0, &img_character_bg);

	// 绘制标题
	setbkmode(TRANSPARENT);
	settextcolor(RGB(80, 134, 85));
	settextstyle(36, 0, _T("微软雅黑"));
	outtextxy(WIDTH0 / 2 - 120, 180, _T("请选择你的宝可梦"));

	// 处理事件和绘制
	ExMessage msg;
	while (peekmessage(&msg)) {

		for (int i = 0; i < 3; i++) {
			btn_character[i]->ProcessEvent(msg);
		}
	}

	for (int i = 0; i < 3; i++) {
		btn_character[i]->Draw();
	}
}



int main() {
	initgraph(WIDTH0, HEIGHT0);

	//加载三个角色的动画图集
	atlas_player_left[0] = new Atlas(_T("img/character1_left_%d.png"), 6);
	atlas_player_right[0] = new Atlas(_T("img/character1_right_%d.png"), 6);
	atlas_player_left[1] = new Atlas(_T("img/character2_left_%d.png"), 6);
	atlas_player_right[1] = new Atlas(_T("img/character2_right_%d.png"), 6);
	atlas_player_left[2] = new Atlas(_T("img/character3_left_%d.png"), 6);
	atlas_player_right[2] = new Atlas(_T("img/character3_right_%d.png"), 6);

	atlas_enemy_left = new Atlas(_T("img/enemy_left_%d.png"), 6);
	atlas_enemy_right = new Atlas(_T("img/enemy_right_%d.png"), 6);

	mciSendString(_T("open mus/bgm.mp3 alias bgm"), NULL, 0, NULL);
	mciSendString(_T("open mus/hit.wav alias hit"), NULL, 0, NULL);

	int score = 0;
	Player* player = nullptr;
	ExMessage msg;
	IMAGE img_menu;
	IMAGE img_background;

	bool is_move_up = false;
	bool is_move_down = false;
	bool is_move_left = false;
	bool is_move_right = false;
	std::vector<Enemy*>enemy_list;
	std::vector<Bullet>bullet_list(3);

	RECT region_btn_start_game, region_btn_quit_game;

	region_btn_start_game.left = (WIDTH0 - BUTTON_WIDTH) / 2;
	region_btn_start_game.right = region_btn_start_game.left + BUTTON_WIDTH;
	region_btn_start_game.top = 430;
	region_btn_start_game.bottom = region_btn_start_game.top + BUTTON_HEIGHT;

	region_btn_quit_game.left = (WIDTH0 - BUTTON_WIDTH) / 2;
	region_btn_quit_game.right = region_btn_quit_game.left + BUTTON_WIDTH;
	region_btn_quit_game.top = 550;
	region_btn_quit_game.bottom = region_btn_quit_game.top + BUTTON_HEIGHT;

	StartGameButton btn_start_game = StartGameButton(region_btn_start_game,
		_T("img/ui_start_idle.png"), _T("img/ui_start_hovered.png"), _T("img/ui_start_pushed.png"));
	QuitGameButton btn_quit_game = QuitGameButton(region_btn_quit_game,
		_T("img/ui_quit_idle.png"), _T("img/ui_quit_hovered.png"), _T("img/ui_quit_pushed.png"));

	loadimage(&img_menu, _T("img/menu.png"), 1280, 720);
	loadimage(&img_background, _T("img/background.png"), 1280, 720);

	BeginBatchDraw();

	while (running) {
		DWORD begin_time = GetTickCount();

		while (peekmessage(&msg)) {
			if (is_game_started) {
				player->ProcessEvent(msg);
			}
			else if (is_character_selection) {
				// 角色选择界面的消息处理在DrawCharacterSelection函数中完成
			}
			else {
				btn_start_game.ProcessEvent(msg);
				btn_quit_game.ProcessEvent(msg);
			}
		}

		if (is_game_started) {
			if (!player) {
				player = new Player();
			}

			player->Move();
			UpdateBullets(bullet_list, *player);
			TryGenerateEnemy(enemy_list);
			//更新敌人位置
			for (Enemy* enemy : enemy_list)
				enemy->Move(*player);

			//检测子弹与敌人的碰撞
			for (Enemy* enemy : enemy_list) {
				for (const Bullet& bullet : bullet_list) {
					if (enemy->CheckBulletCollision(bullet)) {
						enemy->Hurt();
						score++;
					}
				}
			}

			//移除生命值归零的敌人
			for (size_t i = 0; i < enemy_list.size(); i++) {
				Enemy* enemy = enemy_list[i];
				if (!enemy->CheckAlive()) {
					std::swap(enemy_list[i], enemy_list.back());
					enemy_list.pop_back();
					delete enemy;
				}
			}

			// 检测敌人与玩家的碰撞
			bool is_game_over = false;  // 增加游戏结束标志
			for (Enemy* enemy : enemy_list) {
				if (enemy->CheckPlayerCollision(*player)) {
					is_game_over = true;
					break;  // 发现碰撞立即跳出循环
				}
			}

			if (is_game_over) {
				// 游戏结束逻辑
				TCHAR text[128];
				_stprintf_s(text, _T("最终得分为：%d！\n点击确定返回主菜单"), score);

				// 停止音乐
				mciSendString(_T("stop bgm"), NULL, 0, NULL);

				// 显示结果弹窗
				if (MessageBox(GetHWnd(), text, _T("游戏结束"), MB_OKCANCEL | MB_ICONINFORMATION) == IDOK) {
					// 重置游戏状态
					is_game_started = false;
					is_character_selection = false;
					score = 0;
					player->position = { 300, 300 };  // 重置玩家位置

					//删除玩家对象
					if (player) {
						delete player;
						player = nullptr;
					}
					// 清空敌人
					for (auto& enemy : enemy_list) delete enemy;
					enemy_list.clear();
				}
				else {
					running = false;  // 如果点取消则退出游戏
				}
			}
		}



		cleardevice();
		//绘图

		if (is_game_started) {
			putimage(0, 0, &img_background);//铺背景图片

			player->Draw(1000 / 144);
			for (Enemy* enemy : enemy_list)
				enemy->Draw(1000 / 144);
			for (const Bullet& bullet : bullet_list)
				bullet.Draw();
			DrawPlayerScore(score);
		}
		else if (is_character_selection) {
			DrawCharacterSelection();  // 绘制角色选择界面
		}
		else {
			putimage(0, 0, &img_menu);
			btn_start_game.Draw();
			btn_quit_game.Draw();
		}

		FlushBatchDraw();

		DWORD end_time = GetTickCount();
		DWORD delta_time = end_time - begin_time;
		if (delta_time < 1000 / 60) {
			Sleep(1000 / 60 - delta_time);
		}
	}

	// 释放所有角色图集资源
	for (int i = 0; i < 3; i++) {
		delete atlas_player_left[i];
		delete atlas_player_right[i];
	}
	delete atlas_enemy_left;
	delete atlas_enemy_right;


	EndBatchDraw();

	while (!enemy_list.empty()) {
		delete enemy_list.back();
		enemy_list.pop_back();
	}

	return 0;
}