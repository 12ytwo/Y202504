#pragma once
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
private:
	bool CheckCursorHit(int x, int y) {
		return x >= region.left && x <= region.right && y >= region.top && y <= region.bottom;
	}
};

