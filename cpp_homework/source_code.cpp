#include<easyx.h>
#include<vector>
#include<cmath>


IMAGE img_background;
IMAGE img_shadow;

#pragma comment(lib,"MSIMG32.LIB")
#pragma comment(lib,"Winmm.lib")


bool running = true;
bool is_game_started = false;
bool is_character_selection = false;  // ��ɫѡ��״̬
int selected_character = 0;           // ѡ�еĽ�ɫ����

// ��ɫѡ��ť���ֲ���
const int CHAR_BTN_WIDTH = 160;      // ������ť���
const int CHAR_BTN_HEIGHT = 200;     // ������ť�߶�
const int CHAR_BTN_SPACING = 40;     // ��ť���
const int CHAR_BTN_TOP = 300;        // ���붥���Ĺ̶�λ��

//����͸����ͼƬ���Զ��庯��
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
			frame_list.push_back(frame);//����pushback������ͼƬ�����ָ����ӵ���������
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

// �洢��ͬ��ɫ�Ķ���ͼ��
Atlas* atlas_player_left[3];  // 3����ɫ�����ƶ���
Atlas* atlas_player_right[3]; // 3����ɫ�����ƶ���
Atlas* atlas_enemy_left;
Atlas* atlas_enemy_right;

class Animation {
public:
	Animation(Atlas* atlas, int interval) {
		anim_atlas = atlas;
		interval_ms = interval;
	}

	~Animation() = default;

	//��������
	void Play(int x, int y, int delta) {
		timer += delta;

		if (timer >= interval_ms) {
			idx_frame = (idx_frame + 1) % anim_atlas->frame_list.size();
			timer = 0;
		}

		putimage_alpha(x, y, anim_atlas->frame_list[idx_frame]);
	}


private:
	int timer = 0;//������ʱ��
	int idx_frame = 0;//����֡����
	int interval_ms = 0;
	Atlas* anim_atlas;
};

const int WIDTH0 = 1280;//ҳ����
const int HEIGHT0 = 720;//ҳ��߶�

const int BUTTON_WIDTH = 190;
const int BUTTON_HEIGHT = 75;

class Player {
public:
	const int PLAYER_WIDTH = 80;//��ҿ��
	const int PLAYER_HEIGHT = 80;//��Ҹ߶�
	POINT position = { 300,300 };//��ʼ�����λ��
public:
	Player( ) {
		loadimage(&img_shadow, _T("img/shadow_player.png"));
		// ʹ��ѡ�еĽ�ɫͼ��
		anim_left = new Animation(atlas_player_left[selected_character], 45);
		anim_right = new Animation(atlas_player_right[selected_character], 45);
	};

	~Player() {
		delete anim_left;
		delete anim_right;
	}

	void ProcessEvent(const ExMessage& msg) {
		//ͨ��������ʵ�ֽ�ɫ�ƶ�
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
		//��������֪ʶ�����б�����ƶ��ٶ��ر�������
		int dir_x = is_move_right - is_move_left;
		int dir_y = is_move_down - is_move_up;;
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
		if (len_dir != 0) {
			double normalized_x = dir_x / len_dir;
			double normalized_y = dir_y / len_dir;
			position.x += (int)(SPEED * normalized_x);
			position.y += (int)(SPEED * normalized_y);
		}

		//�����λ�ý���У׼����ֹ��ҳ�����Ϸ����
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
		int pos_shadow_y = position.y + PLAYER_HEIGHT - 8;//ƫ��һ���
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
	const int SHADOW_WIDTH = 32;//Ӱ�Ӵ�С
	const int SPEED = 5;//�����ٶ�

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
		// ����ѡ��Ľ�ɫ�ı��ӵ���ɫ
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
	//�������ɱ߽�
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
		//���ӵ���ЧΪ�㣬�ж��Ƿ��ڵ��˾�����
		bool is_overlap_x = bullet.position.x >= position.x && bullet.position.x <= position.x + FRAME_WIDTH;
		bool is_overlap_y = bullet.position.y >= position.y && bullet.position.y <= position.y + FRAME_HEIGHT;
		return is_overlap_x && is_overlap_y;
	}

	bool CheckPlayerCollision(const Player& player) {
		//����������λ�õ�ЧΪ�㣬�ж��Ƿ�����Ҿ�����
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
		int pos_shadow_y = position.y + FRAME_HEIGHT - 31;//ƫ��һ���
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
	const int FRAME_WIDTH = 60;//���˿��
	const int FRAME_HEIGHT = 60;//���˸߶�
	const int SHADOW_WIDTH = 48;//Ӱ�Ӵ�С
	const int SPEED = 4;//�����ٶ�

private:
	IMAGE img_shadow;
	Animation* anim_left;
	Animation* anim_right;
	POINT position = { 0,0 };//��ʼ������λ��;
	bool facing_left = false;
	bool alive = true;
	bool is_move_up = false;
	bool is_move_down = false;
	bool is_move_left = false;
	bool is_move_right = false;
};



//��ť��
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
	//��������
	bool CheckCursorHit(int x, int y) {
		return x >= region.left && x <= region.right && y >= region.top && y <= region.bottom;
	}

private:
	RECT region;//����λ�úʹ�С
	IMAGE img_idle;
	IMAGE img_hovered;
	IMAGE img_pushed;
	Status status = Status::Idle;
};


//��ʼ��Ϸ��ť
class StartGameButton :public Button {
public:
	StartGameButton(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
		:Button(rect, path_img_idle, path_img_hovered, path_img_pushed) {}
	~StartGameButton() = default;

protected:
	void OnClick() {
		is_character_selection = true;  // �����ɫѡ�����
	}
};

//�˳���Ϸ��ť
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

// ��ɫѡ��ť
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
	RECT region;//����λ�úʹ�С
};




//�����µĵ���
void TryGenerateEnemy(std::vector<Enemy*>& enemy_list) {
	const int INTERVAL = 100;
	static int  counter = 0;
	if ((++counter) % INTERVAL == 0) {
		enemy_list.push_back(new Enemy());
	}
};

//�����ӵ���λ��
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

//���Ƶ�ǰ��ҵ÷�
void DrawPlayerScore(int score) {
	static TCHAR text[64];
	_stprintf_s(text, _T("��ǰ��ҵ÷�Ϊ��%d"), score);
	settextstyle(22, 0, _T("΢���ź�"));
	setbkmode(TRANSPARENT);
	settextcolor(RGB(80, 134, 85));
	outtextxy(10, 10, text);
}



// ���ƽ�ɫѡ�����
void DrawCharacterSelection() {
	static IMAGE img_character_bg;
	static CharacterButton* btn_character[3] = { nullptr }; // ʹ���������ť

	// �״μ���ʱ��ʼ��
	if (!btn_character[0]) {
		loadimage(&img_character_bg, _T("img/character_selection_bg.png"), WIDTH0, HEIGHT0);

		// ������ʼX���꣨������У�
		int total_width = CHAR_BTN_WIDTH * 3 + CHAR_BTN_SPACING * 2;
		int start_x = (WIDTH0 - total_width) / 2;

		// ��ʼ��������ť
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

	// ���Ʊ���
	putimage(0, 0, &img_character_bg);

	// ���Ʊ���
	setbkmode(TRANSPARENT);
	settextcolor(RGB(80, 134, 85));
	settextstyle(36, 0, _T("΢���ź�"));
	outtextxy(WIDTH0 / 2 - 120, 180, _T("��ѡ����ı�����"));

	// �����¼��ͻ���
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

	//����������ɫ�Ķ���ͼ��
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
				// ��ɫѡ��������Ϣ������DrawCharacterSelection���������
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
			//���µ���λ��
			for (Enemy* enemy : enemy_list)
				enemy->Move(*player);

			//����ӵ�����˵���ײ
			for (Enemy* enemy : enemy_list) {
				for (const Bullet& bullet : bullet_list) {
					if (enemy->CheckBulletCollision(bullet)) {
						enemy->Hurt();
						score++;
					}
				}
			}

			//�Ƴ�����ֵ����ĵ���
			for (size_t i = 0; i < enemy_list.size(); i++) {
				Enemy* enemy = enemy_list[i];
				if (!enemy->CheckAlive()) {
					std::swap(enemy_list[i], enemy_list.back());
					enemy_list.pop_back();
					delete enemy;
				}
			}

			// ����������ҵ���ײ
			bool is_game_over = false;  // ������Ϸ������־
			for (Enemy* enemy : enemy_list) {
				if (enemy->CheckPlayerCollision(*player)) {
					is_game_over = true;
					break;  // ������ײ��������ѭ��
				}
			}

			if (is_game_over) {
				// ��Ϸ�����߼�
				TCHAR text[128];
				_stprintf_s(text, _T("���յ÷�Ϊ��%d��\n���ȷ���������˵�"), score);

				// ֹͣ����
				mciSendString(_T("stop bgm"), NULL, 0, NULL);

				// ��ʾ�������
				if (MessageBox(GetHWnd(), text, _T("��Ϸ����"), MB_OKCANCEL | MB_ICONINFORMATION) == IDOK) {
					// ������Ϸ״̬
					is_game_started = false;
					is_character_selection = false;
					score = 0;
					player->position = { 300, 300 };  // �������λ��

					//ɾ����Ҷ���
					if (player) {
						delete player;
						player = nullptr;
					}
					// ��յ���
					for (auto& enemy : enemy_list) delete enemy;
					enemy_list.clear();
				}
				else {
					running = false;  // �����ȡ�����˳���Ϸ
				}
			}
		}



		cleardevice();
		//��ͼ

		if (is_game_started) {
			putimage(0, 0, &img_background);//�̱���ͼƬ

			player->Draw(1000 / 144);
			for (Enemy* enemy : enemy_list)
				enemy->Draw(1000 / 144);
			for (const Bullet& bullet : bullet_list)
				bullet.Draw();
			DrawPlayerScore(score);
		}
		else if (is_character_selection) {
			DrawCharacterSelection();  // ���ƽ�ɫѡ�����
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

	// �ͷ����н�ɫͼ����Դ
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