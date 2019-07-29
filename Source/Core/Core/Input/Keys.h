// Copyright 2015-2019 Piperift - All rights reserved
#pragma once

#include "CoreEngine.h"


enum class EKeyPressState : u8 {
	Released, // Key is not pressed
	Press, // The key has been pressed this frame
	Pressed, // Key is being kept pressed
	Release // Key has been released this frame
};

enum class EKeyModifier : u8 {
	None = 0,
	LShift = 1 << 0,
	RShift = 1 << 1,
	LCtrl  = 1 << 2,
	RCtrl  = 1 << 3,
	LAlt   = 1 << 4,
	RAlt   = 1 << 5,
	Shift  = LShift | RShift,
	Ctrl   = LCtrl  | RCtrl,
	Alt    = LAlt   | RAlt
};

enum class EKey : u64 {
	None,

	/**
	 *  \name Usage page 0x07
	 *
	 *  These values are from usage page 0x07 (USB keyboard page).
	 */
	 /* @{ */

	 A = 4,
	 B = 5,
	 C = 6,
	 D = 7,
	 E = 8,
	 F = 9,
	 G = 10,
	 H = 11,
	 I = 12,
	 J = 13,
	 K = 14,
	 L = 15,
	 M = 16,
	 N = 17,
	 O = 18,
	 P = 19,
	 Q = 20,
	 R = 21,
	 S = 22,
	 T = 23,
	 U = 24,
	 V = 25,
	 W = 26,
	 X = 27,
	 Y = 28,
	 Z = 29,

	 N1 = 30,
	 N2 = 31,
	 N3 = 32,
	 N4 = 33,
	 N5 = 34,
	 N6 = 35,
	 N7 = 36,
	 N8 = 37,
	 N9 = 38,
	 N0 = 39,

	 Return = 40,
	 Escape = 41,
	 Backspace = 42,
	 Tab = 43,
	 Space = 44,

	 Minus = 45,
	 Equals = 46,
	 LeftBracket = 47,
	 RightBracket = 48,
	 BackSlash = 49,   /**< Located at the lower Left of the return
				    	*   key on ISO keyboards and at the right end
				    	*   of the QWERTY row on ANSI keyboards.
				    	*   Produces REVERSE SOLIDUS (backslash) and
				    	*   Vertical LINE in a US layout, REVERSE
				    	*   SOLIDUS and Vertical LINE in a UK Mac
				    	*   layout, NUMBER SIGN and TILDE in a UK
				    	*   Windows layout, DOLLAR SIGN and POUND SIGN
				    	*   in a Swiss German layout, NUMBER SIGN and
				    	*   APOSTROPHE in a German layout, GRAVE
				    	*   ACCENT and POUND SIGN in a French Mac
				    	*   layout, and ASTERISK and MICRO SIGN in a
				    	*   French Windows layout.
				    	*/
	NonusHash = 50, /**< ISO USB keyboards actually use this code
					 *   instead of 49 for the same key, but all
					 *   OSes I've seen treat the two codes
					 *   identically. So, as an implementor, unless
					 *   your keyboard generates both of those
					 *   codes and your OS treats them differently,
					 *   you should generate BACKSLASH
					 *   instead of this code. As a user, you
					 *   should not rely on this code because SDL
					 *   will never generate it with most (all?)
					 *   keyboards.
					 */
	Semicolon = 51,
	Apostrophe = 52,
	Grave = 53, /**< Located in the top Left corner (on both ANSI
			     *   and ISO keyboards). Produces GRAVE ACCENT and
			     *   TILDE in a US Windows layout and in US and UK
			     *   Mac layouts on ANSI keyboards, GRAVE ACCENT
			     *   and NOT SIGN in a UK Windows layout, SECTION
			     *   SIGN and Plus-MINUS SIGN in US and UK Mac
			     *   layouts on ISO keyboards, SECTION SIGN and
			     *   DEGREE SIGN in a Swiss German layout (Mac:
			     *   only on ISO keyboards), CIRCUMFLEX ACCENT and
			     *   DEGREE SIGN in a German layout (Mac: only on
			     *   ISO keyboards), SUPERSCRIPT TWO and TILDE in a
			     *   French Windows layout, COMMERCIAL AT and
			     *   NUMBER SIGN in a French Mac layout on ISO
			     *   keyboards, and LESS-THAN SIGN and GREATER-THAN
			     *   SIGN in a Swiss German, German, or French Mac
			     *   layout on ANSI keyboards.
			     */
	Comma = 54,
	Period = 55,
	Slash = 56,

	CapsLock = 57,

	F1 = 58,
	F2 = 59,
	F3 = 60,
	F4 = 61,
	F5 = 62,
	F6 = 63,
	F7 = 64,
	F8 = 65,
	F9 = 66,
	F10 = 67,
	F11 = 68,
	F12 = 69,

	PrintScreen = 70,
	ScrollLock = 71,
	Pause = 72,
	Insert = 73, /**< insert on PC, help on some Mac keyboards (but
								   does send code 73, not 117) */
	Home = 74,
	PageUp = 75,
	Delete = 76,
	End = 77,
	PageDown = 78,
	Right = 79,
	Left = 80,
	Down = 81,
	Up = 82,

	NumLockClear = 83, /**< num lock on PC, clear on Mac keyboards
									 */
	KP_Divide = 84,
	KP_Multiply = 85,
	KP_Minus = 86,
	KP_Plus = 87,
	KP_Enter = 88,
	KP_1 = 89,
	KP_2 = 90,
	KP_3 = 91,
	KP_4 = 92,
	KP_5 = 93,
	KP_6 = 94,
	KP_7 = 95,
	KP_8 = 96,
	KP_9 = 97,
	KP_0 = 98,
	KP_Period = 99,

	NonusBackSlash = 100, /**< This is the additional key that ISO
						   *   keyboards have over ANSI ones,
						   *   located between Left shift and Y.
						   *   Produces GRAVE ACCENT and TILDE in a
						   *   US or UK Mac layout, REVERSE SOLIDUS
						   *   (backslash) and Vertical LINE in a
						   *   US or UK Windows layout, and
						   *   LESS-THAN SIGN and GREATER-THAN SIGN
						   *   in a Swiss German, German, or French
						   *   layout. */
	Application = 101, /**< windows contextual menu, compose */
	Power = 102, /**< The USB document says this is a status flag,
				  *   not a physical key - but some Mac keyboards
				  *   do have a power key. */
	KP_Equals = 103,
	F13 = 104,
	F14 = 105,
	F15 = 106,
	F16 = 107,
	F17 = 108,
	F18 = 109,
	F19 = 110,
	F20 = 111,
	F21 = 112,
	F22 = 113,
	F23 = 114,
	F24 = 115,
	Execute = 116,
	Help = 117,
	Menu = 118,
	Select = 119,
	Stop = 120,
	Again = 121,   /**< redo */
	Undo = 122,
	Cut = 123,
	Copy = 124,
	Paste = 125,
	Find = 126,
	Mute = 127,
	VolumeUp = 128,
	VolumeDown = 129,
	/* not sure whether there's a reason to enable these */
	/*     LOCKINGCAPSLOCK = 130,  */
	/*     LOCKINGNUMLOCK = 131, */
	/*     LOCKINGSCROLLLOCK = 132, */
	KP_Comma = 133,
	KP_EqualsAs400 = 134,

	International1 = 135, /**< used on Asian keyboards, see footnotes in USB doc */
	International2 = 136,
	International3 = 137, /**< Yen */
	International4 = 138,
	International5 = 139,
	International6 = 140,
	International7 = 141,
	International8 = 142,
	International9 = 143,
	Lang1 = 144, /**< Hangul/English toggle */
	Lang2 = 145, /**< Hanja conversion */
	Lang3 = 146, /**< Katakana */
	Lang4 = 147, /**< Hiragana */
	Lang5 = 148, /**< Zenkaku/Hankaku */
	Lang6 = 149, /**< reserved */
	Lang7 = 150, /**< reserved */
	Lang8 = 151, /**< reserved */
	Lang9 = 152, /**< reserved */

	Alterase = 153, /**< Erase-Eaze */
	SysReq = 154,
	Cancel = 155,
	Clear = 156,
	Prior = 157,
	Return2 = 158,
	Separator = 159,
	Out = 160,
	Oper = 161,
	ClearAgain = 162,
	CRSel = 163,
	EXSel = 164,

	KP_00 = 176,
	KP_000 = 177,
	ThousandsSeparator = 178,
	DecimalSeparator = 179,
	CurrencyUnit = 180,
	CurrencySubUnit = 181,
	KP_LeftParen = 182,
	KP_RightParen = 183,
	KP_LeftBrace = 184,
	KP_RightBrace = 185,
	KP_Tab = 186,
	KP_BackSpace = 187,
	KP_A = 188,
	KP_B = 189,
	KP_C = 190,
	KP_D = 191,
	KP_E = 192,
	KP_F = 193,
	KP_XOR = 194,
	KP_Power = 195,
	KP_Percent = 196,
	KP_Less = 197,
	KP_Greater = 198,
	KP_Ampersand = 199,
	KP_DBLAmpersand = 200,
	KP_VerticalBar = 201,
	KP_DBLVerticalBar = 202,
	KP_COLON = 203,
	KP_HASH = 204,
	KP_Space = 205,
	KP_At = 206,
	KP_Exclam = 207,
	KP_MemStore = 208,
	KP_MemRecall = 209,
	KP_MemClear = 210,
	KP_MemAdd = 211,
	KP_MemSubtract = 212,
	KP_MemMultiply = 213,
	KP_MemDivide = 214,
	KP_PlusMinus = 215,
	KP_Clear = 216,
	KP_ClearEntry = 217,
	KP_Binary = 218,
	KP_Octal = 219,
	KP_Decimal = 220,
	KP_HexaDecimal = 221,

	LCtrl = 224,
	LShift = 225,
	LAlt = 226, /**< alt, option */
	LGui = 227, /**< windows, command (apple), meta */
	RCtrl = 228,
	RShift = 229,
	RAlt = 230, /**< alt gr, option */
	RGui = 231, /**< windows, command (apple), meta */

	Mode = 257,    /**< I'm not sure if this is really not covered
					*   by any of the above, but since there's a
					*   special KMOD_MODE for it I'm adding it here
					*/

				 /* @} *//* Usage page 0x07 */

		 		/**
		   	     *  \name Usage page 0x0C
				 *
		 		 *  These values are mapped from usage page 0x0C (USB consumer page).
		 		 */
				/* @{ */

	AudioNext = 258,
	AudioPrev = 259,
	AudioStop = 260,
	AudioPlay = 261,
	AudioMute = 262,
	MediaSelect = 263,
	WWW = 264,
	Mail = 265,
	Calculator = 266,
	Computer = 267,
	AC_Search = 268,
	AC_Home = 269,
	AC_Back = 270,
	AC_Forward = 271,
	AC_Stop = 272,
	AC_Refresh = 273,
	AC_Bookmarks = 274,

	/* @} *//* Usage page 0x0C */

	/**
	 *  \name Walther keys
	 *
	 *  These are values that Christian Walther added (for mac keyboard?).
	 */
	/* @{ */

	BrightNessDown = 275,
	BrightNessUp   = 276,
	DisplaySwitch  = 277, /**< display mirroring/dual display switch, video mode switch */
	KBDIllumToggle = 278,
	KBDIllumDown   = 279,
	KBDIllumUp     = 280,
	Eject          = 281,
	Sleep          = 282,

	App1 = 283,
	App2 = 284,

	/* @} *//* Walther keys */

	/**
	 *  \name Usage page 0x0C (additional media keys)
	 *
	 *  These values are mapped from usage page 0x0C (USB consumer page).
	 */
	 /* @{ */

	 AudioRewind = 285,
	 AudioFastForward = 286,

	 /* @} *//* Usage page 0x0C (additional media keys) */

	 /* Add any other keys here. */

	MouseLeft = 300,
	MouseCenter = 301,
	MouseRight = 302,

	Max = 303
};

enum class EAxis : u8 {
	MouseX,
	MouseY,
	MouseWheelX,
	MouseWheelY,
	Max
};


/** Key Modifier operations */
constexpr u8 operator*(EKeyModifier f)
{
	return static_cast<u8>(f);
}

constexpr EKeyModifier operator|(EKeyModifier lhs, EKeyModifier rhs)
{
	return static_cast<EKeyModifier>((*lhs) | (*rhs));
}

constexpr EKeyModifier& operator|=(EKeyModifier& lhs, EKeyModifier rhs)
{
	lhs = lhs | rhs;
	return lhs;
}

constexpr EKeyModifier operator&(EKeyModifier lhs, EKeyModifier rhs)
{
	return static_cast<EKeyModifier>((*lhs) & (*rhs));
}

constexpr EKeyModifier& operator&=(EKeyModifier& lhs, EKeyModifier rhs)
{
	lhs = lhs & rhs;
	return lhs;
}

constexpr bool operator!(EKeyModifier lhs)
{
	return *lhs <= 0;
}

constexpr EKeyModifier operator~(EKeyModifier lhs)
{
	return static_cast<EKeyModifier>(~*lhs);
}

constexpr bool operator>(EKeyModifier lhs, u8 rhs)
{
	return (*lhs) > rhs;
}

constexpr bool operator<(EKeyModifier lhs, u8 rhs)
{
	return (*lhs) < rhs;
}

constexpr bool operator>(u8 lhs, EKeyModifier rhs)
{
	return lhs > (*rhs);
}

constexpr bool operator<(u8 lhs, EKeyModifier rhs)
{
	return lhs < (*rhs);
}
