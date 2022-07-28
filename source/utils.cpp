// -------------------------------------------------------------
// BMP to MSX
// �e�탆�[�e�B���e�B
//												(C)2000 HRA!
// -------------------------------------------------------------

#include "utils.h"
#include "fileuty.h"

extern HINSTANCE hIns;

// MSX1�̐F�Ɏ������p���b�g�l  { G, R, B }
static const PAL msx1_palette[]={
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 5, 3, 3 },
	{ 6, 4, 4 },
	{ 2, 3, 7 },
	{ 3, 4, 7 },
	{ 3, 5, 2 },
	{ 5, 3, 6 },
	{ 3, 6, 2 },
	{ 4, 6, 3 },
	{ 6, 5, 3 },
	{ 6, 6, 4 },
	{ 4, 2, 1 },
	{ 3, 5, 5 },
	{ 5, 5, 5 },
	{ 7, 7, 7 },
};

// MSX2�f�t�H���g�p���b�g�iMSX2�d���������p���b�g�l�j
static const PAL msx2_palette[] = {
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 6, 1, 1 },
	{ 7, 3, 3 },
	{ 1, 1, 7 },
	{ 3, 2, 7 },
	{ 1, 5, 1 },
	{ 6, 2, 7 },
	{ 1, 7, 1 },
	{ 3, 7, 3 },
	{ 6, 6, 1 },
	{ 6, 6, 4 },
	{ 4, 1, 1 },
	{ 2, 6, 5 },
	{ 5, 5, 5 },
	{ 7, 7, 7 },
};

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

// =====================================================
// �t�@�C�����֘A

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�t�@�C��������͂���
//	2.	����
//		szFileName	...	�t�@�C�����i�[�A�h���X
//		size		...	��L�������̃T�C�Y
//		szTitle		...	�^�C�g��
//		szFilter	...	�t�B���^
//		szExt		...	�f�t�H���g�g���q
//	3.	�Ԓl
//			OK = true
//			�L�����Z�� = false
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
bool GetName( HWND hWnd,char *szFileName,int size,const char *szTitle,const char *szFilter, const char* szExt )
{
	OPENFILENAME	ofn;
	// �t�@�C���������
	szFileName[0]='\0';
	ZeroMemory( &ofn,sizeof( ofn ) );
	ofn.lStructSize		= sizeof( ofn );
	ofn.hInstance		= hIns;
	ofn.hwndOwner		= hWnd;
	ofn.lpstrFilter		= szFilter;
	ofn.lpstrCustomFilter= NULL;
	ofn.nFilterIndex	= 1;
	ofn.Flags			= OFN_LONGNAMES | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
	ofn.lpstrFile		= szFileName;
	ofn.nMaxFile		= size;
	ofn.lpstrFileTitle	= NULL;
	ofn.lpstrInitialDir	= NULL;	// �J�����g
	ofn.lpstrTitle		= szTitle;
	ofn.lpstrDefExt		= szExt;
	return( GetSaveFileName( &ofn )!=FALSE );
}

// =====================================================
// �a�l�o�t�@�C���֘A

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		BMP�t�@�C����ǂݍ���
//	2.	����
//			out		...	�ǂݍ��񂾃A�h���X���i�[����ϐ��̃A�h���X
//			ptr		...	���������[�h�̃|�C���^
//	3.	�Ԓl
//			���� true
//			���s false
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
bool GetBmp( COLORREF **out,int *width,int *height,char **ptr )
{
	BITMAPINFOHEADER	bih;
	// �C���t�H�w�b�_��ǂݍ���
	memcpy( &bih,*ptr,sizeof( bih ) );
	*ptr += sizeof( bih );
	// �t�@�C���̎�ނ𒲂ׂ�
	if( bih.biCompression!=BI_RGB &&
		bih.biCompression!=BI_BITFIELDS ) return false;			// �Ή��`���i�񈳏k�j
	*out = (COLORREF*)LocalAlloc( LMEM_FIXED,sizeof( COLORREF )*bih.biWidth * bih.biHeight + 1 );
	if( *out == NULL ) return false;							// �������m�ێ��s
	// �T�C�Y�̎擾
	*width  = bih.biWidth;
	*height = bih.biHeight;
	// �ϊ�( BMP�t�@�C���`����COLORREF�z�� )
	switch( bih.biBitCount ){
	case 1:		//2�F�p���b�g
		if( !GetBmp1(  &bih,ptr,*out ) ){
			goto _error_exit;
		}
		break;
	case 4:		//16�F�p���b�g
		if( !GetBmp4(  &bih,ptr,*out ) ){
			goto _error_exit;
		}
		break;
	case 8:		//256�F�p���b�g
		if( !GetBmp8(  &bih,ptr,*out ) ){
			goto _error_exit;
		}
		break;
	case 16:	// 16bit�n�C�J���[
		if( !GetBmp16( &bih,ptr,*out ) ){
			goto _error_exit;
		}
		break;
	case 24:	// 24bit�t���J���[
		if( !GetBmp24( &bih,ptr,*out ) ){
			goto _error_exit;
		}
		break;
	case 32:	// 24bit+8bit�t���J���[
		if( !GetBmp32( &bih,ptr,*out ) ){
			goto _error_exit;
		}
		break;
	default:	// ��Ή��̃r�b�g��
		goto _error_exit;
	}
	return true;

	//	�G���[�̏ꍇ
_error_exit:
	LocalFree(*out);
	*out=NULL;
	return false;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		1bit BMP�t�@�C����ǂݍ���ŕϊ�����
//	2.	����
//			bih		...	�r�b�g�}�b�v�C���t�H�w�b�_�̃A�h���X
//			out		...	�o�͐惁�����̃A�h���X
//	3.	�Ԓl
//			���� true / ���s false
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
bool GetBmp1( BITMAPINFOHEADER *bih,char **ptr,COLORREF *pout )
{
	RGBQUAD		p[2];		// �p���b�g���
	int			i,w,x,y;	// �G�p�A���W
	BYTE		c;			// �ǂݍ��񂾃f�[�^
	BYTE		*pinp;		// ���̓|�C���^
	// �p���b�g��ǂݍ���
	w=( (bih->biWidth+7)/8 +3) & 0xFFFFFFFC;
	if( !(bih->biClrUsed) ){ i=2; }else{ i=bih->biClrUsed; }
	memcpy( p,*ptr,sizeof( RGBQUAD )*i );
	*ptr += sizeof( RGBQUAD )*i;
	// �ϊ�
	for( y=bih->biHeight-1; y>=0 ; --y ){
		pinp = (BYTE*)( *ptr + y*w );
		for( x=0; x<bih->biWidth; ++x ){
			c = *pinp;
			c = (c >> ( 7-(x & 7) )) & 1;
			if( (x & 7)==7 ) ++pinp;
			*pout = RGB( p[c].rgbRed,p[c].rgbGreen,p[c].rgbBlue );
			++pout;
		}
	}
	return true;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		4bit BMP�t�@�C����ǂݍ���ŕϊ�����
//	2.	����
//			bih		...	�r�b�g�}�b�v�C���t�H�w�b�_�̃A�h���X
//			out		...	�o�͐惁�����̃A�h���X
//	3.	�Ԓl
//			���� true
//			���s false
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
bool GetBmp4( BITMAPINFOHEADER *bih,char **ptr,COLORREF *pout )
{
	RGBQUAD		p[16];		// �p���b�g���
	int			i,w,x,y;	// �G�p�A���W
	BYTE		c;			// �ǂݍ��񂾃f�[�^
	BYTE		*pinp;		// ���̓|�C���^
	// �p���b�g��ǂݍ���
	w=( (bih->biWidth+1)/2 +3) & 0xFFFFFFFC;
	if( !(bih->biClrUsed) ){ i=16; }else{ i=bih->biClrUsed; }
	memcpy( p,*ptr,sizeof( RGBQUAD )*i );
	*ptr += sizeof( RGBQUAD )*i;
	// �������̊m��
	// �ϊ�
	for( y=bih->biHeight-1; y>=0; --y ){
		pinp = (BYTE*)( *ptr + y*w );
		for( x=0; x<bih->biWidth; ++x ){
			c = *pinp;
			if( x & 1 ){
				c = c & 0x0F;
				++pinp;
			}else{
				c =(c >> 4) & 0x0F;
			}
			*pout = RGB( p[c].rgbRed,p[c].rgbGreen,p[c].rgbBlue );
			++pout;
		}
	}
	return true;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		8bit BMP�t�@�C����ǂݍ���ŕϊ�����
//	2.	����
//			bih		...	�r�b�g�}�b�v�C���t�H�w�b�_�̃A�h���X
//			pout	...	�o�͐惁�����̃A�h���X
//	3.	�Ԓl
//			���� true
//			���s false
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
bool GetBmp8( BITMAPINFOHEADER *bih,char **ptr,COLORREF *pout )
{
	RGBQUAD	p[256];			// �p���b�g���
	int		i,w,x,y;		// �G�p�A���W
	BYTE	c;				// �ǂݍ��񂾃f�[�^
	BYTE	*pinp;			// ���̓|�C���^
	// �p���b�g��ǂݍ���
	w=(bih->biWidth+3) & 0xFFFFFFFC;
	if( !(bih->biClrUsed) ){ i=256; }else{ i=bih->biClrUsed; }
	memcpy( p,*ptr,sizeof( RGBQUAD )*i );
	*ptr += sizeof( RGBQUAD )*i;
	// �ϊ�
	for( y=bih->biHeight-1; y>=0; --y ){
		pinp = (BYTE*)( *ptr + y*w );
		for( x=0; x<bih->biWidth; ++x ){
			c = *pinp;
			*pout = RGB( p[c].rgbRed,p[c].rgbGreen,p[c].rgbBlue );
			++pout;
			++pinp;
		}
	}
	return true;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		16bit BMP�t�@�C����ǂݍ���ŕϊ�����
//	2.	����
//			bih		...	�r�b�g�}�b�v�C���t�H�w�b�_�̃A�h���X
//			pout	...	�o�͐惁�����̃A�h���X
//	3.	�Ԓl
//			���� true
//			���s false
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
bool GetBmp16( BITMAPINFOHEADER *bih,char **ptr,COLORREF *pout )
{
	int		w,x,y;			// �G�p�A���W
	BYTE	r,g,b;			// �q�f�a
	WORD	c;				// �ǂݍ��񂾃f�[�^
	BYTE	*pinp;			// ���̓|�C���^
	// ���͕�
	w=(bih->biWidth*2+3) & 0xFFFFFFFC;
	// �ϊ�
	for( y=bih->biHeight-1; y>=0; --y ){
		pinp = (BYTE*)( *ptr + y*w );
		for( x=0; x<bih->biWidth; ++x ){
			c = (WORD)pinp[0]+((WORD)pinp[1]<<8);
			b = (c & 0x001F)     *255/31;
			g =((c & 0x07E0)>> 5)*255/63;
			r =((c & 0xF800)>>11)*255/31;
			*pout = RGB( r,g,b );
			pout++;
			pinp+=2;
		}
	}
	return true;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		24bit BMP�t�@�C����ǂݍ���ŕϊ�����
//	2.	����
//			bih		...	�r�b�g�}�b�v�C���t�H�w�b�_�̃A�h���X
//			pout	...	�o�͐惁�����̃A�h���X
//	3.	�Ԓl
//			���� true
//			���s false
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
bool GetBmp24( BITMAPINFOHEADER *bih,char **ptr,COLORREF *pout )
{
	int		w,x,y;			// �G�p�A���W
	BYTE	r,g,b;			// �q�f�a
	BYTE	*pinp;			// ���̓|�C���^
	// ���͕�
	w = (bih->biWidth*3+3) & 0xFFFFFFFC;
	// �ϊ�
	for( y=bih->biHeight-1; y>=0; --y ){
		pinp = (BYTE*)( *ptr + y*w );
		for( x=0; x<bih->biWidth; ++x ){
			b = pinp[0];
			g = pinp[1];
			r = pinp[2];
			pinp+=3;
			*pout = RGB( r,g,b );
			pout++;
		}
	}
	return true;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		32bit BMP�t�@�C����ǂݍ���ŕϊ�����
//	2.	����
//			bih		...	�r�b�g�}�b�v�C���t�H�w�b�_�̃A�h���X
//			pout	...	�o�͐惁�����̃A�h���X
//	3.	�Ԓl
//			���� true
//			���s false
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
bool GetBmp32( BITMAPINFOHEADER *bih,char **ptr,COLORREF *pout )
{
	int		w,x,y;			// �G�p�A���W
	BYTE	r,g,b;			// �q�f�a
	BYTE	*pinp;			// ���̓|�C���^
	// ���͕�
	w = bih->biWidth*4;
	// �ϊ�
	for( y=bih->biHeight-1; y>=0; --y ){
		pinp = (BYTE*)( *ptr + y*w );
		for( x=0; x<bih->biWidth; ++x ){
			b = pinp[0];
			g = pinp[1];
			r = pinp[2];
			pinp+=4;
			*pout = RGB( r,g,b );
			pout++;
		}
	}
	return true;
}

// =====================================================
// �ݒ�֘A

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�p���b�g���l�r�w�P�̐F�ɂ���
//	2.	����
//		pal	...	(O)	�p���b�g�̃A�h���X
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
void set_msx1_palette( PAL *pal )
{
	memcpy( pal, msx1_palette, sizeof(PAL)*16 );
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�p���b�g���l�r�w�P�̐F�ɂ���
//	2.	����
//		pal	...	(O)	�p���b�g�̃A�h���X
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
void set_msx2_palette( PAL *pal ){
	memcpy( pal, msx2_palette, sizeof( PAL ) * 16 );
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�ݒ���f�t�H���g�ɂ���
//	2.	����
//		�Ȃ�
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
void GetDefCfg( SETTING *Mode )
{
	memset( Mode,0,sizeof( SETTING ) );
	memcpy( Mode->Col,msx1_palette,sizeof(PAL)*16 );
	Mode->Mode = MD_SC5;			// SCREEN5
	Mode->Gosaval = 0.43f;			// �덷�g�U�W�� 0.430
	Mode->err = 0;					// �؂�̂Č덷 0
	Mode->Gosa = true;				// �덷�g�U ON
	Mode->Pal = false;				// �Œ�p���b�g ���g�p
	Mode->Inter = false;			// �C���^�[���[�X ���g�p
	Mode->Resize = false;			// �T�C�Y���� ���g�p
	Mode->SelCol = 0;				// �F�I�����[�h
	Mode->PltMode = PLT_BSAVE;		// ��گďo��Ӱ�� BSAVE �Ɍ���
	Mode->AutoName = true;			// �����t�@�C��������
	Mode->AlgoMode = ALGO_RECALC;	// ���R�搶���A���S���Y��
	Mode->JKrc = true;				// �F�����̍Čv�Z
	Mode->ErrAlgo = EALGO_DITH2;	// �G���[�����A���S���Y��
	Mode->PreView = 3;				// �t�@�C���ƃv���r���[�ɏo��
	Mode->Seido = SEIDO_24;			// 24bit ���x
	Mode->ErrAdd = EADD_ROTATE;		// �f�B�U���Z���@ ��]
	Mode->NonZero = false;			// �O�Ԃ̐F���g��
	Mode->FourceZero = FZ_NONE;		// �����[�������Ȃ�
	Mode->FZColor = 0;				// �����[��������F
	Mode->Tile = false;				// �^�C��
	Mode->bDefault = false;			// ��Ƀf�t�H���g�R�s�[
	Mode->CnvMode = CM_LAST;		// �ŏI�ϊ��ݒ���̗p
	Mode->GosaRatio = 0.5;			// �덷�g�U X-Y��
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�ݒ��ǂݍ���
//	2.	����
//		�Ȃ�
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
bool GetCfgFile( SETTING *Mode,const char *sCfgFile )
{
	FU_FILE		*hFile;

	// �ݒ�t�@�C�����J��
	GetDefCfg(Mode);	// �t�@�C����������΃f�t�H���g�ݒ�ɂ���
	hFile = fu_open( sCfgFile, "rb" );
	if( hFile == FU_INVALID_HANDLE ){
		return false;
	}
	// �ݒ��ǂݍ���
	fu_read( hFile, Mode, sizeof( SETTING ) );
	fu_close( hFile );
	// �s���Ȓl�����
	Mode->PreView		= Mode->PreView & 3;
	if( Mode->PreView == 0 ) Mode->PreView = 1;
	Mode->ErrAlgo		= Mode->ErrAlgo % EALGO_MAX;
	Mode->AlgoMode		= Mode->AlgoMode % ALGO_MAX;
	Mode->SelCol		= Mode->SelCol % SELCOL_MAX;
	Mode->Mode			= Mode->Mode % MD_MAX;
	Mode->PltMode		= Mode->PltMode % PLT_MAX;
	Mode->Seido			= Mode->Seido % SEIDO_MAX;
	Mode->ErrAdd		= Mode->ErrAdd % EADD_MAX;
	Mode->FourceZero	= Mode->FourceZero % FZ_MAX;
	if( Mode->err > 255 ) Mode->err = 255;
	if( Mode->Gosaval < 0.0f ) Mode->Gosaval = 0.0f;
	if( Mode->Gosaval > 0.5f ) Mode->Gosaval = 0.5f;
	if( Mode->GosaRatio < 0.0f ) Mode->GosaRatio = 0.0f;
	if( Mode->GosaRatio > 1.0f ) Mode->GosaRatio = 1.0f;
	if( Mode->Mode == MD_SC2 || Mode->Mode == MD_SC3 ) {
		Mode->Inter = false;
	}

	return true;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�ݒ��ۑ�����
//	2.	����
//		�Ȃ�
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
void SaveCfgFile( SETTING *Mode,const char *sCfgFile ) {
	FU_FILE		*hFile;

	// �ݒ�t�@�C�����J��
	hFile = fu_open( sCfgFile, "wb" );
	if( hFile == FU_INVALID_HANDLE ) return;		// �t�@�C�������Ȃ���Εۑ����Ȃ�

	// �ݒ����������
	fu_write( hFile, Mode, sizeof( SETTING ) );
	fu_close( hFile );
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�p�X��ǂݍ���
//	2.	����
//		sPathFile	...	(I)	�p�X�f�[�^�̓������t�@�C����
//		sPath		...	(I)	�p�X���i�[����ϐ�
//		len			...	(I)	���R�[�h��
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
bool GetPathFile( const char *sPathFile, char *sPath, int len ) {
	FU_FILE		*hFile;
	char		s_temp[ MAX_PATH ];
	char		s_path[ MAX_PATH ];
	char		*ptr;

	//	�f�t�H���g���쐬����
	strcpy( s_temp, GetCommandLine() );
	if( s_temp[ 0 ] == '\"' ) {
		strcpy( s_path, &s_temp[1] );
	} else {
		strcpy( s_path, s_temp );
	}
	ptr  = strrchr( s_path, '\\' );
	if( ptr != NULL ) ptr[1] = '\0';
	strcat( s_path, "plugin\\" );

	// �p�X�t�@�C�����J��
	hFile = fu_open( sPathFile, "rb" );
	if( hFile == FU_INVALID_HANDLE ){
		strcpy( sPath, s_path );
		return false;
	}
	if( !fu_read( hFile, sPath, len ) ) {
		strcpy( sPath, s_path );
	}
	fu_close( hFile );
	
	return true;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�p�X��ۑ�����
//	2.	����
//		sPathFile	...	(I)	�p�X�f�[�^���i�[����t�@�C����
//		sPath		...	(I)	�p�X���i�[���Ă���ϐ�
//		len			...	(I)	���R�[�h��
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
void SavePathFile( const char *sPathFile, char *sPath, int len ) {
	FU_FILE		*hFile;

	// �p�X�t�@�C�����J��
	hFile = fu_open( sPathFile, "wb" );
	if( hFile == FU_INVALID_HANDLE ){
		return;		// �t�@�C�������Ȃ���Εۑ����Ȃ�
	}
	// �ݒ����������
	fu_write( hFile, sPath, len );
	fu_close( hFile );
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�ݒ���f�t�H���g�ɂ���
//	2.	����
//		�Ȃ�
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
void GetDefCustom( void ) {

	memcpy( convert7to255_r, init_convert7to255_r, sizeof( init_convert7to255_r ) );
	memcpy( convert7to255_g, init_convert7to255_g, sizeof( init_convert7to255_g ) );
	memcpy( convert7to255_b, init_convert7to255_b, sizeof( init_convert7to255_b ) );

	memcpy( convert7to255_s8r, init_convert7to255_s8r, sizeof( init_convert7to255_s8r ) );
	memcpy( convert7to255_s8g, init_convert7to255_s8g, sizeof( init_convert7to255_s8g ) );
	memcpy( convert3to255_s8b, init_convert3to255_s8b, sizeof( init_convert3to255_s8b ) );

	memcpy( convert31to255_s12r, init_convert31to255_s12r, sizeof( init_convert31to255_s12r ) );
	memcpy( convert31to255_s12g, init_convert31to255_s12g, sizeof( init_convert31to255_s12g ) );
	memcpy( convert31to255_s12b, init_convert31to255_s12b, sizeof( init_convert31to255_s12b ) );
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�ݒ��ǂݍ���
//	2.	����
//		�Ȃ�
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
bool GetCustomFile( const char *sCustomFile ) {
	FU_FILE *hFile;

	// �ݒ�t�@�C�����J��
	GetDefCustom();			// �t�@�C����������΃f�t�H���g�ݒ�ɂ���
	hFile = fu_open( sCustomFile, "rb" );
	if( hFile == FU_INVALID_HANDLE ){
		return false;
	}
	// �ݒ��ǂݍ���
	fu_read( hFile, convert7to255_r, sizeof( convert7to255_r ) );
	fu_read( hFile, convert7to255_g, sizeof( convert7to255_g ) );
	fu_read( hFile, convert7to255_b, sizeof( convert7to255_b ) );
	fu_read( hFile, convert7to255_s8r, sizeof( convert7to255_s8r ) );
	fu_read( hFile, convert7to255_s8g, sizeof( convert7to255_s8g ) );
	fu_read( hFile, convert3to255_s8b, sizeof( convert3to255_s8b ) );
	fu_read( hFile, convert31to255_s12r, sizeof( convert31to255_s12r ) );
	fu_read( hFile, convert31to255_s12g, sizeof( convert31to255_s12g ) );
	fu_read( hFile, convert31to255_s12b, sizeof( convert31to255_s12b ) );
	fu_close( hFile );
	return true;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�ݒ��ۑ�����
//	2.	����
//		�Ȃ�
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
void SaveCustomFile( const char *sCustomFile ) {
	FU_FILE *hFile;

	// �ݒ�t�@�C�����J��
	hFile = fu_open( sCustomFile, "wb" );
	if( hFile == FU_INVALID_HANDLE ) return;		// �t�@�C�������Ȃ���Εۑ����Ȃ�

	// �ݒ����������
	fu_write( hFile, convert7to255_r, sizeof( convert7to255_r ) );
	fu_write( hFile, convert7to255_g, sizeof( convert7to255_g ) );
	fu_write( hFile, convert7to255_b, sizeof( convert7to255_b ) );
	fu_write( hFile, convert7to255_s8r, sizeof( convert7to255_s8r ) );
	fu_write( hFile, convert7to255_s8g, sizeof( convert7to255_s8g ) );
	fu_write( hFile, convert3to255_s8b, sizeof( convert3to255_s8b ) );
	fu_write( hFile, convert31to255_s12r, sizeof( convert31to255_s12r ) );
	fu_write( hFile, convert31to255_s12g, sizeof( convert31to255_s12g ) );
	fu_write( hFile, convert31to255_s12b, sizeof( convert31to255_s12b ) );
	fu_close( hFile );
}

