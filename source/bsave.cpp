// -------------------------------------------------------------
// BMP to MSX
// �a�r�`�u�d�ۑ�
//										(C)2000 HRA!
// -------------------------------------------------------------

#include "bsave.h"
#include "utils.h"
#include "fileuty.h"

// -------------------------------------------------------------
// �ݒ�
// -------------------------------------------------------------
static const char* smode="235678AC5678AC";			// �t�@�C�����̍Ō�ɂ���L��

// -------------------------------------------------------------
// �v���g�^�C�v�錾
// -------------------------------------------------------------
static int _SaveHeader( FU_FILE *hf, int mode, int width, int height, int paltbl );
static int _SaveBodyS2( FU_FILE *hf, const byte* ptr, int *outadr );
static int _SaveBodyS3( FU_FILE *hf, const byte* ptr, int *outadr );
static int _SaveBodyAL( FU_FILE *hf, const byte* ptr, int width, int height, int pitch, int *outadr );
static int _SaveColorS2( FU_FILE *hf, const byte* ptr, int *outadr );
static int _SavePaletteTable( FU_FILE *hf, int paltbl, PAL *pal, int *outadr );
static void _GetOutFilename( char* szOutFile, const char* szInFile, int nScrMode, int nInter, bool pal );
static void _SaveCSVFile( HWND hWnd, const char *szFileName, SETTING *Mode );

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		BSAVE �`���ŉ摜��ۑ�����
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		bmp		...	(I)	�摜�f�[�^�̃A�h���X
//		width	...	(I)	��
//		height	...	(I)	����
//		inter	...	(I)	�C���^�[���[�X
//		paltbl	...	(I)	�p���b�g�e�[�u���̃A�h���X( 0 �Ȃ�t�����Ȃ� )
//	3.	�Ԓl
//		�G���[�R�[�h
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
int bsvSaveBmp( HWND hWnd, const char *szInFileName, byte* bmp, int width, int height, SETTING *Mode ) {
	char			szFileName[ MAX_PATH ];
	int				swidth, sheight, interc, outadr;
	byte*			ptr;
	FU_FILE			*hf;
	int				paltbl	= PalTblAdr[ Mode->Mode ];
	bool			inter	= Mode->Inter;
	int				ecode	= 0;
#ifdef _ENGLISH
	const char		*szTitle[]={
						"Save As",
						"Save As ( Odd line )",
						"Save As ( Even line )",
					};
#else
	const char		*szTitle[]={
						"���O��t���ĕۑ�",
						"���O��t���ĕۑ��i����C���j",
						"���O��t���ĕۑ��i�������C���j",
					};
#endif

	// �C���^�[���[�X�ƒʏ�Z�[�u�̋��
	if( inter ){
		interc  = 2;			// �t�@�C����
		swidth  = width*2;
		sheight = height/2;
	}else{
		interc	= 1;
		swidth  = width;
		sheight = height;
	}

	//	�ۑ�����
	ptr = bmp;
	while( interc ){

		//	�t�@�C����
		hf = FU_INVALID_HANDLE;
		if( Mode->AutoName ){	//	��������̏ꍇ
			_GetOutFilename( szFileName, szInFileName, Mode->Mode, interc-1+( inter?1:0 ), (Mode->PltMode==PLT_BSAVE ) );
		}else{					//	�蓮����̏ꍇ
			if( !GetName( hWnd, szFileName, MAX_PATH, szTitle[ interc-1+( inter?1:0 ) ], cszDefExp, NULL ) ){
				break;
			}
		}
		
		//	�t�@�C�����J��
		hf = fu_open( szFileName, "wb" );
		if( hf == FU_INVALID_HANDLE ) {
			ecode = BSV_ERR_CREATE;
			break;
		}

		//	�w�b�_
		if( Mode->PltMode == PLT_BSAVE ) {
			ecode = _SaveHeader( hf, Mode->Mode, width, sheight, paltbl );
		} else {
			ecode = _SaveHeader( hf, Mode->Mode, width, sheight, 0 );
		}
		if( ecode != BSV_NOERR ) break;

		// �s�N�Z���̕ۑ�
		switch( Mode->Mode ) {
		case MD_SC2:	//	SCREEN 2
			ecode = _SaveBodyS2( hf, ptr, &outadr );
			break;
		case MD_SC3:	//	SCREEN 3
			ecode = _SaveBodyS3( hf, ptr, &outadr );
			break;
		default:		//	SCREEN 5 �ȍ~
			ecode = _SaveBodyAL( hf, ptr, width, sheight, swidth, &outadr );
		}
		if( ecode != BSV_NOERR ) break;
		
		// �p���b�g�e�[�u�����L��Εۑ�����
		if( paltbl!=0 && Mode->PltMode==PLT_BSAVE && Mode->Mode < MD_SC5_256L ) {
			ecode = _SavePaletteTable( hf, paltbl, Mode->Col, &outadr );
			if( ecode != BSV_NOERR ) break;
		}

		//	�J���[�e�[�u��( SCREEN 2/4 )
		if( Mode->Mode == MD_SC2 ) {
			ecode = _SaveColorS2( hf, ptr, &outadr );
			if( ecode != BSV_NOERR ) break;
		}

		// �t�@�C�������
		fu_close( hf );
		hf = FU_INVALID_HANDLE;

		// �p���b�g�t�@�C�����o�͂���ꍇ
		if( paltbl != 0 ){
			switch( Mode->PltMode ) {
			case PLT_PL:	// �O���t�T�E���X *.PL? �`��
				if( GetName( hWnd,szFileName,MAX_PATH,PLSave ,cszDefPL, NULL ) ) {
					bsvSavePLFile( hWnd,szFileName,Mode );
				}
				break;
			case PLT_CSV:	// �b�r�u�`��
				if( GetName( hWnd,szFileName,MAX_PATH,CSVSave,cszDefCSV, CfgCsv ) ) {
					_SaveCSVFile( hWnd,szFileName,Mode );
				}
				break;
			}
		}
		// ���̃t�@�C��
		ptr=bmp+width;
		interc--;
	}
	if( hf != FU_INVALID_HANDLE ) fu_close( hf );
	return ecode;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		PL? �`���Ńp���b�g��ۑ�����
//	2.	����
//		szFileName	...	(I)	�t�@�C����
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
void bsvSavePLFile( HWND hWnd, const char *szFileName, SETTING *Mode ) {
	FU_FILE	*hf;
	byte	Buf[32];
	int		i;

	// �t�@�C�����J��
	hf = fu_open( szFileName, "wb" );
	if( hf == FU_INVALID_HANDLE ) {
		MessageBox( hWnd,WriteErr,MsgCap,MB_OK | MB_ICONWARNING );
		return;		// �t�@�C�������Ȃ�
	}
	// �o�̓f�[�^���쐬����
	for( i=0;i<16;++i ) {
		Buf[ i*2+0 ] = (Mode->Col[i].red<<4) | Mode->Col[i].blue;
		Buf[ i*2+1 ] =  Mode->Col[i].green;
	}
	// �f�[�^���o�͂���
	for( i=0;i<8;++i ) {
		fu_write( hf,Buf, sizeof( Buf ) );
	}
	// �I��
	fu_close( hf );
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		BSAVȨ�ٓ�����گĂ�ǂݍ���
//	2.	����
//		mode		...	(I)	��گĊi�[��
//		szFileName	...	(I)	�t�@�C����
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
void bsvLoadPLBsaveFile( SETTING *mode, const char *szFileName ) {
	FU_FILE	*hf;
	byte	Buf[ 32 ];
	int		i, adr;

	//	�g���q������Ӱ�ނ𔻒肷��
	i = szFileName[ strlen( szFileName )-1 ];
	if( (i=='0') || (i=='1') ) {
		//	�C���^�[���[�X�摜
		i = szFileName[ strlen( szFileName )-2 ];
	}
	switch( i ) {
	case '2':	i = MD_SC2;	break;
	case '3':	i = MD_SC3;	break;
	case '4':	i = MD_SC2;	break;
	case '5':	i = MD_SC5;	break;
	case '6':	i = MD_SC6;	break;
	case '7':	i = MD_SC7;	break;
	default:	return;				//	���[�h�s���摜
	}

	//	�p���b�g�e�[�u���̃I�t�Z�b�g = �A�h���X + sizeof( BSAVE Header ) ���擾
	adr = PalTblAdr[ i ] + 7;

	//	�t�@�C�����J��
	hf = fu_open( szFileName, "rb" );
	if( hf == FU_INVALID_HANDLE ) return;

	//	�p���b�g�e�[�u�����V�[�N
	fu_seek( hf, adr, FU_SEEK_TOP );

	// �f�[�^����͂���
	memset( Buf, 0, sizeof( Buf ) );
	if( fu_read( hf, Buf, sizeof(Buf) ) ) {
		// ���̓f�[�^�𔽉f����
		for( i = 0; i < 16; ++i ) {
			mode->Col[ i ].red  = (Buf[ i * 2 + 0 ] >> 4 ) & 0x07;
			mode->Col[ i ].blue =  Buf[ i * 2 + 0 ]        & 0x07;
			mode->Col[ i ].green=  Buf[ i * 2 + 1 ]        & 0x07;
		}
	}
	// �I��
	fu_close( hf );
}

// =============================================================
//	���[�J���֐�

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�J���[�e�[�u���̕ۑ�
//	2.	����
//		hf		...	(I)	�t�@�C���n���h��
//		ptr		...	(I)	MSX VRAM �C���[�W
//		outadr	...	(I/O)	�o�̓o�C�g�����i�[���Ă���ϐ��̃A�h���X
//	3.	�Ԓl
//		�G���[�R�[�h
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static int _SaveColorS2( FU_FILE *hf, const byte* ptr, int *outadr ) {
	byte	dmy;

	// �J���[�e�[�u���܂ł� 0 �Ŗ��߂�
	dmy = 0;
	while( *outadr < 0x2000 ) {
		if( !fu_write( hf, &dmy, 1 ) ) return BSV_ERR_WRITE;
		(*outadr)++;
	}
	//	�J���[�e�[�u��
	if( !fu_write( hf, ptr + 0x2000, 0x1800 ) ) return BSV_ERR_WRITE;
	*outadr += 0x1800;

	return BSV_NOERR;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�p���b�g�e�[�u���̕ۑ�
//	2.	����
//		hf		...	(I)	�t�@�C���n���h��
//		paltbl	...	(I)	�p���b�g�e�[�u���̃A�h���X
//		pal		...	(I)	�p���b�g�l
//		outadr	...	(I/O)	�o�̓o�C�g�����i�[���Ă���ϐ��̃A�h���X
//	3.	�Ԓl
//		�G���[�R�[�h
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static int _SavePaletteTable( FU_FILE *hf, int paltbl, PAL *pal, int *outadr ) {
	int		i;
	byte	dmy[] = { 0, 0 };

	// �p���b�g�e�[�u���܂ł� 0 �Ŗ��߂�
	while( *outadr < paltbl ) {
		if( !fu_write( hf, dmy, 1 ) ) return BSV_ERR_WRITE;
		(*outadr)++;
	}

	// �p���b�g���o�͂���
	for( i = 0; i < 16; ++i ) {
		dmy[0]=(byte)(( pal[i].red<<4 ) | pal[i].blue );
		dmy[1]=(byte)(  pal[i].green );
		if( !fu_write( hf, dmy, 2 ) ) return BSV_ERR_WRITE;
		*outadr += 2;
	}

	return BSV_NOERR;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		Screen2 �f�[�^�̖{�̂�ۑ�����
//	2.	����
//		hf		...	(I)	�t�@�C���n���h��
//		ptr		...	(I)	MSX VRAM�C���[�W
//		outadr	...	(O)	�o�̓f�[�^�����i�[����A�h���X
//	3.	�Ԓl
//		�G���[�R�[�h
//	4.	���l
//		�G���[���͏o�̓o�C�g����Ԃ��Ȃ�
// -------------------------------------------------------------
static int _SaveBodyS2( FU_FILE *hf, const byte* ptr, int *outadr ) {
	int		i;
	byte	sPGen[ 768 ];

	//	�p�^�[���W�F�l���[�^�e�[�u��
	if( !fu_write( hf, ptr, 0x1800 ) ) return BSV_ERR_WRITE;

	//	�p�^�[���l�[���e�[�u��
	for( i = 0; i < 768; ++i ) {
		sPGen[ i ] = (unsigned char)( i & 255 );
	}
	if( !fu_write( hf, sPGen, 768 ) ) return BSV_ERR_WRITE;

	//	�o�̓o�C�g��
	*outadr = 0x1800 + 768;

	return BSV_NOERR;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		Screen3 �f�[�^�̖{�̂�ۑ�����
//	2.	����
//		hf		...	(I)	�t�@�C���n���h��
//		ptr		...	(I)	MSX VRAM�C���[�W
//		outadr	...	(O)	�o�̓f�[�^�����i�[����A�h���X
//	3.	�Ԓl
//		�G���[�R�[�h
//	4.	���l
//		�G���[���͏o�̓o�C�g����Ԃ��Ȃ�
// -------------------------------------------------------------
static int _SaveBodyS3( FU_FILE *hf, const byte* ptr, int *outadr ) {
	int		i, j, k;
	byte	sPGen[ 768 ];

	//	�p�^�[���W�F�l���[�^�e�[�u��
	if( !fu_write( hf, ptr, 0x800 ) ) return BSV_ERR_WRITE;

	//	�p�^�[���l�[���e�[�u��
	for( i = 0; i < 6; ++i ) {
		for( j = 0; j < 32; ++j ) {
			for( k = 0; k < 4; ++k ) {
				sPGen[ k*32 + j + i*128 ] = (unsigned char)( j + i*32 );
			}
		}
	}
	if( !fu_write( hf, sPGen, 768 ) ) return BSV_ERR_WRITE;

	//	�o�̓o�C�g��
	*outadr = 0x800 + 768;

	return BSV_NOERR;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�f�[�^�̖{�̂�ۑ�����
//	2.	����
//		hf		...	(I)	�t�@�C���n���h��
//		ptr		...	(I)	MSX VRAM�C���[�W
//		width	...	(I)	�摜���i�o�C�g���j
//		height	...	(I)	�摜����
//		pitch	...	(I)	�ړ��P�ʁi�o�C�g���j
//		outadr	...	(O)	�o�̓f�[�^�����i�[����A�h���X
//	3.	�Ԓl
//		�G���[�R�[�h
//	4.	���l
//		�G���[���͏o�̓o�C�g����Ԃ��Ȃ�
// -------------------------------------------------------------
static int _SaveBodyAL( FU_FILE *hf, const byte* ptr, int width, int height, int pitch, int *outadr ) {
	int	i;

	*outadr = 0;
	for( i=0; i<height; i++ ) {
		if( !fu_write( hf, ptr + i * pitch, width ) ) return BSV_ERR_WRITE;
		*outadr += width;
	}

	return BSV_NOERR;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		bsave �w�b�_����������
//	2.	����
//		hf	...	(I)	�t�@�C���n���h��
//		mode	...	(I)	MSX�X�N���[�����[�h
//		width	...	(I)	�摜��
//		height	...	(I)	�摜����
//		paltbl	...	(I)	�p���b�g�e�[�u���i�ۑ����Ȃ��ꍇ�� 0�j
//	3.	�Ԓl
//		�G���[�R�[�h
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static int _SaveHeader( FU_FILE *hf, int mode, int width, int height, int paltbl ) {
	BSAVEHEADER		bsh;

	//	�w�b�_�̍\�z
	bsh.type	= 0xFE;
	bsh.start	= 0;
	switch( mode ) {
	case MD_SC2:
		bsh.end	= 0x37FF;
		break;
	default:
		if( paltbl ) {
			bsh.end	= paltbl + 32 - 1;
		} else {
			bsh.end	= width * height - 1;
		}
	}
	bsh.run		= 0;

	//	�ۑ�
	if( !fu_write( hf, &bsh, sizeof(BSAVEHEADER) ) ) return BSV_ERR_WRITE;

	return BSV_NOERR;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�t�@�C���������肷��
//	2.	����
//		szOutFile	...	(I)	�t�@�C�������i�[����A�h���X
//		szInFile	...	(I)	�\�[�X�̃t�@�C���� (*.BMP)
//		nScrMode	...	(I)	�o�͉�ʃ��[�h
//		nInter		...	(I)	0 = �m�[�}��
//							1 = �C���^�[���[�X(Page0)
//							2 = �C���^�[���[�X(Page1)
//		pal			...	(I)	false = �p���b�g����
//							true = �p���b�g�L��
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�\�[�X�̃t�@�C�����͕K�� "�`.BMP" �̌`������Ă��Ȃ�
//		��΂Ȃ�Ȃ��B�t�@�C�����i�[�A�h���X�̓\�[�X�̃t�@�C
//		�����ȏ�̃T�C�Y�łȂ���΂Ȃ�Ȃ��B
// -------------------------------------------------------------
static void _GetOutFilename( char* szOutFile, const char* szInFile, int nScrMode, int nInter, bool pal ) {
	char *p;
	char tmp[]={ ".SRX" };

	if( pal && (nScrMode<MD_SC8 || (nScrMode>=MD_SC5_256L && nScrMode<MD_SC8_256L)) ) {
		tmp[2]='C';
	}else{
		tmp[2]='R';
	}
	switch( nInter ){
	case 1:
		tmp[2]=smode[nScrMode];
		tmp[3]='0';
		break;
	case 2:
		tmp[2]=smode[nScrMode];
		tmp[3]='1';
		break;
	default:
		tmp[3]=smode[nScrMode];
	}
	strcpy( szOutFile,szInFile );
	p=strrchr( szOutFile,'.' );
	strcpy( p,tmp );
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		CSV �`���Ńp���b�g��ۑ�����
//	2.	����
//		szFileName	...	(I)	�t�@�C����
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static void _SaveCSVFile( HWND hWnd, const char *szFileName, SETTING *Mode ) {
	FU_FILE	*hf;
	byte	Buf[2];
	int		i;
	char	szBuf[32];
	int		palnum;

	// �t�@�C�����J��
	hf = fu_open( szFileName, "wb" );
	if( hf == FU_INVALID_HANDLE ) {
		MessageBox( hWnd, WriteErr, MsgCap,MB_OK | MB_ICONWARNING );
		return;		// �t�@�C�������Ȃ�
	}

	// �f�[�^���o�͂���
	fu_write( hf, cszCSVHead, lstrlen(cszCSVHead) );
	palnum = ( Mode->Mode == MD_SC6 || Mode->Mode == MD_SC6_256L ) ? 4 : 16;
	for( i = 0; i < palnum; ++i ){
		Buf[ 0 ] = (Mode->Col[i].red<<4) | Mode->Col[i].blue;
		Buf[ 1 ] =  Mode->Col[i].green;
		wsprintf( szBuf,"%02X,%02X%s", Buf[0], Buf[1], (i==15)?"\x0D\x0A":"," );
		fu_write( hf, szBuf, lstrlen(szBuf) );
	}
	// �I��
	fu_close( hf );
}
