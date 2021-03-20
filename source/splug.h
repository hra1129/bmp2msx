// -------------------------------------------------------------
//	susie plug in �C���^�[�t�F�[�X���[�`��
//	(C)2001 t.hara
// -------------------------------------------------------------

#ifndef	_SPLUG_H_

// -------------------------------------------------------------
//	�C���N���[�h

#include <windows.h>

// -------------------------------------------------------------
//	�ݒ�

#define	SPI_COUNT	64		//	�ő�v���O�C����
#define	SPI_MAXLEN	3120	//	�t�B���^�E�`�����o�b�t�@�̃T�C�Y

// -------------------------------------------------------------
//	�^

typedef struct {
	char	name[ MAX_PATH ];
} SPINAME;

typedef	int ( WINAPI *SPI_GetPluginInfo )( int infono, LPSTR buf, int buflen );
typedef int ( WINAPI *SPI_IsSupported )( LPSTR filename, DWORD dw );
typedef int ( WINAPI *SPI_GetPicture )( LPSTR buf, long len, unsigned int flag, HANDLE *pHBInfo, HANDLE *pHBm, FARPROC lpPrgressCallback, long lData );

typedef struct {
	HINSTANCE			hSpi;
	SPI_GetPluginInfo	GetPluginInfo;
	SPI_IsSupported		IsSupported;
	SPI_GetPicture		GetPicture;
} SPIIF;

// -------------------------------------------------------------
//	�֐��v���g�^�C�v

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�v���O�C���̔F��
//	2.	����
//		s_path	...	(I)	PlugIn �t�H���_�̑��݂���p�X�ɂ���t�@�C���̃p�X
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
void spi_initialize( const char* s_path );

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�v���O�C���ɃA�^�b�`����
//	2.	����
//		s_path	...	(I)	�v���O�C���t�@�C����
//		p_if	...	(I)	�C���^�[�t�F�[�X�ϐ��̃A�h���X
//	3.	�Ԓl
//		true	...	����
//		false	...	���s
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
bool spi_load( const char* s_path, SPIIF *p_if );

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�v���O�C�����f�^�b�`����
//	2.	����
//		p_if	...	(O)	�C���^�[�t�F�[�X�ϐ��̃A�h���X
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
void spi_release( SPIIF *p_if );

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�t�B���^��Ԃ�
//	2.	����
//		�Ȃ�
//	3.	�Ԓl
//		�t�B���^������
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
const char* spi_get_filter( void );

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�摜��ǂݍ���
//	2.	����
//		cszFileName			...	(I)	�t�@�C����
//		ProgressCallback	...	(I)	�o�ߕ\���֐�
//	3.	�Ԓl
//		!NULL	...	BMP�t�@�C���C���[�W�|�C���^
//		NULL	...	���s
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
char* spi_load_image( const char* cszFileName, FARPROC ProgressCallback );

#endif	//	_SPLUG_H_
