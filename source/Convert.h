// -------------------------------------------------------------
// BMP to MSX
//											(C)2000/5/6 HRA!
// -------------------------------------------------------------

#ifndef _CONVERTER_H_
#define	_CONVERTER_H_

#include <windows.h>

typedef bool ( *PROGRESS )( int );		// �o�ߕ\���p�R�[���o�b�N�֐��^
typedef unsigned int uint;
typedef unsigned char uchar;

typedef struct {
	int		w;			// ��
	int		h;			// ����
} SSIZE;

typedef struct {
	int		green;		// ��
	int		red;		// ��
	int		blue;		// ��
} PAL;

typedef struct {
	COLORREF	c;		// �F
	int			p[2];	// �^�C���Ɏg����p���b�g�̔ԍ�
} TAILPAT;

#define TAILMAX	1575	// �^�C�������ő吔

typedef struct {
	uint		Mode;		// �ϊ��惂�[�h MD_SC??
	uint		err;		// ���̒l��菬�����덷�͖�������
	bool		Gosa;		// �덷�g�U ����:true / ���Ȃ�:false
	bool		Inter;		// �C���^�[���[�X ����:true / ���Ȃ�:false
	uint		SelCol;		// �F�I�����[�h 0:���U�I�� / 1:�����z�I��
	float		Gosaval;	// �덷�g�U�W�� ( 0.000�`0.500 )
	bool		Pal;		// �Œ�p���b�g ����:true / ���Ȃ�:false
	bool		Resize;		// �T�C�Y����
	PAL			Col[16];	// SC5/SC7 �ɂ�����l�r�w���p���b�g�w��
	uint		PltMode;	// �p���b�g�o�̓��[�h
	bool		AutoName;	// �����t�@�C��������
	uint		AlgoMode;	// ���R�搶���A���S���Y���ԍ�
	bool		JKrc;		// �F�����̍Čv�Z
	uint		ErrAlgo;	// �f�B�U�p�^�[��
	uint		PreView;	// 1 �t�@�C���̂ݏo��,2 �v���r���[�̂ݏo��,3 �t�@�C���ƃv���r���[�o��
	uchar		PalEn[16];	// 2 �p���b�g�g�p����(�Œ�) / 1 �p���b�g�g�p������ / 0 �p���b�g�g�p����(����)
	uint		Seido;		// �F�Č����x
	uint		ErrAdd;		// �f�B�U���Z���@
	bool		NonZero;	// �O�Ԃ̐F���g��Ȃ�
	uint		FourceZero;	// �����[����
	COLORREF	FZColor;	// �����[��������F
	uint		FZX;		// �����[�����w���W
	uint		FZY;		// �����[�����x���W
	bool		Tile;		// �Ԃ����^�C�����g��
	int			SortMode;	// ��گĂ̿�ĕ��@
	bool		bDefault;	// ��ɍĕϊ�����̫��
	int			CnvMode;	// �ϊ��ݒ�
	bool		b192;		// 192���C�����[�h
	int			Resample;	// ���ޒ���ػ����
	int			SizeMode;	// ���ޒ���Ӱ��
	COLORREF	FCColor;	// �w�i�F
} SETTING;

// ��ʃ��[�h
enum {
	MD_SC2,				// SCREEN 2
	MD_SC3,				// SCREEN 3
	MD_SC5,				// SCREEN 5
	MD_SC6,				// SCREEN 6
	MD_SC7,				// SCREEN 7
	MD_SC8,				// SCREEN 8
	MD_SC10,			// SCREEN 10
	MD_SC12,			// SCREEN 12
	MD_SC5_256L,		// SCREEN 5 (256 lines)
	MD_SC6_256L,		// SCREEN 6 (256 lines)
	MD_SC7_256L,		// SCREEN 7 (256 lines)
	MD_SC8_256L,		// SCREEN 8 (256 lines)
	MD_SC10_256L,		// SCREEN 10 (256 lines)
	MD_SC12_256L,		// SCREEN 12 (256 lines)
	MD_MAX,
};

// �p���b�g����A���S���Y��
enum {
	SELCOL_BUNSAN=0,	// ���U�I��
	SELCOL_TABUNPU,		// �����z�I��
	SELCOL_MAX,
};

// �p���b�g�o�̓��[�h
enum {
	PLT_NONE=0,			// �o�͂��Ȃ�
	PLT_BSAVE,			// �摜�ɓ���
	PLT_PL,				// *.PL? �`��
	PLT_CSV,			// CSV�`��
	PLT_COUNT,			// �o�̓��[�h�̎�ސ�
	PLT_MAX,
};

// ���R�搶���A���S���Y��
enum {
	ALGO_NONE,			// ����
	ALGO_RECALC,		// �t�Z�ɂ��덷�����i�P�x�Č��D��j
	ALGO_POWERFUL,		// �p���t�����p�@�̃��C�g���[�V���O�@�Ɏg���Ă�����
	ALGO_NTSC2,			// NTSC�������ǔ�
	ALGO_RECALC2,		// �t�Z�ɂ��덷�����i�F�����Č��D��j
	ALGO_MAX,
};

// �f�B�U
enum {
	EALGO_NONE,			// ����
	EALGO_DITH1,		// �f�B�U�P
	EALGO_DITH2,		// �f�B�U�Q
	EALGO_DITH3,		// �f�B�U�R
	EALGO_DITH4,		// �f�B�U�S
	EALGO_DITH5,		// �f�B�U�T
	EALGO_DITH6,		// �f�B�U�U
	EALGO_DITH7,		// �f�B�U�V
	EALGO_DITH8,		// �f�B�U�W
	EALGO_MAX,
};

// �f�B�U���Z���@
enum {
	EADD_ROTATE,		// ��]
	EADD_NONE,			// ���ʑ�
	EADD_X,				// �w���炵
	EADD_Y,				// �x���炵
	EADD_MAX,
};

// �F�Č����x
enum {
	SEIDO_24,			// 24bit(0bit�̂�)
	SEIDO_21,			// 21bit(1bit�̂�)
	SEIDO_18,			// 18bit(2bit�̂�)
	SEIDO_15,			// 15bit(3bit�̂�)
	SEIDO_12,			// 12bit(4bit�̂�)
	SEIDO_MAX,
};

// �p���b�g�g�p����
enum {
	PALEN_AUTO,			// �p���b�g�g�p���i�����j
	PALEN_NONE,			// �p���b�g�g�p������
	PALEN_USE,			// �p���b�g�g�p���i�Œ�j
	PALEN_MAX,
};

// �����[����
enum {
	FZ_NONE,			// �g��Ȃ�
	FZ_COLOR,			// �w��F
	FZ_MATRIX,			// �w����W�̐F
	FZ_MAX,
};

//	�\�[�g
enum {
	SM_IGNORE,			//	�\�[�g���Ȃ�
	SM_INCAUTO,			//	��������̂ݏ���
	SM_DECAUTO,			//	��������̂ݍ~��
	SM_INC,				//	����
	SM_DEC,				//	�~��
};

//	�ϊ��ݒ�
enum {
	CM_DEFAULT,			//	�f�t�H���g�ݒ�
	CM_LAST,			//	�ŏI�ϊ��̐ݒ�
	CM_IMAGE,			//	�\���摜�̕ϊ��ݒ�
};

//	�T�C�Y����
enum {
	RS_NORMAL,			//	�P������
	RS_ANTIALIAS,		//	�A���`�G�C���A�X�t������
};

//	�A�X�y�N�g��
enum {
	RM_FORCE,			//	�����I�ɏo�̓T�C�Y
	RM_MSX,				//	�l�r�w���@�\���ŏc���䂪����Ȃ��悤�ɒ���
};

//	RGB�l����e�[�u���C���f�b�N�X�ɕϊ�
int convert_rgb_to_palette( const int *p_convert_table, int n, int v );

//	�k������
bool cnvNormResize( COLORREF *in , int inwidth , int inheight ,
			    COLORREF *out, int outwidth, int outheight, int wwidth, int wheight, PROGRESS prog, int seido );
bool cnvAntiResize( COLORREF *in , int inwidth , int inheight ,
			    COLORREF *out, int outwidth, int outheight, int wwidth, int wheight, PROGRESS prog, int seido );

//	��������
bool cnvCopy( COLORREF *in ,int inwidth ,int inheight ,
			  COLORREF *out,int outwidth,int outheight,PROGRESS prog,int seido );
bool cnvCopyInter(	COLORREF *in ,int inwidth ,int inheight ,
					COLORREF *out,int outwidth,int outheight,PROGRESS prog,int seido );

//	���F����
bool cnvRecolor( COLORREF *in,int width,int height,unsigned char *out,SETTING *CnvMode,PROGRESS prog,
				 COLORREF *pal,TAILPAT *tail,int tailcnt );

//	���F�����i���R��R���o�[�^�j
bool cnvNtcolor( COLORREF *in,int width,int height,unsigned char *out,SETTING *CnvMode,PROGRESS prog );

//	�F��r
int cnvColorTblCompare( const void *tbl1,const void *tbl2 );

//	�F�擾
bool cnvGetPalette( COLORREF *in,int width,int height,COLORREF *pal,int mode,int cnt,int pp,
				    bool FourceZero,COLORREF FZColor );
bool cnvGetPaletteS8( COLORREF *pal );

//	�^�C���p�^�[���擾(SC5/SC7��p)
int cnvCreateTail4( PAL *pal,uchar *palen,bool zeroen,TAILPAT *tail, int mode );

//	�v���r���[�쐬�i�t�ϊ��j
void DrawScreen( const unsigned char *bmp,HDC hDC,const SETTING *Mode );

//	�p���b�g�̃\�[�g
void cnvSortPalette( SETTING* Mode, COLORREF* Pal );

//	�v���r���[�̂a�l�o�ۑ�
bool cnvSaveBmpFile( const char *szInFileName, LPBYTE bmp, int width, int height, SETTING *Mode );


// MSX�K�� �� Win�K�� ( 0�`255 ) �ϊ��e�[�u��
extern const int init_convert7to255_r[ 8 ];
extern const int init_convert7to255_g[ 8 ];
extern const int init_convert7to255_b[ 8 ];

// MSX�K�� SCREEN8 �� Win�K�� ( 0�`255 ) �ϊ��e�[�u��
extern const int init_convert7to255_s8r[ 8 ];
extern const int init_convert7to255_s8g[ 8 ];
extern const int init_convert3to255_s8b[ 4 ];

// MSX�K�� SCREEN12 �� Win�K�� ( 0�`255 ) �ϊ��e�[�u��
extern const int init_convert31to255_s12r[ 32 ];
extern const int init_convert31to255_s12g[ 32 ];
extern const int init_convert31to255_s12b[ 32 ];

// MSX�K�� �� Win�K�� ( 0�`255 ) �ϊ��e�[�u��
extern int convert7to255_r[ 8 ];
extern int convert7to255_g[ 8 ];
extern int convert7to255_b[ 8 ];

// MSX�K�� SCREEN8 �� Win�K�� ( 0�`255 ) �ϊ��e�[�u��
extern int convert7to255_s8r[ 8 ];
extern int convert7to255_s8g[ 8 ];
extern int convert3to255_s8b[ 4 ];

// MSX�K�� SCREEN12 �� Win�K�� ( 0�`255 ) �ϊ��e�[�u��
extern int convert31to255_s12r[ 32 ];
extern int convert31to255_s12g[ 32 ];
extern int convert31to255_s12b[ 32 ];

#endif	// ndef _CONVERTER_H_
