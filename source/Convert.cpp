// -------------------------------------------------------------
// BMP to MSX
// �ϊ�����
//										(C)2000/5/6 HRA!
// -------------------------------------------------------------

#include "Convert.h"
#include "math.h"
#include "fileuty.h"

// -------------------------------------------------------------
// �^
// -------------------------------------------------------------
typedef struct {
	COLORREF			c;		// �F
	int					n;		// ���̐�
} COLORTBL;

// -------------------------------------------------------------
// �ݒ�
// -------------------------------------------------------------
const int dith[][8][8]={	// �f�B�U�����O�p�^�[��
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
		{8 ,0 ,8 ,8 ,0 ,8 ,0 ,8 },
		{0 ,8 ,0 ,0 ,8 ,0 ,8 ,0 },
		{8 ,0 ,8 ,8 ,0 ,8 ,0 ,8 },
		{0 ,8 ,0 ,0 ,8 ,0 ,8 ,0 },
		{8 ,0 ,8 ,8 ,0 ,8 ,0 ,8 },
		{0 ,8 ,0 ,0 ,8 ,0 ,8 ,0 },
		{8 ,0 ,8 ,8 ,0 ,8 ,0 ,8 },
		{0 ,8 ,0 ,0 ,8 ,0 ,8 ,0 },
	}
};

// MSX�K���i 0�`7 ) �� Win�K�� ( 0�`255 ) �ϊ��e�[�u��
const int cc[]={
	0, 36, 73, 109, 146, 182, 219, 255
};

// MSX�K���i 0�`3 ) SCREEN8�� �� Win�K�� ( 0�`255 ) �ϊ��e�[�u��
const int cb[]={
/*	0, 85, 170, 255	*/
	0, 73, 146, 255
};

//	SCREEN2 �̃J���[�e�[�u���A�h���X
#define	SC2COLOR	0x2000

//	SCREEN2/3 �ϊ��p��ƈ�
static unsigned char vram[ 0x4000 ];

// -------------------------------------------------------------
// �v���g�^�C�v
// -------------------------------------------------------------
static inline COLORREF GetPix( COLORREF *in,int width,int height,int x,int y );
static inline void SetPix( COLORREF *out,int width,int height,int x,int y,COLORREF c );
static inline int Abs( int a );
static inline int AdjustNum( int n,int min,int max );
static inline void PutDither( int *r,int *g,int *b,int mode,int ErrAdd,int x,int y,COLORREF c );
static inline int Interval( int r1,int g1,int b1,int r2,int g2,int b2 );

static int cnvCreateHistgram( COLORREF *in,int size,COLORTBL **tbl,COLORREF *pal,int pp,
							  bool FourceZero,COLORREF FZColor );
static bool cnvCompare( PAL* Pal1, PAL* Pal2 );

static bool cnvRecolor8( COLORREF *in,int width,int height,
						unsigned char *out,SETTING *CnvMode,PROGRESS prog,COLORREF *pal,
						TAILPAT *tail,int tailcnt );
static bool cnvRecolor5( COLORREF *in,int width,int height,
						unsigned char *out,SETTING *CnvMode,PROGRESS prog,COLORREF *pal,
						TAILPAT *tail,int tailcnt );

static bool cnvSC5toSC2( unsigned char *out, PROGRESS prog, COLORREF *pal );
static bool cnvSC5toSC3( unsigned char *out, PROGRESS prog );

static void DrawScreen2( const unsigned char *bmp, HDC hDC, const SETTING *Mode );
static void DrawScreen3( const unsigned char *bmp, HDC hDC, const SETTING *Mode );
static void DrawScreen5( const unsigned char *bmp, HDC hDC, const SETTING *Mode );
static void DrawScreen6( const unsigned char *bmp, HDC hDC, const SETTING *Mode );
static void DrawScreen7( const unsigned char *bmp, HDC hDC, const SETTING *Mode );
static void DrawScreen8( const unsigned char *bmp, HDC hDC, const SETTING *Mode );
static void DrawScreenC( const unsigned char *bmp, HDC hDC, const SETTING *Mode );

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
bool cnvCopy( COLORREF *in ,int inwidth ,int inheight ,
			  COLORREF *out,int outwidth,int outheight, PROGRESS prog, int seido )
{
	int	x,y,w;
	int	hh;
	COLORREF mask;

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
bool cnvCopyInter( COLORREF *in ,int inwidth ,int inheight ,
			  COLORREF *out,int outwidth,int outheight,PROGRESS prog,int seido )
{
	int	x,y;
	int	ww, hh;
	COLORREF c1,c2;
	COLORREF mask;

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
			out[ x ]=RGB(	( GetRValue( c1 ) + GetRValue( c2 ) ) / 2,
							( GetGValue( c1 ) + GetGValue( c2 ) ) / 2,
							( GetBValue( c1 ) + GetBValue( c2 ) ) / 2 ) & mask;
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
bool cnvNormResize( COLORREF *in , int inwidth , int inheight ,
			    COLORREF *out, int outwidth, int outheight, int wwidth, int wheight, PROGRESS prog, int seido )
{
	int			x, y, xx, yy;
	COLORREF	c;
	COLORREF	mask;

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
			c   = GetPix( in, inwidth, inheight, xx, yy );
			SetPix( out, outwidth, outheight, x, y, RGB(GetRValue( c ), GetGValue( c ), GetBValue( c )) & mask );
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
bool cnvAntiResize( COLORREF *in , int inwidth , int inheight ,
			    COLORREF *out, int outwidth, int outheight, int wwidth, int wheight, PROGRESS prog, int seido )
{
	int			x, y, xx, yy, ix, iy, ixm1, ixm2, iym1, iym2, ixw, iyw;
	float		ax, ay, prx, pgx, pbx, pry, pgy, pby, am;
	COLORREF	c;
	COLORREF	mask;
	bool		bWZoom;	//	���g��
	bool		bHZoom;	//	�����g��

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
					c   = GetPix( in, inwidth, inheight, xx, yy );
					prx = GetRValue( c ) * (1-ax) * (1-ay);
					pgx = GetGValue( c ) * (1-ax) * (1-ay);
					pbx = GetBValue( c ) * (1-ax) * (1-ay);
					c	= GetPix( in, inwidth, inheight, xx+1, yy );
					prx += GetRValue( c ) * ax * (1-ay);
					pgx += GetGValue( c ) * ax * (1-ay);
					pbx += GetBValue( c ) * ax * (1-ay);
					c   = GetPix( in, inwidth, inheight, xx, yy+1 );
					prx += GetRValue( c ) * (1-ax) * ay;
					pgx += GetGValue( c ) * (1-ax) * ay;
					pbx += GetBValue( c ) * (1-ax) * ay;
					c	= GetPix( in, inwidth, inheight, xx+1, yy+1 );
					prx += GetRValue( c ) * ax * ay;
					pgx += GetGValue( c ) * ax * ay;
					pbx += GetBValue( c ) * ax * ay;
					ax = 1.0f;
				} else {
					//	�����k���̏ꍇ
					if( ixw == 0 ){
						c	= GetPix( in, inwidth, inheight, ixm1, yy );
						prx = GetRValue( c ) * (1-ay);
						pgx = GetGValue( c ) * (1-ay);
						pbx = GetBValue( c ) * (1-ay);
						c	= GetPix( in, inwidth, inheight, ixm1, yy+1 );
						prx += GetRValue( c ) * ay;
						pgx += GetGValue( c ) * ay;
						pbx += GetBValue( c ) * ay;
						ax  = am = 1.0f;
					}else{
						ax = 0;					// �d�݂̍��v�l�i�w�������j
						prx = pgx = pbx = 0;	// �s�N�Z���l���v�l�i�w�������j
						for( ix = ixm1; ix <= ixm2; ++ix ){
							am = float( ix - ixm1 ) / ixw;
							if( am < 0 ) am = -am;
							am = 1.0f - am;
							c = GetPix( in, inwidth, inheight, ix, yy );
							prx += GetRValue( c ) * am * (1-ay);
							pgx += GetGValue( c ) * am * (1-ay);
							pbx += GetBValue( c ) * am * (1-ay);
							c = GetPix( in, inwidth, inheight, ix, yy+1 );
							prx += GetRValue( c ) * am * ay;
							pgx += GetGValue( c ) * am * ay;
							pbx += GetBValue( c ) * am * ay;
							ax  += am;
						}	// ix
					}
				}
				SetPix( out, outwidth, outheight, x, y,
						RGB( int( prx/ax ), int( pgx/ax ), int( pbx/ax ) ) & mask );
			} else {
				//	�����k���̏ꍇ
				for( iy = iym1; iy <= iym2; ++iy ){
					if( bWZoom ) {
						//	�����g��̏ꍇ
						xx  = x * inwidth / wwidth;
						ax  = (float(x) * inwidth / wwidth) - float(xx);	//	�ʒu xx+1 �ւ̈�v�x
						c   = GetPix( in, inwidth, inheight, xx, iy );
						prx = GetRValue( c ) * (1-ax);
						pgx = GetGValue( c ) * (1-ax);
						pbx = GetBValue( c ) * (1-ax);
						c = GetPix( in, inwidth, inheight, xx+1, iy );
						prx += GetRValue( c ) * ax;
						pgx += GetGValue( c ) * ax;
						pbx += GetBValue( c ) * ax;
						ax = 1.0f;
					} else {
						//	�����k���̏ꍇ
						if( ixw == 0 ){
							c = GetPix( in, inwidth, inheight, ixm1, iy );
							prx = GetRValue( c );
							pgx = GetGValue( c );
							pbx = GetBValue( c );
							ax  = am = 1.0f;
						}else{
							ax = 0;					// �d�݂̍��v�l�i�w�������j
							prx = pgx = pbx = 0;	// �s�N�Z���l���v�l�i�w�������j
							for( ix = ixm1; ix <= ixm2; ++ix ){
								am = float( ix - ixm1 ) / ixw;
								if( am < 0 ) am = -am;
								am = 1.0f - am;
								c = GetPix( in, inwidth, inheight, ix, iy );
								prx += GetRValue( c ) * am;
								pgx += GetGValue( c ) * am;
								pbx += GetBValue( c ) * am;
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
				SetPix( out, outwidth, outheight, x, y,
						RGB( int( pry/ay ), int( pgy/ay ), int( pby/ay ) ) & mask );
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
static inline void PutDither( int *r, int *g, int *b, int mode, int ErrAdd, int x, int y, COLORREF c )
{
	if( mode >= EALGO_DITH1 ){
		switch( ErrAdd ){
		case EADD_ROTATE:
			*r = GetRValue( c ) + dith[ mode - EALGO_DITH1 ][       x & 7  ][ y & 7 ] * 2 - 16;
			*g = GetGValue( c ) + dith[ mode - EALGO_DITH1 ][       y & 7  ][ x & 7 ] * 2 - 16;
			*b = GetBValue( c ) + dith[ mode - EALGO_DITH1 ][ 7 - ( x & 7 )][ y & 7 ] * 2 - 16;
			break;
		case EADD_NONE:
			*r = GetRValue( c ) + dith[ mode - EALGO_DITH1 ][ y & 7 ][ x & 7 ] * 2 - 16;
			*g = GetGValue( c ) + dith[ mode - EALGO_DITH1 ][ y & 7 ][ x & 7 ] * 2 - 16;
			*b = GetBValue( c ) + dith[ mode - EALGO_DITH1 ][ y & 7 ][ x & 7 ] * 2 - 16;
			break;
		case EADD_X:
			*r = GetRValue( c ) + dith[ mode - EALGO_DITH1 ][ y & 7 ][ ( x + 0 ) & 7 ] * 2 - 16;
			*g = GetGValue( c ) + dith[ mode - EALGO_DITH1 ][ y & 7 ][ ( x + 1 ) & 7 ] * 2 - 16;
			*b = GetBValue( c ) + dith[ mode - EALGO_DITH1 ][ y & 7 ][ ( x + 2 ) & 7 ] * 2 - 16;
			break;
		case EADD_Y:
			*r = GetRValue( c ) + dith[ mode - EALGO_DITH1 ][ ( y + 0 ) & 7 ][ x & 7 ] * 2 - 16;
			*g = GetGValue( c ) + dith[ mode - EALGO_DITH1 ][ ( y + 1 ) & 7 ][ x & 7 ] * 2 - 16;
			*b = GetBValue( c ) + dith[ mode - EALGO_DITH1 ][ ( y + 2 ) & 7 ][ x & 7 ] * 2 - 16;
			break;
		}
	}else{
		*r = GetRValue( c );
		*g = GetGValue( c );
		*b = GetBValue( c );
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
bool cnvRecolor( COLORREF *in,int width,int height,
					unsigned char *out,SETTING *CnvMode,PROGRESS prog,COLORREF *pal,
					TAILPAT *tail,int tailcnt )
{
	bool	bRet;

	if( CnvMode->Mode == MD_SC8 || CnvMode->Mode == MD_SC8_256L ) {
		//	256�F�r�b�g�}�b�v�֕ϊ�����
		return cnvRecolor8( in, width, height, out, CnvMode, prog, pal, tail, tailcnt );
	} else {
		//	16�F�r�b�g�}�b�v�֕ϊ�����
		bRet = cnvRecolor5( in, width, height, out, CnvMode, prog, pal, tail, tailcnt );

		//	����ȉ�ʁiSC2/3/4) �ɂ��킹�ĕϊ�����		
		switch( CnvMode->Mode ) {
		case MD_SC2:
			bRet = bRet & cnvSC5toSC2( out, prog, pal );
			break;
		case MD_SC3:
			bRet = bRet & cnvSC5toSC3( out, prog );
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
static bool cnvRecolor8( COLORREF *in,int width,int height,
					unsigned char *out,SETTING *CnvMode,PROGRESS prog,COLORREF *pal,
					TAILPAT *tail,int tailcnt )
{
	int				x,y,ptr;
	int				cr,cg,cb;		// ����f�̂q�f�a
	int				er,eg,eb;		// �덷�i�����֊ۂ߂�j
	int				n;
	COLORREF		c,cc,*pin,FZC;
	signed short	*errbuf[2], *errbuf0, *errbuf1;
	int k			= CnvMode->Gosa ? int(CnvMode->Gosaval*1024) : 0;
	int ee			= CnvMode->err;
	COLORREF mask;
	bool			ret = false;

	// �덷�~�ϗp�ޯ̧�̍쐬
	memset( errbuf, 0, sizeof( errbuf ) );
	for( x = 0; x < 2; ++x ) {
		errbuf[ x ] = (signed short*)LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT, ( width + 5 ) * 4 * sizeof( signed short ) );
		if( errbuf[ x ]==NULL ) goto l_exit;
	}

	// ���x�����p�}�X�N�쐬
	mask = ~(( 1 << CnvMode->Seido ) - 1 );
	mask = mask & (( mask << 8 ) | 0xFF ) & (( mask << 16 ) | 0xFFFF );

	FZC = CnvMode->FourceZero ? CnvMode->FZColor & mask : -1;

	// �S�s�N�Z�����r�߂�
	ptr = 0;
	pin = in;

	if( !CnvMode->Tile ){
		// ��^�C�����[�h
		if( k ) {
			//	�덷�g�U�L��
			for( y = 0; y < height; ++y ){
				if( !prog( y * 100 / height ) ) goto l_exit;
				errbuf0 = errbuf[ y & 1 ];
				errbuf1 = errbuf[ 1 - ( y & 1 ) ];

				for( x = 0; x < width; ++x ){
					cc = c = *pin;
					pin++;
					
					// �f�B�U����
					PutDither( &cr, &cg, &cb, CnvMode->ErrAlgo, CnvMode->ErrAdd, x, y, c );
					
					// �덷���v�Z�ɓ����
					cr += errbuf0[ x * 4 + 0 ];
					cg += errbuf0[ x * 4 + 1 ];
					cb += errbuf0[ x * 4 + 2 ];

					// �ł��߂��F��������
					er = ( cr + 18 ) * 7 / 255;
					eg = ( cg + 18 ) * 7 / 255;
					eb = ( cb + 36 ) * 3 / 255;
					er = AdjustNum( er, 0, 7 );
					eg = AdjustNum( eg, 0, 7 );
					eb = AdjustNum( eb, 0, 3 );
					n = ( er << 2 ) | ( eg << 5 ) | eb;
					if( n == 0 && CnvMode->NonZero ) n = 0x04;
					cc=RGB( ::cc[ ( n >> 2) & 0x07 ], ::cc[ ( n >> 5)  & 0x07 ], ::cb[ n & 0x03 ] );

					// �덷�����͂̃s�N�Z���֊g�U������
					er = AdjustNum( ( cr - GetRValue( cc )) * k / 1024 ,-32768, 32767 );
					eg = AdjustNum( ( cg - GetGValue( cc )) * k / 1024 ,-32768, 32767 );
					eb = AdjustNum( ( cb - GetBValue( cc )) * k / 1024 ,-32768, 32767 );
					// ���ׂȌ덷�͏��ł�����
					if( Abs(er) < (signed)CnvMode->err ) er=0;
					if( Abs(eg) < (signed)CnvMode->err ) eg=0;
					if( Abs(eb) < (signed)CnvMode->err ) eb=0;
					// �E�Ɋg�U
					errbuf0[(x+1)*4+0]+= (signed short)er;
					errbuf0[(x+1)*4+1]+= (signed short)eg;
					errbuf0[(x+1)*4+2]+= (signed short)eb;
					// ���Ɋg�U
					errbuf1[ x*4 +0] = (signed short)er;
					errbuf1[ x*4 +1] = (signed short)eg;
					errbuf1[ x*4 +2] = (signed short)eb;

					// ���ʂ��o�͂���
					if( CnvMode->FourceZero && cc==FZC ) n=0;	// �����[����
					out[ptr]=n;
					// ���̏o�͐�
					++ptr;
				}	// x
			}	// y
		} else {
			//	�덷�g�U����
			for( y = 0; y < height; ++y ){
				if( !prog( y * 100 / height ) ) goto l_exit;
				errbuf0 = errbuf[ y & 1 ];
				errbuf1 = errbuf[ 1 - ( y & 1 ) ];

				for( x = 0; x < width; ++x ){
					cc = c = *pin;
					pin++;
					
					// �f�B�U����
					PutDither( &cr, &cg, &cb, CnvMode->ErrAlgo, CnvMode->ErrAdd, x, y, c );
					
					// �ł��߂��F��������
					er = ( cr + 18 ) * 7 / 255;
					eg = ( cg + 18 ) * 7 / 255;
					eb = ( cb + 36 ) * 3 / 255;
					er = AdjustNum( er, 0, 7 );
					eg = AdjustNum( eg, 0, 7 );
					eb = AdjustNum( eb, 0, 3 );
					n = ( er << 2 ) | ( eg << 5 ) | eb;
					if( n == 0 && CnvMode->NonZero ) n = 0x04;
					cc=RGB( ::cc[ ( n >> 2) & 0x07 ], ::cc[ ( n >> 5)  & 0x07 ], ::cb[ n & 0x03 ] );

					// ���ʂ��o�͂���
					if( CnvMode->FourceZero && cc==FZC ) n=0;	// �����[����
					out[ptr]=n;
					// ���̏o�͐�
					++ptr;
				}	// x
			}	// y
		}
	}else{
		// �^�C�����[�h
		if( k ) {
			//	�덷�g�U�L��
			for( y = 0; y < height; ++y ){
				if( !prog( y * 100 / height ) ) goto l_exit;
				errbuf0 = errbuf[ y & 1 ];
				errbuf1 = errbuf[ 1 - ( y & 1 ) ];

				for( x = 0; x < width; ++x ){
					cc = c = *pin;
					pin++;
					
					// �f�B�U����
					PutDither( &cr, &cg, &cb, CnvMode->ErrAlgo, CnvMode->ErrAdd, x, y, c );
					
					// �덷���v�Z�ɓ����
					cr += errbuf0[ x * 4 + 0 ];
					cg += errbuf0[ x * 4 + 1 ];
					cb += errbuf0[ x * 4 + 2 ];

					// �ł��߂��F��������
					er = ( cr +  9 ) * 14 / 255;
					eg = ( cg +  9 ) * 14 / 255;
					eb = ( cb + 18 ) *  6 / 255;
					er = AdjustNum( er, 0, 14 );
					eg = AdjustNum( eg, 0, 14 );
					eb = AdjustNum( eb, 0,  6 );
					n = (( er >> 1 ) << 2 ) | (( eg >> 1 ) << 5 ) | ( eb >> 1 );
					if( ( x ^ y ) & 1 ){
						n += (( er & 1 ) << 2 ) | (( eg & 1 ) << 5 ) | ( eb & 1 );
					}
					if( n == 0 && CnvMode->NonZero ) n = 0x04;
					cc=RGB( ::cc[( n >> 2 ) & 0x07 ], ::cc[ ( n >> 5 ) & 0x07 ], ::cb[ n & 0x03 ] );

					// �덷�����͂̃s�N�Z���֊g�U������
					er = AdjustNum( ( cr - GetRValue( cc )) * k / 1024 ,-32768, 32767 );
					eg = AdjustNum( ( cg - GetGValue( cc )) * k / 1024 ,-32768, 32767 );
					eb = AdjustNum( ( cb - GetBValue( cc )) * k / 1024 ,-32768, 32767 );
					// ���ׂȌ덷�͏��ł�����
					if( Abs(er) < (signed)CnvMode->err ) er=0;
					if( Abs(eg) < (signed)CnvMode->err ) eg=0;
					if( Abs(eb) < (signed)CnvMode->err ) eb=0;
					// �E�Ɋg�U
					errbuf0[(x+1)*4+0]+= (signed short)er;
					errbuf0[(x+1)*4+1]+= (signed short)eg;
					errbuf0[(x+1)*4+2]+= (signed short)eb;
					// ���Ɋg�U
					errbuf1[ x*4 +0] = (signed short)er;
					errbuf1[ x*4 +1] = (signed short)eg;
					errbuf1[ x*4 +2] = (signed short)eb;

					// ���ʂ��o�͂���
					if( CnvMode->FourceZero && cc==FZC ) n=0;	// �����[����
					out[ptr]=n;
					// ���̏o�͐�
					++ptr;
				}	// x
			}	// y
		} else {
			//	�덷�g�U����
			for( y = 0; y < height; ++y ){
				if( !prog( y * 100 / height ) ) goto l_exit;
				errbuf0 = errbuf[ y & 1 ];
				errbuf1 = errbuf[ 1 - ( y & 1 ) ];

				for( x = 0; x < width; ++x ){
					cc = c = *pin;
					pin++;
					
					// �f�B�U����
					PutDither( &cr, &cg, &cb, CnvMode->ErrAlgo, CnvMode->ErrAdd, x, y, c );
					
					// �ł��߂��F��������
					er = ( cr +  9 ) * 14 / 255;
					eg = ( cg +  9 ) * 14 / 255;
					eb = ( cb + 18 ) *  6 / 255;
					er = AdjustNum( er, 0, 14 );
					eg = AdjustNum( eg, 0, 14 );
					eb = AdjustNum( eb, 0,  6 );
					n = (( er >> 1 ) << 2 ) | (( eg >> 1 ) << 5 ) | ( eb >> 1 );
					if( ( x ^ y ) & 1 ){
						n += (( er & 1 ) << 2 ) | (( eg & 1 ) << 5 ) | ( eb & 1 );
					}
					if( n == 0 && CnvMode->NonZero ) n = 0x04;
					cc=RGB( ::cc[( n >> 2 ) & 0x07 ], ::cc[ ( n >> 5 ) & 0x07 ], ::cb[ n & 0x03 ] );

					// ���ʂ��o�͂���
					if( CnvMode->FourceZero && cc==FZC ) n=0;	// �����[����
					out[ptr]=n;
					// ���̏o�͐�
					++ptr;
				}	// x
			}	// y
		}	//	�덷�g�U
	}	//	�^�C�� or ��^�C��

	ret = true;
l_exit:
	if( errbuf[0] != NULL ) LocalFree( errbuf[0] );
	if( errbuf[1] != NULL ) LocalFree( errbuf[1] );
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
static bool cnvRecolor5( COLORREF *in,int width,int height,
					unsigned char *out,SETTING *CnvMode,PROGRESS prog,COLORREF *pal,
					TAILPAT *tail,int tailcnt )
{
	int				x,y,z,d,ptr;
	int				cr,cg,cb;		// ����f�̂q�f�a
	int				er,eg,eb;		// �덷�i�����֊ۂ߂�j
	int				r,n,nr;
	COLORREF		c,cc,*pin,FZC;
	signed short	*errbuf[2], *errbuf0, *errbuf1;
	int palnum;
	int k			= CnvMode->Gosa ? int(CnvMode->Gosaval*1024) : 0;
	int ee			= CnvMode->err;
	COLORREF mask;
	bool			ret = false;

	// �덷�~�ϗp�ޯ̧�̍쐬
	memset( errbuf, 0, sizeof( errbuf ) );
	for( x = 0; x < 2; ++x ) {
		errbuf[ x ] = (signed short*)LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT, ( width + 5 ) * 4 * sizeof( signed short ) );
		if( errbuf[ x ]==NULL ) goto l_exit;
	}

	// ���x�����p�}�X�N�쐬
	mask = ~(( 1 << CnvMode->Seido ) - 1 );
	mask = mask & (( mask << 8 ) | 0xFF ) & (( mask << 16 ) | 0xFFFF );

	// �p���b�g��
	switch( CnvMode->Mode ){
	case MD_SC6:
	case MD_SC6_256L:
		palnum = 4;		break;
	default:
		palnum = 16;	break;
	}
	FZC = CnvMode->FourceZero ? CnvMode->FZColor & mask : -1;

	// �S�s�N�Z�����r�߂�
	ptr = 0;
	pin = in;

	if( !CnvMode->Tile ){
		// ��^�C�����[�h
		if( k ) {
			//	�덷�g�U�L��
			for( y = 0; y < height; ++y ){
				if( !prog( y * 100 / height ) ) goto l_exit;
				errbuf0 = errbuf[ y & 1 ];
				errbuf1 = errbuf[ 1 - ( y & 1 ) ];

				for( x = 0; x < width; ++x ){
					cc = c = *pin;
					pin++;
					
					// �f�B�U����
					PutDither( &cr, &cg, &cb, CnvMode->ErrAlgo, CnvMode->ErrAdd, x, y, c );
					
					// �덷���v�Z�ɓ����
					cr += errbuf0[ x * 4 + 0 ];
					cg += errbuf0[ x * 4 + 1 ];
					cb += errbuf0[ x * 4 + 2 ];

					// �ł��߂��F��������
					nr = 0x7FFFFFFFL;

					if( CnvMode->NonZero ) z = 1; else z = 0;
					for( ; z < palnum; ++z ){
						if( CnvMode->PalEn[ z ] == PALEN_NONE ) continue;
						// �߂����H
						c = pal[ z ];
						r = Interval( cr, cg, cb, GetRValue(c), GetGValue(c), GetBValue(c) );
						if( r < nr ){
							nr = r;
							n  = z;
							cc = c;
							if( r == 0 ) break;	// ���S��v�Ȃ甲����
						}
					}	// z

					// �덷�����͂̃s�N�Z���֊g�U������
					er = AdjustNum( ( cr - GetRValue( cc )) * k / 1024 ,-32768, 32767 );
					eg = AdjustNum( ( cg - GetGValue( cc )) * k / 1024 ,-32768, 32767 );
					eb = AdjustNum( ( cb - GetBValue( cc )) * k / 1024 ,-32768, 32767 );
					// ���ׂȌ덷�͏��ł�����
					if( Abs(er) < (signed)CnvMode->err ) er=0;
					if( Abs(eg) < (signed)CnvMode->err ) eg=0;
					if( Abs(eb) < (signed)CnvMode->err ) eb=0;
					// �E�Ɋg�U
					errbuf0[(x+1)*4+0]+= (signed short)er;
					errbuf0[(x+1)*4+1]+= (signed short)eg;
					errbuf0[(x+1)*4+2]+= (signed short)eb;
					// ���Ɋg�U
					errbuf1[ x*4 +0] = (signed short)er;
					errbuf1[ x*4 +1] = (signed short)eg;
					errbuf1[ x*4 +2] = (signed short)eb;

					// ���ʂ��o�͂���
					if( CnvMode->FourceZero && cc==FZC ) n=0;	// �����[����
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
		} else {
			//	�덷�g�U����
			for( y = 0; y < height; ++y ){
				if( !prog( y * 100 / height ) ) goto l_exit;
				errbuf0 = errbuf[ y & 1 ];
				errbuf1 = errbuf[ 1 - ( y & 1 ) ];

				for( x = 0; x < width; ++x ){
					cc = c = *pin;
					pin++;
					
					// �f�B�U����
					PutDither( &cr, &cg, &cb, CnvMode->ErrAlgo, CnvMode->ErrAdd, x, y, c );
					
					// �ł��߂��F��������
					nr = 0x7FFFFFFFL;

					if( CnvMode->NonZero ) z = 1; else z = 0;
					for( ; z < palnum; ++z ){
						if( CnvMode->PalEn[ z ] == PALEN_NONE ) continue;
						// �߂����H
						c = pal[ z ];
						r = Interval( cr, cg, cb, GetRValue(c), GetGValue(c), GetBValue(c) );
						if( r < nr ){
							nr = r;
							n  = z;
							cc = c;
							if( r == 0 ) break;	// ���S��v�Ȃ甲����
						}
					}	// z

					// ���ʂ��o�͂���
					if( CnvMode->FourceZero && cc==FZC ) n=0;	// �����[����
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
		}	//	�덷�g�U
	} else {
		// �^�C�����[�h
		if( k ) {
			//	�덷�g�U�L��
			for( y = 0; y < height; ++y ){
				if( !prog( y * 100 / height ) ) goto l_exit;
				errbuf0 = errbuf[ y & 1 ];
				errbuf1 = errbuf[ 1 - ( y & 1 ) ];

				for( x = 0; x < width; ++x ){
					cc = c = *pin;
					pin++;
					
					// �f�B�U����
					PutDither( &cr, &cg, &cb, CnvMode->ErrAlgo, CnvMode->ErrAdd, x, y, c );
					
					// �덷���v�Z�ɓ����
					cr += errbuf0[ x * 4 + 0 ];
					cg += errbuf0[ x * 4 + 1 ];
					cb += errbuf0[ x * 4 + 2 ];

					// �ł��߂��F��������
					nr = 0x7FFFFFFFL;

					for( z = 0; z < tailcnt; ++z ){	// �^�C���p�^�[��
						// �߂����H
						c = tail[ z ].c;
						r = Interval( cr, cg, cb, GetRValue(c), GetGValue(c), GetBValue(c) );
						if( r < nr ){
							nr = r;
							n  = tail[z].p[ ( x ^ y ) & 1 ];
							cc = c;
							if( r == 0 ) break;	// ���S��v�Ȃ甲����
						}
					}	// z

					// �덷�����͂̃s�N�Z���֊g�U������
					er = AdjustNum( ( cr - GetRValue( cc )) * k / 1024 ,-32768, 32767 );
					eg = AdjustNum( ( cg - GetGValue( cc )) * k / 1024 ,-32768, 32767 );
					eb = AdjustNum( ( cb - GetBValue( cc )) * k / 1024 ,-32768, 32767 );
					// ���ׂȌ덷�͏��ł�����
					if( Abs(er) < (signed)CnvMode->err ) er=0;
					if( Abs(eg) < (signed)CnvMode->err ) eg=0;
					if( Abs(eb) < (signed)CnvMode->err ) eb=0;
					// �E�Ɋg�U
					errbuf0[(x+1)*4+0]+= (signed short)er;
					errbuf0[(x+1)*4+1]+= (signed short)eg;
					errbuf0[(x+1)*4+2]+= (signed short)eb;
					// ���Ɋg�U
					errbuf1[ x*4 +0] = (signed short)er;
					errbuf1[ x*4 +1] = (signed short)eg;
					errbuf1[ x*4 +2] = (signed short)eb;

					// ���ʂ��o�͂���
					if( CnvMode->FourceZero && cc==FZC ) n=0;	// �����[����
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
		} else {
			//	�덷�g�U����
			for( y = 0; y < height; ++y ){
				if( !prog( y * 100 / height ) ) goto l_exit;
				errbuf0 = errbuf[ y & 1 ];
				errbuf1 = errbuf[ 1 - ( y & 1 ) ];

				for( x = 0; x < width; ++x ){
					cc = c = *pin;
					pin++;
					
					// �f�B�U����
					PutDither( &cr, &cg, &cb, CnvMode->ErrAlgo, CnvMode->ErrAdd, x, y, c );
					
					// �ł��߂��F��������
					nr = 0x7FFFFFFFL;

					for( z = 0; z < tailcnt; ++z ){	// �^�C���p�^�[��
						// �߂����H
						c = tail[ z ].c;
						r = Interval( cr, cg, cb, GetRValue(c), GetGValue(c), GetBValue(c) );
						if( r < nr ){
							nr = r;
							n  = tail[z].p[ ( x ^ y ) & 1 ];
							cc = c;
							if( r == 0 ) break;	// ���S��v�Ȃ甲����
						}
					}	// z

					// ���ʂ��o�͂���
					if( CnvMode->FourceZero && cc==FZC ) n=0;	// �����[����
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
		}	//	�덷�g�U
	}	//	�^�C�� or ��^�C��
	ret = true;
l_exit:
	if( errbuf[0] != NULL ) LocalFree( errbuf[0] );
	if( errbuf[1] != NULL ) LocalFree( errbuf[1] );
	return ret;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		SCREEN5 �C���[�W�� SCREEN2/4 �C���[�W�֕ϊ�����
//	2.	����
//		out		...	VRAM�C���[�W
//		prog	...	�o�ߕ\���֐�
//	3.	�Ԓl
//		true	...	����
//		false	...	���s
//	4.	���l
//		�C���^�[���[�X���[�h�͎g���Ȃ�
// -------------------------------------------------------------
static bool cnvSC5toSC2( unsigned char *out, PROGRESS prog, COLORREF *pal )
{
	int	x, y, i, j, adr, dadr;
	int	cc, c, p;
	int	cnt, idx1, idx2, len, l;
	int	ccnt[ 8 ];
	int	ccol[ 8 ];
	COLORREF	c1, c2;

	memset( vram, 0, sizeof( vram ) );

	adr  = 0;
	for( y = 0; y < 192; ++y ) {
		for( x = 0; x < 32; ++x ) {
			//	�W�h�b�g�y�A�̒��ōł������g���Ă���Q�F��I��
			cnt  = 0;
			idx1 = 0;							//	�ł������F�̃C���f�b�N�X
			for( j = 0; j < 8; ++j ) {			//	8�h�b�g�Ȃ߂�
				cc = out[ adr + j/2 ];
				if( j & 1 ) {
					cc = cc & 0x0F;
				} else {
					cc = (cc >> 4) & 0x0F;
				}
				for( i = 0; i < cnt; ++i ) {	//	�����F������
					if( cc == ccol[ i ] ) {
						++ccnt[ i ];			//		�������F�̐��𑝉�
						if( ccnt[ i ] > ccnt[ idx1 ] ) {	//	�ł�����
							idx1 = i;
						}
						break;
					}
				}
				if( i == cnt ) {				//	�V�����F
					ccol[ cnt ] = cc;			//		�������F
					ccnt[ cnt ] = 1;			//		�������F�̐�
					++cnt;
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
				c1= pal[ ccol[ idx1 ] ];
				for( i = 0; i < cnt; ++i ) {
					if( i == idx1 ) continue;
					cc = ccol[i];
					l = Interval( GetRValue( pal[cc] ), GetGValue( pal[cc] ), GetBValue( pal[cc] ), GetRValue( c1 ), GetGValue( c1 ), GetBValue( c1 ));
					if( l >= len ) {
						len = l;
						idx2 = i;
					}
				}

				c = ( ccol[ idx1 ] << 4 ) | ccol[ idx2 ];
				p = 0;
				c2= pal[ ccol[ idx2 ] ];
				for( j = 0; j < 8; ++j ) {
					cc = out[ adr + j/2 ];
					if( j & 1 ) {
						cc = cc & 0x0F;
					} else {
						cc = (cc >> 4) & 0x0F;
					}
					//	�I�΂ꂽ�Q�F�Ƃ̐F�������r
					p = p << 1;
					if( Interval( GetRValue( pal[cc] ), GetGValue( pal[cc] ), GetBValue( pal[cc] ), GetRValue( c1 ), GetGValue( c1 ), GetBValue( c1 )) 
					  < Interval( GetRValue( pal[cc] ), GetGValue( pal[cc] ), GetBValue( pal[cc] ), GetRValue( c2 ), GetGValue( c2 ), GetBValue( c2 )) ) {
						p = p | 1;
					}
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

	memcpy( out, vram, sizeof( vram ) );
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
bool cnvNtcolor( COLORREF *in ,int width ,int height ,unsigned char *out ,
				 SETTING *CnvMode ,PROGRESS prog )
{
	int				x, y, z, xx;
	int				z255p31[32];
	COLORREF		c;
	int				r[ 5 ], g[ 5 ], b[ 5 ];
	int				vy, min, tyy, txx;
	int				rr, gg, bb, er, eg, eb;
	int				jj, kk, yy[ 4 ], ii[ 4 ], s;
	int				mask;
	unsigned char	*pout = out;
	short			*ptmp,*tmp = NULL;
	int				*errbuf[ 2 ], *errbuf0, *errbuf1;

	int		k		= CnvMode->Gosa ? int( CnvMode->Gosaval * 1024 ) : 0;
	int		algo	= CnvMode->AlgoMode;
	bool	rc		= CnvMode->JKrc;
	int		ealgo	= CnvMode->ErrAlgo;
	bool	sc10	= ( CnvMode->Mode == MD_SC10 || CnvMode->Mode == MD_SC10_256L );
	int		zero	= CnvMode->FourceZero ? (sc10 ? 0x10 : 0x08) : 0;
	bool	ret		= false;

	//	��ƕϐ�
	for( y = 0; y < 32; ++y ) {
		z255p31[ y ] = y * 255 / 31;
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
				c = GetPix( in, width, height, x + z, y );
				r[ z ] = GetRValue( c ) + errbuf0[ txx + 0 ];
				g[ z ] = GetGValue( c ) + errbuf0[ txx + 1 ];
				b[ z ] = GetBValue( c ) + errbuf0[ txx + 2 ];
				vy   = AdjustNum( b[ z ] / 2 + r[ z ] / 4 + g[ z ] / 8, 0, 255 );
				jj += r[ z ] - vy;
				kk += g[ z ] - vy;
				txx += 3;
			}
			// J,K �̒l�𕽋ς��ĂS��f�y�A�� J,K �l���m�肷��
			jj = AdjustNum( jj / 4, -256, 255 );
			kk = AdjustNum( kk / 4, -256, 255 );

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
					rr = AdjustNum( r[z], 0, 255 );
					gg = AdjustNum( g[z], 0, 255 );
					bb = AdjustNum( b[z], 0, 255 );
					for( vy = 0; vy < 255; ++vy ){
						s = Interval( rr, gg, bb,
							AdjustNum( vy + jj, 0, 255 ),  
							AdjustNum( vy + kk, 0, 255 ), 
							AdjustNum( vy * 5 / 4 - jj / 2 - kk / 4, 0, 255 ) );
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
					rr = AdjustNum( r[z], 0, 255 );
					gg = AdjustNum( g[z], 0, 255 );
					bb = AdjustNum( b[z], 0, 255 );
					for( vy = 0; vy < 255; ++vy ){
						s = Interval( rr, gg, bb,
							AdjustNum( vy + jj, 0, 255 ),  
							AdjustNum( vy + kk, 0, 255 ), 
							AdjustNum( vy * 5 / 4 - jj / 2 - kk / 4, 0, 255 ) );
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
				yy[ z ] = AdjustNum( tyy, 0, 255 );
				if( k ){	// �덷�g�U
					// �Z�o���� ��Y,J,K ���� R,G,B �l�i MSX�ŕ\�������F �j���v�Z
					rr = z255p31[ AdjustNum( jj + yy[ z ], 0, 255 ) >> 3 ];
					gg = z255p31[ AdjustNum( kk + yy[ z ], 0, 255 ) >> 3 ];
					bb = z255p31[ AdjustNum( 5 * yy[ z ] / 4 - jj / 2 - kk / 4, 0, 255 ) >> 3 ];
					// �덷�����͂̃s�N�Z���֊g�U������
					er = AdjustNum( ( r[ z ] - rr ) * k / 1024 , -32768, 32767 );
					eg = AdjustNum( ( g[ z ] - gg ) * k / 1024 , -32768, 32767 );
					eb = AdjustNum( ( b[ z ] - bb ) * k / 1024 , -32768, 32767 );

					// ���ׂȌ덷�͏��ł�����
					if( Abs( er ) >= (signed)CnvMode->err ) {
						errbuf0[ txx + 3 ] += er;
						errbuf1[ txx + 0 ] = er;
					} else {
						errbuf1[ txx + 0 ] = 0;
					}
					if( Abs( eg ) >= (signed)CnvMode->err ) {
						errbuf0[ txx + 4 ] += eg;
						errbuf1[ txx + 1 ] = eg;
					} else {
						errbuf1[ txx + 1 ] = 0;
					}
					if( Abs( eb ) >= (signed)CnvMode->err ) {
						errbuf0[ txx + 5 ] += eb;
						errbuf1[ txx + 2 ] = eb;
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
			jj = AdjustNum( jj * 31 / 255, -32, 31 );	// �����_�ȉ��؂�̂�
			kk = AdjustNum( kk * 31 / 255, -32, 31 );	// �����_�ȉ��؂�̂�
			
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
				*out=( kk     & 7) | ( AdjustNum( yy[0], 0, 255 ) & mask );
				if( *out==0 ) *out= zero;
				++out;
				*out=((kk>>3) & 7) | ( AdjustNum( yy[1], 0, 255 ) & mask );
				if( *out==0 ) *out= zero;
				++out;
				*out=( jj     & 7) | ( AdjustNum( yy[2], 0, 255 ) & mask );
				if( *out==0 ) *out= zero;
				++out;
				*out=((jj>>3) & 7) | ( AdjustNum( yy[3], 0, 255 ) & mask );
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
			z  = AdjustNum( vy, 0, 255 ) & mask;				//	���ۂɍ̗p����x�l
			jj = ( vy - z255p31[ z >> 3 ] ) * k / 1024;			//	�덷
			if( jj >= (signed)CnvMode->err ) {
				// �덷�̊g�U
				if( x != width - 1 ) {
					*(ptmp + 1) = (short)AdjustNum( (int)*(ptmp +     1) + jj, -32768, 32767 );
				}
				*(ptmp + width) = (short)AdjustNum( (int)*(ptmp + width) + jj, -32768, 32767 );
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
int cnvColorTblCompare( const void *tbl1,const void *tbl2 )
{
	return( (long)(((COLORTBL*)tbl2)->n )-(long)(((COLORTBL*)tbl1)->n ) );
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
bool cnvGetPalette( COLORREF *in,int width,int height,COLORREF *pal,int mode,int cnt,int pp,
				    bool FourceZero,COLORREF FZColor )
{
	COLORREF	c;
	COLORTBL	*tbl, tmp;
	int			n, i, j;
	int			r, g, b, v, w, y;

	// �Q�`�P�U�F���������ł��Ȃ�
	if( cnt < 2 || cnt > 16 || cnt <= pp ) return false;
	
	// �q�X�g�O�����̍쐬
	n=cnvCreateHistgram( in, width * height, &tbl, pal, pp, FourceZero, FZColor );
	if( tbl == NULL ) return false;
	ZeroMemory( pal + pp, sizeof( COLORREF ) * ( cnt - pp ) );	

	// �F���\�[�g����
	qsort( tbl, n, sizeof( COLORTBL ), cnvColorTblCompare );

	if( mode==0 ){
		// �ړI�̐F���ɂȂ�܂Ŏ����F���폜��������
		i=0;
		while( n > cnt ){
			// ���ڐF���擾
			c = tbl[ i ].c;
			r = GetRValue( c );
			g = GetGValue( c );
			b = GetBValue( c );
			y = -1;							// �I�o�����F�̃C���f�b�N�X
			w = 0x7FFFFFFFL;				// �I�o�����F�̋���
			for( j = i + 1; j < n; ++j ){	// ���ڐF�ɍł��߂��Ďg�p�������Ȃ��F����������
				if( j < pp ) continue;
				c = tbl[ j ].c;
				v = Interval( r, g, b, GetRValue( c ), GetGValue( c ), GetBValue( c ) );
				if( w < v ) continue;
				y = j;
				w = v;
			}
			if( y > 0 ){			// �������Č��������F�𒅖ڐF�ɋz������
				tbl[ i ].n += tbl[ y ].n;
				if( tbl[ i ].n < 0 )
					tbl[ i ].n = 0x7FFFFFFFL;
				if( y < n - 1 ) CopyMemory( &tbl[ y ], &tbl[ y + 1 ], sizeof( COLORTBL ) * ( n - 1 - y ) );
				--n;
				for( j = i; j > 0; --j ) {
					if( tbl[ j ].n <= tbl[ j-1 ].n ) break;
					tmp			= tbl[ j ];
					tbl[ j ]	= tbl[ j-1 ];
					tbl[ j-1 ]	= tmp;
				}
			}
			++i;					// ���̐F
			if( i >= n ) i = 0;		// ������܂ŏz���Č�������
		}
	} else {
		//	�����z�I�F
		for( i = 0; i < pp; ++i ){
			// ���ڐF�i�I��F���g�p�̐F�j���擾
			c = tbl[ i ].c;
			r = GetRValue( c );
			g = GetGValue( c );
			b = GetBValue( c );
			y = -1;
			for( j = pp; j < n; ++j ){	// ���ڐF�ƈ�v����F����������i�P�����Ȃ��j
				c = tbl[ j ].c;
				if( (r == GetRValue( c )) && (g == GetGValue( c )) && (b == GetBValue( c )) ) {
					y = j;
					break;
				}
			}
			if( y > 0 ){			// �������Č��������F���폜����
				if( y < n - 1 ) CopyMemory( &tbl[ y ], &tbl[ y + 1 ], sizeof( COLORTBL ) * ( n - 1 - y ) );
				--n;
			}
		}
	}

	// �e�[�u�����̃p���b�g�擪 cnt ���̗p����
	for( i = 0; i < n && i < cnt; ++i ){
		pal[ i ] = tbl[ i ].c;
	}

	// ���������������
	LocalFree( tbl );
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
static int cnvCreateHistgram( COLORREF *in,int size,COLORTBL **tbl,COLORREF *pal,int pp,
							  bool FourceZero,COLORREF FZColor )
{
	COLORTBL	hash[ 512 ];
	int			cnv[ 256 ];
	int			i,n,cnt,t,r,g,b;
	COLORREF	c;
	// NULL �N���A
	ZeroMemory( hash,sizeof( hash ) );
	*tbl=NULL;
	cnt=0;		// �F�̎g�p��

	//	�ϊ��e�[�u���𐶐�����
	for( i = 0; i < 256; ++i ) {
		cnv[i] = ( i + 18 ) * 7 / 255;
	}

	// ��f�̐��𐔂���
	for( i = 0; i < size; ++i ){
		c = in[ i ];
		if( c == FZColor && FourceZero ) continue;
		r = cnv[ GetRValue( c ) ];
		g = cnv[ GetGValue( c ) ];
		b = cnv[ GetBValue( c ) ];
		n = r * 64 + g * 8 + b;		//	�n�b�V���֐�
		hash[ n ].c = RGB( cc[r], cc[g], cc[b] );
		++hash[ n ].n;
		if( hash[ n ].n == 1 ) ++cnt;
	}	// for

	// �J���[�e�[�u�����m�ۂ���
	*tbl=(COLORTBL*) LocalAlloc( LMEM_FIXED, sizeof( COLORTBL ) * ( cnt + pp ) );
	if( *tbl == NULL ) return 0;

	// �n�b�V���\���� tbl �֕ϊ�
	t=0;
	for( i = 0; i < pp ; ++i ){
		(*tbl)[ t ].c = pal[ i ];
		(*tbl)[ t ].n = 0x7FFFFFFF;
		++t;
	}
	for( i = 0; i < 512; ++i ){
		if( hash[i].n == 0 ) continue;
		(*tbl)[ t ] = hash[ i ];
		++t;
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
int cnvCreateTail4( PAL *pal,uchar *palen,bool zeroen,TAILPAT *tail, int mode )
{
	int i, j, k, l;
	int r,g,b;
	int rr,gg,bb;
	int n=0,m;
	bool ok;
	int	pnum;
	// ���ׂĂ̐F�̑g�ݍ��킹����������
	l = zeroen ? 0 : 1;
	pnum = ( mode == MD_SC6 || mode == MD_SC6_256L ) ? 4 : 16;
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
					if( Interval( r, g, b, rr, gg, bb ) <
						Interval( pal[ tail[ k ].p[ 0 ] ].red,pal[ tail[ k ].p[ 0 ] ].green,pal[ tail[ k ].p[ 0 ] ].blue,
								  pal[ tail[ k ].p[ 1 ] ].red,pal[ tail[ k ].p[ 1 ] ].green,pal[ tail[ k ].p[ 1 ] ].blue ) ){
						m = k;
					}else{
						m = -1;
						break;
					}
				}
			}
			if( m != -1 ){
				tail[ m ].c = RGB( ( cc[ r ] + cc[ rr ] ) / 2, ( cc[ g ] + cc[ gg ] ) / 2, ( cc[ b ] + cc[ bb ] ) / 2 );
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
static bool cnvCompare( PAL* Pal1, PAL* Pal2 )
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
void cnvSortPalette( SETTING* Mode, COLORREF* Pal )
{
	PAL			Col[16]  , tBakCol;	// SC5/SC7 �ɂ�����l�r�w���p���b�g�w��
	uchar		PalEn[16], tBakEn;
	COLORREF	pal[16]  , tBakPal;
	int			cnt, z, x, pnum;

	//	�p���b�g���R�s�[
	cnt  = 0;
	pnum = ( Mode->Mode == MD_SC6 || Mode->Mode == MD_SC6_256L ) ? 4 : 16;
	for( z = 0; z < pnum; ++z ) {
		if( z == 0 && Mode->NonZero ) continue;
		switch( Mode->SortMode ) {
		case SM_INCAUTO:
		case SM_DECAUTO:
			if( Mode->PalEn[ z ] != PALEN_AUTO ) continue;
			Col[ cnt ]   = Mode->Col[ z ];
			PalEn[ cnt ] = Mode->PalEn[ z ];
			pal[ cnt ]   = Pal[ z ];
			++cnt;
			break;
		case SM_INC:
		case SM_DEC:
			Col[ cnt ]   = Mode->Col[ z ];
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
				if( cnvCompare( &Col[ x ], &Col[ x + 1 ] ) ) {
					tBakCol        = Col[ x ];
					Col[ x ]       = Col[ x + 1 ];
					Col[ x + 1 ]   = tBakCol;
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
				if( !cnvCompare( &Col[ x ], &Col[ x + 1 ] ) ) {
					tBakCol        = Col[ x ];
					Col[ x ]       = Col[ x + 1 ];
					Col[ x + 1 ]   = tBakCol;
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
			Mode->Col[ z ]   = Col[ cnt ];
			Mode->PalEn[ z ] = PalEn[ cnt ];
			Pal[ z ]         = pal[ cnt ];
			++cnt;
			break;
		case SM_INC:
		case SM_DEC:
			Mode->Col[ z ]   = Col[ cnt ];
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
static inline COLORREF GetPix( COLORREF *in,int width,int height,int x,int y )
{
	// �͈͊O�C��
	x = AdjustNum( x,0,width-1  );
	y = AdjustNum( y,0,height-1 );
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
static inline void SetPix( COLORREF *out,int width,int height,int x,int y,COLORREF c )
{
	// �͈͊O�C��
	x = AdjustNum( x,0,width-1  );
	y = AdjustNum( y,0,height-1 );
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
static inline int Abs( int a )
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
static inline int Interval( int r1, int g1, int b1, int r2, int g2, int b2 )
{
	int r, g, b;
	r = r2 - r1;
	g = g2 - g1;
	b = b2 - b1;
	return( r*r + g*g + b*b );
}

// -------------------------------------------------------------
//	�v���r���[����

// -----------------------------------------------------
//	1.	���{�ꖼ
//		���C�����̎擾
//	2.	����
//		Mode	...	(I)	�摜�̐ݒ�
//	3.	�Ԓl
//		���C����
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
int _GetHeight( const SETTING *Mode )
{
	if( Mode->b192 ) {
		return 192;
	} else {
		return 212;
	}
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�u�q�`�l�C���[�W����v���r���[�̍쐬
//	2.	����
//		bmp		...	(I)	�u�q�`�l�C���[�W
//		hDC		...	(I)	�v���r���[�o�̓f�o�C�X
//		Mode	...	(I)	�摜�̐ݒ�
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
void DrawScreen( const unsigned char *bmp,HDC hDC,const SETTING *Mode )
{
	RECT	rc = { 0, 0, 512, 424 };

	FillRect( hDC, &rc, ( HBRUSH )GetStockObject( BLACK_BRUSH ) );

	switch( Mode->Mode ){
	case MD_SC2:
		DrawScreen2( bmp, hDC, Mode);	return;
	case MD_SC3:
		DrawScreen3( bmp, hDC, Mode);	return;
	case MD_SC5:
	case MD_SC5_256L:
		DrawScreen5( bmp, hDC, Mode );	return;
	case MD_SC6:
	case MD_SC6_256L:
		DrawScreen6( bmp, hDC, Mode );	return;
	case MD_SC7:
	case MD_SC7_256L:
		DrawScreen7( bmp, hDC, Mode );	return;
	case MD_SC8:
	case MD_SC8_256L:
		DrawScreen8( bmp, hDC, Mode );	return;
	case MD_SC10:
	case MD_SC12:
	case MD_SC10_256L:
	case MD_SC12_256L:
		DrawScreenC( bmp, hDC, Mode );	return;
	}
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�u�q�`�l�C���[�W����v���r��(SCREEN2)�̍쐬
//	2.	����
//		bmp	...	(I)	�u�q�`�l�C���[�W
//		hDC	...	(I)	�v���r���[�o�̓f�o�C�X
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static void DrawScreen2( const unsigned char *bmp,HDC hDC,const SETTING *Mode )
{
	int r[2],g[2],b[2];
	int x, y, adr, i, c, bit;

	for( y=0; y<384; y+=2 ){
		for( x=0; x<512; x+=16 ){
			adr=y/2;
			adr=( adr >> 3 )*256 + ( x / 16 )*8 + ( adr & 7 );
			// ���ʂS�r�b�g�i�o�b�N�O�����h�j
			r[0] = cc[ Mode->Col[ bmp[ adr + SC2COLOR ] & 0x0F ].red   ];
			g[0] = cc[ Mode->Col[ bmp[ adr + SC2COLOR ] & 0x0F ].green ];
			b[0] = cc[ Mode->Col[ bmp[ adr + SC2COLOR ] & 0x0F ].blue  ];
			// ��ʂS�r�b�g�i�t�H�A�O�����h�j
			r[1] = cc[ Mode->Col[ bmp[ adr + SC2COLOR ] >> 4 ].red   ];
			g[1] = cc[ Mode->Col[ bmp[ adr + SC2COLOR ] >> 4 ].green ];
			b[1] = cc[ Mode->Col[ bmp[ adr + SC2COLOR ] >> 4 ].blue  ];

			//	�W�h�b�g�y�A�̕`��
			c = bmp[ adr ];
			for( i = 0; i < 8; ++i ) {
				bit = ( c & 0x80 ) >> 7;
				SetPixel( hDC, x + i*2 + 0 , y + 20, RGB( r[ bit ],g[ bit ],b[ bit ] ) );
				SetPixel( hDC, x + i*2 + 1 , y + 20, RGB( r[ bit ],g[ bit ],b[ bit ] ) );
				SetPixel( hDC, x + i*2 + 0 , y + 21, RGB( r[ bit ],g[ bit ],b[ bit ] ) );
				SetPixel( hDC, x + i*2 + 1 , y + 21, RGB( r[ bit ],g[ bit ],b[ bit ] ) );
				c = c << 1;
			}

		}
	}
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�u�q�`�l�C���[�W����v���r��(SCREEN3)�̍쐬
//	2.	����
//		bmp	...	(I)	�u�q�`�l�C���[�W
//		hDC	...	(I)	�v���r���[�o�̓f�o�C�X
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static void DrawScreen3( const unsigned char *bmp,HDC hDC,const SETTING *Mode )
{
	int r,g,b;
	int x,y,adr,i;

	for( y=0; y<384; ++y ){
		for( x=0; x<512; x+=16 ){
			adr = y/8;
			adr = ( adr >> 3 )*256 + (x/16)*8 + (adr&7);
			// ��ʂS�r�b�g�i�����j
			r=cc[ Mode->Col[ bmp[ adr ] >> 4 ].red   ];
			g=cc[ Mode->Col[ bmp[ adr ] >> 4 ].green ];
			b=cc[ Mode->Col[ bmp[ adr ] >> 4 ].blue  ];
			for( i=0; i<8; ++i ) {
				SetPixel( hDC, x+i  , y+20, RGB( r,g,b ) );
			}
			// ���ʂS�r�b�g�i�E���j
			r=cc[ Mode->Col[ bmp[ adr ] & 0x0F ].red   ];
			g=cc[ Mode->Col[ bmp[ adr ] & 0x0F ].green ];
			b=cc[ Mode->Col[ bmp[ adr ] & 0x0F ].blue  ];
			for( i=0; i<8; ++i ) {
				SetPixel( hDC, x+i+8, y+20, RGB( r,g,b ) );
			}

		}
	}
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�u�q�`�l�C���[�W����v���r��(SCREEN5)�̍쐬
//	2.	����
//		bmp	...	(I)	�u�q�`�l�C���[�W
//		hDC	...	(I)	�v���r���[�o�̓f�o�C�X
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static void DrawScreen5( const unsigned char *bmp,HDC hDC,const SETTING *Mode )
{
	int r,g,b;
	int x,y,adr,t,h;

	h = _GetHeight( Mode );
	t = 212 - h;
	h = h * 2;

	for( y=0; y<h; ++y ){
		for( x=0; x<512; x+=4 ){
			if( Mode->Inter ) adr=y; else adr=y/2;
			adr=adr*128+x/4;
			// ��ʂS�r�b�g�i�����j
			r=cc[ Mode->Col[ bmp[ adr ] >> 4 ].red   ];
			g=cc[ Mode->Col[ bmp[ adr ] >> 4 ].green ];
			b=cc[ Mode->Col[ bmp[ adr ] >> 4 ].blue  ];
			SetPixel( hDC,x+0,y+t,RGB( r,g,b ) );
			SetPixel( hDC,x+1,y+t,RGB( r,g,b ) );
			// ���ʂS�r�b�g�i�E���j
			r=cc[ Mode->Col[ bmp[ adr ] & 0x0F ].red   ];
			g=cc[ Mode->Col[ bmp[ adr ] & 0x0F ].green ];
			b=cc[ Mode->Col[ bmp[ adr ] & 0x0F ].blue  ];
			SetPixel( hDC,x+2,y+t,RGB( r,g,b ) );
			SetPixel( hDC,x+3,y+t,RGB( r,g,b ) );
		}
	}
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�u�q�`�l�C���[�W����v���r��(SCREEN6)�̍쐬
//	2.	����
//		bmp	...	(I)	�u�q�`�l�C���[�W
//		hDC	...	(I)	�v���r���[�o�̓f�o�C�X
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static void DrawScreen6( const unsigned char *bmp, HDC hDC, const SETTING *Mode )
{
	int r,g,b;
	int x,y,adr,t,h;

	h = _GetHeight( Mode );
	t = 212 - h;
	h = h * 2;

	for( y=0; y<h; ++y ){
		for( x=0; x<512; x+=4 ){
			if( Mode->Inter ) adr=y; else adr=y/2;
			adr=adr*128+x/4;
			// ��ʂQ�r�b�g�i�����j
			r=cc[ Mode->Col[ (bmp[ adr ] >> 6) & 0x03 ].red   ];
			g=cc[ Mode->Col[ (bmp[ adr ] >> 6) & 0x03 ].green ];
			b=cc[ Mode->Col[ (bmp[ adr ] >> 6) & 0x03 ].blue  ];
			SetPixel( hDC, x+0, y+t, RGB( r, g, b ) );
			// ��ʂQ�r�b�g�i�������j
			r=cc[ Mode->Col[ (bmp[ adr ] >> 4) & 0x03 ].red   ];
			g=cc[ Mode->Col[ (bmp[ adr ] >> 4) & 0x03 ].green ];
			b=cc[ Mode->Col[ (bmp[ adr ] >> 4) & 0x03 ].blue  ];
			SetPixel( hDC, x+1, y+t, RGB( r, g, b ) );
			// ���ʂQ�r�b�g�i���E���j
			r=cc[ Mode->Col[ (bmp[ adr ] >> 2) & 0x03 ].red   ];
			g=cc[ Mode->Col[ (bmp[ adr ] >> 2) & 0x03 ].green ];
			b=cc[ Mode->Col[ (bmp[ adr ] >> 2) & 0x03 ].blue  ];
			SetPixel( hDC, x+2, y+t, RGB( r, g, b ) );
			// ���ʂQ�r�b�g�i�E���j
			r=cc[ Mode->Col[ (bmp[ adr ]     ) & 0x03 ].red   ];
			g=cc[ Mode->Col[ (bmp[ adr ]     ) & 0x03 ].green ];
			b=cc[ Mode->Col[ (bmp[ adr ]     ) & 0x03 ].blue  ];
			SetPixel( hDC, x+3, y+t, RGB( r, g, b ) );
		}
	}
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�u�q�`�l�C���[�W����v���r��(SCREEN7)�̍쐬
//	2.	����
//		bmp	...	(I)	�u�q�`�l�C���[�W
//		hDC	...	(I)	�v���r���[�o�̓f�o�C�X
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static void DrawScreen7( const unsigned char *bmp, HDC hDC, const SETTING *Mode )
{
	int r,g,b;
	int x,y,adr,t,h;

	h = _GetHeight( Mode );
	t = 212 - h;
	h = h * 2;

	for( y=0; y<h; ++y ){
		for( x=0; x<512; x+=2 ){
			if( Mode->Inter ) adr=y; else adr=y/2;
			adr=adr*256+x/2;
			// ��ʂS�r�b�g�i�����j
			r=cc[ Mode->Col[ bmp[ adr ] >> 4 ].red   ];
			g=cc[ Mode->Col[ bmp[ adr ] >> 4 ].green ];
			b=cc[ Mode->Col[ bmp[ adr ] >> 4 ].blue  ];
			SetPixel( hDC, x+0, y+t, RGB( r, g, b ) );
			// ���ʂS�r�b�g�i�E���j
			r=cc[ Mode->Col[ bmp[ adr ] & 0x0F ].red   ];
			g=cc[ Mode->Col[ bmp[ adr ] & 0x0F ].green ];
			b=cc[ Mode->Col[ bmp[ adr ] & 0x0F ].blue  ];
			SetPixel( hDC, x+1, y+t, RGB( r, g, b ) );
		}
	}
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�u�q�`�l�C���[�W����v���r��(SCREEN8)�̍쐬
//	2.	����
//		bmp	...	(I)	�u�q�`�l�C���[�W
//		hDC	...	(I)	�v���r���[�o�̓f�o�C�X
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static void DrawScreen8( const unsigned char *bmp, HDC hDC, const SETTING *Mode )
{
	int r,g,b;
	int x,y,adr,t,h;

	h = _GetHeight( Mode );
	t = 212 - h;
	h = h * 2;

	for( y = 0; y < h; ++y ){
		for( x = 0; x < 512; x += 2 ){
			if( Mode->Inter ) adr = y; else adr = y / 2;
			adr = adr * 256 + x / 2;
			r=cc[(bmp[ adr ] & 0x1C) >> 2 ];
			g=cc[(bmp[ adr ] & 0xE0) >> 5 ];
			b=cb[ bmp[ adr ] & 0x03       ];
			SetPixel( hDC, x + 0, y+t, RGB( r, g, b ) );
			SetPixel( hDC, x + 1, y+t, RGB( r, g, b ) );
		}
	}
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�u�q�`�l�C���[�W����v���r��(SCREEN10/12)�̍쐬
//	2.	����
//		bmp		...	(I)	�u�q�`�l�C���[�W
//		hDC		...	(I)	�v���r���[�o�͐�f�o�C�X
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		���u�q�`�l�C���[�W�͂l�r�w��̍\���Ƃ͈قȂ�
// -------------------------------------------------------------
static void DrawScreenC( const unsigned char *bmp, HDC hDC, const SETTING *Mode )
{
	int r,g,b,j,k,yy[4];
	int x,y,z,adr,t,h;

	h = _GetHeight( Mode );
	t = 212 - h;
	h = h * 2;

	for( y = 0; y < h; ++y ){
		for( x = 0; x < 512; x += 8 ){
			if( Mode->Inter ) adr = y; else adr = y / 2;
			adr = adr * 256 + x / 2;
			//	�P�x�����̎擾
			for( z=0; z<4; ++z ){
				yy[ z ] = ( bmp[ adr + z ] & 0xF8 ) >> 3;
			}
			//	�F�����̎擾
			k = ( bmp[ adr + 0 ] & 0x07 ) | (( bmp[ adr + 1 ] & 0x07 ) << 3 );
			if( k > 31 ) k = k - 64;	//	5bit �̂Q�̕���
			j = ( bmp[ adr + 2 ] & 0x07 ) | (( bmp[ adr + 3 ] & 0x07 ) << 3 );
			if( j > 31 ) j = j - 64;	//	5bit �̂Q�̕���
			// �`��
			for( z = 0; z < 4; ++z ){
				r = AdjustNum( yy[ z ] + j							, 0, 31 )* 255 / 31;
				g = AdjustNum( yy[ z ] + k							, 0, 31 )* 255 / 31;
				b = AdjustNum( 5 * yy[ z ] / 4 - j / 2 - k / 4		, 0, 31 )* 255 / 31;
				if( r == 246 && g == 246 && b == 255 ) {
					r = r;
				}
				SetPixel( hDC, x + z * 2 + 0, y+t, RGB( r, g, b) );
				SetPixel( hDC, x + z * 2 + 1, y+t, RGB( r, g, b) );
			}
		}
	}
}

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
static inline int AdjustNum( int n, int min, int max )
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

	DrawScreen( bmp, hDC, Mode );
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
