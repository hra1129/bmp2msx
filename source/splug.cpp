// -------------------------------------------------------------
//	susie plug in �C���^�[�t�F�[�X���[�`��
//	(C)2001 t.hara
// -------------------------------------------------------------

// -------------------------------------------------------------
//	�C���N���[�h

#include <windows.h>
#include <string.h>
#include "splug.h"

// -------------------------------------------------------------
//	�ݒ�

// -------------------------------------------------------------
//	�O���[�o��

static int		spicount = 0;				//	�Ή��`���F����
static SPINAME	spiname[ SPI_COUNT ];		//	�v���O�C���t�@�C����
static char		spifilter[ SPI_MAXLEN ];	//	�t�@�C���t�B���^�ꗗ

// -------------------------------------------------------------
//	�֐��v���g�^�C�v

static bool spi_add_string( char* buf, const char* str, int size );

// -------------------------------------------------------------
//	�֐���`

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
void spi_initialize( const char* s_path )
{
	char				path[ MAX_PATH ], *ptr;
	HANDLE				hFind;
	WIN32_FIND_DATA		ff;
	SPIIF				spiif;
	BOOL				bOut;
	char				s_filter[ 128 ];
	char				s_type[ 128 ];
	char				s_folder[ MAX_PATH ];
	int					i;

	//	�ϐ��̃N���A
	spicount = 0;
	memset( spiname, 0, sizeof( spiname ) );
	memset( spifilter, 0, sizeof( spifilter ) );

	//	�v���O�C���t�H���_�p�X�̍쐬
	strcpy( path, s_path );
	ptr = strrchr( path, '\\' );
	if( ptr == NULL ) return;
	*ptr = '\0';
	strcpy( s_folder, path );
	strcpy( ptr, "\\*.spi" );

	//	����
	memset( &ff, 0, sizeof( ff ) );
	ff.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
	hFind = FindFirstFile( path, &ff );
	if( hFind == NULL ) return;

	do {
		bOut = TRUE;
		//	�A�^�b�`
		wsprintf( path, "%s\\%s", s_folder, ff.cFileName );
		if( spi_load( path, &spiif ) ) {

			//	�Ή��`����S�ēo�^
			i = 0;
			do {
				if( (spiif.GetPluginInfo( 2*i+2, s_filter, sizeof( s_filter	) ) == 0) ||
					(spiif.GetPluginInfo( 2*i+3, s_type	, sizeof( s_type	) ) == 0) ) break;
				s_filter[ sizeof( s_filter ) - 1 ] = '\0';
				s_type[ sizeof( s_type ) - 1 ] = '\0';
				if( !spi_add_string( spifilter, s_type	, sizeof( spifilter	) ) || 
					!spi_add_string( spifilter, s_filter, sizeof( spifilter	) ) ) break;
				if( i == 0 ) strcpy( spiname[ spicount ].name, path );
				++spicount;
				++i;
			} while( spicount < SPI_COUNT );

			//	�f�^�b�`
			spi_release( &spiif );
		}

		//	���̃t�@�C��
		bOut = bOut && FindNextFile( hFind, &ff );
	} while( bOut && (spicount < SPI_COUNT) );
}

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
bool spi_load( const char* s_path, SPIIF *p_if )
{
	char	s_buf[8];

	//	�v���O�C���̓ǂݍ���
	p_if->hSpi = LoadLibrary( s_path );
	if( p_if->hSpi == NULL ) return false;	//	DLL ����Ȃ�

	//	�֐��A�h���X�̎擾
	p_if->GetPluginInfo		= (SPI_GetPluginInfo) GetProcAddress( p_if->hSpi, "GetPluginInfo" );
	p_if->GetPicture		= (SPI_GetPicture)	GetProcAddress( p_if->hSpi, "GetPicture" );
	p_if->IsSupported		= (SPI_IsSupported)	GetProcAddress( p_if->hSpi, "IsSupported" );

	//	�G���[�`�F�b�N
	if( p_if->GetPicture == NULL || p_if->GetPluginInfo == NULL || p_if->IsSupported == NULL ) {
		FreeLibrary( p_if->hSpi );
		p_if->hSpi = NULL;
		return false;
	}

	//	�o�[�W�����`�F�b�N
	if( (p_if->GetPluginInfo)( 0, s_buf, sizeof( s_buf ) ) < 4 ) {
		FreeLibrary( p_if->hSpi );
		p_if->hSpi = NULL;
		return false;
	}
	if( strcmp( s_buf, "00IN" ) ) {
		FreeLibrary( p_if->hSpi );
		p_if->hSpi = NULL;
		return false;
	}

	return true;
}

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
void spi_release( SPIIF *p_if )
{
	if( p_if->hSpi != NULL ) {
		FreeLibrary( p_if->hSpi );
	}

	memset( p_if, 0, sizeof( SPIIF ) );
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		������̒ǉ�
//	2.	����
//		buf		...	(O)	�ǉ���
//		str		...	(I)	�ǉ����镶����
//		size	...	(I)	buf �̃T�C�Y
//	3.	�Ԓl
//		true	...	����
//		false	...	���s
//	4.	���l
//		buf �̖��g�p�����͂��炩���� 0 �Ŗ��߂Ă�������
// -------------------------------------------------------------
static bool spi_add_string( char* buf, const char* str, int size )
{
	char	*ptr;

	//	buf �̐K�������o����
	ptr = buf;
	--size;		//	�Ō�� '\0' �̕�
	while( *ptr != '\0' ) {
		size -= strlen( ptr ) + 1;
		ptr  += strlen( ptr ) + 1;
	}

	//	�������ǉ�
	if( (signed)strlen( str ) > size ) return false;
	strcpy( ptr, str );
	return true;
}

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
const char* spi_get_filter( void )
{
	return spifilter;
}

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
char* spi_load_image( const char* cszFileName, FARPROC ProgressCallback )
{
	int					i, n, w;
	SPIIF				sif;
	HLOCAL				pBmp, pInfo;
	char*				pImage;
	BITMAPINFOHEADER	*bi;
	int					c;

	//	�v���O�C�������ԂɌĂяo��
	pBmp		= NULL;
	pInfo		= NULL;
	sif.hSpi	= NULL;
	for( i = 0; i < spicount; ++i ) {
		//	��
		if( spiname[i].name[0] == '\0' ) continue;
		//	�v���O�C�����A�^�b�`
		if( !spi_load( spiname[i].name, &sif ) ) continue;
		//	�摜��ǂݍ���
		n = (sif.GetPicture)( (LPSTR)cszFileName, 0, 0, &pInfo, &pBmp, ProgressCallback, 0 );
		//	�ǂݍ��߂�������
		if( n == 0 ) break;
		//	�v���O�C�����f�^�b�`
		spi_release( &sif );
	}

	//	�ǂݍ��߂Ȃ������ꍇ
	if( pBmp == NULL || pInfo == NULL ) {
		if( pInfo != NULL ) LocalFree( pInfo );
		if( pBmp  != NULL ) LocalFree( pBmp  );
		spi_release( &sif );
		return NULL;
	}

	//	�ǂݍ��߂��ꍇ
	bi = (BITMAPINFOHEADER*) LocalLock( pInfo );

	//	�摜�̕��ɑΉ�����o�C�g�� �� RGBQUAD �̐�
	switch( bi->biBitCount ) {
	case 1:
		w = ( (bi->biWidth + 7) / 8 + 3 ) & ~3;
		c = 2;		//	�p���b�g
		break;
	case 4:
		w = ( (bi->biWidth + 1) / 2 + 3 ) & ~3;
		c = 16;		//	�p���b�g
		break;
	case 8:
		w = ( bi->biWidth + 3 ) & ~3;
		c = 256;	//	�p���b�g
		break;
	case 16:
		w = ( bi->biWidth * 2 + 3 ) & ~3;
		c = 3;		//	�r�b�g�}�X�N
		break;
	case 24:
		w = ( bi->biWidth * 3 + 3 ) & ~3;
		c = 0;		//	�Ȃ�
		break;
	case 32:
		w = bi->biWidth * 4;
		c = 3;		//	�r�b�g�}�X�N
		break;
	}
	c = bi->biSize + c * sizeof( RGBQUAD );

	//	���������m��
	pImage = (char*) LocalAlloc( LPTR, c + w * bi->biHeight );
	if( pImage == NULL ) {
		LocalFree( pInfo );
		LocalFree( pBmp  );
		spi_release( &sif );
		return NULL;
	}

	//	�������փR�s�[
	memcpy( pImage    , bi, c );
	memcpy( pImage + c, LocalLock( pBmp ), w * bi->biHeight );

	//	�I��
	LocalFree( pInfo );
	LocalFree( pBmp  );
	spi_release( &sif );
	return pImage;
}
