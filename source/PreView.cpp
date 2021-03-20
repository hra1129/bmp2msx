// -------------------------------------------------------------
//	TITLE	:	BMP to MSX �v���r���[�Ǘ��֘A
//	INPUT	:	HRA!
// -------------------------------------------------------------

// =============================================================
//	�C���N���[�h

#include <windows.h>
#include <windowsx.h>

#include "config.h"
#include "convert.h"
#include "PreView.h"
#include "ProgProc.h"
#include "splug.h"
#include "fileuty.h"

// =============================================================
//	�֐��v���g�^�C�v

static int _bdAddFile( const char *cszFileName );
static int WINAPI ProgressCallback( int nNum, int nDenom, long lData );

// =============================================================
//	�ϐ�

BMPDATA	tBmpview[ PV_MAX ];		// �a�l�o�t�@�C�����
PVDATA	tPreview[ PV_MAX ];		// �v���r���[���
int		pvIndex = -1;			// ���ݕ\�����̃v���r���[
int		nTime	= 0;			// �v���r���[�o�^�����i�Â��̌����j

// =====================================================
// �v���r���[�֘A

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�v���r���[�ϐ��̏�����
//	2.	����
//		�Ȃ�
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
void pvInitialize( void )
{
	ZeroMemory( tPreview, sizeof( tPreview ) );
	ZeroMemory( tBmpview, sizeof( tBmpview ) );
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		���g�p�̃v���r���[����������
//	2.	����
//		�Ȃ�
//	3.	�Ԓl
//		-1	...	�󂫖���
//		n	...	�󂫃v���r���[�ւ̃C���f�b�N�X
//	4.	���l
//		�ʏ�A�󂫂������Ȃ��Ă����ꍇ�͂����Ƃ��Â�
//		�v���r���[���폜���ċ󂫂��쐬����B����������
//		-1 ���Ԃ�͓̂���ȏꍇ�̂݁B
// -----------------------------------------------------
int pvBlankIndex( void )
{
	int i, j, p;
	j = 0x7FFFFFFF;
	for( i=0; i<PV_MAX; ++i ){
		if( tPreview[i].hBmp == NULL ) return i;
		if( tPreview[i].nTime < j ) {
			j = tPreview[i].nTime;	//	�ł��ӂ邢�v���r��
			p = i;
		}
	}
	//	�󂫂������ꍇ�͍ł��ӂ邢���̂��폜����
	if( j == 0x7FFFFFFF ) return -1;						//	�v���r���[�f�[�^�����Ă�ꍇ or �Q�O�����ȏ�ϊ������ꍇ (^_^;
	pvDelete( p );
	return p;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�w��̃v���r���[���폜����
//	2.	����
//		Index	...	(I)	�폜�Ώۂ̃C���f�b�N�X
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
void pvDelete( int Index )
{
	if( tPreview[Index].hBmp == NULL ) return;
	DeleteObject( tPreview[Index].hBmpSrc );
	DeleteObject( tPreview[Index].hBmp );
	LocalFree( tPreview[Index].bmp );
	tPreview[Index].hBmpSrc = NULL;
	tPreview[Index].hBmp = NULL;
	tPreview[Index].bmp = NULL;
	tPreview[Index].nTime = 0x7FFFFFFF;
	bdDelete(tPreview[Index].nSrc);
	if( pvIndex == Index ) pvIndex = pvNextPreview( pvIndex );
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�S�v���r���[���폜����
//	2.	����
//		�Ȃ�
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
void pvAllDelete( void )
{
	int i;
	for( i=0; i<PV_MAX; ++i ){
		pvDelete( i );
	}
	nTime = 0;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		���̃v���r���[����������
//	2.	����
//		Index	...	(I)	���݂̃C���f�b�N�X
//	3.	�Ԓl
//		-1	...	�v���r���[�������
//		n	...	���̃C���f�b�N�X
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
int pvNextPreview( int Index )
{
	int i;
	Index=(Index+1)%PV_MAX;
	for( i=0; i<PV_MAX; ++i,Index=(Index+1)%PV_MAX ){
		if( tPreview[Index].hBmp != NULL ) return Index;
	}
	return -1;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�O�̃v���r���[����������
//	2.	����
//		Index	...	(I)	���݂̃C���f�b�N�X
//	3.	�Ԓl
//		-1	...	�v���r���[�������
//		n	...	�O�̃C���f�b�N�X
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
int pvPrevPreview( int Index )
{
	int i;
	Index=(Index-1+PV_MAX)%PV_MAX;
	for( i=0; i<PV_MAX; ++i,Index=(Index-1+PV_MAX)%PV_MAX ){
		if( tPreview[Index].hBmp != NULL ) return Index;
	}
	return -1;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�v���r���[�Ƀr�b�g�}�b�v��ǉ�����
//	2.	����
//		Index	...	(I)	�󂫃v���r���[�̃C���f�b�N�X
//	3.	�Ԓl
//		true	...	����
//		false	...	���s
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
bool pvAddPreview( int Index )
{
	HDC hDC;
	if( tPreview[ Index ].hBmp != NULL || Index<0 ) return false;
	hDC = GetDC( NULL );
	tPreview[ Index ].hBmp    = CreateCompatibleBitmap( hDC, cnMAXWidth, cnMAXHeight );
	tPreview[ Index ].hBmpSrc = CreateCompatibleBitmap( hDC, cnMAXWidth, cnMAXHeight );
	ReleaseDC( NULL, hDC );
	if( tPreview[ Index ].hBmp    == NULL ||
		tPreview[ Index ].hBmpSrc == NULL ){	// �쐬�Ɏ��s�����烁������������ăG���[��Ԃ�
		if( tPreview[ Index ].hBmp    != NULL ) DeleteObject( tPreview[ Index ].hBmp    );
		if( tPreview[ Index ].hBmpSrc != NULL ) DeleteObject( tPreview[ Index ].hBmpSrc );
		tPreview[ Index ].hBmp    = NULL;
		tPreview[ Index ].hBmpSrc = NULL;
		tPreview[ Index ].nTime   = 0x7FFFFFFF;
		return false;
	}
	tPreview[ Index ].nTime = nTime;
	if( nTime < 0x7FFFFFFF ) ++nTime;
	return true;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�v���r���[�i�ϊ����j�Ƀr�b�g�}�b�v��`�悷��
//	2.	����
//		hDC		...	(I)	�������f�o�C�X�i�ꎞ�I�Ɏg�p����j
//		Index	...	(I)	�v���r���[�̃C���f�b�N�X
//		in		...	(I)	�摜�C���[�W
//		width	...	(I)	�摜�C���[�W�̕�
//		height	...	(I)	�摜�C���[�W�̍���
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
void pvDrawBmp( HDC hDC, int Index, COLORREF *in, int width, int height )
{
	RECT	r = { 0, 0, cnMAXWidth, cnMAXHeight };
	int x,y,t,h;
	COLORREF c;

	HBITMAP hMemBM = SelectBitmap( hDC,tPreview[Index].hBmpSrc );	// �ꎞ�I�ɉ��z��ʗp�������c�b���g�p����

	if( (height < 212) || (height == 384) ) {
		t = 212 - 192;
		h = cnMAXHeight - t*2;
		FillRect( hDC, &r, (HBRUSH)GetStockObject( BLACK_BRUSH ) );
		for( y=0; y<h; ++y ){
			for( x=0; x<cnMAXWidth; ++x ){
				c=*( in + x*width/cnMAXWidth + (y*height/h)*width );
				SetPixel( hDC,x,t+y,c );
			}
		}
	} else {
		for( y=0; y<cnMAXHeight; ++y ){
			for( x=0; x<cnMAXWidth; ++x ){
				c=*( in + x*width/cnMAXWidth + (y*height/cnMAXHeight)*width );
				SetPixel( hDC,x,y,c );
			}
		}
	}
	SelectBitmap( hDC,hMemBM );							// ���ɖ߂�
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�v���r���[�ԍ���A�Ԃ֕ϊ�����
//	2.	����
//		Index	...	(I)	�v���r���[�̃C���f�b�N�X
//	3.	�Ԓl
//		�Ή�����ԍ�
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
int pvGetNum( int Index )
{
	int i,r;
	if( Index==-1 ) return -1;
	r=0;
	for( i=0; i<=Index; ++i ){
		if( tPreview[i].bmp != NULL ) ++r;
	}
	return r;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�v���r���[�̓o�^�����擾����
//	2.	����
//		�Ȃ�
//	3.	�Ԓl
//		�o�^��
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
int pvGetCnt( void )
{
	int i,r;
	r=0;
	for( i=0; i<PV_MAX; ++i ){
		if( tPreview[i].bmp != NULL ) ++r;
	}
	return r;
}

// =====================================================
// �摜�t�@�C���C���[�W�֘A

// -----------------------------------------------------
//	1.	���{�ꖼ
//		���g�p�̉摜�t�@�C���C���[�W����������
//	2.	����
//		�Ȃ�
//	3.	�Ԓl
//		-1	...	�󂫖���
//		n	...	�󂫃v���r���[�̃C���f�b�N�X
//	4.	���l
//		�Ȃ�
// -----------------------------------------------------
int bdBlankIndex( void )
{
	int i;

	//	�v���r���[�ɋ󂫂�������΋󂫂����
	pvBlankIndex();

	//	�󂫂���������
	for( i=0; i<PV_MAX; ++i ){
		if( tBmpview[i].n == 0 ) return i;
	}
	return -1;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�w��̉摜�t�@�C���C���[�W���폜����
//	2.	����
//		Index	...	(I)	�폜�Ώۂ̉摜�C���f�b�N�X
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�摜�����݂��Ȃ��Ă����Ȃ��B
//		�i�A���A�摜�Ǘ��z��͏������ς݂̏ꍇ�Ɍ���j
// -----------------------------------------------------
void bdDelete( int Index )
{
	if( tBmpview[Index].n == 0 ) return;
	if( (--tBmpview[Index].n)==0 ){
		LocalFree( tBmpview[Index].image );
	}
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		BMP�t�@�C�����摜�t�@�C���C���[�W�ɒǉ�����
//	2.	����
//		cszFileName	...	(I)	�t�@�C����
//	3.	�Ԓl
//		-1	...	�ǉ��Ɏ��s
//		n	...	�t�@�C���C���[�W�̃C���f�b�N�X
//	4.	���l
//		�摜�t�@�C���C���[�W�̎Q�ƃJ�E���^�� 1 �ɂȂ�
// -----------------------------------------------------
static int _bdAddFile( const char *cszFileName ) {
	BITMAPFILEHEADER	bfh;
	int					nSize;
	FU_FILE				*hFile;
	int					nIndex;
	char				szBuf[ MAX_PATH ];

	// �摜�t�@�C���C���[�W�̋󂫂�����
	nIndex = bdBlankIndex();
	if( nIndex == -1 ) return -1;
	// �t�@�C�����J��
	hFile = fu_open( cszFileName, "rb" );
	if( hFile == FU_INVALID_HANDLE ) {
		wsprintf( szBuf,"%s ���J���܂���B",cszFileName );
		MessageBox( NULL,szBuf,MsgCap,MB_OK | MB_ICONWARNING );
		return -1;	// �t�@�C���I�[�v�����s
	}
	// �t�@�C���T�C�Y
	nSize = (int)fu_length( hFile )-sizeof(bfh);
	if( nSize < sizeof(BITMAPINFOHEADER) ) goto exit;

		// �t�@�C���w�b�_��ǂݍ���
	if( !fu_read( hFile, &bfh, sizeof(bfh) ) ) goto exit;

	// �w�b�_���m�F����
	if( bfh.bfType != 'MB' ) goto exit;

	// ���������m�ۂ���
	tBmpview[ nIndex ].image = (char*)LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT,nSize );
	if( tBmpview[ nIndex ].image==NULL ) goto exit;

	// �t�@�C����ǂݍ���
	fu_read( hFile, tBmpview[ nIndex ].image, nSize );
	// �I��
	tBmpview[ nIndex ].n = 1;
	lstrcpy( tBmpview[ nIndex ].sFileName, cszFileName );
	fu_close( hFile );
	return nIndex;
exit:
	fu_close( hFile );
	return -1;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		�摜�t�@�C�����摜�t�@�C���C���[�W�ɒǉ�����
//	2.	����
//		cszFileName	...	(I)	�t�@�C����
//	3.	�Ԓl
//		-1	...	�ǉ��Ɏ��s
//		n	...	�t�@�C���C���[�W�̃C���f�b�N�X
//	4.	���l
//		�摜�t�@�C���C���[�W�̎Q�ƃJ�E���^�� 1 �ɂȂ�
// -----------------------------------------------------
int bdAddFile( const char *cszFileName )
{
	int		nIndex;
	bool	bProg = false;
	char	szBuf[ MAX_PATH ];

	//	BMP�t�@�C���Ƃ��ēǂݍ���ł݂�
	nIndex = _bdAddFile( cszFileName );
	if( nIndex > -1 ) return nIndex;

	// �摜�t�@�C���C���[�W�̋󂫂�����
	nIndex = bdBlankIndex();
	if( nIndex == -1 ) return -1;

	//	�o�ߕ\���p�_�C�A���O�\��
	GetFileTitle( cszFileName, szBuf, sizeof( szBuf ) );
	prShow( szBuf );

	//	�v���O�C���ɓǂݍ��܂���
	tBmpview[ nIndex ].image = spi_load_image( cszFileName, (FARPROC)ProgressCallback );

	//	�o�ߕ\���p�_�C�A���O����
	prDestroy();

	//	�G���[�`�F�b�N
	if( tBmpview[ nIndex ].image == NULL ) return -1;

	// �I��
	tBmpview[ nIndex ].n = 1;
	lstrcpy( tBmpview[ nIndex ].sFileName , cszFileName );

	return nIndex;
}

// -----------------------------------------------------
//	1.	���{�ꖼ
//		BMP�t�@�C���C���[�W���摜�t�@�C���C���[�W�ɒǉ�����
//	2.	����
// 		bmp	...	(I)	�a�l�o�t�@�C���C���[�W
// 3.	�Ԓl
//		-1	...	�ǉ��Ɏ��s
//		n	...	�t�@�C���C���[�W�̃C���f�b�N�X
// 4.	���l
//		�摜�t�@�C���C���[�W�̎Q�ƃJ�E���^�� 1 �ɂȂ�
// -----------------------------------------------------
int bdAddImage( char *bmp )
{
	int					nIndex;
	// �摜�t�@�C���C���[�W�̋󂫂�����
	nIndex = bdBlankIndex();
	if( nIndex == -1 ) return -1;
	// ���������L������
	tBmpview[ nIndex ].image = bmp;
	// �I��
	tBmpview[ nIndex ].n = 1;
	tBmpview[ nIndex ].sFileName[0]='\0';
	return nIndex;
}

// =====================================================
//	�R�[���o�b�N

// -----------------------------------------------------
//	1.	���{�ꖼ
//		Susie PlugIn �p�̌o�ߕ\���֐�
//	2.	����
//		nNum	...	(I)	�o��
//		nDenom	...	(I)	nNum �̏I���l
//		lData	...	(I)	�A�v����`�̒l�i���g�p�j
//	3.	�Ԓl
//		0	...	���s
//		!0	...	���~
//	4.	���l
//		prProg ���Ăяo�� I/F �����֐�
// -----------------------------------------------------
static int WINAPI ProgressCallback( int nNum, int nDenom, long lData )
{
	if( nDenom == 0 ) return 0;
	prProg( nNum * 100 / nDenom );
	return 0;
}
