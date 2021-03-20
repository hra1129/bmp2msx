// -------------------------------------------------------------
//	TITLE	:	BMP to MSX �v���r���[�Ǘ��֘A
//	INPUT	:	HRA!
// -------------------------------------------------------------

#ifndef	_PREVIEW_H_
#define	_PREVIEW_H_

#include <windows.h>

// =============================================================
//	�^

// �I���W�i���摜
typedef struct {
	char*	image;					// �摜�t�@�C���C���[�W
	char	sFileName[ MAX_PATH ];	// �ϊ����̃t�@�C�����i�N���b�v�{�[�h�̏ꍇ�� "" �������j
	int		n;						// �Q�ƃJ�E���^
} BMPDATA;

// �v���r���[
typedef struct {
	HBITMAP		hBmp;					// �v���r���[�摜�i�f�B�X�e�B�l�[�V�����j
	HBITMAP		hBmpSrc;				// �v���r���[�摜�i�\�[�X�j
	int			nSrc;					// �I���W�i���摜�̃C���f�b�N�X
	int			nTime;					// �o�^�A�ԁi�Â��̌����j
	SETTING		tMode;					// �v���r���[�摜�������̐ݒ�
	unsigned char*	bmp;				// �ϊ����ʂ̂u�q�`�l�C���[�W
} PVDATA;

// =============================================================
//	���J�֐�

void pvInitialize( void );
int pvBlankIndex( void );
void pvDelete( int Index );
void pvAllDelete( void );
int pvNextPreview( int Index );
int pvPrevPreview( int Index );
bool pvAddPreview( int Index );
void pvDrawBmp( HDC hDC, int Index, COLORREF *in, int width, int height );
int pvGetNum( int Index );
int pvGetCnt( void );

int bdBlankIndex( void );
void bdDelete( int Index );
int bdAddFile( const char *cszFileName );
int bdAddImage( char *bmp );

// =============================================================
//	���J�ϐ�

extern BMPDATA	tBmpview[ PV_MAX ];		// �a�l�o�t�@�C�����
extern PVDATA	tPreview[ PV_MAX ];		// �v���r���[���
extern int		pvIndex;				// ���ݕ\�����̃v���r���[
extern int		nTime;					// �v���r���[�o�^�����i�Â��̌����j

#endif	//	_PREVIEW_H_
