/*
	ZinptSns.c	�L�[�{�[�h�A�W���C�X�e�B�b�N�A�}�E�X�̃{�^������\������
	XC�R���p�C�� �F cc /Y ZinptSns.c
	GCC�R���p�C���F GCC -ldos -liocs ZinptSns.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <doslib.h>
#include <iocslib.h>

#define KEY_BIT_MASK  (0x00000001) /* �L�[�̃r�b�g�ʒu���}�X�N */
#define KEY_BIT_SENS  (0x00000001) /* �L�[�̃r�b�g�ʒu�������o */
#define KEY_GRP_NUM   (16) /* �L�[�O���[�v�̐� */
#define KEY_BIT_NUM   ( 8) /* �L�[���̃r�b�g�� */

#define JOY_NO_1      (0) /* �W���C�X�e�B�b�N1 */
#define JOY_NO_2      (1) /* �W���C�X�e�B�b�N2 */
#define JOY_BTN_NUM   (8) /* �W���C�X�e�B�b�N�f�[�^�̃r�b�g�� */
#define JOY_FF_MAX    (3) /* �W���C�X�e�B�b�N�f�[�^���A������FF�������� */

#define MUS_DATA_MASK (0x000000FF) /* �}�E�X�f�[�^����8bit�}�X�N */
#define MUS_STAT_NUM  (6) /* �}�E�X��Ԃ̐� */
#define MUS_DEAD_ZONE (0) /* �}�E�X�s���m�͈� */

#define scanCode(group, bit) ((((group) & 0x0F)<<3) | ((bit) & 0x07)) /* �X�L�����R�[�h�쐬 */

int key_grup_tmp = 0x00; /* �����ꂽ�L�[�O���[�v���L�� */
int key_code_tmp = 0x00; /* �����ꂽ�L�[�R�[�h���L�� */
int key_bit_tmp  = 0x00; /* �����ꂽ�L�[�r�b�g���L�� */
int joy_data_tmp = 0xFF; /* �����ꂽ�W���C�X�e�B�b�N�{�^�����L�� */
int joy_FF_count = 0;    /* �{�^����������Ă����� */
int mus_flag_tmp = 0x00; /* �}�E�X��ԃt���O���L�� */
int mus_data_flg = 0x00; /* �}�E�X��Ԃ̃t���O�ۑ� */

/* �L�[�R�[�h�O���[�v */
char *key_map[KEY_GRP_NUM][KEY_BIT_NUM] = {
  /* 0 */ {"����`", "ESC", "1", "2", "3", "4", "5", "6"},
  /* 1 */ {"7", "8", "9", "0", "-", "^", "\\", "BS"},
  /* 2 */ {"TAB", "Q", "W", "E", "R", "T", "Y", "U"},
  /* 3 */ {"I", "O", "P", "@", "[", "CR", "A", "S"},
  /* 4 */ {"D", "F", "G", "H", "J", "K", "L", ";"},
  /* 5 */ {":", "]", "Z", "X", "C", "V", "B", "N"},
  /* 6 */ {"M", ",", ".", "/", "_", "SP", "HOME", "DEL"},
  /* 7 */ {"Roll up", "Roll down", "UNDO", "��", "��", "��", "��", "CLR"},
  /* 8 */ {"(/)", "(*)", "(-)", "(7)", "(8)", "(9)", "(+)", "(4)"},
  /* 9 */ {"(5)", "(6)", "(=)", "(1)", "(2)", "(3)", "ENTER", "(0)"},
  /* A */ {"(,)", "(.)", "�L��", "�o�^", "HELP", "XF1", "XF2", "XF3"},
  /* B */ {"XF4", "XF5", "����", "���[�}��", "�R�[�h", "CAPS", "INS", "�Ђ炪��"},
  /* C */ {"�S�p", "BREAK", "COPY", "F1", "F2", "F3", "F4", "F5"},
  /* D */ {"F6", "F7", "F8", "F9", "F10", "����`", "����`", "����`"},
  /* E */ {"SHIFT", "CTRL", "OPT.1", "OPT.2", "����`", "����`", "����`", "����`"},
  /* F */ {"����`", "����`", "����`", "����`", "����`", "����`", "����`", "����`"}
};

/* �W���C�X�e�B�b�N�{�^��*/
char *joy_button[JOY_BTN_NUM] = {"��", "��", "��", "�E", "", "A�{�^��", "B�{�^��", ""};

/* �}�E�X��� */
char *mus_status[MUS_STAT_NUM] = {"��", "��", "��", "�E", "���{�^��", "�E�{�^��"};


/* CR�L�[���������܂ő҂� */
void chkCRkeyReleased()
{
	while (BITSNS(0x03) == 0x20 || BITSNS(0x09) == 0x40)
		;
}

/* �L�[�o�b�t�@����ɂ��� */
void keyFlash() {
	while (B_KEYSNS() != 0x00) { /* �L�[��s���͂̊m�F */
		INPOUT(0xFF); /* �L�[�҂������̃R���\�[�����ړ��� */
	}
}

/* �L�[������� */
int keySens()
{
	int key_code, key_group, key_bit;

	keyFlash(); /* �L�[����ǂ� */

	for (key_group = 0; key_group < KEY_GRP_NUM; key_group++) {
		key_code = BITSNS(key_group); /* ���͏�Ԃ̓ǂݏo�� */

		if (key_code != 0) { /* �Ή��L�[�̃r�b�g��1 �� ���͂��� */
			/* �����L�[�����������Ă���̂����m�F */
			if (key_group == key_grup_tmp && key_code == key_code_tmp) {
				return scanCode(key_grup_tmp, key_bit_tmp); /* �����L�[�Ȃ̂ŏI�� */
			}

			/* �قȂ�L�[�������ꂽ�̂ŃL�[�����X�V */
			key_grup_tmp = key_group;
			key_code_tmp = key_code;

			/* �L�[���̕\�� */
			printf("key_group=%02X ", key_group);
			printf("key_code=%02X ", key_code);

			for (key_bit = 0; key_bit <= KEY_BIT_NUM; key_bit++) {
				if ((key_code & KEY_BIT_MASK) == KEY_BIT_SENS) {
					/* �Ή��L�[�����o���� */
					printf("[%s]\n", key_map[key_group][key_bit]);

					/* �Ή��L�[�����o�����̂ŃL�[�����X�V */
					key_bit_tmp  = key_bit;

					break;
				}
				key_code >>= 1; /* �E�V�t�g�Ŏ���bit��LSB���� */
			}

			return scanCode(key_group, key_bit); /* �X�L�����R�[�h�ŕԂ� */
		}
	}

	/* �L�[�������ꂽ */
	key_grup_tmp = 0x00;
	key_code_tmp = 0x00;
	key_bit_tmp  = 0x00;
	return scanCode(key_grup_tmp, key_bit_tmp); /* �X�L�����R�[�h�ŕԂ� */
}

/* �W���C�X�e�B�b�N�{�^����� */
unsigned char joySens(int stickno)
{
	unsigned char joy_data;
	int  bit;

	joy_data = (char)(JOYGET(stickno) & 0x000000FF); /* �W���C�X�e�B�b�N�̃f�[�^�ǂݏo�� */

	if (joy_data == 0xFF) { /* �{�^�������������Ă���̂�FF���Ԃ邱�Ƃ�����̂� */
		joy_FF_count++;
		if (joy_FF_count > JOY_FF_MAX) { /* FF���A��������{�^���������ꂽ�ƔF�� */
			joy_data_tmp = 0xFF;
			joy_FF_count = 0;
		}
		return joy_data_tmp; /* �m�肷��܂ł͑O�̃L�[�f�[�^��Ԃ� */
	}

	joy_FF_count = 0; /* FF�J�E���g���N���A */

	if (joy_data == joy_data_tmp) {
		return joy_data; /* �����{�^����Ԃ������Ă��� */
	}

	joy_data_tmp = joy_data; /* �{�^�������X�V */

	printf("Joystick %01d : joy_data = %02X ", stickno, joy_data);

	/* �f�[�^�̃r�b�g��� */
	for (bit = 0; bit < JOY_BTN_NUM; bit++) {
		if ( (joy_data & 0x01) == 0x00) { /* LSB���m�F 0:ON, 1:OFF */
			printf("[%s]", joy_button[bit]);
		}
		joy_data >>= 1; /* �Ebit�V�t�g�Ō����r�b�g��LSB�Ɉړ� */
	}

	putchar('\n');
	return joy_data; /* �W���C�X�e�B�b�N�̃f�[�^��Ԃ� */
}

/* �}�E�X�{�^����� */
int musSens()
{
	int mus_data, mus_flag_bit;
	char mus_X, mus_Y, mus_L, mus_R;

	mus_data = MS_GETDT(); /* �}�E�X��Ԃ̓ǂݏo���F�r�b�g�z�u XXXXYYYYLLLLRRRR */

	mus_X = (char)((mus_data >> 24) & MUS_DATA_MASK);
	mus_Y = (char)((mus_data >> 16) & MUS_DATA_MASK);
	mus_L = (char)((mus_data >>  8) & MUS_DATA_MASK);
	mus_R = (char)((mus_data      ) & MUS_DATA_MASK);

	mus_data_flg = 0x00; /* �e��Ԃ̔�����Ԃ��i�[�����t���O 1=ON */

	if (mus_X > MUS_DEAD_ZONE) { /* X�����ړ��� */
		mus_data_flg |= 0x08;    /* �E */
	}
	else if (mus_X < -1 * MUS_DEAD_ZONE) {
		mus_data_flg |= 0x04;    /* �� */
	}
	if (mus_Y > MUS_DEAD_ZONE) { /* Y�����ړ��� */
		mus_data_flg |= 0x02;    /* �� */
	}
	else if (mus_Y < -1 * MUS_DEAD_ZONE) {
		mus_data_flg |= 0x01;    /* �� */
	}
	if (mus_L != 0) { /* ���{�^�� */
		mus_data_flg |= 0x10; /* ON */
	}
	else {
		mus_data_flg &= 0xEF; /* OFF */
	}
	if (mus_R != 0) { /* �E�{�^�� */
		mus_data_flg |= 0x20; /* ON */
	}
	else {
		mus_data_flg &= 0xDF; /* OFF */
	}

	if (mus_flag_tmp == mus_data_flg) {
		return mus_data; /* �}�E�X��ԃt���O�������Ȃ̂ŏI�� */
	}
	mus_flag_tmp = mus_data_flg; /* �}�E�X��ԃt���O���X�V */

	if (mus_data == 0x00) {
		return mus_data; /* ������Ȃ̂ŏI�� */
	}

	printf("mus_data %08X ", mus_data);

	/* �t���O�̃r�b�g��� */
	for (mus_flag_bit = 0; mus_flag_bit < MUS_STAT_NUM; mus_flag_bit++) {
		if (mus_data_flg & 0x00000001 != 0) {
			printf("%s ", mus_status[mus_flag_bit]);
		}
		mus_data_flg >>= 1; /* �Ebit�V�t�g�Ō����r�b�g��LSB�Ɉړ� */
	}

	putchar('\n');
	return mus_data; /* �}�E�X�̈ړ��ʂƃ{�^����Ԃ�Ԃ� */
}

void main()
{
	int key_sens;

	printf("ZinptSns : �L�[�{�[�h�A�W���C�X�e�B�b�N�A�}�E�X�̃{�^������\��\n");
	printf("[ESC]�L�[�ŏI���i���ʕt���̃L�[�\���̓e���L�[�{�^���j\n");

	MS_CURON(); /* �}�E�X�J�[�\��ON */
	MS_CURST(384, 256); /* �}�E�X�J�[�\���̍��W�w�� */
	SKEY_MOD(0, 0, 0); /* �\�t�g�L�[�{�[�h������ */
	chkCRkeyReleased(); /* CR�L�[���������܂ő҂� */

	while (1) {
		key_sens = keySens(); /* �L�[������Ԃ̓ǂݏo�� */
		if (key_sens == 0x01) { /* Scan code = �L�[�O���[�v * 8 + �L�[�r�b�g�ʒu */
			break; /* ESC�L�[�������ꂽ�̂ŏI�� */
		}

		joySens(JOY_NO_1); /* �W���C�X�e�B�b�N1��Ԃ̓ǂݏo�� */
		joySens(JOY_NO_2); /* �W���C�X�e�B�b�N2��Ԃ̓ǂݏo�� */

		musSens(); /* �}�E�X��Ԃ̓ǂݏo�� */
	}

	MS_CUROF(); /* �}�E�X�J�[�\��OFF */
	SKEY_MOD(-1, 0, 0); /* �\�t�g�L�[�{�[�h����������ɖ߂� */

	exit(0);
}
