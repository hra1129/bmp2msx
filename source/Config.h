// -------------------------------------------------------------
// BMP to MSX
//											(C)2000 HRA!
// -------------------------------------------------------------

#ifndef _CONFIG_H_
#define _CONFIG_H_

// -------------------------------------------------------------
// �ݒ�
// -------------------------------------------------------------
#define	CFG_VERSION		"ver 2022_07_17 (C)2000-2022 HRA!"	//	�o�[�W�����ԍ�

#define	COLCUR1			RGB(255,255,240)				//	�J���[�e�[�u�����݈ʒu�J�[�\���F�P�i�����j
#define	COLCUR2			RGB(15,15,0)					//	�J���[�e�[�u�����݈ʒu�J�[�\���F�Q�i�O���j
#define PV_MAX			50								//	�v���r���[�L���ő吔
#define cnDataBk1		RGB(0,0,120)					//	�ݒ�\���̔w�i�i�ݒ�A�ϊ����ʉ摜�j
#define cnDataBk2		RGB(120,0,0)					//	�ݒ�\���̔w�i�i�ϊ����摜�j
#define cnDataText		RGB(255,255,255)				//	�ݒ�\���̔w�i
#define	cnWidth			512								//	��
#define	cnHeight		424								//	����
#define	cnZoomMax		4								//	�g�嗦�ő�

//	���S�C���[�W�̃T�C�Y
#define cnBGWidth		256
#define cnBGHeight		212

//	�ő�摜�T�C�Y
#define	cnMAXWidth		512
#define	cnMAXHeight		424

//	�w���v�t�@�C����
#define	cszHelpName		"bmp2msx.chm"

#ifdef _ENGLISH

#define CNWS			(WS_CAPTION | WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX | WS_BORDER)
#define CNWSS			(WS_CAPTION | WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX | WS_BORDER | WS_SIZEBOX )
#define	CSZCLASSNAME	"BMP2MSX"
#define	CSZCLASSNAMESUB	"BMP2MSXSUB"
#define	CSZTITLE		"BMP to MSX"
#define	CSZTITLESUB		"Original File"

#define	BgColor			RGB(10,0,127)
#define	FrColor			RGB(255,255,255)				//	�����̐F
#define CfgFile			"BMP2MSX.B2M"					//	�ݒ�t�@�C��
#define PathFile		"BMP2MSX.CFG"					//	�ݒ�t�@�C��
#define	CfgExt			"B2M"							//	�ݒ�t�@�C���̃f�t�H���g�g���q
#define	CfgBmp			"BMP"							//	BMP�t�@�C���̊g���q
#define	CfgCsv			"CSV"							//	CSV�t�@�C���̊g���q
#define PLSave			"Save As(*.PL?)"
#define	PLLoad			"Load Palette File"
#define ScrptLoad		"Load Script File"
#define	CSVSave			"Save As(*.CSV)"
#define cszDefExp		"MSX BSAVE Files(*.SC?/*.SR?)\0*.SC?;*.SR?\0All Files(*.*)\0*.*\0"
#define	cszDefPL		"MSX Palette Files(*.PL?)\0*.PL?\0"
#define	cszDefPLL		"MSX Palette/BSAVE Files(*.PL?/*.Sxx)\0*.PL?;*.S??\0"
#define	cszDefCSV		"CSV Files(*.CSV/*.TXT)\0*.CSV;*.TXT\0"
#define cszDefScrpt		"Script Files(*.TXT)\0*.TXT\0"
#define cszDefPlug		"PlugIn(*.SPI)\0*.SPI\0"
#define cszDefCfg		"BMPtoMSX Setting Files(*.B2M)\0*.B2M\0"
#define	cszCSVHead		";MSX PALETTE DATA(&Hrb,&H0g)\x0D\x0A"
#define cszClipName		"Clip board"
#define cszClipFile		"bmpmsx.bmp"
#define	cszBmpSave		"Save As(*.BMP)"
#define	cszBmpFilter	 "BMP Files(*.BMP)\0*.BMP\0"

#define cszSetting		"Default"						// �ݒ�_�C�A���O�̃^�C�g��
#define cszRedo			"Reconvert"						// �ĕϊ��_�C�A���O�̃^�C�g��
#define cszScrSetting	"Setting(Script mode)"			// �X�N���v�g���߂ɂ��ݒ�_�C�A���O�̃^�C�g��

// -------------------------------------------------------------
// �G���[/�x�����b�Z�[�W
// -------------------------------------------------------------
#define MsgCap			"Error"							// �G���[���b�Z�[�W�̃^�C�g��
#define WriteErr		"Write error"					// �t�@�C���������݃G���[
#define cszNotEnoughMem	"Not enough memory"				// �������s��
#define cszPalCnt		"Cannot create palette.\nNeed two palettes or more."	// �p���b�g�s��
#define MsgCapWrn		"Warning"						// �x���i�m�F�j���b�Z�[�W�̃^�C�g��
#define cszDel			"Delete now preview.\nOk?"
#define cszAllDel		"Delete all preview.\nOk?"
#define cszCannotAddPV	"Cannot create preview."
#define cszClipErr		"Clipboard picture cannot reconvert."
#define cszBDErr		"Read error"
#define cszNoSupp		"Read error. Unkown file type."
#define cszSmall		"Read error. Too small."
#define cszOpenErr		"Cannot open file."
#define cszNoText		"Unkown file."
#define cszMainClk		"Do not open an image yet. (Please choose setting among a menu)"

// �X�N���v�g�G���[/�x��
#define cszSyntax		"Syntax error."
#define cszNoString		"Cannot find string."
#define cszNoDelim		"Cannot find string tarminater \". "
#define cszNoCfg		"Read error ( Setting file )."
#define cszNoFile		"File name is blank."
#define cszExit			"End of script."
#define cszScrMode		"Abnormal screen mode."
#define cszNoFlag		"Abnormal flag name."
#define	cszNoBool		"Abnormal flag data."
#define cszWrnClear		"Clear edit text.\nOk?"
#define cszPal			"Abnormal palette setting."
#define cszPMode		"Abnormal palette mode."
#define cszCutErr		"Abnormal 'ignore error' number."
#define cszGosaVal		"Abnormal 'error multiply' number."
#define cszSelMode		"Abnormal palette mode."

#else

#define CNWS			(WS_CAPTION | WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX | WS_BORDER)
#define CNWSS			(WS_CAPTION | WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX | WS_BORDER | WS_SIZEBOX )
#define	CSZCLASSNAME	"BMP2MSX"
#define	CSZCLASSNAMESUB	"BMP2MSXSUB"
#define	CSZTITLE		"BMP to MSX"
#define	CSZTITLESUB		"�ؼ��ى摜"

#define	BgColor			RGB(10,0,127)
#define	FrColor			RGB(255,255,255)			//	�����̐F
#define CfgFile			"BMP2MSX.B2M"				//	�ݒ�t�@�C��
#define PathFile		"BMP2MSX.CFG"				//	�ݒ�t�@�C��
#define	CfgExt			"B2M"						//	�ݒ�t�@�C���̃f�t�H���g�g���q
#define	CfgBmp			"BMP"						//	BMP�t�@�C���̊g���q
#define	CfgCsv			"CSV"						//	CSV�t�@�C���̊g���q
#define	CfgSpi			"SPI"						//	�v���O�C���̊g���q
#define PLSave			"���O��t���ĕۑ�(*.PL?�`��)"
#define	PLLoad			"��گ�̧�ٓǂݍ���"
#define ScrptLoad		"������̧�ٓǂݍ���"
#define	CSVSave			"���O��t���ĕۑ�(CSV�`��)"
#define cszDefExp		"MSX�摜̧��(*.SC?/*.SR?)\0*.SC?;*.SR?\0���ׂĂ�̧��(*.*)\0*.*\0"
#define	cszDefPL		"MSX��گ�̧��(*.PL?)\0*.PL?\0"
#define	cszDefPLL		"MSX��گ�̧��(*.PL?/*.Sxx)\0*.PL?;*.S??\0"
#define	cszDefCSV		"CSV̧��(*.CSV/*.TXT)\0*.CSV;*.TXT\0"
#define cszDefScrpt		"������̧��(*.TXT)\0*.TXT\0"
#define cszDefPlug		"SPI��׸޲�(*.SPI)\0*.SPI\0"
#define cszDefCfg		"BMPtoMSX�ݒ�̧��(*.B2M)\0*.B2M\0"
#define	cszCSVHead		";MSX PALETTE DATA(&Hrb,&H0g)\x0D\x0A"
#define cszClipName		"�N���b�v�{�[�h���摜"
#define cszClipFile		"bmpmsx.bmp"
#define	cszBmpSave		"BMP�ŕۑ�"
#define	cszBmpFilter	"BMP̧��(*.BMP)\0*.BMP\0"

#define cszSetting		"��̫�Đݒ�"			// �ݒ�_�C�A���O�̃^�C�g��
#define cszRedo			"�ĕϊ�"				// �ĕϊ��_�C�A���O�̃^�C�g��
#define cszScrSetting	"�ϊ��ݒ�(������Ӱ��)"	// �X�N���v�g���߂ɂ��ݒ�_�C�A���O�̃^�C�g��

// -------------------------------------------------------------
// �G���[/�x�����b�Z�[�W
// -------------------------------------------------------------
#define MsgCap			"Error"						// �G���[���b�Z�[�W�̃^�C�g��
#define WriteErr		"�������݂Ɏ��s���܂����B"	// �t�@�C���������݃G���[
#define cszNotEnoughMem	"�������s���ł��B"		// �������s��
#define cszPalCnt		"�摜�𐶐��ł��܂���B\n�p���b�g�͂Q�ȏ�g�p�\�ɂ��Ă��������B"	// �p���b�g�s��
#define MsgCapWrn		"Warning"					// �x���i�m�F�j���b�Z�[�W�̃^�C�g��
#define cszDel			"�\�����̃v���r���[���폜���܂��B\n��낵���ł����H"
#define cszAllDel		"�L�����Ă���v���r���[�����ׂč폜���܂��B\n��낵���ł����H"
#define cszCannotAddPV	"����ȏ�v���r���[���L���ł��܂���B"
#define cszClipErr		"�N���b�v�{�[�h���琶�������摜�͍ĕϊ��ł��܂���B"
#define cszBDErr		"�w�肳�ꂽ�t�@�C�����ǂݍ��߂Ȃ����A����ȏ�摜���L���ł��܂���B"
#define cszNoSupp		"�ǂݍ��߂܂���B��Ή��̌`���ł��B"
#define cszSmall		"�摜�����������܂��B"
#define cszOpenErr		"�t�@�C�����J���܂���B"
#define cszNoText		"�ُ�ȃt�@�C���ł��B�ǂݍ��߂܂���B"
#define cszMainClk		"�܂��摜���J���Ă��܂���i�ݒ�̓��j���[����I�����Ă��������j"

// �X�N���v�g�G���[/�x��
#define cszSyntax		"���@���s���ł��B"
#define cszNoString		"������̂���ׂ��Ƃ���ɕ����񂪂���܂���B"
#define cszNoDelim		"������̏I�[������ \" ��������܂���B"
#define cszNoCfg		"�ݒ�t�@�C���̓ǂݍ��݂Ɏ��s���܂����B"
#define cszNoFile		"�t�@�C�������w�肳��Ă��܂���B"
#define cszExit			"�X�N���v�g���I�����܂����B"
#define cszScrMode		"��ʃ��[�h�̎w�肪�ُ�ł��B"
#define cszNoFlag		"�t���O���̎w�肪�ُ�ł��B"
#define	cszNoBool		"�t���O�l�̎w�肪�ُ�ł��B"
#define cszWrnClear		"�ҏW���̃X�N���v�g���N���A����܂��B��낵���ł����H"
#define cszPal			"�p���b�g�ݒ肪�ُ�ł��B"
#define cszPMode		"�p���b�g�̓��샂�[�h�w�肪�ُ�ł��B"
#define cszCutErr		"�؂�̂Č덷�̒l�w�肪�ُ�ł��B"
#define cszGosaVal		"�덷�g�U�W���̎w��l���ُ�ł��B"
#define cszSelMode		"�I�F���[�h�̎w��l���ُ�ł��B"

#endif

#endif	// CONFIG_H_
