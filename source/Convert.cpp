// -------------------------------------------------------------
// BMP to MSX
// �ϊ�����
//										(C)2000/5/6 HRA!
// -------------------------------------------------------------

#include <malloc.h>
#include "Convert.h"
#include "math.h"
#include "fileuty.h"

// -------------------------------------------------------------
// �^
// -------------------------------------------------------------
struct C_COLOR_TABLE {
	C_COLOR				c;		// �F
	int					n;		// ���̐�
};

// -------------------------------------------------------------
// �ݒ�
// -------------------------------------------------------------

const int screen_mode[] = { 2, 3, 5, 6, 7, 8, 10, 12, 5, 6, 7, 8, 10, 12 };	// ��ʃ��[�h�ԍ�

// ��ʃT�C�Y
const C_SCREEN_SIZE screen_size[] = {
	{ 256, 192 },	// screen 2/4
	{ 64, 48 },	// screen 3
	{ 256, 212 },	// screen 5
	{ 512, 212 },	// screen 6
	{ 512, 212 },	// screen 7
	{ 256, 212 },	// screen 8
	{ 256, 212 },	// screen 10/11
	{ 256, 212 },	// screen 12
	{ 256, 256 },	// screen 5 (256 lines)
	{ 512, 256 },	// screen 6 (256 lines)
	{ 512, 256 },	// screen 7 (256 lines)
	{ 256, 256 },	// screen 8 (256 lines)
	{ 256, 256 },	// screen 10/11 (256 lines)
	{ 256, 256 },	// screen 12 (256 lines)
};


// �f�B�U�����O�p�^�[��
const int dith[][8][8]={
	{	// �f�B�U�P�i�^�������_���j
		{0 ,15,7 ,14,6 ,15,5 ,15},
		{13,4 ,9 ,2 ,12,7 ,8 ,2 },
		{6 ,12,1 ,10,7 ,13,4 ,10},
		{11,3 ,8 ,5 ,11,0 ,12,3 },
		{5 ,15,0 ,14,2 ,9 ,1 ,14},
		{9 ,6 ,10,4 ,12,3 ,13,0 },
		{1 ,13,3 ,8 ,5 ,10,4 ,11},
		{14,2 ,9 ,6 ,11,1 ,8 ,7 },
	},{	// �f�B�U�Q�i�^�C���j
		{0 ,11,2 ,9 ,0 ,11,2 ,9 },
		{15,4 ,13,6 ,15,4 ,13,6 },
		{3 ,8 ,1 ,10,3 ,8 ,1 ,10},
		{12,7 ,14,5 ,12,7 ,14,5 },
		{0 ,11,2 ,9 ,0 ,11,2 ,9 },
		{15,4 ,13,6 ,15,4 ,13,6 },
		{3 ,8 ,1 ,10,3 ,8 ,1 ,10},
		{12,7 ,14,5 ,12,7 ,14,5 },
	},{	// �f�B�U�R�i�c���^�C���j
		{0 ,11,2 ,9 ,0 ,11,2 ,9 },
		{0 ,11,2 ,9 ,0 ,11,2 ,9 },
		{15,4 ,13,6 ,15,4 ,13,6 },
		{15,4 ,13,6 ,15,4 ,13,6 },
		{3 ,8 ,1 ,10,3 ,8 ,1 ,10},
		{3 ,8 ,1 ,10,3 ,8 ,1 ,10},
		{12,7 ,14,5 ,12,7 ,14,5 },
		{12,7 ,14,5 ,12,7 ,14,5 },
	},{	// �f�B�U�S�i�����K�j
		{0 ,1 ,2 ,3 ,3 ,4 ,5 ,6 },
		{11,12,13,4 ,2 ,13,14,7 },
		{10,15,14,5 ,1 ,12,15,8 },
		{9 ,8 ,7 ,6 ,0 ,11,10,9 },
		{5 ,6 ,0 ,1 ,2 ,3 ,3 ,4 },
		{14,7 ,11,12,13,4 ,2 ,13},
		{15,8 ,10,15,14,5 ,1 ,12},
		{10,9 ,9 ,8 ,7 ,6 ,0 ,11},
	},{	// �f�B�U�T�i�Ԃԁj
		{9 ,10,7 ,0 ,5 ,8 ,9 ,7 },
		{11,13,8 ,4 ,6 ,11,13,12},
		{14,15,14,10,11,14,15,14},
		{12,13,11,11,12,12,13,11},
		{9 ,8 ,10,12,13,11,12,10},
		{5 ,4 ,9 ,14,15,14,11,8 },
		{4 ,0 ,6 ,11,13,12,10,7 },
		{8 ,5 ,7 ,5 ,9 ,10,9 ,9 },
	},{	// �f�B�U�U�i�e�^�C���j
		{3 ,11,3 ,11,3 ,11,3 ,11},
		{15,7 ,15,7 ,15,7 ,15,7 },
		{3 ,11,3 ,11,3 ,11,3 ,11},
		{15,7 ,15,7 ,15,7 ,15,7 },
		{3 ,11,3 ,11,3 ,11,3 ,11},
		{15,7 ,15,7 ,15,7 ,15,7 },
		{3 ,11,3 ,11,3 ,11,3 ,11},
		{15,7 ,15,7 ,15,7 ,15,7 },
	},{	// �f�B�U�V�i��^�C���j
		{0 ,7 ,1 ,6 ,0 ,7 ,1 ,6 },
		{10,2 ,8 ,4 ,10,2 ,8 ,4 },
		{2 ,5 ,0 ,6 ,2 ,5 ,0 ,6 },
		{8 ,4 ,9 ,3 ,8 ,4 ,9 ,3 },
		{0 ,7 ,1 ,6 ,0 ,7 ,1 ,6 },
		{10,2 ,8 ,4 ,10,2 ,8 ,4 },
		{2 ,5 ,0 ,6 ,2 ,5 ,0 ,6 },
		{8 ,4 ,9 ,3 ,8 ,4 ,9 ,3 },
	},{	// �f�B�U�W�i�n�[�t�g�[���j
		{15, 0,15, 0,15, 0,15, 0 },
		{ 0,15, 0,15, 0,15, 0,15 },
		{15, 0,15, 0,15, 0,15, 0 },
		{ 0,15, 0,15, 0,15, 0,15 },
		{15, 0,15, 0,15, 0,15, 0 },
		{ 0,15, 0,15, 0,15, 0,15 },
		{15, 0,15, 0,15, 0,15, 0 },
		{ 0,15, 0,15, 0,15, 0,15 },
	}
};

// MSX�K�� �� Win�K�� ( 0�`255 ) �ϊ��e�[�u��
const int init_convert7to255_r[]={
	0, 36, 73, 109, 146, 182, 219, 255
};

const int init_convert7to255_g[] = {
	0, 36, 73, 109, 146, 182, 219, 255
};

const int init_convert7to255_b[] = {
	0, 36, 73, 109, 146, 182, 219, 255
};

// MSX�K�� SCREEN8 �� Win�K�� ( 0�`255 ) �ϊ��e�[�u��
const int init_convert7to255_s8r[] = {
	0, 36, 73, 109, 146, 182, 219, 255
};

const int init_convert7to255_s8g[] = {
	0, 36, 73, 109, 146, 182, 219, 255
};

const int init_convert3to255_s8b[]={
	0, 73, 146, 255
};

// MSX�K�� SCREEN12 �� Win�K�� ( 0�`255 ) �ϊ��e�[�u��
const int init_convert31to255_s12r[] = {
	 0 * 255 / 31,  1 * 255 / 31,  2 * 255 / 31,  3 * 255 / 31,
	 4 * 255 / 31,  5 * 255 / 31,  6 * 255 / 31,  7 * 255 / 31,
	 8 * 255 / 31,  9 * 255 / 31, 10 * 255 / 31, 11 * 255 / 31,
	12 * 255 / 31, 13 * 255 / 31, 14 * 255 / 31, 15 * 255 / 31,
	16 * 255 / 31, 17 * 255 / 31, 18 * 255 / 31, 19 * 255 / 31,
	20 * 255 / 31, 21 * 255 / 31, 22 * 255 / 31, 23 * 255 / 31,
	24 * 255 / 31, 25 * 255 / 31, 26 * 255 / 31, 27 * 255 / 31,
	28 * 255 / 31, 29 * 255 / 31, 30 * 255 / 31, 31 * 255 / 31,
};

const int init_convert31to255_s12g[] = {
	 0 * 255 / 31,  1 * 255 / 31,  2 * 255 / 31,  3 * 255 / 31,
	 4 * 255 / 31,  5 * 255 / 31,  6 * 255 / 31,  7 * 255 / 31,
	 8 * 255 / 31,  9 * 255 / 31, 10 * 255 / 31, 11 * 255 / 31,
	12 * 255 / 31, 13 * 255 / 31, 14 * 255 / 31, 15 * 255 / 31,
	16 * 255 / 31, 17 * 255 / 31, 18 * 255 / 31, 19 * 255 / 31,
	20 * 255 / 31, 21 * 255 / 31, 22 * 255 / 31, 23 * 255 / 31,
	24 * 255 / 31, 25 * 255 / 31, 26 * 255 / 31, 27 * 255 / 31,
	28 * 255 / 31, 29 * 255 / 31, 30 * 255 / 31, 31 * 255 / 31,
};

const int init_convert31to255_s12b[] = {
	 0 * 255 / 31,  1 * 255 / 31,  2 * 255 / 31,  3 * 255 / 31,
	 4 * 255 / 31,  5 * 255 / 31,  6 * 255 / 31,  7 * 255 / 31,
	 8 * 255 / 31,  9 * 255 / 31, 10 * 255 / 31, 11 * 255 / 31,
	12 * 255 / 31, 13 * 255 / 31, 14 * 255 / 31, 15 * 255 / 31,
	16 * 255 / 31, 17 * 255 / 31, 18 * 255 / 31, 19 * 255 / 31,
	20 * 255 / 31, 21 * 255 / 31, 22 * 255 / 31, 23 * 255 / 31,
	24 * 255 / 31, 25 * 255 / 31, 26 * 255 / 31, 27 * 255 / 31,
	28 * 255 / 31, 29 * 255 / 31, 30 * 255 / 31, 31 * 255 / 31,
};

// MSX�K�� �� Win�K�� ( 0�`255 ) �ϊ��e�[�u��
int convert7to255_r[ 8 ];
int convert7to255_g[ 8 ];
int convert7to255_b[ 8 ];

// MSX�K�� SCREEN8 �� Win�K�� ( 0�`255 ) �ϊ��e�[�u��
int convert7to255_s8r[ 8 ];
int convert7to255_s8g[ 8 ];
int convert3to255_s8b[ 4 ];

// MSX�K�� SCREEN12 �� Win�K�� ( 0�`255 ) �ϊ��e�[�u��
int convert31to255_s12r[ 32 ];
int convert31to255_s12g[ 32 ];
int convert31to255_s12b[ 32 ];

//	SCREEN2/3 �ϊ��p��ƈ�
static unsigned char vram[ 0x4000 ];

// -------------------------------------------------------------
// �v���g�^�C�v
// -------------------------------------------------------------
static inline C_COLOR _get_pixel( C_COLOR *in, int width, int height, int x, int y );
static inline void _set_pixel( C_COLOR *out, int width, int height, int x, int y, C_COLOR c );
static inline int _abs( int a );
static inline void _put_dither_pattern( int *r, int *g, int *b, int screen_mode, int ErrAdd, int x, int y, C_COLOR c );
static inline int _distance( int r1, int g1, int b1, int r2, int g2, int b2 );
static inline int _distance_itp( int r1, int g1, int b1, int r2, int g2, int b2 );

static int cnvCreateHistgram( C_COLOR *in,int size,C_COLOR_TABLE **tbl,C_COLOR *pal,int pp,
							  bool FourceZero,C_COLOR FZColor );
static bool cnvCompare( C_PALETTE* Pal1, C_PALETTE* Pal2 );

static bool cnvRecolor8( C_COLOR *in,int width,int height,
						unsigned char *out,SETTING *CnvMode,PROGRESS prog,C_COLOR *pal,
						C_TILE_PATTERN *tail,int tailcnt );
static bool cnvRecolor5( C_COLOR *in,int width,int height,
						unsigned char *out,SETTING *CnvMode,PROGRESS prog,C_COLOR *pal,
						C_TILE_PATTERN *tail,int tailcnt );

static bool cnvSC5toSC2( unsigned char *out, PROGRESS prog, C_COLOR *pal );
static bool cnvSC5toSC3( unsigned char *out, PROGRESS prog );

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�œK���p���b�g����p���b�g�l�����߂�
//	2.	����
//		p_convert_table . (I)	�e�[�u��
//		n ............... (I)	�e�[�u���̃T�C�Y
//		v ............... (I)	�œK���p���b�g (0�`255)
//	3.	�Ԓl
//		�p���b�g�l
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
int convert_rgb_to_palette( const int *p_convert_table, int n, int v ){
	int minimum_diff = 999, i, diff;
	int index = 0;

	for( i = 0; i < n; i++ ){
		diff = p_convert_table[ i ] - v;
		if( diff < 0 ){
			diff = -diff;
		}
		if( diff < minimum_diff ){
			index = i;
			minimum_diff = diff;
		}
	}
	return index;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�������[�`��
//	2.	����
//		in			...	(I)	���͉摜�i�[�������̃A�h���X
//		inwidth		...	(I)	���͉摜�̕�
//		inheight	...	(I)	���͉摜�̍���
//		out			...	(I)	�o�͉摜�i�[�惁�����̃A�h���X
//		outwidth	...	(I)	�o�͉摜�̕�
//		outheight	...	(I)	�o�͉摜�̍���
//		prog		...	(I)	�o�ߕ\���֐�
//		seido		...	(I)	�؂�̂Ă�r�b�g��
//	3.	�Ԓl
//		true	...	����
//		false	...	���s
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
bool cnvCopy( C_COLOR *in ,int inwidth ,int inheight ,
			  C_COLOR *out,int outwidth,int outheight, PROGRESS prog, int seido )
{
	int	x,y,w;
	int	hh;
	C_COLOR mask;

	// ���x�����p�}�X�N�쐬
	mask = ~((1<<seido)-1);
	mask = mask & ((mask<<8) | 0xFF) & ((mask<<16) | 0xFFFF);
	// �o�͐�S�s�N�Z�����r�߂�
	w = outwidth;
	if( w>inwidth ) w=inwidth;
	if( outheight < inheight ) {
		hh = outheight; 
	} else {
		hh = inheight;
	}
	for( y= 0; y < hh; ++y ){
		if( !prog( y * 100 / outheight ) ) return false;	// �o�߂�\��
		for( x = 0; x < w; ++x ){
			out[ x ] = in[ x ] & mask;
		}
		out += outwidth;
		in  += inwidth;
	}
	return true;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�C���^�[���[�X���[�h�p�������[�`��
//	2.	����
//		in			...	(I)	���͉摜�i�[�������̃A�h���X
//		inwidth		...	(I)	���͉摜�̕�
//		inheight	...	(I)	���͉摜�̍���
//		out			...	(I)	�o�͉摜�i�[�惁�����̃A�h���X
//		outwidth	...	(I)	�o�͉摜�̕�
//		outheight	...	(I)	�o�͉摜�̍���
//		prog		...	(I)	�o�ߕ\���֐�
//		seido		...	(I)	�؂�̂Ă�r�b�g��
//	3.	�Ԓl
//		true	...	����
//		false	...	���s
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
bool cnvCopyInter( C_COLOR *in ,int inwidth ,int inheight ,
			  C_COLOR *out,int outwidth,int outheight,PROGRESS prog,int seido )
{
	int	x,y;
	int	ww, hh;
	C_COLOR c1,c2;
	C_COLOR mask;

	if( outheight < inheight ) hh = outheight; else hh = inheight;
	if( outwidth  < inwidth  ) ww = outwidth;  else ww = inwidth / 2;

	// ���x�����p�}�X�N�쐬
	mask = ~(( 1 << seido ) - 1 );
	mask = mask & (( mask << 8 ) | 0xFF ) & (( mask << 16 ) | 0xFFFF );
	
	// �o�͐�S�s�N�Z�����r�߂�
	for( y = 0; y < hh; ++y ){
		if( !prog( y * 100 / outheight ) ) return false;	// �o�߂�\��
		for( x = 0; x < ww; ++x ){	
			c1=in[ x * 2 + 0];
			c2=in[ x * 2 + 1];
			out[ x ]=GET_RGB(	( GET_RED(   c1 ) + GET_RED(   c2 ) ) / 2,
								( GET_GREEN( c1 ) + GET_GREEN( c2 ) ) / 2,
								( GET_BLUE(  c1 ) + GET_BLUE(  c2 ) ) / 2 ) & mask;
		}
		out += outwidth;
		in  += inwidth;
	}
	return true;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�m�[�}���g��k�����[�`��
//	2.	����
//		in			...	(I)	���͉摜�i�[�������̃A�h���X
//		inwidth		...	(I)	���͉摜�̕�
//		inheight	...	(I)	���͉摜�̍���
//		out			...	(I)	�o�͉摜�i�[�惁�����̃A�h���X
//		outwidth	...	(I)	�o�͉摜�������̕�
//		outheight	...	(I)	�o�͉摜�������̍���
//		wwidth		...	(I) �o�͂��镝
//		wheight		...	(I)	�o�͂��鍂��
//		prog		...	(I)	�o�ߕ\���֐�
//		seido		...	(I)	�؂�̂Ă�r�b�g��
//	3.	�Ԓl
//		true	...	����
//		false	...	���s
//	4.	���l
//		�r���o�ߕ\���֐������s����ƒ��f���Ď��s��Ԃ�
// -------------------------------------------------------------
bool cnvNormResize( C_COLOR *in , int inwidth , int inheight ,
			    C_COLOR *out, int outwidth, int outheight, int wwidth, int wheight, PROGRESS prog, int seido )
{
	int			x, y, xx, yy;
	C_COLOR	c;
	C_COLOR	mask;

	// ���x�����p�}�X�N�쐬
	mask = ~(( 1 << seido ) - 1 );
	mask = mask & (( mask << 8 ) | 0xFF ) & (( mask << 16 ) | 0xFFFF );

	// ���͂Əo�͂������T�C�Y�Ȃ炻�̂܂ܕ���
	if( (inwidth == outwidth) && (inheight == outheight) &&
		(inwidth == wwidth  ) && (inheight == wheight  ) ){
		for( x = 0; x < inwidth * inheight; ++x ){
			out[ x ] = in[ x ] & mask;
		}
		return true;
	}

	// �o�͐�S�s�N�Z�����r�߂�
	for( y = 0; y < wheight; ++y ){
		//	�o�߂�\��
		if( !prog( y * 100 / wheight ) ) return false;
		//	���͉摜�̍��W�֕ϊ�
		yy   = y * inheight / wheight;
		for( x = 0; x < wwidth; ++x ){
			// ���͉摜�̍��W�ɕϊ�
			xx  = x * inwidth / wwidth;
			c   = _get_pixel( in, inwidth, inheight, xx, yy );
			_set_pixel( out, outwidth, outheight, x, y, GET_RGB(GET_RED( c ), GET_GREEN( c ), GET_BLUE( c )) & mask );
		}	// x
	}	// y
	return true;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�A���`�G�C���A�X���g��k�����[�`��
//	2.	����
//		in			...	(I)	���͉摜�i�[�������̃A�h���X
//		inwidth		...	(I)	���͉摜�̕�
//		inheight	...	(I)	���͉摜�̍���
//		out			...	(I)	�o�͉摜�i�[�惁�����̃A�h���X
//		outwidth	...	(I)	�o�͉摜�������̕�
//		outheight	...	(I)	�o�͉摜�������̍���
//		wwidth		...	(I) �o�͂��镝
//		wheight		...	(I)	�o�͂��鍂��
//		prog		...	(I)	�o�ߕ\���֐�
//		seido		...	(I)	�؂�̂Ă�r�b�g��
//	3.	�Ԓl
//		true	...	����
//		false	...	���s
//	4.	���l
//		�r���o�ߕ\���֐������s����ƒ��f���Ď��s��Ԃ�
// -------------------------------------------------------------
bool cnvAntiResize( C_COLOR *in , int inwidth , int inheight ,
			    C_COLOR *out, int outwidth, int outheight, int wwidth, int wheight, PROGRESS prog, int seido )
{
	int			x, y, xx, yy, ix, iy, ixm1, ixm2, iym1, iym2, ixw, iyw;
	float		ax, ay, prx, pgx, pbx, pry, pgy, pby, am;
	C_COLOR	c;
	C_COLOR	mask;
	bool		bWZoom;	//	���g��
	bool		bHZoom;	//	�����g��

	ax = 0;

	// ���x�����p�}�X�N�쐬
	mask = ~(( 1 << seido ) - 1 );
	mask = mask & (( mask << 8 ) | 0xFF ) & (( mask << 16 ) | 0xFFFF );

	// ���͂Əo�͂������T�C�Y�Ȃ炻�̂܂ܕ���
	if( (inwidth == outwidth) && (inheight == outheight) &&
		(inwidth == wwidth  ) && (inheight == wheight  ) ){
		for( x = 0; x < inwidth * inheight; ++x ){
			out[ x ] = in[ x ] & mask;
		}
		return true;
	}

	// �g�傩�k����
	bWZoom = ( inwidth < wwidth );
	bHZoom = ( inheight< wheight);

	// �o�͐�S�s�N�Z�����r�߂�
	for( y = 0; y < wheight; ++y ){
		//	�o�߂�\��
		if( !prog( y * 100 / wheight ) ) return false;
		//	���͉摜�̍��W�֕ϊ�
		iym1 = int(( float( y )        ) * inheight / wheight );	//	�㑤
		iym2 = int(( float( y ) + 1.0f ) * inheight / wheight );	//	����
		iyw  = iym2 - iym1;
		yy   = y * inheight / wheight;
		for( x = 0; x < wwidth; ++x ){
			// ���͉摜�̍��W�ɕϊ�
			ixm1 = int(( float( x )        ) * inwidth / wwidth );	//	����
			ixm2 = int(( float( x ) + 1.0f ) * inwidth / wwidth );	//	�E��
			ixw  = ixm2 - ixm1;
			// �o�͐�P�s�N�Z���ɑΉ�������͉摜�̃s�N�Z���W�����ς����߂�
			ay = 0;						// �d�݂̍��v�l�i�x�������j
			pry = pgy = pby = 0;		// �s�N�Z���l���v�l�i�x�������j
			if( bHZoom ) {
				//	�����g��̏ꍇ
				if( bWZoom ) {
					//	�����g��̏ꍇ
					xx  = x * inwidth / wwidth;
					ax  = (float(x) * inwidth / wwidth ) - float(xx);	//	�ʒu xx+1 �ւ̈�v�x
					ay  = (float(y) * inheight/ wheight) - float(yy);	//	�ʒu xx+1 �ւ̈�v�x
					c   = _get_pixel( in, inwidth, inheight, xx, yy );
					prx = GET_RED( c ) * (1-ax) * (1-ay);
					pgx = GET_GREEN( c ) * (1-ax) * (1-ay);
					pbx = GET_BLUE( c ) * (1-ax) * (1-ay);
					c	= _get_pixel( in, inwidth, inheight, xx+1, yy );
					prx += GET_RED( c ) * ax * (1-ay);
					pgx += GET_GREEN( c ) * ax * (1-ay);
					pbx += GET_BLUE( c ) * ax * (1-ay);
					c   = _get_pixel( in, inwidth, inheight, xx, yy+1 );
					prx += GET_RED( c ) * (1-ax) * ay;
					pgx += GET_GREEN( c ) * (1-ax) * ay;
					pbx += GET_BLUE( c ) * (1-ax) * ay;
					c	= _get_pixel( in, inwidth, inheight, xx+1, yy+1 );
					prx += GET_RED( c ) * ax * ay;
					pgx += GET_GREEN( c ) * ax * ay;
					pbx += GET_BLUE( c ) * ax * ay;
					ax = 1.0f;
				} else {
					//	�����k���̏ꍇ
					if( ixw == 0 ){
						c	= _get_pixel( in, inwidth, inheight, ixm1, yy );
						prx = GET_RED( c ) * (1-ay);
						pgx = GET_GREEN( c ) * (1-ay);
						pbx = GET_BLUE( c ) * (1-ay);
						c	= _get_pixel( in, inwidth, inheight, ixm1, yy+1 );
						prx += GET_RED( c ) * ay;
						pgx += GET_GREEN( c ) * ay;
						pbx += GET_BLUE( c ) * ay;
						ax  = am = 1.0f;
					}else{
						ax = 0;					// �d�݂̍��v�l�i�w�������j
						prx = pgx = pbx = 0;	// �s�N�Z���l���v�l�i�w�������j
						for( ix = ixm1; ix <= ixm2; ++ix ){
							am = float( ix - ixm1 ) / ixw;
							if( am < 0 ) am = -am;
							am = 1.0f - am;
							c = _get_pixel( in, inwidth, inheight, ix, yy );
							prx += GET_RED( c ) * am * (1-ay);
							pgx += GET_GREEN( c ) * am * (1-ay);
							pbx += GET_BLUE( c ) * am * (1-ay);
							c = _get_pixel( in, inwidth, inheight, ix, yy+1 );
							prx += GET_RED( c ) * am * ay;
							pgx += GET_GREEN( c ) * am * ay;
							pbx += GET_BLUE( c ) * am * ay;
							ax  += am;
						}	// ix
					}
				}
				_set_pixel( out, outwidth, outheight, x, y,
						GET_RGB( int( prx/ax ), int( pgx/ax ), int( pbx/ax ) ) & mask );
			} else {
				//	�����k���̏ꍇ
				for( iy = iym1; iy <= iym2; ++iy ){
					if( bWZoom ) {
						//	�����g��̏ꍇ
						xx  = x * inwidth / wwidth;
						ax  = (float(x) * inwidth / wwidth) - float(xx);	//	�ʒu xx+1 �ւ̈�v�x
						c   = _get_pixel( in, inwidth, inheight, xx, iy );
						prx = GET_RED( c ) * (1-ax);
						pgx = GET_GREEN( c ) * (1-ax);
						pbx = GET_BLUE( c ) * (1-ax);
						c = _get_pixel( in, inwidth, inheight, xx+1, iy );
						prx += GET_RED( c ) * ax;
						pgx += GET_GREEN( c ) * ax;
						pbx += GET_BLUE( c ) * ax;
						ax = 1.0f;
					} else {
						//	�����k���̏ꍇ
						if( ixw == 0 ){
							c = _get_pixel( in, inwidth, inheight, ixm1, iy );
							prx = GET_RED( c );
							pgx = GET_GREEN( c );
							pbx = GET_BLUE( c );
							ax  = am = 1.0f;
						}else{
							ax = 0;					// �d�݂̍��v�l�i�w�������j
							prx = pgx = pbx = 0;	// �s�N�Z���l���v�l�i�w�������j
							for( ix = ixm1; ix <= ixm2; ++ix ){
								am = float( ix - ixm1 ) / ixw;
								if( am < 0 ) am = -am;
								am = 1.0f - am;
								c = _get_pixel( in, inwidth, inheight, ix, iy );
								prx += GET_RED( c ) * am;
								pgx += GET_GREEN( c ) * am;
								pbx += GET_BLUE( c ) * am;
								ax  += am;
							}	// ix
						}
					}

					if( iyw == 0 ){
						am = 1.0f;
					}else{
						am = float( iy - iym1 ) / iyw;
						if( am < 0 ) am = -am;
						am = 1.0f - am;
					}
					pry += prx * am;
					pgy += pgx * am;
					pby += pbx * am;
					ay  += am;
				}	// iy
				ay *= ax;
				_set_pixel( out, outwidth, outheight, x, y,
						GET_RGB( int( pry/ay ), int( pgy/ay ), int( pby/ay ) ) & mask );
			}
		}	// x
	}	// y
	return true;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�f�B�U����
//	2.	����
//		r		...	�Ԃ̏o�͐�
//		g		...	�΂̏o�͐�
//		b		...	�̏o�͐�
//		mode	...	�f�B�U�̎��
//		ErrAdd	...	�f�B�U�̓K�p���@
//		x		...	�w���W
//		y		...	�x���W
//		c		...	�F
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static inline void _put_dither_pattern( int *r, int *g, int *b, int screen_mode, int ErrAdd, int x, int y, C_COLOR c )
{
	if( screen_mode >= EALGO_DITH1 ){
		switch( ErrAdd ){
		case EADD_ROTATE:
			*r = GET_RED(   c ) + dith[ screen_mode - EALGO_DITH1 ][       x & 7  ][ y & 7 ];
			*g = GET_GREEN( c ) + dith[ screen_mode - EALGO_DITH1 ][       y & 7  ][ x & 7 ];
			*b = GET_BLUE(  c ) + dith[ screen_mode - EALGO_DITH1 ][ 7 - ( x & 7 )][ y & 7 ];
			break;
		case EADD_NONE:
			*r = GET_RED(   c ) + dith[ screen_mode - EALGO_DITH1 ][ y & 7 ][ x & 7 ];
			*g = GET_GREEN( c ) + dith[ screen_mode - EALGO_DITH1 ][ y & 7 ][ x & 7 ];
			*b = GET_BLUE(  c ) + dith[ screen_mode - EALGO_DITH1 ][ y & 7 ][ x & 7 ];
			break;
		case EADD_X:
			*r = GET_RED(   c ) + dith[ screen_mode - EALGO_DITH1 ][ y & 7 ][ ( x + 0 ) & 7 ];
			*g = GET_GREEN( c ) + dith[ screen_mode - EALGO_DITH1 ][ y & 7 ][ ( x + 1 ) & 7 ];
			*b = GET_BLUE(  c ) + dith[ screen_mode - EALGO_DITH1 ][ y & 7 ][ ( x + 2 ) & 7 ];
			break;
		case EADD_Y:
			*r = GET_RED(   c ) + dith[ screen_mode - EALGO_DITH1 ][ ( y + 0 ) & 7 ][ x & 7 ];
			*g = GET_GREEN( c ) + dith[ screen_mode - EALGO_DITH1 ][ ( y + 1 ) & 7 ][ x & 7 ];
			*b = GET_BLUE(  c ) + dith[ screen_mode - EALGO_DITH1 ][ ( y + 2 ) & 7 ][ x & 7 ];
			break;
		}
	}else{
		*r = GET_RED(   c );
		*g = GET_GREEN( c );
		*b = GET_BLUE(  c );
	}
}

// -------------------------------------------------------------
//	���F����

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		���F���[�`���isc2(4)/3/5/6/7/8�p�j
//	2.	����
//		in		...	�摜���̓������i���e�͔j�󂳂�܂��j
//		width	...	�摜��
//		height	...	�摜����
//		out		...	�摜�o�̓�����
//		CnvMode	...	���݂̐ݒ�
//		prog	...	�o�ߕ\���֐�
//		pal		...	�p���b�g
//	3.	�Ԓl
//		false	...	���s
//		true	...	����
//	4.	���l
//		���̂Ƃ��냁�����s���ȊO�Ŏ��s���邱�Ƃ͂Ȃ�	
// -------------------------------------------------------------
bool cnvRecolor( C_COLOR *in,int width,int height,
					unsigned char *out,SETTING *CnvMode,PROGRESS prog,C_COLOR *pal,
					C_TILE_PATTERN *tail,int tailcnt )
{
	bool	bRet;

	if( CnvMode->screen_mode == MD_SC8 || CnvMode->screen_mode == MD_SC8_256L ) {
		//	256�F�r�b�g�}�b�v�֕ϊ�����
		return cnvRecolor8( in, width, height, out, CnvMode, prog, pal, tail, tailcnt );
	} else {
		//	16�F�r�b�g�}�b�v�֕ϊ�����
		bRet = cnvRecolor5( in, width, height, out, CnvMode, prog, pal, tail, tailcnt );

		//	����ȉ�ʁiSC2/3/4) �ɂ��킹�ĕϊ�����		
		switch( CnvMode->screen_mode ) {
		case MD_SC2:
			bRet = bRet && cnvSC5toSC2( out, prog, pal );
			break;
		case MD_SC3:
			bRet = bRet && cnvSC5toSC3( out, prog );
			break;
		}
	}
	return bRet;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		���F���[�`���isc8�p�j
//	2.	����
//		in		...	�摜���̓������i���e�͔j�󂳂�܂��j
//		width	...	�摜��
//		height	...	�摜����
//		out		...	�摜�o�̓�����
//		CnvMode	...	���݂̐ݒ�
//		prog	...	�o�ߕ\���֐�
//		pal		...	�p���b�g
//	3.	�Ԓl
//		false	...	���s
//		true	...	����
//	4.	���l
//		�������Ƃ𐔃J���ɏ����Ă���͍̂������̂��߁B
// -------------------------------------------------------------
static bool cnvRecolor8( C_COLOR *in,int width,int height,
					unsigned char *out,SETTING *CnvMode,PROGRESS p_progress_cbr,C_COLOR *pal,
					C_TILE_PATTERN *tail,int tailcnt ) {
	int				x, y, ptr, buffer_size;
	int				cr, cg, cb;		// ����f�̂q�f�a
	int				er, eg, eb;		// �덷�i�����֊ۂ߂�j
	int				n;
	C_COLOR c,cc,*pin;
	C_COLOR fource_zero_color = -1;
	signed short *p_diffusion_error[2] = { nullptr, nullptr };
	signed short *p_x_diffusion_error, *p_y_diffusion_error;
	int diffusion_error_coef = 0;
	int x_diffusion_error_coef = 0;
	int y_diffusion_error_coef = 0;
	int ee = CnvMode->err;
	C_COLOR bit_depth_mask;
	bool ret = false;
	int convert14to255_s8r[ 15 ] = { 0 };
	int convert14to255_s8g[ 15 ] = { 0 };
	int convert6to255_s8b[ 7 ] = { 0 };

	// �덷�g�U�W��
	if( CnvMode->diffusion_error_enable ) {
		diffusion_error_coef = int( CnvMode->diffusion_error_coef * 1024 );
		x_diffusion_error_coef = int( CnvMode->diffusion_error_x_weight * 256. );
		y_diffusion_error_coef = 256 - x_diffusion_error_coef;
	}

	// �덷�~�ϗp�o�b�t�@�̍쐬
	buffer_size = ( width + 5 ) * 4 * sizeof( signed short );
	for( x = 0; x < 2; x++ ) {
		p_diffusion_error[ x ] = (signed short*) malloc( buffer_size );
		if( p_diffusion_error[ x ]==NULL ) goto l_exit;
		memset( p_diffusion_error[ x ], 0, buffer_size );
	}

	// 2�{�T�C�Y�ϊ��e�[�u���쐬
	for( x = 0; x < 15; x++ ) {
		if( (x & 1) == 0 ) {
			convert14to255_s8r[ x ] = convert7to255_s8r[ x >> 1 ];
			convert14to255_s8g[ x ] = convert7to255_s8g[ x >> 1 ];
		}
		else {
			convert14to255_s8r[ x ] = ( convert7to255_s8r[ x >> 1 ] + convert7to255_s8r[ ( x >> 1 ) + 1 ] ) >> 1;
			convert14to255_s8g[ x ] = ( convert7to255_s8g[ x >> 1 ] + convert7to255_s8g[ ( x >> 1 ) + 1 ] ) >> 1;
		}
		if( x < 7 ) {
			if( ( x & 1 ) == 0 ){
				convert6to255_s8b[ x ] = convert3to255_s8b[ x >> 1 ];
			}
			else{
				convert6to255_s8b[ x ] = ( convert3to255_s8b[ x >> 1 ] + convert3to255_s8b[ ( x >> 1 ) + 1 ] ) >> 1;
			}
		}
	}

	// ���x�����p�}�X�N�쐬
	bit_depth_mask = ( ~(( 1 << CnvMode->Seido ) - 1 ) ) & 0xFF;	//	8bit mask
	bit_depth_mask = ( bit_depth_mask << 16 ) | ( bit_depth_mask << 8 ) | bit_depth_mask;

	// ���� 0�� �̐F
	if( CnvMode->FourceZero ) {
		fource_zero_color = CnvMode->FZColor & bit_depth_mask;
	}

	// �S�s�N�Z�����r�߂�
	ptr = 0;
	pin = in;

	for( y = 0; y < height; ++y ){
		if( !p_progress_cbr( y * 100 / height ) ) goto l_exit;
		p_x_diffusion_error = p_diffusion_error[ y & 1 ];
		p_y_diffusion_error = p_diffusion_error[ 1 - ( y & 1 ) ];

		for( x = 0; x < width; ++x ){
			cc = c = *pin;
			pin++;
			
			// �f�B�U����
			_put_dither_pattern( &cr, &cg, &cb, CnvMode->ErrAlgo, CnvMode->ErrAdd, x, y, c );
			
			if( diffusion_error_coef ) {
				// �덷���v�Z�ɓ����
				cr += p_x_diffusion_error[ x * 4 + 0 ];
				cg += p_x_diffusion_error[ x * 4 + 1 ];
				cb += p_x_diffusion_error[ x * 4 + 2 ];
			}

			// �ł��߂��F��������
			if( CnvMode->Tile ) {
				//	�^�C�����[�h
				er = convert_rgb_to_palette( convert14to255_s8r, 15, cr );
				eg = convert_rgb_to_palette( convert14to255_s8g, 15, cg );
				eb = convert_rgb_to_palette( convert6to255_s8b ,  7, cb );
				if( ( x ^ y ) & 1 ){
					er = ( er + 1 ) >> 1;
					eg = ( eg + 1 ) >> 1;
					eb = ( eb + 1 ) >> 1;
					er = range_limiter( er, 0, 7 );
					eg = range_limiter( eg, 0, 7 );
					eb = range_limiter( eb, 0, 3 );
				}
				else {
					er >>= 1;
					eg >>= 1;
					eb >>= 1;
				}
			}
			else {
				//	��^�C�����[�h
				er = convert_rgb_to_palette( convert7to255_s8r, 8, cr );
				eg = convert_rgb_to_palette( convert7to255_s8g, 8, cg );
				eb = convert_rgb_to_palette( convert3to255_s8b, 4, cb );
			}
			if( er == 0 && eg == 0 && eb == 0 && CnvMode->NonZero ) {
				er = 0;
				eg = 1;
				eb = 0;
			}
			n = ( er << 2 ) | ( eg << 5 ) | eb;
			cc = GET_RGB( convert7to255_s8r[ er ], convert7to255_s8g[ eg ], convert3to255_s8b[ eb ] );

			if( diffusion_error_coef ) {
				// �덷�����͂̃s�N�Z���֊g�U������
				er = range_limiter( ( cr - GET_RED( cc )) * diffusion_error_coef / 1024 ,-32768, 32767 );
				eg = range_limiter( ( cg - GET_GREEN( cc )) * diffusion_error_coef / 1024 ,-32768, 32767 );
				eb = range_limiter( ( cb - GET_BLUE( cc )) * diffusion_error_coef / 1024 ,-32768, 32767 );
				// ���ׂȌ덷�͏��ł�����
				if( _abs( er ) < (signed)CnvMode->err ) er = 0;
				if( _abs( eg ) < (signed)CnvMode->err ) eg = 0;
				if( _abs( eb ) < (signed)CnvMode->err ) eb = 0;
				// �E�Ɋg�U
				p_x_diffusion_error[ ( x + 1 ) * 4 + 0 ] += (signed short)( ( er * x_diffusion_error_coef ) >> 8 );
				p_x_diffusion_error[ ( x + 1 ) * 4 + 1 ] += (signed short)( ( eg * x_diffusion_error_coef ) >> 8 );
				p_x_diffusion_error[ ( x + 1 ) * 4 + 2 ] += (signed short)( ( eb * x_diffusion_error_coef ) >> 8 );
				// ���Ɋg�U
				p_y_diffusion_error[ x * 4 + 0 ] = (signed short)( ( er * y_diffusion_error_coef ) >> 8 );
				p_y_diffusion_error[ x * 4 + 1 ] = (signed short)( ( eg * y_diffusion_error_coef ) >> 8 );
				p_y_diffusion_error[ x * 4 + 2 ] = (signed short)( ( eb * y_diffusion_error_coef ) >> 8 );
			}

			// ���ʂ��o�͂���
			if( CnvMode->FourceZero && cc == fource_zero_color ) n = 0;	// �����[����
			out[ ptr ] = n;
			// ���̏o�͐�
			++ptr;
		}	// x
	}	// y
	ret = true;
l_exit:
	if( p_diffusion_error[0] != nullptr ) free( p_diffusion_error[0] );
	if( p_diffusion_error[1] != nullptr ) free( p_diffusion_error[1] );
	return ret;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		���F���[�`���isc2(4)/3/5/6/7�p�j
//	2.	����
//		in		...	�摜���̓������i���e�͔j�󂳂�܂��j
//		width	...	�摜��
//		height	...	�摜����
//		out		...	�摜�o�̓�����
//		CnvMode	...	���݂̐ݒ�
//		prog	...	�o�ߕ\���֐�
//		pal		...	�p���b�g
//	3.	�Ԓl
//		false	...	���s
//		true	...	����
//	4.	���l
//		���̂Ƃ��냁�����s���ȊO�Ŏ��s���邱�Ƃ͂Ȃ�	
// -------------------------------------------------------------
static bool cnvRecolor5( C_COLOR *in,int width,int height,
					unsigned char *out,SETTING *CnvMode,PROGRESS prog,C_COLOR *pal,
					C_TILE_PATTERN *tail,int tailcnt ) {
	int x, y, z, d, ptr;
	int cr, cg, cb;		// ����f�̂q�f�a
	int pr, pg, pb;		// ��f�l
	int er, eg, eb;		// �덷�i�����֊ۂ߂�j
	int r, n, nr;
	C_COLOR c, cc, *pin;
	C_COLOR fource_zero_color = -1;
	signed short *p_diffusion_error[2] = { nullptr, nullptr };
	signed short *p_x_diffusion_error, *p_y_diffusion_error;
	int palnum;
	int diffusion_error_coef = 0;
	int x_diffusion_error_coef = 0;
	int y_diffusion_error_coef = 0;
	int ee = CnvMode->err;
	C_COLOR bit_depth_mask;
	bool ret = false;
	int color_num;

	// �덷�g�U�W��
	if( CnvMode->diffusion_error_enable ){
		diffusion_error_coef = int( CnvMode->diffusion_error_coef * 1024 );
		x_diffusion_error_coef = int( CnvMode->diffusion_error_x_weight * 256. );
		y_diffusion_error_coef = 256 - x_diffusion_error_coef;
	}

	// �덷�~�ϗp�o�b�t�@�̍쐬
	memset( p_diffusion_error, 0, sizeof( p_diffusion_error ) );
	for( x = 0; x < 2; ++x ) {
		p_diffusion_error[ x ] = (signed short*)LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT, ( width + 5 ) * 4 * sizeof( signed short ) );
		if( p_diffusion_error[ x ]==NULL ) goto l_exit;
	}

	// ���x�����p�}�X�N�쐬
	bit_depth_mask = ~(( 1 << CnvMode->Seido ) - 1 );
	bit_depth_mask = bit_depth_mask & (( bit_depth_mask << 8 ) | 0xFF ) & (( bit_depth_mask << 16 ) | 0xFFFF );

	// �p���b�g��
	switch( CnvMode->screen_mode ){
	case MD_SC6:
	case MD_SC6_256L:
		palnum = 4;		break;
	default:
		palnum = 16;	break;
	}
	fource_zero_color = CnvMode->FourceZero ? CnvMode->FZColor & bit_depth_mask : -1;

	if( CnvMode->Tile ) {
		color_num = tailcnt;
	}
	else {
		color_num = palnum;
	}

	// �S�s�N�Z�����r�߂�
	ptr = 0;
	pin = in;
	n = 0;

	for( y = 0; y < height; ++y ){
		if( !prog( y * 100 / height ) ) goto l_exit;
		p_x_diffusion_error = p_diffusion_error[ y & 1 ];
		p_y_diffusion_error = p_diffusion_error[ 1 - ( y & 1 ) ];

		for( x = 0; x < width; ++x ){
			cc = c = *pin;
			pin++;
			
			// �f�B�U����
			_put_dither_pattern( &cr, &cg, &cb, CnvMode->ErrAlgo, CnvMode->ErrAdd, x, y, c );
			
			if( diffusion_error_coef ) {
				// �덷���v�Z�ɓ����
				cr += p_x_diffusion_error[ x * 4 + 0 ];
				cg += p_x_diffusion_error[ x * 4 + 1 ];
				cb += p_x_diffusion_error[ x * 4 + 2 ];
			}

			// �ł��߂��F��������
			nr = 0x7FFFFFFFL;

			if( CnvMode->NonZero ) {
				z = 1; 
			}
			else {
				z = 0;
			}
			// ��r����F�͎��ۂɕ\���ł���F�Ŗ�����΂Ȃ�Ȃ����߁A�͈͊O���N���b�v����
			pr = range_limiter( cr, 0, 255 );
			pg = range_limiter( cg, 0, 255 );
			pb = range_limiter( cb, 0, 255 );

			for( ; z < color_num; ++z ){
				// �߂����H
				if( CnvMode->Tile ) {
					// �^�C�����[�h
					c = tail[ z ].c;
				}
				else {
					// ��^�C�����[�h
					if( CnvMode->PalEn[ z ] == PALEN_NONE ) {
						//	���̃p���b�g�͎g�p�֎~
						continue;
					}
					c = pal[ z ];
				}
				r = _distance_itp( pr, pg, pb, GET_RED(c), GET_GREEN(c), GET_BLUE(c) );
				if( r < nr ){
					nr = r;
					if( CnvMode->Tile ) {
						// �^�C�����[�h
						n  = tail[z].p[ ( x ^ y ) & 1 ];
					}
					else {
						// ��^�C�����[�h
						n  = z;
					}
					cc = c;
					if( r == 0 ) break;	// ���S��v�Ȃ甲����
				}
			}	// z

			if( diffusion_error_coef ) {
				// �덷�����͂̃s�N�Z���֊g�U������
				er = range_limiter( ( cr - GET_RED(   cc )) * diffusion_error_coef / 1024 ,-32768, 32767 );
				eg = range_limiter( ( cg - GET_GREEN( cc )) * diffusion_error_coef / 1024 ,-32768, 32767 );
				eb = range_limiter( ( cb - GET_BLUE(  cc )) * diffusion_error_coef / 1024 ,-32768, 32767 );
				// ���ׂȌ덷�͏��ł�����
				if( _abs( er ) < (signed)CnvMode->err ) er = 0;
				if( _abs( eg ) < (signed)CnvMode->err ) eg = 0;
				if( _abs( eb ) < (signed)CnvMode->err ) eb = 0;
				// �E�Ɋg�U
				p_x_diffusion_error[ ( x + 1 ) * 4 + 0 ] += (signed short)( ( er * x_diffusion_error_coef ) >> 8 );
				p_x_diffusion_error[ ( x + 1 ) * 4 + 1 ] += (signed short)( ( eg * x_diffusion_error_coef ) >> 8 );
				p_x_diffusion_error[ ( x + 1 ) * 4 + 2 ] += (signed short)( ( eb * x_diffusion_error_coef ) >> 8 );
				// ���Ɋg�U
				p_y_diffusion_error[ x * 4 + 0 ] = (signed short)( ( er * y_diffusion_error_coef ) >> 8 );
				p_y_diffusion_error[ x * 4 + 1 ] = (signed short)( ( eg * y_diffusion_error_coef ) >> 8 );
				p_y_diffusion_error[ x * 4 + 2 ] = (signed short)( ( eb * y_diffusion_error_coef ) >> 8 );
			}

			// ���ʂ��o�͂���
			if( CnvMode->FourceZero && cc == fource_zero_color ) n = 0;	// �����[����
			if( palnum == 4 ) {
				// 4�F
				if( x & 3 ){
					d=out[ ptr >> 2 ];
					d= d | ( n & 0x03 ) << ( ( 3 - ( x & 3 ) ) * 2 );
				}else{
					d= n << 6;
				}
				out[ ptr >> 2 ] = d;
			} else {
				// 16�F
				if( x & 1 ){
					d=out[ptr>>1];
					d=( d & 0xF0 ) | n;
				}else{
					d= n<<4;
				}
				out[ptr>>1]=d;
			}
			// ���̏o�͐�
			++ptr;
		}	// x
	}	// y
	ret = true;
l_exit:
	if( p_diffusion_error[0] != NULL ) LocalFree( p_diffusion_error[0] );
	if( p_diffusion_error[1] != NULL ) LocalFree( p_diffusion_error[1] );
	return ret;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�J���[�p���b�g�̖��邳���擾����
//	2.	����
//		p_pal	...	�p���b�g�ɑΉ�����F
//		c ......... �p���b�g�ԍ�
//	3.	�Ԓl
//		�P�x���x�� 0-65535
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static int _get_palette_luminance( C_COLOR *p_pal, int c ) {
	int r, g, b;

	r = GET_RED( p_pal[ c ] );
	g = GET_GREEN( p_pal[ c ] );
	b = GET_BLUE( p_pal[ c ] );
	return (77 * r + 150 * g + 29 * b);
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		SCREEN5 �C���[�W�� SCREEN2/4 �C���[�W�֕ϊ�����
//	2.	����
//		p_image	...	VRAM�C���[�W
//		prog	...	�o�ߕ\���֐�
//		p_pal	...	�p���b�g�ɑΉ�����F
//	3.	�Ԓl
//		true	...	����
//		false	...	���s
//	4.	���l
//		�C���^�[���[�X���[�h�͎g���Ȃ�
// -------------------------------------------------------------
static bool cnvSC5toSC2( unsigned char *p_image, PROGRESS prog, C_COLOR *p_pal )
{
	int	x, y, i, j, adr, dadr;
	int	cc, c, p, lum1, lum2;
	int	cnt, idx1, idx2, len, l;
	int	ccnt[ 8 ] = { 0 };
	int	ccol[ 8 ] = { 0 };
	C_COLOR	c1, c2;

	memset( vram, 0, sizeof( vram ) );

	adr  = 0;
	for( y = 0; y < 192; y++ ) {
		for( x = 0; x < 32; x++ ) {
			//	�W�h�b�g�g�̒��Ŏg���Ă���F�𒲂ׂ�F cnt = �F��
			cnt  = 0;
			idx1 = 0;							//	�ł������F�̃C���f�b�N�X
			for( j = 0; j < 8; j++ ) {			//	8�h�b�g�Ȃ߂�
				cc = p_image[ adr + j / 2 ];
				if( j & 1 ) {
					cc = cc & 0x0F;
				} else {
					cc = (cc >> 4) & 0x0F;
				}
				for( i = 0; i < cnt; ++i ) {	//	�����F������
					if( cc == ccol[ i ] ) {
						ccnt[ i ]++;			//		�������F�̐��𑝉�
						if( ccnt[ i ] > ccnt[ idx1 ] ) {	//	�ł�����
							idx1 = i;
						}
						break;
					}
				}
				if( i == cnt ) {				//	�V�����F
					ccol[ cnt ] = cc;			//		�������F
					ccnt[ cnt ] = 1;			//		�������F�̐�
					cnt++;
				}
			}

			if( cnt < 2 ) {
				//	�P�F�����Ȃ��ꍇ
				c = ccol[ 0 ];					//	�J���[�e�[�u���Ɋi�[����l
				p = 0x00;						//	�p�^�[���W�F�l���[�^�e�[�u���Ɋi�[����l
			} else {
				//	�Q�F�ȏ�̏ꍇ

				//	�P�F�ڂƍł��قȂ�Q�F�ڂ�I������i�R���g���X�g�d���j
				len  = 0;
				c1= p_pal[ ccol[ idx1 ] ];
				for( i = 0; i < cnt; ++i ) {
					if( i == idx1 ) continue;
					cc = ccol[i];
					l = _distance_itp( GET_RED( p_pal[cc] ), GET_GREEN( p_pal[cc] ), GET_BLUE( p_pal[cc] ), GET_RED( c1 ), GET_GREEN( c1 ), GET_BLUE( c1 ));
					if( l >= len ) {
						len = l;
						idx2 = i;
					}
				}

				c = ( ccol[ idx1 ] << 4 ) | ccol[ idx2 ];
				p = 0;
				c2= p_pal[ ccol[ idx2 ] ];
				for( j = 0; j < 8; j++ ) {
					cc = p_image[ adr + j / 2 ];
					if( j & 1 ) {
						cc = cc & 0x0F;
					} else {
						cc = (cc >> 4) & 0x0F;
					}
					//	�I�΂ꂽ�Q�F�Ƃ̐F�������r
					p = p << 1;
					if( _distance_itp( GET_RED( p_pal[cc] ), GET_GREEN( p_pal[cc] ), GET_BLUE( p_pal[cc] ), GET_RED( c1 ), GET_GREEN( c1 ), GET_BLUE( c1 )) 
					  < _distance_itp( GET_RED( p_pal[cc] ), GET_GREEN( p_pal[cc] ), GET_BLUE( p_pal[cc] ), GET_RED( c2 ), GET_GREEN( c2 ), GET_BLUE( c2 )) ) {
						p = p | 1;
					}
				}

			}

			//	���邢�F�̕��� �p�^�[���W�F�l���[�^��bit�l = 1 �̕��Ɋ��蓖�Ă�
			if( cnt >= 2 ) {
				lum1 = _get_palette_luminance( p_pal, c & 0x0F );
				lum2 = _get_palette_luminance( p_pal, ( c >> 4 ) & 0x0F );
				if( ( lum1 == lum2 && (c & 0x0F) > ((c >> 4) & 0x0F) ) || (lum1 > lum2) ) {
					p = p ^ 255;
					c = ((c << 4) & 0xF0) | ((c >> 4) & 0x0F);
				}
			}
			else {
				//	1�F���������ꍇ�ł��A���邢�F�Ȃ�O�i�F (�p�^�[���W�F�l���[�^�� bit�l = 1) �ɕύX����
				lum1 = _get_palette_luminance( p_pal, c & 0x0F );
				if( lum1 >= (128 << 8) ) {
					p = p ^ 255;
					c = ( ( c << 4 ) & 0xF0 ) | ( ( c >> 4 ) & 0x0F );
				}
			}

			//	�i�[
			dadr = ( y & 7 ) + ( x * 8 ) + ( y >> 3 )*256;
			vram[ dadr			  ] = p;
			vram[ dadr + SC2COLOR ] = c;
			//	8�h�b�g�ړ�
			adr += 4;
		}
	}

	memcpy( p_image, vram, sizeof( vram ) );
	return true;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		SCREEN5 �C���[�W�� SCREEN3 �C���[�W�֕ϊ�����
//	2.	����
//		out		...	VRAM�C���[�W
//		prog	...	�o�ߕ\���֐�
//	3.	�Ԓl
//		true	...	����
//		false	...	���s
//	4.	���l
//		�C���^�[���[�X���[�h�͎g���Ȃ�
// -------------------------------------------------------------
static bool cnvSC5toSC3( unsigned char *out, PROGRESS prog )
{
	int	x, y, yy, adr;

	memset( vram, 0, sizeof( vram ) );

	adr = 0;
	for( y = 0; y < 6; ++y ) {
		for( yy = 0; yy < 8; ++yy ) {
			for( x = 0; x < 32; ++x ) {
				vram[ x*8 + yy + y*256 ] = out[ adr ];
				++adr;
			}
		}
	}

	memcpy( out, vram, sizeof( vram ) );
	return true;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		���F���[�`���i���R���p�j
//	2.	����
//		in		...	�摜���̓�����
//		width	...	�摜�� ( 256 )
//		height	...	�摜���� ( 212 )
//		out		...	�摜�o�̓�����
//		CnvMode	...	���݂̐ݒ�
//		prog	...	�o�ߕ\���֐�
//	3.	�Ԓl
//		false	...	���s
//		true	...	����
//	4.	���l
//		���̂Ƃ��냁�����s���ȊO�Ŏ��s���邱�Ƃ͂Ȃ�
// -------------------------------------------------------------
bool cnvNtcolor( C_COLOR *in ,int width ,int height ,unsigned char *out ,
				 SETTING *CnvMode ,PROGRESS prog )
{
	int				x, y, z, xx;
	int				convert31to255_s12y[ 32 ];
	C_COLOR		c;
	int				r[ 5 ] = { 0 }, g[ 5 ] = { 0 }, b[ 5 ] = { 0 };
	int				vy, min, tyy, txx;
	int				rr, gg, bb, er, eg, eb;
	int				jj, kk, yy[ 4 ] = { 0 }, ii[ 4 ] = { 0 }, s;
	int				mask;
	unsigned char	*pout = out;
	short			*ptmp,*tmp = NULL;
	int				*errbuf[ 2 ], *errbuf0, *errbuf1;

	int		k		= CnvMode->diffusion_error_enable ? int( CnvMode->diffusion_error_coef * 1024 ) : 0;
	int		kx		= int( CnvMode->diffusion_error_x_weight * 256. );
	int		ky		= 256 - kx;
	int		algo	= CnvMode->AlgoMode;
	bool	rc		= CnvMode->JKrc;
	int		ealgo	= CnvMode->ErrAlgo;
	bool	sc10	= ( CnvMode->screen_mode == MD_SC10 || CnvMode->screen_mode == MD_SC10_256L );
	int		zero	= CnvMode->FourceZero ? (sc10 ? 0x10 : 0x08) : 0;
	bool	ret		= false;

	tyy = 0;

	for( y = 0; y < 32; y++ ){
		convert31to255_s12y[ y ] = (convert31to255_s12r[ y ] * 4 + convert31to255_s12g[ y ] * 8  + convert31to255_s12b[ y ] * 2 + 7) / 14;
		convert31to255_s12y[ y ] = range_limiter( convert31to255_s12y[ y ], 0, 255 );
	}
	// ��ʃ��[�h�ɍ��킹���}�X�N��ݒ�
	if( sc10 ){
		mask=0xF0;
	}else{
		mask=0xF8;
	}
	// �덷�~�ϗp�ޯ̧�̍쐬
	memset( errbuf, 0, sizeof( errbuf ) );
	for( x = 0; x < 2; ++x ) {
		errbuf[ x ] = (signed int*)LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT, ( width + 5 ) * 3 * sizeof( int ) );
		if( errbuf[ x ]==NULL ) goto l_exit;
	}
	// �덷�g�U�̏ꍇ�́A�������� �x�����ׂĐ������Ă���g�U����
	if( k ){
		ptmp = tmp = (short*)LocalAlloc( LMEM_FIXED, sizeof(short) * width * ( height + 1 ) );
		if( tmp == NULL ) goto l_exit;
	}
	for( y = 0; y < height; ++y ){
		if( !prog( y * 100 / height ) ) goto l_exit;	// �o�߂�\��

		errbuf0 = errbuf[ y & 1 ];
		errbuf1 = errbuf[ 1 - ( y & 1 ) ];
		for( x = xx = 0; x < width; x += 4, xx += 12 ){
			//	�W���I�ȕϊ��ɂ��e��f�� J,K �̒l���m�肷��
			jj = kk = 0;
			txx = xx;
			for( z = 0; z < 4; ++z ){
				c = _get_pixel( in, width, height, x + z, y );
				r[ z ] = convert31to255_s12r[ convert_rgb_to_palette( convert31to255_s12r, 32, GET_RED( c ) + errbuf0[ txx + 0 ] ) ];
				g[ z ] = convert31to255_s12g[ convert_rgb_to_palette( convert31to255_s12g, 32, GET_GREEN( c ) + errbuf0[ txx + 1 ] ) ];
				b[ z ] = convert31to255_s12b[ convert_rgb_to_palette( convert31to255_s12b, 32, GET_BLUE( c ) + errbuf0[ txx + 2 ] ) ];
				vy   = range_limiter( b[ z ] / 2 + r[ z ] / 4 + g[ z ] / 8, 0, 255 );
				jj += r[ z ] - vy;
				kk += g[ z ] - vy;
				txx += 3;
			}
			// J,K �̒l�𕽋ς��ĂS��f�y�A�� J,K �l���m�肷��
			jj = range_limiter( jj / 4, -256, 255 );
			kk = range_limiter( kk / 4, -256, 255 );

			// Y �̎Z�o�Ɣ͈͊O�؂��
			txx = xx;
			for( z = 0; z < 4; ++z ){
				// �A���S���Y���̓K�p
				switch( algo ){
				case ALGO_NONE:		// ����
					tyy = (b[ z ] + ( r[ z ] + g[ z ] / 2 ) / 2 ) / 2;
					break;
				case ALGO_RECALC:	// �P�x�Čv�Z�ɂ��덷�����A���S���Y���i�掿�E�P�x�Č��D��j
					tyy = 0;
					min = 0x7FFFFFFF;	//	�\���傫�Ȓl�i���̈Ӗ��j
					rr = range_limiter( r[z], 0, 255 );
					gg = range_limiter( g[z], 0, 255 );
					bb = range_limiter( b[z], 0, 255 );
					for( vy = 0; vy < 255; ++vy ){
						s = _distance_itp( rr, gg, bb,
							range_limiter( vy + jj, 0, 255 ),  
							range_limiter( vy + kk, 0, 255 ), 
							range_limiter( vy * 5 / 4 - jj / 2 - kk / 4, 0, 255 ) );
						if( min <= s ) break;	//	�덷���L����͂��߂��炤���؂�
						min = s; tyy = vy;
					}
					break;
				case ALGO_POWERFUL:		// �p���t�����p�@�̌v�Z��
					ii[ z ]= b[ z ] + r[ z ] * 2 + g[ z ] * 4;
					tyy = ( ( ii[ z ] * 4 ) - 6 * jj - 15 * kk ) / 29;
					break;
				case ALGO_NTSC2:		// ��������
					tyy = ( r[ z ] * 27 + g[ z ] * 28 + b[ z ] * 50 ) / 115;
					break;
				case ALGO_RECALC2:	// �P�x�Čv�Z�ɂ��덷�����A���S���Y���i�F�����Č��D��j
					tyy = 0;
					min = 0x7FFFFFFF;	//	�\���傫�Ȓl�i���̈Ӗ��j
					rr = range_limiter( r[z], 0, 255 );
					gg = range_limiter( g[z], 0, 255 );
					bb = range_limiter( b[z], 0, 255 );
					for( vy = 0; vy < 255; ++vy ){
						s = _distance_itp( rr, gg, bb,
							range_limiter( vy + jj, 0, 255 ),  
							range_limiter( vy + kk, 0, 255 ), 
							range_limiter( vy * 5 / 4 - jj / 2 - kk / 4, 0, 255 ) );
						if( min <= s ) break;	//	�덷���L����͂��߂��炤���؂�
						min = s; tyy = vy * 61 / 64;
					}
					break;
				}	// switch
				// �f�B�U�p�^�[��
				if( ealgo >= EALGO_DITH1 ){			// �f�B�U�����O
					tyy += dith[ ealgo-EALGO_DITH1 ][ y & 7 ][ ( x + z ) & 7 ] / 2;
				}
				// �͈͊O���O
				yy[ z ] = range_limiter( tyy, 0, 255 );
				if( k ){	// �덷�g�U
					// �Z�o���� ��Y,J,K ���� R,G,B �l�i MSX�ŕ\�������F �j���v�Z
					rr = convert31to255_s12r[ convert_rgb_to_palette( convert31to255_s12r, 32, range_limiter( jj + yy[ z ], 0, 255 ) ) ];
					gg = convert31to255_s12g[ convert_rgb_to_palette( convert31to255_s12g, 32, range_limiter( kk + yy[ z ], 0, 255 ) ) ];
					bb = convert31to255_s12b[ convert_rgb_to_palette( convert31to255_s12b, 32, range_limiter( 5 * yy[ z ] / 4 - jj / 2 - kk / 4, 0, 255 ) ) ];
					// �덷�����͂̃s�N�Z���֊g�U������
					er = range_limiter( ( r[ z ] - rr ) * k / 1024 , -32768, 32767 );
					eg = range_limiter( ( g[ z ] - gg ) * k / 1024 , -32768, 32767 );
					eb = range_limiter( ( b[ z ] - bb ) * k / 1024 , -32768, 32767 );

					// ���ׂȌ덷�͏��ł�����
					if( _abs( er ) >= (signed)CnvMode->err ) {
						errbuf0[ txx + 3 ] += (er * kx) >> 8;
						errbuf1[ txx + 0 ] = (er * ky) >> 8;
					} else {
						errbuf1[ txx + 0 ] = 0;
					}
					if( _abs( eg ) >= (signed)CnvMode->err ) {
						errbuf0[ txx + 4 ] += (eg * kx) >> 8;
						errbuf1[ txx + 1 ] = (eg * ky) >> 8;
					} else {
						errbuf1[ txx + 1 ] = 0;
					}
					if( _abs( eb ) >= (signed)CnvMode->err ) {
						errbuf0[ txx + 5 ] += (eb * kx) >> 8;
						errbuf1[ txx + 2 ] = (eb * ky) >> 8;
					} else {
						errbuf1[ txx + 2 ] = 0;
					}
					txx += 3;
				}
			}	// for z

			// �F�����̍Čv�Z
			if( rc ){
				jj = kk = 0;
				for( z = 0; z < 4; ++z ){
					jj += r[ z ] - yy[ z ];
					kk += g[ z ] - yy[ z ];
				}
				jj = ( jj + 2 ) / 4;	// �l�̌ܓ��a����
				kk = ( kk + 2 ) / 4;	// �l�̌ܓ��a����
			}
			// J,K �͈͊O�؂��
			jj = range_limiter( jj * 31 / 255, -32, 31 );	// �����_�ȉ��؂�̂�
			kk = range_limiter( kk * 31 / 255, -32, 31 );	// �����_�ȉ��؂�̂�
			
			// �o��
			if( k ){			// �x�l�덷�g�U
				*(ptmp++)=yy[0];
				*(ptmp++)=yy[1];
				*(ptmp++)=yy[2];
				*(ptmp++)=yy[3];
				*(out++ )= kk     & 7;
				*(out++ )=(kk>>3) & 7;
				*(out++ )= jj     & 7;
				*(out++ )=(jj>>3) & 7;
			}else{
				*out=( kk     & 7) | ( range_limiter( yy[0], 0, 255 ) & mask );
				if( *out==0 ) *out= zero;
				++out;
				*out=((kk>>3) & 7) | ( range_limiter( yy[1], 0, 255 ) & mask );
				if( *out==0 ) *out= zero;
				++out;
				*out=( jj     & 7) | ( range_limiter( yy[2], 0, 255 ) & mask );
				if( *out==0 ) *out= zero;
				++out;
				*out=((jj>>3) & 7) | ( range_limiter( yy[3], 0, 255 ) & mask );
				if( *out==0 ) *out= zero;
				++out;
			}
		}	// for x
	}	// for y
	if( !k ) goto l_done;

	// �덷�g�U
	ptmp=tmp;
	for( y = 0; y < height; ++y ){
		if( !prog( y * 100 / height ) ) goto l_exit;	// �o�߂�\��
		for( x = 0; x < width; ++x ){
			// �덷�̎Z�o
			vy = ((*ptmp) >> 3 ) * 255 / 31;					//	�̗p�������x�l
			z  = range_limiter( vy, 0, 255 ) & mask;				//	���ۂɍ̗p����x�l
			jj = ( vy - convert31to255_s12y[ convert_rgb_to_palette( convert31to255_s12y, 32, z ) ] ) * k / 1024;			//	�덷
			if( jj >= (signed)CnvMode->err ) {
				// �덷�̊g�U
				if( x != width - 1 ) {
					*(ptmp + 1) = (short)range_limiter( (int)*(ptmp +     1) + jj, -32768, 32767 );
				}
				*(ptmp + width) = (short)range_limiter( (int)*(ptmp + width) + jj, -32768, 32767 );
			}
			++ptmp;
			// �f�[�^�̏o��
			*pout |= z;
			if( *pout == 0 ) *pout = zero;
			++pout;
		}
	}
l_done:
	ret = true;

l_exit:
	if( tmp		  != NULL ) LocalFree( tmp );
	if( errbuf[0] != NULL ) LocalFree( errbuf[0] );
	if( errbuf[1] != NULL ) LocalFree( errbuf[1] );
	return ret;
}

// -------------------------------------------------------------
// �\�[�g�p�F�e�[�u����r�֐�(�~���\�[�g�p)
//
// �����F	tbl1		�F�P�A�h���X
//			tbl2		�F�Q�A�h���X
// �Ԓl�F	tbl1<tbl2	����
//			tbl1=tbl2	0
//			tbl1>tbl2	����
// -------------------------------------------------------------
int cnvColorTblCompare( const void *tbl1, const void *tbl2 ){
	//	�q�X�g�O�����̃J�E���g�l�ő召��r����
	return( (long)( ( (C_COLOR_TABLE *)tbl2 )->n ) - (long)( ( (C_COLOR_TABLE *)tbl1 )->n ) );
}

// -------------------------------------------------------------
// ���F�p�p���b�g�i�Q�F�`�P�U�F�j�����߂�
//
// �����F	in			�摜�i���e�̓\�[�g����܂��j
//			width		��
//			height		����
//			pal			�p���b�g�i16�j�̃A�h���X
//			mode		0 �ʏ� / 1 �����z�I��
//			cnt			�g�p����p���b�g�̐�
//			pp			����ς݃p���b�g�̐�
//			FourceZero	�����[�����̐F������ true / �Ȃ� false
//			FZColor		�����[�����̐F
// �Ԓl�F	���� true / ���s false
// -------------------------------------------------------------
bool cnvGetPalette( C_COLOR *in,int width,int height,C_COLOR *pal,int screen_mode,int cnt,int fixed_color_num,
				    bool FourceZero,C_COLOR FZColor )
{
	C_COLOR	c;
	C_COLOR_TABLE *p_color_table, swap_buffer;
	int			n, base_color, compare_color, compare_count_distance;
	int			r, g, b, compare_color_distance, delete_color_distance, delete_base_color, delete_color, delete_count_distance;

	// �Q�`�P�U�F���������ł��Ȃ�
	if( cnt < 2 || cnt > 16 || cnt <= fixed_color_num ) return false;
	
	// �q�X�g�O�����̍쐬
	n = cnvCreateHistgram( in, width * height, &p_color_table, pal, fixed_color_num, FourceZero, FZColor );
	if( p_color_table == NULL ) {
		return false;
	}
	memset( pal + fixed_color_num, 0, sizeof( C_COLOR ) * ( cnt - fixed_color_num ) );	

	// �F���\�[�g���� (�Œ�F�́A�q�X�g�J�E���g�����Ȃ̂ŁA��ʂɏW�܂�)
	qsort( p_color_table, n, sizeof( C_COLOR_TABLE ), cnvColorTblCompare );

	if( screen_mode == 0 ){
		// �ړI�̐F���ɂȂ�܂Ŏ����F���폜��������
		while( n > cnt ){
			delete_base_color = -1;					// ��ƂȂ�F�̃C���f�b�N�X
			delete_color = -1;						// �폜�ΏۂƂ��đI�������F�̃C���f�b�N�X
			delete_color_distance = 0x7FFFFFFF;		// �폜�ΏۂƂ��đI�������F�̋���
			delete_count_distance = 0;				// �q�X�g�J�E���g�̍�
			for( base_color = 0; base_color < (n - 1); base_color++ ) {
				// ���ڐF
				c = p_color_table[ base_color ].c;
				r = GET_RED(   c );
				g = GET_GREEN( c );
				b = GET_BLUE(  c );
				for( compare_color = base_color + 1; compare_color < n; compare_color++ ){	// ���ڐF(base_color)�ɍł��߂��Ďg�p�������Ȃ��F(y)����������
					if( compare_color < fixed_color_num ) {
						//	0 �` pp-1 �́A�ύX���Ă͂����Ȃ��Œ�F�Ȃ̂őΏۊO
						continue;
					}
					c = p_color_table[ compare_color ].c;
					compare_color_distance = _distance_itp( r, g, b, GET_RED( c ), GET_GREEN( c ), GET_BLUE( c ) );
					compare_count_distance = (p_color_table[ base_color ].n - p_color_table[ compare_color ].n);
					if( delete_count_distance > compare_count_distance ) {
						//	�O��̃q�b�g���� �q�X�g�O�����̃J�E���g���̍��������� �̂őΏۊO
						//continue;
					}
					if( delete_color_distance < compare_color_distance ) {
						//	�O��̃q�b�g���� ���ĂȂ� �̂őΏۊO
						continue;
					}
					delete_base_color = base_color;
					delete_color = compare_color;
					delete_color_distance = compare_color_distance;
					delete_count_distance = compare_count_distance;
				}
			}
			if( delete_color > 0 ){			// �������Č��������F�𒅖ڐF�ɋz������
				if( delete_base_color >= fixed_color_num ) {
					p_color_table[ delete_base_color ].n += p_color_table[ delete_color ].n;
				}
				if( delete_color < (n - 1) ) {
					//	�폜�Ώۂ��Ō��1�łȂ���΁A�폜���ċl�߂�
					memcpy( &p_color_table[ delete_color ], &p_color_table[ delete_color + 1 ], sizeof( C_COLOR_TABLE ) * ( n - 1 - delete_color ) );
				}
				n--;
				//	�������ɂȂ�悤�Ƀ\�[�g������
				for( compare_color = delete_color; compare_color > 0; compare_color-- ) {
					if( p_color_table[ compare_color ].n <= p_color_table[ compare_color - 1 ].n ) {
						break;
					}
					swap_buffer                        = p_color_table[ compare_color     ];
					p_color_table[ compare_color     ] = p_color_table[ compare_color - 1 ];
					p_color_table[ compare_color - 1 ] = swap_buffer;
				}
			}
		}
	} else {
		//	�����z�I�F
	}

	// �e�[�u�����̃p���b�g�擪 cnt ���̗p����
	for( base_color = 0; base_color < n && base_color < cnt; ++base_color ){
		pal[ base_color ] = p_color_table[ base_color ].c;
	}

	// ���������������
	LocalFree( p_color_table );
	return true;
}

// -------------------------------------------------------------
// ���F�p�p���b�g�i256�F�j�����߂�
//
// �����F	pal			�p���b�g�i16�j�̃A�h���X
// �Ԓl�F	���� true / ���s false
// ���l�F	SCREEN8 �F�ԍ��̃r�b�g�}�b�v
//			(MSB) [G][G][G][R][R][R][B][B] (LSB)
// -------------------------------------------------------------
bool cnvGetPaletteS8( C_COLOR *pal ){
	int			i, r, g, b;

	// �q�X�g�O�����̍쐬
	ZeroMemory( pal, sizeof( C_COLOR ) * 256 );

	for( i = 0; i < 256; ++i ){
		// ���ڐF�i�I��F���g�p�̐F�j���擾
		r = convert7to255_r[ ( i >> 2 ) & 7 ];
		g = convert7to255_g[ ( i >> 5 ) & 7 ];
		b = convert7to255_b[ i & 3 ];
		pal[ i ] = GET_RGB( r, g, b );
	}
	return true;
}

// -------------------------------------------------------------
// �q�X�g�O�������쐬����
//
// �����F	in			�摜������
//			size		�摜�������̃T�C�Y
//			tbl			�J���[�e�[�u�����i�[����|�C���^�̃A�h���X
//			Pal			�������Œǉ�����p���b�g
//			pp			Pal �̐�
//			FourceZero	���O����F������ true / �Ȃ� false
//			FZColor		���O����F
// �Ԓl�F	tbl �̃T�C�Y
// -------------------------------------------------------------
static int cnvCreateHistgram( C_COLOR *in, int size, C_COLOR_TABLE **tbl, C_COLOR *pal, int pp,
		bool FourceZero, C_COLOR FZColor ) {

	C_COLOR_TABLE hash[ 512 ];
	int i, n, using_color_num, t, r, g, b;
	C_COLOR	c;

	// NULL �N���A
	*tbl = NULL;
	using_color_num = 0;		// �F�̎g�p��

	memset( hash, 0, sizeof( hash ) );
	for( i = 0; i < 512; i++ ) {
		r = convert7to255_r[ (i >> 6) & 7 ];
		g = convert7to255_g[ (i >> 3) & 7 ];
		b = convert7to255_b[ (i >> 0) & 7 ];
		hash[ i ].c = GET_RGB( r, g, b );
	}

	// ��f�̐��𐔂���
	for( i = 0; i < size; i++ ) {
		c = in[ i ];
		if( c == FZColor && FourceZero ) continue;
		r = convert_rgb_to_palette( convert7to255_r, 8, GET_RED(   c ) );
		g = convert_rgb_to_palette( convert7to255_g, 8, GET_GREEN( c ) );
		b = convert_rgb_to_palette( convert7to255_b, 8, GET_BLUE(  c ) );
		n = r * 64 + g * 8 + b;		//	�n�b�V���֐�
		hash[ n ].n++;
		if( hash[ n ].n == 1 ) {
			using_color_num++;
		}
	}	// for

	// �J���[�e�[�u�����m�ۂ���
	*tbl=(C_COLOR_TABLE*) LocalAlloc( LMEM_FIXED, sizeof( C_COLOR_TABLE ) * ( using_color_num + pp ) );
	if( *tbl == NULL ) return 0;

	// �n�b�V���\���� tbl �֕ϊ�
	t=0;
	for( i = 0; i < pp ; ++i ){
		//	�m���ɍ̗p����F�́A�q�X�g�O�����̃J�E���g�l�� �� (0x7FFFFFFF) �ɂ��ē���Ă���
		(*tbl)[ t ].c = pal[ i ];
		(*tbl)[ t ].n = 0x7FFFFFFF;
		//	hash �̒��̓���F���̗p���Ȃ����߂ɁA�q�X�g�O�����̃J�E���g�l�� 0 �ɂ��Ă���
		r = convert_rgb_to_palette( convert7to255_r, 8, GET_RED(   pal[ i ] ) );
		g = convert_rgb_to_palette( convert7to255_g, 8, GET_GREEN( pal[ i ] ) );
		b = convert_rgb_to_palette( convert7to255_b, 8, GET_BLUE(  pal[ i ] ) );
		n = r * 64 + g * 8 + b;		//	�n�b�V���֐�
		hash[ n ].n = 0;
		//	����
		t++;
	}
	for( i = 0; i < 512; ++i ){
		if( hash[ i ].n == 0 ) {
			continue;
		}
		(*tbl)[ t ] = hash[ i ];
		t++;
	}
	return t;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�^�C���p�^�[�������߂�(SC5/SC7��p)
//	2.	����
//		pal		...	(I)	����ς݂̃p���b�g
//		palen	...	(I)	�g�p�\�p���b�g
//		zeroen	...	(I)	�[�����g�p�\���ۂ�
//		tail	...	(I)	�^�C���p�^�[���i�[��
//		mode	...	(I)	�X�N���[�����[�h
//	3.	�Ԓl
//		�쐬�����^�C���p�^�[���̐�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
int cnvCreateTail4( C_PALETTE *pal,uint8_t *palen,bool zeroen,C_TILE_PATTERN *tail, int screen_mode )
{
	int i, j, k, l;
	int r,g,b;
	int rr,gg,bb;
	int n=0,m;
	bool ok;
	int	pnum;
	// ���ׂĂ̐F�̑g�ݍ��킹����������
	l = zeroen ? 0 : 1;
	pnum = ( screen_mode == MD_SC6 || screen_mode == MD_SC6_256L ) ? 4 : 16;
	for( i = l; i < pnum; ++i ){
		if( palen[ i ] == 1 ) continue;
		r = pal[ i ].red;
		g = pal[ i ].green;
		b = pal[ i ].blue;
		for( j = l; j < pnum; ++j ){
			if( palen[ j ] == 1 ) continue;
			rr = pal[ j ].red;
			gg = pal[ j ].green;
			bb = pal[ j ].blue;
			ok=true;
			m=n;
			if( i != j ){
				for( k = 0; k < n; ++k ){
					if( ( r + rr ) / 2 != ( pal[ tail[ k ].p[ 0 ] ].red   + pal[ tail[ k ].p[ 1 ] ].red   ) / 2 ||
						( g + gg ) / 2 != ( pal[ tail[ k ].p[ 0 ] ].green + pal[ tail[ k ].p[ 1 ] ].green ) / 2 ||
						( b + bb ) / 2 != ( pal[ tail[ k ].p[ 0 ] ].blue  + pal[ tail[ k ].p[ 1 ] ].blue  ) / 2 ) continue;
					if( _distance_itp( r, g, b, rr, gg, bb ) <
						_distance_itp( pal[ tail[ k ].p[ 0 ] ].red,pal[ tail[ k ].p[ 0 ] ].green,pal[ tail[ k ].p[ 0 ] ].blue,
								  pal[ tail[ k ].p[ 1 ] ].red,pal[ tail[ k ].p[ 1 ] ].green,pal[ tail[ k ].p[ 1 ] ].blue ) ){
						m = k;
					}else{
						m = -1;
						break;
					}
				}
			}
			if( m != -1 ){
				tail[ m ].c = GET_RGB( ( convert7to255_r[ r ] + convert7to255_r[ rr ] ) / 2, ( convert7to255_g[ g ] + convert7to255_g[ gg ] ) / 2, ( convert7to255_b[ b ] + convert7to255_b[ bb ] ) / 2 );
				tail[ m ].p[ 0 ] = i;
				tail[ m ].p[ 1 ] = j;
				if( m == n ) ++n;
			}
		}
	}
	return n;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�p���b�g�̑召��r
//	2.	����
//		Pal1	...	(I)	�p���b�g�P�̃A�h���X
//		Pal2	...	(I)	�p���b�g�Q�̃A�h���X
//	3.	�Ԓl
//		true	...	Pal2 �̕���������
//		false	...	Pal2 �̕����傫��
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static bool cnvCompare( C_PALETTE* Pal1, C_PALETTE* Pal2 )
{
	int	i;
	int	mask;

	mask = 4;
	for( i = 0; i < 3; ++i ) {
		if( ( Pal1->red   & mask ) > ( Pal2->red   & mask ) ) return true;
		if( ( Pal1->red   & mask ) < ( Pal2->red   & mask ) ) return false;
		if( ( Pal1->green & mask ) > ( Pal2->green & mask ) ) return true;
		if( ( Pal1->green & mask ) < ( Pal2->green & mask ) ) return false;
		if( ( Pal1->blue  & mask ) > ( Pal2->blue  & mask ) ) return true;
		if( ( Pal1->blue  & mask ) < ( Pal2->blue  & mask ) ) return false;
		mask = mask >> 1;
	}
	//	����
	return false;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�p���b�g�̃\�[�g
//	2.	����
//		Mode	...	(I)	�ݒ���e
//		Pal		...	(I)	�p���b�g�̒l
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
void cnvSortPalette( SETTING* Mode, C_COLOR* Pal )
{
	C_PALETTE			color_palette[16] = { 0 }, tBakCol;	// SC5/SC7 �ɂ�����l�r�w���p���b�g�w��
	uint8_t		PalEn[16] = { 0 }, tBakEn;
	C_COLOR	pal[16] = { 0 }, tBakPal;
	int			cnt, z, x, pnum;

	//	�p���b�g���R�s�[
	cnt  = 0;
	pnum = ( Mode->screen_mode == MD_SC6 || Mode->screen_mode == MD_SC6_256L ) ? 4 : 16;
	for( z = 0; z < pnum; ++z ) {
		if( z == 0 && Mode->NonZero ) continue;
		switch( Mode->SortMode ) {
		case SM_INCAUTO:
		case SM_DECAUTO:
			if( Mode->PalEn[ z ] != PALEN_AUTO ) continue;
			color_palette[ cnt ]   = Mode->color_palette[ z ];
			PalEn[ cnt ] = Mode->PalEn[ z ];
			pal[ cnt ]   = Pal[ z ];
			++cnt;
			break;
		case SM_INC:
		case SM_DEC:
			color_palette[ cnt ]   = Mode->color_palette[ z ];
			PalEn[ cnt ] = Mode->PalEn[ z ];
			pal[ cnt ]   = Pal[ z ];
			++cnt;
			break;
		}
	}
	if( cnt <= 1 ) return;

	//	�\�[�g
	switch( Mode->SortMode ) {
	case SM_INCAUTO:
	case SM_INC:
		for( z = cnt - 1; z >= 0; --z ) {
			for( x = 0; x < z; ++x ) {
				if( cnvCompare( &color_palette[ x ], &color_palette[ x + 1 ] ) ) {
					tBakCol        = color_palette[ x ];
					color_palette[ x ]       = color_palette[ x + 1 ];
					color_palette[ x + 1 ]   = tBakCol;
					tBakEn         = PalEn[ x ];
					PalEn[ x ]     = PalEn[ x + 1 ];
					PalEn[ x + 1 ] = tBakEn;
					tBakPal        = pal[ x ];
					pal[ x ]       = pal[ x + 1 ];
					pal[ x + 1 ]   = tBakPal;
				}
			}
		}
		break;
	case SM_DECAUTO:
	case SM_DEC:
		for( z = cnt - 1; z >= 0; --z ) {
			for( x = 0; x < z; ++x ) {
				if( !cnvCompare( &color_palette[ x ], &color_palette[ x + 1 ] ) ) {
					tBakCol        = color_palette[ x ];
					color_palette[ x ]       = color_palette[ x + 1 ];
					color_palette[ x + 1 ]   = tBakCol;
					tBakEn         = PalEn[ x ];
					PalEn[ x ]     = PalEn[ x + 1 ];
					PalEn[ x + 1 ] = tBakEn;
					tBakPal        = pal[ x ];
					pal[ x ]       = pal[ x + 1 ];
					pal[ x + 1 ]   = tBakPal;
				}
			}
		}
		break;
	}

	//	�i�[
	cnt = 0;
	for( z = 0; z < pnum; ++z ) {
		if( z == 0 && Mode->NonZero ) continue;
		switch( Mode->SortMode ) {
		case SM_INCAUTO:
		case SM_DECAUTO:
			if( Mode->PalEn[ z ] != PALEN_AUTO ) continue;
			Mode->color_palette[ z ]   = color_palette[ cnt ];
			Mode->PalEn[ z ] = PalEn[ cnt ];
			Pal[ z ]         = pal[ cnt ];
			++cnt;
			break;
		case SM_INC:
		case SM_DEC:
			Mode->color_palette[ z ]   = color_palette[ cnt ];
			Mode->PalEn[ z ] = PalEn[ cnt ];
			Pal[ z ]         = pal[ cnt ];
			++cnt;
			break;
		}
	}
}

// -------------------------------------------------------------
//	���[�e�B���e�B

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�s�N�Z���l�擾
//	2.	����
//		in		...	(I)	�摜������
//		width	...	(I)	��
//		height	...	(I)	����
//		x		...	(I)	���W
//		y		...	(I)	���W
//	3.	�Ԓl
//		�F
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static inline C_COLOR _get_pixel( C_COLOR *in,int width,int height,int x,int y )
{
	// �͈͊O�C��
	x = range_limiter( x,0,width-1  );
	y = range_limiter( y,0,height-1 );
	// �w��̈ʒu�̐F��Ԃ�
	return( *( in + x + y*width ) );
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�s�N�Z���z�u
//	2.	����
//		out		...	(I)	�摜������
//		width	...	(I)	��
//		height	...	(I)	����
//		x		...	(I)	���W
//		y		...	(I)	���W
//		c		...	(I)	�F
// 3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static inline void _set_pixel( C_COLOR *out,int width,int height,int x,int y,C_COLOR c )
{
	// �͈͊O�C��
	x = range_limiter( x,0,width-1  );
	y = range_limiter( y,0,height-1 );
	// �w��̈ʒu�֐F���i�[
	*( out + x + y*width )=c;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		��Βl�����߂�
//	2.	����
//		a	...	(I)	�Ώ�
//	3.	�Ԓl
//		a �̐�Βl
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static inline int _abs( int a )
{
	return( ( a < 0 ) ? -a : a );
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�Q�F�̋��������߂�i�����j
//	2.	����
//		r1	...	(I)	�F�P��
//		g1	...	(I)	�F�P��
//		b1	...	(I)	�F�P��
//		r2	...	(I)	�F�Q��
//		g2	...	(I)	�F�Q��
//		b2	...	(I)	�F�Q��
//	3.	�Ԓl
//		����
//	4.	���l
//		�h�����h��������r�p�̒l�ł���A�X�P�[���ɈӖ��͂Ȃ�
// -------------------------------------------------------------
static inline int _distance( int r1, int g1, int b1, int r2, int g2, int b2 )
{
	int r, g, b;
	r = r2 - r1;
	g = g2 - g1;
	b = b2 - b1;
	return( r*r + g*g + b*b );
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		RGB �� ICtCp�ϊ�
//	2.	����
//		p_i		...	(O)	���x
//		p_ct	...	(O)	��
//		p_cp	...	(O)	�ԗ�
//		r		...	(I)	�� (0-255)
//		g		...	(I)	�� (0-255)
//		b		...	(I)	�� (0-255)
//	3.	�Ԓl
//		����
//	4.	���l
//		�h�����h��������r�p�̒l�ł���A�X�P�[���ɈӖ��͂Ȃ�
// -------------------------------------------------------------
static void _rgb_to_itp( double *p_i, double *p_ct, double *p_cp, int r, int g, int b ){
	double l, m, s;

	l = ( 1688. * r + 2146. * g +  262. * b ) / ( 4096. * 255. );
	m = (  683. * r + 2951. * g +  462. * b ) / ( 4096. * 255. );
	s = (   99. * r +  309. * g + 3688. * b ) / ( 4096. * 255. );

	l = 0.5 * sqrt( l );
	m = 0.5 * sqrt( m );
	s = 0.5 * sqrt( s );

	*p_i  = ( 2048. * l + 2048. * m             ) / 4096.;
	*p_ct = ( 3625. * l - 7465. * m + 3840. * s ) / 4096.;
	*p_cp = ( 9500. * l - 9212. * m -  288. * s ) / 4096.;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�Q�F�̋��������߂�i�����j ICtCp (HLG) Version
//	2.	����
//		r1	...	(I)	�F�P��
//		g1	...	(I)	�F�P��
//		b1	...	(I)	�F�P��
//		r2	...	(I)	�F�Q��
//		g2	...	(I)	�F�Q��
//		b2	...	(I)	�F�Q��
//	3.	�Ԓl
//		����
//	4.	���l
//		�h�����h��������r�p�̒l�ł���A�X�P�[���ɈӖ��͂Ȃ�
// -------------------------------------------------------------
static inline int _distance_itp( int r1, int g1, int b1, int r2, int g2, int b2 ){
	double i1, t1, p1, i2, t2, p2, i, t, p;

	_rgb_to_itp( &i1, &t1, &p1, r1, g1, b1 );
	_rgb_to_itp( &i2, &t2, &p2, r2, g2, b2 );
	i = i1 - i2;
	t = t1 - t2;
	p = p1 - p2;
	return( (int)(sqrt(i * i + t * t + p * p) * 65536.) );
}

// -------------------------------------------------------------
//	�v���r���[����

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�͈͊O���l�̕␳
//	2.	����
//		n	...	(I)	���l
//		min	...	(I)	�ŏ��l
//		max	...	(I)	�ő�l
//	3.	�Ԓl
//		�␳��� n
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
int range_limiter( int n, int min, int max )
{
	if( n < min ) return min;
	if( n > max ) return max;
	return n;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		BSAVE �`���ŉ摜��ۑ�����
//	2.	����
//		bmp		...	(I)	�摜�f�[�^�̃A�h���X
//		width	...	(I)	��
//		height	...	(I)	����
//		inter	...	(I)	�C���^�[���[�X
//		paltbl	...	(I)	�p���b�g�e�[�u���̃A�h���X( 0 �Ȃ�t�����Ȃ� )
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
bool cnvSaveBmpFile( const char *szInFileName, LPBYTE bmp, int width, int height, SETTING *Mode )
{
	HDC					hDC   = NULL, hSDC;
	BITMAPINFOHEADER	bi;
	BITMAPFILEHEADER	bf;
	int					nSize;
	int					w;
	FU_FILE				*hFile = FU_INVALID_HANDLE;
	HBITMAP				hBMP  = NULL;
	char				*pMem = NULL;
	bool				bRet;

	bRet = false;
	//	ͯ�ނ̍쐬
	memset( &bf, 0, sizeof( bf ) );
	memset( &bi, 0, sizeof( bi ) );

	w		= ( width * 3 + 3 ) & 0xFFFC;
	nSize	= w * height;

	bf.bfType			= 'MB';
	bf.bfOffBits		= sizeof( bf ) + sizeof( bi );
	bf.bfSize			= sizeof( bf ) + sizeof( bi ) + w * height;

	bi.biBitCount		= 24;
	bi.biCompression	= BI_RGB;
	bi.biHeight			= height;
	bi.biPlanes			= 1;
	bi.biSize			= sizeof( bi );
	bi.biSizeImage		= nSize;
	bi.biWidth			= width;
	bi.biXPelsPerMeter	= 1000;
	bi.biYPelsPerMeter	= 1000;

	//	���z��ʂ��쐬����
	hSDC = GetDC( NULL );
	hDC  = CreateCompatibleDC( hSDC );
	ReleaseDC( NULL, hSDC );
	if( hDC == NULL ) goto exit;
	hBMP = CreateDIBSection( hDC, (BITMAPINFO*)&bi, DIB_RGB_COLORS, (void**)&pMem, NULL, NULL );
	if( hBMP == NULL || pMem == NULL ) goto exit;
	SelectObject( hDC, hBMP );
	GdiFlush();

	draw_screen( bmp, hDC, Mode );
	GdiFlush();

	//	�t�@�C���o��
	hFile = fu_open( szInFileName, "wb" );
	if( hFile == FU_INVALID_HANDLE ) goto exit;

	fu_write( hFile, &bf, sizeof( bf ) );
	fu_write( hFile, &bi, sizeof( bi ) );
	fu_write( hFile, pMem, nSize       );

	bRet = true;
exit:
	//	���\�[�X�̉��
	if( hFile!= FU_INVALID_HANDLE ) fu_close( hFile );
	if( hDC  != NULL ) DeleteDC( hDC );
	if( hBMP != NULL ) DeleteObject( hBMP );

	return bRet;
}
