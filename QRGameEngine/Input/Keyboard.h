#pragma once

class Keyboard
{
public:
	enum class Key : uint8_t
	{
		BACKSPACE = 8, TAB,
		ENTER = 13,
		LCTRL = 17, RCTRL,
		CAPSLOCK = 20,
		ESCAPE = 27,
		SPACEBAR = 32,
		LEFTARROW = 37, UPARROW, RIGHTARROW, DOWNARROW,
		NUM0 = 48, NUM1, NUM2, NUM3, NUM4, NUM5, NUM6, NUM7, NUM8, NUM9,
		A = 65, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
		F1 = 112, F2, F3, F4,
		LSHIFT = 16, RSHIFT = 160, LALT, RALT,
		PERIOD = 190,
	};

private:
	enum class KeyPress : uint8_t
	{
		UP = 0,
		DOWN_PRESSED = 1,
		DOWN = 2
	};

private:
	struct KeyPressed
	{
		KeyPress backspace, tab;
		KeyPress enter;
		KeyPress ralt, lalt;
		KeyPress capslock;
		KeyPress escape;
		KeyPress spacebar;
		KeyPress left_arrow, up_arrow, right_arrow, down_arrow;
		KeyPress num0, num1, num2, num3, num4, num5, num6, num7, num8, num9;
		KeyPress a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z;
		KeyPress f1, f2, f3, f4;
		KeyPress lshift, rshift, lctrl, rctrl;
		KeyPress period;
	} key_pressed;

private:
	static Keyboard* s_keyboard;
	
private:
	void SetKey(const Key& key, bool pressed);
	void KeyAlreadyPressed(KeyPress& key_press, bool pressed, bool update_key = false);
	KeyPress GetKeyPress(const Key& key);

public:
	Keyboard();
	static Keyboard* Get();

	void KeyDown(const Key& key);
	void KeyUp(const Key& key);

	bool GetKeyPressed(const Key& key);
	bool GetKeyDown(const Key& key);

	void UpdateKeys(bool pressed = true);
};

