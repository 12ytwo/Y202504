#include<easyx.h>
#include<graphics.h>
#include<string>
#include<vector>


IMAGE img_background;
IMAGE img_shadow;

//����͸����ͼƬ���Զ��庯��
#pragma comment(lib,"MSIMG32.LIB")

inline void putimage_alpha(int x, int y, IMAGE* img) {
	int w = img->getwidth();
	int h = img->getheight();
	AlphaBlend(GetImageHDC(NULL), x, y, w, h,
		GetImageHDC(img), 0, 0, w, h, { AC_SRC_OVER,0,255,AC_SRC_ALPHA });
}


class Animation {
public:
	Animation(LPCTSTR path, int num, int interval) {
		interval_ms = interval;

		TCHAR path_file[256];
		for (size_t i = 0; i < num; i++) {
			_stprintf_s(path_file, path, i);

			IMAGE* frame = new IMAGE();
			loadimage(frame, path_file);
			frame_list.push_back(frame);//����pushback������ͼƬ�����ָ����ӵ���������
		}

	}

	~Animation() {
		for (size_t i = 0; i < frame_list.size(); i++) {
			delete frame_list[i];
		}
	}

	//��������
	void Play(int x, int y, int delta) {
		timer += delta;

		if (timer >= interval_ms) {
			idx_frame = (idx_frame + 1) % frame_list.size();
			timer = 0;
		}

		putimage_alpha(x, y, frame_list[idx_frame]);
	}


private:
	int timer = 0;//������ʱ��
	int idx_frame = 0;//����֡����
	int interval_ms = 0;
	std::vector<IMAGE*>frame_list;
};




const int WIDTH0 = 1280;//ҳ����
const int HEIGHT0 = 720;//ҳ��߶�

class Player {
public:
	const int PLAYER_WIDTH = 80;//��ҿ��
	const int PLAYER_HEIGHT = 80;//��Ҹ߶�
public:
	Player() {
		loadimage(&img_shadow, _T("img/shadow_player.png"));
		anim_left = new Animation(_T("img/player_left_%d.png"), 6, 45);
		anim_right = new Animation(_T("img/player_right_%d.png"), 6, 45);
	};

	~Player() {
		delete anim_left;
		delete anim_right;
	}

	void ProcessEvent(ExMessage& msg) {
		while (peekmessage(&msg)) {
			//��ȡ����

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
	const int PLAYER_WIDTH = 80;//��ҿ��
	const int PLAYER_HEIGHT = 80;//��Ҹ߶�
	const int SHADOW_WIDTH = 32;//Ӱ�Ӵ�С
	const int SPEED = 3;//�����ٶ�
	POINT position = { 300,300 };//��ʼ�����λ��

private:
	IMAGE img_shadow;
	Animation* anim_left;
	Animation* anim_right;
	bool is_move_up = false;
	bool is_move_down = false;
	bool is_move_left = false;
	bool is_move_right = false;

};




class Enemy {
public:
	Enemy() {
		loadimage(&img_shadow, _T("img/shadow_enemy.png"));
		anim_left = new Animation(_T("img/enemy_left_%d.png"), 6, 45);
		anim_right = new Animation(_T("img/enemy_right_%d.png"), 6, 45);

		//�������ɱ߽�
		enum class SpawnEdge {
			Up = 0,
			Down,
			Left,
			Right

		};

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
		return false;
	}

	bool CheckPlayerCollision(const Player& player) {
		//����������λ�õ�ЧΪ�㣬�ж��Ƿ�����Ҿ�����
		POINT check_position = { position.x + FRAME_WIDTH / 2,position.y + FRAME_HEIGHT / 2 };
		return false;
	}

	void Move(const Player& player) {
		const POINT& player_position = player.GetPosition();
		int dir_x = player_position.x - position.x;
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
		int pos_shadow_y = position.y + FRAME_HEIGHT - 8;//ƫ��һ���
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
private:
	const int FRAME_WIDTH = 80;//���˿��
	const int FRAME_HEIGHT = 80;//���˸߶�
	const int SHADOW_WIDTH = 48;//Ӱ�Ӵ�С
	const int SPEED = 2;//�����ٶ�

private:
	IMAGE img_shadow;
	Animation* anim_left;
	Animation* anim_right;
	POINT position = { 0,0 };//��ʼ������λ��};
	bool facing_left = false; 
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
		setlinecolor(RED);
		setfillcolor(RED);
		fillcircle(position.x, position.y, R);
	}

private:
	const int R = 10;
};




int idx_current_anim = 0;

const int PLAYER_ANIM_NUM = 5;//���嶯��֡������


//�Ѷ���֡ͼƬȫ������
  // �������飬�й��ɵ�����ͼƬ
IMAGE img_player_left[PLAYER_ANIM_NUM];
IMAGE img_player_right[PLAYER_ANIM_NUM];
//���붯�����Զ��庯��
void LoadAnimation() {

	for (size_t i = 0; i < PLAYER_ANIM_NUM; i++) {
		std::wstring path = L"img/player_left_" + std::to_wstring(i) + L".png";
		loadimage(&img_player_left[i], path.c_str(), 100, 100);
	}

	for (size_t i = 0; i < PLAYER_ANIM_NUM; i++) {
		std::wstring path = L"img/player_right_" + std::to_wstring(i) + L".png";
		loadimage(&img_player_right[i], path.c_str(), 100, 100);
	}
}


Animation anim_left_player(_T("img/player_left_%d.png"), 6, 45);
Animation anim_right_player(_T("img/player_right_%d.png"), 6, 45);


//�����µĵ���
void TryGenerateEnemy(std::vector<Enemy*>& enemy_list) {
	const int INTERVAL = 100;
	static int  counter = 0;
	if ((++counter) % INTERVAL == 0) {
		enemy_list.push_back(new Enemy());
			}
};



int main() {
	initgraph(WIDTH0, HEIGHT0);

	bool running = true;

	Player player;

	ExMessage msg;

	std::vector<Enemy*>enemy_list;


	bool is_move_up = false;
	bool is_move_down = false;
	bool is_move_left = false;
	bool is_move_right = false;

	Bullet bullet;

	LoadAnimation();


	loadimage(&img_background, _T("img/background.png"), 1280, 720);

	BeginBatchDraw();

	while (running) {
		DWORD begin_time = GetTickCount();


		while (peekmessage(&msg)) {
			player.ProcessEvent(msg);
		}


		player.Move();
		TryGenerateEnemy(enemy_list);
		for (Enemy* enemy : enemy_list)
			enemy->Move(player);


		static int counter = 0;
		if (++counter % 5 == 0) {
			idx_current_anim++;
		}

		//ʹ����ѭ������
		idx_current_anim = idx_current_anim % PLAYER_ANIM_NUM;


		cleardevice();
		//��ͼ

		putimage(0, 0, &img_background);//�̱���ͼƬ

		player.Draw(1000 / 144);
		for (Enemy* enemy : enemy_list)
			enemy->Draw(1000 / 144);


		FlushBatchDraw();

		DWORD end_time = GetTickCount();
		DWORD delta_time = end_time - begin_time;

		if (delta_time < 1000 / 60) {
			Sleep(1000 / 60 - delta_time);
		}
	}

	return 0;
}