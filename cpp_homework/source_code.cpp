#include<easyx.h>
#include<graphics.h>
#include<string>
#include<vector>


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

//����͸����ͼƬ���Զ��庯��,ֱ�����ͺ���
#pragma comment(lib,"MSIMG32.LIB")

inline void putimage_alpha(int x, int y, IMAGE* img) {
	int w = img->getwidth();
	int h = img->getheight();
	AlphaBlend(GetImageHDC(NULL), x, y, w, h,
		GetImageHDC(img), 0, 0, w, h, { AC_SRC_OVER,0,255,AC_SRC_ALPHA });
}

//ʵ�ֽ�ɫ�ƶ�
POINT player_pos = { 300,300 };//��ʼ�����λ��
const int PLAYER_SPEED = 3;//�����ٶ�

int main() {
	initgraph(1280, 720);
	bool running = true;

	ExMessage msg;

	IMAGE img_background;


	bool is_move_up = false;
	bool is_move_down = false;
	bool is_move_left = false;
	bool is_move_right = false;

	LoadAnimation();
	loadimage(&img_background, _T("img/background.png"), 1280, 720);

	BeginBatchDraw();

	while (running) {
		DWORD begin_time = GetTickCount();

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
		if (is_move_up)
			player_pos.y -= PLAYER_SPEED;
		if (is_move_down)
			player_pos.y += PLAYER_SPEED;
		if (is_move_left)
			player_pos.x -= PLAYER_SPEED;
		if (is_move_right)
			player_pos.x += PLAYER_SPEED;

		static int counter = 0;
		if (++counter % 5 == 0) {
			idx_current_anim++;
		}

		//ʹ����ѭ������
		idx_current_anim = idx_current_anim % PLAYER_ANIM_NUM;


		cleardevice();
		//��ͼ

		//�̱���ͼƬ
		putimage(0, 0, &img_background);
		//������Ⱦ������֡������������
		putimage_alpha(player_pos.x, player_pos.y, &img_player_left[idx_current_anim]);

		FlushBatchDraw();

		DWORD end_time = GetTickCount();
		DWORD delta_time = end_time - begin_time;

		if (delta_time < 1000 / 60) {
			Sleep(1000 / 60 - delta_time);
		}
	}


	return 0;
}