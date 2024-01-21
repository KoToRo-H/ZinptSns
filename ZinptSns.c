/*
	ZinptSns.c	キーボード、ジョイスティック、マウスのボタン情報を表示する
	XCコンパイル ： cc /Y ZinptSns.c
	GCCコンパイル： GCC -ldos -liocs ZinptSns.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <doslib.h>
#include <iocslib.h>

#define KEY_BIT_MASK  (0x00000001) /* キーのビット位置情報マスク */
#define KEY_BIT_SENS  (0x00000001) /* キーのビット位置情報を検出 */
#define KEY_GRP_NUM   (16) /* キーグループの数 */
#define KEY_BIT_NUM   ( 8) /* キー情報のビット数 */

#define JOY_NO_1      (0) /* ジョイスティック1 */
#define JOY_NO_2      (1) /* ジョイスティック2 */
#define JOY_BTN_NUM   (8) /* ジョイスティックデータのビット数 */
#define JOY_FF_MAX    (3) /* ジョイスティックデータが連続してFFだった回数 */

#define MUS_DATA_MASK (0x000000FF) /* マウスデータ下位8bitマスク */
#define MUS_STAT_NUM  (6) /* マウス状態の数 */
#define MUS_DEAD_ZONE (0) /* マウス不感知範囲 */

#define scanCode(group, bit) ((((group) & 0x0F)<<3) | ((bit) & 0x07)) /* スキャンコード作成 */

int key_grup_tmp = 0x00; /* 押されたキーグループを記憶 */
int key_code_tmp = 0x00; /* 押されたキーコードを記憶 */
int key_bit_tmp  = 0x00; /* 押されたキービットを記憶 */
int joy_data_tmp = 0xFF; /* 押されたジョイスティックボタンを記憶 */
int joy_FF_count = 0;    /* ボタンが離されていた回数 */
int mus_flag_tmp = 0x00; /* マウス状態フラグを記憶 */
int mus_data_flg = 0x00; /* マウス状態のフラグ保存 */

/* キーコードグループ */
char *key_map[KEY_GRP_NUM][KEY_BIT_NUM] = {
  /* 0 */ {"未定義", "ESC", "1", "2", "3", "4", "5", "6"},
  /* 1 */ {"7", "8", "9", "0", "-", "^", "\\", "BS"},
  /* 2 */ {"TAB", "Q", "W", "E", "R", "T", "Y", "U"},
  /* 3 */ {"I", "O", "P", "@", "[", "CR", "A", "S"},
  /* 4 */ {"D", "F", "G", "H", "J", "K", "L", ";"},
  /* 5 */ {":", "]", "Z", "X", "C", "V", "B", "N"},
  /* 6 */ {"M", ",", ".", "/", "_", "SP", "HOME", "DEL"},
  /* 7 */ {"Roll up", "Roll down", "UNDO", "←", "↑", "→", "↓", "CLR"},
  /* 8 */ {"(/)", "(*)", "(-)", "(7)", "(8)", "(9)", "(+)", "(4)"},
  /* 9 */ {"(5)", "(6)", "(=)", "(1)", "(2)", "(3)", "ENTER", "(0)"},
  /* A */ {"(,)", "(.)", "記号", "登録", "HELP", "XF1", "XF2", "XF3"},
  /* B */ {"XF4", "XF5", "かな", "ローマ字", "コード", "CAPS", "INS", "ひらがな"},
  /* C */ {"全角", "BREAK", "COPY", "F1", "F2", "F3", "F4", "F5"},
  /* D */ {"F6", "F7", "F8", "F9", "F10", "未定義", "未定義", "未定義"},
  /* E */ {"SHIFT", "CTRL", "OPT.1", "OPT.2", "未定義", "未定義", "未定義", "未定義"},
  /* F */ {"未定義", "未定義", "未定義", "未定義", "未定義", "未定義", "未定義", "未定義"}
};

/* ジョイスティックボタン*/
char *joy_button[JOY_BTN_NUM] = {"上", "下", "左", "右", "", "Aボタン", "Bボタン", ""};

/* マウス状態 */
char *mus_status[MUS_STAT_NUM] = {"上", "下", "左", "右", "左ボタン", "右ボタン"};


/* CRキーが離されるまで待つ */
void chkCRkeyReleased()
{
	while (BITSNS(0x03) == 0x20 || BITSNS(0x09) == 0x40)
		;
}

/* キーバッファを空にする */
void keyFlash() {
	while (B_KEYSNS() != 0x00) { /* キー先行入力の確認 */
		INPOUT(0xFF); /* キー待ち無しのコンソール直接入力 */
	}
}

/* キー押下状態 */
int keySens()
{
	int key_code, key_group, key_bit;

	keyFlash(); /* キーを空読み */

	for (key_group = 0; key_group < KEY_GRP_NUM; key_group++) {
		key_code = BITSNS(key_group); /* 入力状態の読み出し */

		if (key_code != 0) { /* 対応キーのビットが1 → 入力あり */
			/* 同じキーを押し続けているのかを確認 */
			if (key_group == key_grup_tmp && key_code == key_code_tmp) {
				return scanCode(key_grup_tmp, key_bit_tmp); /* 同じキーなので終了 */
			}

			/* 異なるキーが押されたのでキー情報を更新 */
			key_grup_tmp = key_group;
			key_code_tmp = key_code;

			/* キー情報の表示 */
			printf("key_group=%02X ", key_group);
			printf("key_code=%02X ", key_code);

			for (key_bit = 0; key_bit <= KEY_BIT_NUM; key_bit++) {
				if ((key_code & KEY_BIT_MASK) == KEY_BIT_SENS) {
					/* 対応キーを検出した */
					printf("[%s]\n", key_map[key_group][key_bit]);

					/* 対応キーを検出したのでキー情報を更新 */
					key_bit_tmp  = key_bit;

					break;
				}
				key_code >>= 1; /* 右シフトで次のbitをLSB側へ */
			}

			return scanCode(key_group, key_bit); /* スキャンコードで返す */
		}
	}

	/* キーが離された */
	key_grup_tmp = 0x00;
	key_code_tmp = 0x00;
	key_bit_tmp  = 0x00;
	return scanCode(key_grup_tmp, key_bit_tmp); /* スキャンコードで返す */
}

/* ジョイスティックボタン状態 */
unsigned char joySens(int stickno)
{
	unsigned char joy_data;
	int  bit;

	joy_data = (char)(JOYGET(stickno) & 0x000000FF); /* ジョイスティックのデータ読み出し */

	if (joy_data == 0xFF) { /* ボタンを押し続けているのにFFが返ることがあるので */
		joy_FF_count++;
		if (joy_FF_count > JOY_FF_MAX) { /* FFが連続したらボタンが離されたと認識 */
			joy_data_tmp = 0xFF;
			joy_FF_count = 0;
		}
		return joy_data_tmp; /* 確定するまでは前のキーデータを返す */
	}

	joy_FF_count = 0; /* FFカウントをクリア */

	if (joy_data == joy_data_tmp) {
		return joy_data; /* 同じボタン状態が続いている */
	}

	joy_data_tmp = joy_data; /* ボタン情報を更新 */

	printf("Joystick %01d : joy_data = %02X ", stickno, joy_data);

	/* データのビット解析 */
	for (bit = 0; bit < JOY_BTN_NUM; bit++) {
		if ( (joy_data & 0x01) == 0x00) { /* LSBを確認 0:ON, 1:OFF */
			printf("[%s]", joy_button[bit]);
		}
		joy_data >>= 1; /* 右bitシフトで検査ビットをLSBに移動 */
	}

	putchar('\n');
	return joy_data; /* ジョイスティックのデータを返す */
}

/* マウスボタン状態 */
int musSens()
{
	int mus_data, mus_flag_bit;
	char mus_X, mus_Y, mus_L, mus_R;

	mus_data = MS_GETDT(); /* マウス状態の読み出し：ビット配置 XXXXYYYYLLLLRRRR */

	mus_X = (char)((mus_data >> 24) & MUS_DATA_MASK);
	mus_Y = (char)((mus_data >> 16) & MUS_DATA_MASK);
	mus_L = (char)((mus_data >>  8) & MUS_DATA_MASK);
	mus_R = (char)((mus_data      ) & MUS_DATA_MASK);

	mus_data_flg = 0x00; /* 各状態の発生状態を格納したフラグ 1=ON */

	if (mus_X > MUS_DEAD_ZONE) { /* X方向移動量 */
		mus_data_flg |= 0x08;    /* 右 */
	}
	else if (mus_X < -1 * MUS_DEAD_ZONE) {
		mus_data_flg |= 0x04;    /* 左 */
	}
	if (mus_Y > MUS_DEAD_ZONE) { /* Y方向移動量 */
		mus_data_flg |= 0x02;    /* 下 */
	}
	else if (mus_Y < -1 * MUS_DEAD_ZONE) {
		mus_data_flg |= 0x01;    /* 上 */
	}
	if (mus_L != 0) { /* 左ボタン */
		mus_data_flg |= 0x10; /* ON */
	}
	else {
		mus_data_flg &= 0xEF; /* OFF */
	}
	if (mus_R != 0) { /* 右ボタン */
		mus_data_flg |= 0x20; /* ON */
	}
	else {
		mus_data_flg &= 0xDF; /* OFF */
	}

	if (mus_flag_tmp == mus_data_flg) {
		return mus_data; /* マウス状態フラグが同じなので終了 */
	}
	mus_flag_tmp = mus_data_flg; /* マウス状態フラグを更新 */

	if (mus_data == 0x00) {
		return mus_data; /* 無操作なので終了 */
	}

	printf("mus_data %08X ", mus_data);

	/* フラグのビット解析 */
	for (mus_flag_bit = 0; mus_flag_bit < MUS_STAT_NUM; mus_flag_bit++) {
		if (mus_data_flg & 0x00000001 != 0) {
			printf("%s ", mus_status[mus_flag_bit]);
		}
		mus_data_flg >>= 1; /* 右bitシフトで検査ビットをLSBに移動 */
	}

	putchar('\n');
	return mus_data; /* マウスの移動量とボタン状態を返す */
}

void main()
{
	int key_sens;

	printf("ZinptSns : キーボード、ジョイスティック、マウスのボタン情報を表示\n");
	printf("[ESC]キーで終了（括弧付きのキー表示はテンキーボタン）\n");

	MS_CURON(); /* マウスカーソルON */
	MS_CURST(384, 256); /* マウスカーソルの座標指定 */
	SKEY_MOD(0, 0, 0); /* ソフトキーボードを消去 */
	chkCRkeyReleased(); /* CRキーが離されるまで待つ */

	while (1) {
		key_sens = keySens(); /* キー押下状態の読み出し */
		if (key_sens == 0x01) { /* Scan code = キーグループ * 8 + キービット位置 */
			break; /* ESCキーが押されたので終了 */
		}

		joySens(JOY_NO_1); /* ジョイスティック1状態の読み出し */
		joySens(JOY_NO_2); /* ジョイスティック2状態の読み出し */

		musSens(); /* マウス状態の読み出し */
	}

	MS_CUROF(); /* マウスカーソルOFF */
	SKEY_MOD(-1, 0, 0); /* ソフトキーボードを自動制御に戻す */

	exit(0);
}
