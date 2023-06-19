#include "pch.h"
#include "Keyboard.h"

Keyboard* Keyboard::s_keyboard;

Keyboard* Keyboard::Get()
{
	return s_keyboard;
}

Keyboard::Keyboard()
{
	s_keyboard = this;

	UpdateKeys(false);
}

void Keyboard::KeyDown(const Key& key)
{
	SetKey(key, true);
}

void Keyboard::KeyUp(const Key& key)
{
	SetKey(key, false);
}

bool Keyboard::GetKeyPressed(const Key& key)
{
	return GetKeyPress(key) == KeyPress::DOWN_PRESSED;
}

bool Keyboard::GetKeyDown(const Key& key)
{
	KeyPress key_press = GetKeyPress(key);
	return (key_press == KeyPress::DOWN) || (key_press == KeyPress::DOWN_PRESSED);
}

void Keyboard::UpdateKeys(bool pressed)
{
	KeyAlreadyPressed(key_pressed.backspace, pressed, true);
	KeyAlreadyPressed(key_pressed.enter, pressed, true);
	KeyAlreadyPressed(key_pressed.ralt, pressed, true);
	KeyAlreadyPressed(key_pressed.lalt, pressed, true);
	KeyAlreadyPressed(key_pressed.capslock, pressed, true);
	KeyAlreadyPressed(key_pressed.escape, pressed, true);
	KeyAlreadyPressed(key_pressed.spacebar, pressed, true);

	//Arrows
	KeyAlreadyPressed(key_pressed.left_arrow, pressed, true);
	KeyAlreadyPressed(key_pressed.right_arrow, pressed, true);
	KeyAlreadyPressed(key_pressed.down_arrow, pressed, true);
	KeyAlreadyPressed(key_pressed.up_arrow, pressed, true);

	//Numbers
	KeyAlreadyPressed(key_pressed.num0, pressed, true);
	KeyAlreadyPressed(key_pressed.num1, pressed, true);
	KeyAlreadyPressed(key_pressed.num2, pressed, true);
	KeyAlreadyPressed(key_pressed.num3, pressed, true);
	KeyAlreadyPressed(key_pressed.num4, pressed, true);
	KeyAlreadyPressed(key_pressed.num5, pressed, true);
	KeyAlreadyPressed(key_pressed.num6, pressed, true);
	KeyAlreadyPressed(key_pressed.num7, pressed, true);
	KeyAlreadyPressed(key_pressed.num8, pressed, true);
	KeyAlreadyPressed(key_pressed.num9, pressed, true);

	//Letters
	KeyAlreadyPressed(key_pressed.a, pressed, true);
	KeyAlreadyPressed(key_pressed.b, pressed, true);
	KeyAlreadyPressed(key_pressed.c, pressed, true);
	KeyAlreadyPressed(key_pressed.d, pressed, true);
	KeyAlreadyPressed(key_pressed.e, pressed, true);
	KeyAlreadyPressed(key_pressed.f, pressed, true);
	KeyAlreadyPressed(key_pressed.g, pressed, true);
	KeyAlreadyPressed(key_pressed.h, pressed, true);
	KeyAlreadyPressed(key_pressed.i, pressed, true);
	KeyAlreadyPressed(key_pressed.j, pressed, true);
	KeyAlreadyPressed(key_pressed.k, pressed, true);
	KeyAlreadyPressed(key_pressed.l, pressed, true);
	KeyAlreadyPressed(key_pressed.m, pressed, true);
	KeyAlreadyPressed(key_pressed.n, pressed, true);
	KeyAlreadyPressed(key_pressed.o, pressed, true);
	KeyAlreadyPressed(key_pressed.p, pressed, true);
	KeyAlreadyPressed(key_pressed.q, pressed, true);
	KeyAlreadyPressed(key_pressed.r, pressed, true);
	KeyAlreadyPressed(key_pressed.s, pressed, true);
	KeyAlreadyPressed(key_pressed.t, pressed, true);
	KeyAlreadyPressed(key_pressed.u, pressed, true);
	KeyAlreadyPressed(key_pressed.v, pressed, true);
	KeyAlreadyPressed(key_pressed.w, pressed, true);
	KeyAlreadyPressed(key_pressed.x, pressed, true);
	KeyAlreadyPressed(key_pressed.y, pressed, true);
	KeyAlreadyPressed(key_pressed.z, pressed, true);

	//
	KeyAlreadyPressed(key_pressed.lshift, pressed, true);
	KeyAlreadyPressed(key_pressed.rshift, pressed, true);
	KeyAlreadyPressed(key_pressed.lctrl, pressed, true);
	KeyAlreadyPressed(key_pressed.rctrl, pressed, true);
	KeyAlreadyPressed(key_pressed.period, pressed, true);
}

void Keyboard::SetKey(const Key& key, bool pressed)
{
	switch (key)
	{
	case Key::BACKSPACE:
		KeyAlreadyPressed(key_pressed.backspace, pressed);
		break;
	case Key::ENTER:
		KeyAlreadyPressed(key_pressed.enter, pressed);
		break;
	case Key::RALT:
		KeyAlreadyPressed(key_pressed.ralt, pressed);
		break;
	case Key::LALT:
		KeyAlreadyPressed(key_pressed.lalt, pressed);
		break;
	case Key::CAPSLOCK:
		KeyAlreadyPressed(key_pressed.capslock, pressed);
		break;
	case Key::ESCAPE:
		KeyAlreadyPressed(key_pressed.escape, pressed);
		break;
	case Key::SPACEBAR:
		KeyAlreadyPressed(key_pressed.spacebar, pressed);
		break;

		//Arrows
	case Key::LEFTARROW:
		KeyAlreadyPressed(key_pressed.left_arrow, pressed);
		break;
	case Key::RIGHTARROW:
		KeyAlreadyPressed(key_pressed.right_arrow, pressed);
		break;
	case Key::DOWNARROW:
		KeyAlreadyPressed(key_pressed.down_arrow, pressed);
		break;
	case Key::UPARROW:
		KeyAlreadyPressed(key_pressed.up_arrow, pressed);
		break;

		//Numbers
	case Key::NUM0:
		KeyAlreadyPressed(key_pressed.num0, pressed);
		break;
	case Key::NUM1:
		KeyAlreadyPressed(key_pressed.num1, pressed);
		break;
	case Key::NUM2:
		KeyAlreadyPressed(key_pressed.num2, pressed);
		break;
	case Key::NUM3:
		KeyAlreadyPressed(key_pressed.num3, pressed);
		break;
	case Key::NUM4:
		KeyAlreadyPressed(key_pressed.num4, pressed);
		break;
	case Key::NUM5:
		KeyAlreadyPressed(key_pressed.num5, pressed);
		break;
	case Key::NUM6:
		KeyAlreadyPressed(key_pressed.num6, pressed);
		break;
	case Key::NUM7:
		KeyAlreadyPressed(key_pressed.num7, pressed);
		break;
	case Key::NUM8:
		KeyAlreadyPressed(key_pressed.num8, pressed);
		break;
	case Key::NUM9:
		KeyAlreadyPressed(key_pressed.num9, pressed);
		break;

		//Letters
	case Key::A:
		KeyAlreadyPressed(key_pressed.a, pressed);
		break;
	case Key::B:
		KeyAlreadyPressed(key_pressed.b, pressed);
		break;
	case Key::C:
		KeyAlreadyPressed(key_pressed.c, pressed);
		break;
	case Key::D:
		KeyAlreadyPressed(key_pressed.d, pressed);
		break;
	case Key::E:
		KeyAlreadyPressed(key_pressed.e, pressed);
		break;
	case Key::F:
		KeyAlreadyPressed(key_pressed.f, pressed);
		break;
	case Key::G:
		KeyAlreadyPressed(key_pressed.g, pressed);
		break;
	case Key::H:
		KeyAlreadyPressed(key_pressed.h, pressed);
		break;
	case Key::I:
		KeyAlreadyPressed(key_pressed.i, pressed);
		break;
	case Key::J:
		KeyAlreadyPressed(key_pressed.j, pressed);
		break;
	case Key::K:
		KeyAlreadyPressed(key_pressed.k, pressed);
		break;
	case Key::L:
		KeyAlreadyPressed(key_pressed.l, pressed);
		break;
	case Key::M:
		KeyAlreadyPressed(key_pressed.m, pressed);
		break;
	case Key::N:
		KeyAlreadyPressed(key_pressed.n, pressed);
		break;
	case Key::O:
		KeyAlreadyPressed(key_pressed.o, pressed);
		break;
	case Key::P:
		KeyAlreadyPressed(key_pressed.p, pressed);
		break;
	case Key::Q:
		KeyAlreadyPressed(key_pressed.q, pressed);
		break;
	case Key::R:
		KeyAlreadyPressed(key_pressed.r, pressed);
		break;
	case Key::S:
		KeyAlreadyPressed(key_pressed.s, pressed);
		break;
	case Key::T:
		KeyAlreadyPressed(key_pressed.t, pressed);
		break;
	case Key::U:
		KeyAlreadyPressed(key_pressed.u, pressed);
		break;
	case Key::V:
		KeyAlreadyPressed(key_pressed.v, pressed);
		break;
	case Key::W:
		KeyAlreadyPressed(key_pressed.w, pressed);
		break;
	case Key::X:
		KeyAlreadyPressed(key_pressed.x, pressed);
		break;
	case Key::Y:
		KeyAlreadyPressed(key_pressed.y, pressed);
		break;
	case Key::Z:
		KeyAlreadyPressed(key_pressed.z, pressed);
		break;

		//
	case Key::LSHIFT:
		KeyAlreadyPressed(key_pressed.lshift, pressed);
		break;
	case Key::RSHIFT:
		KeyAlreadyPressed(key_pressed.rshift, pressed);
		break;
	case Key::LCTRL:
		KeyAlreadyPressed(key_pressed.lctrl, pressed);
		break;
	case Key::RCTRL:
		KeyAlreadyPressed(key_pressed.rctrl, pressed);
		break;
	case Key::PERIOD:
		KeyAlreadyPressed(key_pressed.period, pressed);
		break;
	}
}

void Keyboard::KeyAlreadyPressed(KeyPress& key_press, bool pressed, bool update_key)
{
	if (!pressed)
	{
		key_press = KeyPress::UP;
		return;
	}
	if (pressed && update_key && key_press == KeyPress::UP)
	{
		return;
	}
	if (pressed && key_press == KeyPress::DOWN)
	{
		return;
	}
	if (pressed && key_press == KeyPress::DOWN_PRESSED)
	{
		key_press = KeyPress::DOWN;
		return;
	}
	key_press = KeyPress::DOWN_PRESSED;
}

Keyboard::KeyPress Keyboard::GetKeyPress(const Key& key)
{
	switch (key)
	{
	case Key::BACKSPACE:
		return key_pressed.backspace;
	case Key::ENTER:
		return key_pressed.enter;
	case Key::RALT:
		return key_pressed.ralt;
	case Key::LALT:
		return key_pressed.lalt;
	case Key::CAPSLOCK:
		return key_pressed.capslock;
	case Key::ESCAPE:
		return key_pressed.escape;
	case Key::SPACEBAR:
		return key_pressed.spacebar;

		//Arrows
	case Key::LEFTARROW:
		return key_pressed.left_arrow;
	case Key::RIGHTARROW:
		return key_pressed.right_arrow;
	case Key::DOWNARROW:
		return key_pressed.down_arrow;
	case Key::UPARROW:
		return key_pressed.up_arrow;

		//Numbers
	case Key::NUM0:
		return key_pressed.num0;
	case Key::NUM1:
		return key_pressed.num1;
	case Key::NUM2:
		return key_pressed.num2;
	case Key::NUM3:
		return key_pressed.num3;
	case Key::NUM4:
		return key_pressed.num4;
	case Key::NUM5:
		return key_pressed.num5;
	case Key::NUM6:
		return key_pressed.num6;
	case Key::NUM7:
		return key_pressed.num7;
	case Key::NUM8:
		return key_pressed.num8;
	case Key::NUM9:
		return key_pressed.num9;

		//Letters
	case Key::A:
		return key_pressed.a;
	case Key::B:
		return key_pressed.b;
	case Key::C:
		return key_pressed.c;
	case Key::D:
		return key_pressed.d;
	case Key::E:
		return key_pressed.e;
	case Key::F:
		return key_pressed.f;
	case Key::G:
		return key_pressed.g;
	case Key::H:
		return key_pressed.h;
	case Key::I:
		return key_pressed.i;
	case Key::J:
		return key_pressed.j;
	case Key::K:
		return key_pressed.k;
	case Key::L:
		return key_pressed.l;
	case Key::M:
		return key_pressed.m;
	case Key::N:
		return key_pressed.n;
	case Key::O:
		return key_pressed.o;
	case Key::P:
		return key_pressed.p;
	case Key::Q:
		return key_pressed.q;
	case Key::R:
		return key_pressed.r;
	case Key::S:
		return key_pressed.s;
	case Key::T:
		return key_pressed.t;
	case Key::U:
		return key_pressed.u;
	case Key::V:
		return key_pressed.v;
	case Key::W:
		return key_pressed.w;
	case Key::X:
		return key_pressed.x;
	case Key::Y:
		return key_pressed.y;
	case Key::Z:
		return key_pressed.z;

		//
	case Key::LSHIFT:
		return key_pressed.lshift;
	case Key::RSHIFT:
		return key_pressed.rshift;
	case Key::LCTRL:
		return key_pressed.lctrl;
	case Key::RCTRL:
		return key_pressed.rctrl;
	case Key::PERIOD:
		return key_pressed.period;
	}

	assert(false);
	return KeyPress::UP;
}
