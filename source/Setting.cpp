// -------------------------------------------------------------
// BMP to MSX
// �ݒ�_�C�A���O
//											(C)2000/5/6 HRA!
// -------------------------------------------------------------

#include <windows.h>
#include <windowsx.h>
#include "Config.h"
#include "Setting.h"
#include "Resource.h"
#include <commctrl.h>
#include "Convert.h"
#include "utils.h"
#include "winuty.h"
#include "bsave.h"
#include "fileuty.h"

// -------------------------------------------------------------
// �e�[�u��
// -------------------------------------------------------------
// �o�͉�ʃ��[�h�R���{�{�b�N�X�̓��e
const char *ScrModeName[]={
	"SCREEN2/4",
	"SCREEN3",
	"SCREEN5",
	"SCREEN6",
	"SCREEN7",
	"SCREEN8",
	"SCREEN10/11",
	"SCREEN12",
	"SCREEN5(256lines)",
	"SCREEN6(256lines)",
	"SCREEN7(256lines)",
	"SCREEN8(256lines)",
	"SCREEN10/11(256lines)",
	"SCREEN12(256lines)",
};

//	�c�[���`�b�v��`
typedef struct {
	int		nId;
	char*	sStr;
} TOOLTIPS;

#ifdef _ENGLISH	// �p��(English)
// �p���b�g�o�̓��[�h
const char *PltMode[]={		// ���v�f�ԍ��� PLT_XXXX �̒�`�ƈ�v����
	"no output",
	"into the picture file( COLOR=RESTORE ver. )",
	"*.PL? ( (C)BIT2 Graphsaurus v2.x ver. )",
	"*.CSV ( CSV text file )",
};

// �ٺ�ؽ�і�
const char *AlgoName[]={		// ���v�f�ԍ��� ALGO_XXXX �̒�`�ƈ�v����
	"no use",
	"error fix (give brightness)",
	"MSX2+ POWERFUL BOOK ver.",
	"NTSC fix(R:G:B=40:49:6)",
	"error fix (give tone)"
};

// �f�B�U�p�^�[����
const char *ErrAlgoName[]={	// ���v�f�ԍ��� EALGO_XXXX �̒�`�ƈ�v����
	"no use",
	"random dots",
	"net dots",
	"net dots(double length)",
	"pattern of brick",
	"pattern of grain",
	"rough net",
	"lite net",
	"half tone",
};

// �v���r���[���[�h
const char *PreViewName[]={
	"file",
	"preview",
	"file and preview",
};

// �F�Č����x
const char *ColorSeido[]={
	"8bit",
	"7bit",
	"6bit",
	"5bit",
	"4bit",
};

// �f�B�U���Z���@
const char *ErrAlgo2Name[]={
	"rotate",
	"pile up",
	"move X",
	"move Y",
};

// �p���b�g�g�p����
const char *PalEnName[]={
	"automatic color",
	"no use",
	"customize color",
};

// �����[����
const char *FZColorName[]={
	"no use",
	"appointed color No. -> No.0",
	"appointed (X,Y) color -> No.0",
};

const char *cOpenFile = "Open File";

static const TOOLTIPS ttips[] = {
	IDC_CMBSCREEN	,	"select a screen mode of MSX",
	IDC_PREVIEW		,	"select an output point (a preview, a file)",
	IDC_CMBERR		,	"select shape of pattern to increase the number of color",
	IDC_CMBERR2		,	"select it how I apply dither pattern in each element of RGB",
	IDC_SEIDO		,	"use high rank several bits of a former image as an effective value",
	IDC_GOSA		,	"scatter an error and do not stand out and do it",
	IDC_GOSAVAL		,	"appoint a value to let the error that spread splash it on each other. small and spread becomes weak",
	IDC_GOSAERR		,	"appoint the threshold value cutting off the error that spread. then spread becomes greatly weak",
	IDC_GOSARATIO	,	"Specifies the ratio of the right direction (X direction) to the bottom direction (Y direction) of the error to be spread. X:Y",
	IDC_INTER		,	"use an inter race mode of MSX and double resolution of perpendicular course",
	IDC_RESIZE		,	"an image is destroyed by the ratio regulating an input image in output size forcibly",
	IDC_JKRC		,	"I calculate J/K value which is most suitable for Y value which I calculated after Y decision again and revise it",
	IDC_AUTONAME	,	"I make the output file name from the input file name and do not display a file excellent input column",
	IDC_CMBALGO		,	"select a method of SCREEN10/12",
	IDC_SELCOL		,	"I select a lot of colors using with precedence (a color of outline lines may not be selected)",
	IDC_PAL			,	"I do not generate a color of a palette automatically",
	IDC_CHKZERO		,	"I do not use a color of the 0 and move it in other colors (for TPSET)",
	IDC_TILE		,	"A net of ratio handles a design as an usual color at 1:1 of extremely near color and increases a color",
	IDC_ENREVS		,	"apply automatic decision setting of color set beneath to all palettes",
	IDC_PALEN		,	"I select it whether I decide a color automatically whether I appoint a value",
	IDC_FZERO		,	"I select a method of the color appointment that turns a color of specification into a color of the 0 forcibly (for TPSET)",
	IDC_FZCOLOR		,	"It is a button to appoint a color to turn into the 0 forcibly",
	IDC_FZC			,	"It is a color to turn into the 0 forcibly",
	IDC_FZ_X		,	"It is a coordinate at gathering a color from an image to turn into the 0 forcibly",
	IDC_FZ_Y		,	"It is a coordinate at gathering a color from an image to turn into the 0 forcibly",
	IDC_CMBPLT		,	"I appoint an output method of a palette",
	IDC_LOADPLT		,	"Reading adds to a palette file",
	IDC_MSX1COLOR	,	"turn a palette into a color of MSX1",
	IDC_MSX2COLOR	,	"turn a palette into a color of MSX2 default",
	IDC_192			,	"I output an image for a 192 line (an inter race, 384 line) when I put a check.",
	IDC_SORT		,	"I appoint a method to let a color stand in line of a palette.",
	IDC_SAVEAS		,	"I store setting displaying in a file currently.",
	IDC_LOAD		,	"Reading adds to the setting file which I stored formerly.",
	IDC_DEFAULT		,	"I adopt setting contents displaying as \'default setting\'.",
	IDC_CHKDEF		,	"Setting at having done re-conversion is copied immediately by default setting when I put a check.",
	IDC_CMBCNV		,	"I appoint setting to adopt when I convert a new image file into.",
	IDC_FILLC		,	"I smash it, and a method of coating to initialize it is a color",
	IDC_SIZEMODE	,	"I appoint a decision method of aspect ratio",
	IDC_RESAMPLE	,	"I appoint a sample method of color.",
};

const char *SortMode[] = {
	"Ignore",
	"Incriment(auto only)",
	"Decriment(auto only)",
	"Incriment",
	"Decriment",
};

const char *UseSetting[] = {
	"Default",
	"Last convert",
	"Indication image",
};

const char *Resample[] = {
	"normal",
	"antialiasing",
};

const char *SizeMode[] = {
	"output size forcibly",
	"look like aspect ratio on MSX",
};

#else			// ���{��(Japanese)

// �p���b�g�o�̓��[�h
const char *PltMode[]={		// ���v�f�ԍ��� PLT_XXXX �̒�`�ƈ�v����
	"�o�͂��Ȃ�",
	"�摜�Ɍ���(COLOR=RESTORE�p)",
	"�O���t�T�E���X *.PL? �`��",
	"CSV�`��",
};

// �ٺ�ؽ�і�
const char *AlgoName[]={		// ���v�f�ԍ��� ALGO_XXXX �̒�`�ƈ�v����
	"�Ȃ�",
	"�덷�����i�P�x�d���j",
	"MSX2+����ي��p�@ڲ�ڕ���",
	"NTSC��(R:G:B=40:49:6)",
	"�덷�����i�F�����d���j"
};

// �f�B�U�p�^�[����
const char *ErrAlgoName[]={	// ���v�f�ԍ��� EALGO_XXXX �̒�`�ƈ�v����
	"�Ȃ�",
	"�^�������_��",
	"���x�ȖԂ���",
	"�c���̖Ԃ���",
	"�����K�͗l",
	"�ԂԖ͗l",
	"�e���Ԃ���",
	"�����Ԃ���",
	"�n�[�t�g�[��",
};

// �v���r���[���[�h
const char *PreViewName[]={
	"�t�@�C���֏o��",
	"�v���r���[�̂ݍ쐬",
	"�v���r���[���쐬���ăt�@�C���֏o��",
};

// �F�Č����x
const char *ColorSeido[]={
	"8bit",
	"7bit",
	"6bit",
	"5bit",
	"4bit",
};

// �f�B�U���Z���@
const char *ErrAlgo2Name[]={
	"��]",
	"���ʑ�",
	"�w���炵",
	"�x���炵",
};

// �p���b�g�g�p����
const char *PalEnName[]={
	"�F��������",
	"�g�p������",
	"�ݒ�F���g�p",
};

// �����[����
const char *FZColorName[]={
	"�g�p���Ȃ�",
	"�w��F��ۉ�",
	"�w����W�F��ۉ�",
};

const char *cOpenFile = "�t�@�C�����J��";

static const TOOLTIPS ttips[] = {
	IDC_CMBSCREEN	,	"MSX �̉�ʃ��[�h��I�����܂�",
	IDC_PREVIEW		,	"�o�͐�i�v���r���[�A�t�@�C���j��I�����܂�",
	IDC_CMBERR		,	"�[���I�ɐF���𑝂₷���߂̃p�^�[���̌`���I�����܂�",
	IDC_CMBERR2		,	"RGB�̊e�v�f�ɂǂ̂悤�Ƀf�B�U�p�^�[����K�p���邩�I�����܂�",
	IDC_SEIDO		,	"���摜�̏�ʉ��r�b�g��L���l�Ƃ��Ďg�p���邩�I�����܂�",
	IDC_GOSA		,	"�덷���g�U���Ėڗ����Ȃ����܂�",
	IDC_GOSAVAL		,	"�g�U�����덷�Ɋ|�����킹��l���w�肵�܂��A����������Ɗg�U���キ�Ȃ�܂�",
	IDC_GOSAERR		,	"�g�U�����덷��؂�̂Ă�~���l���w�肵�܂��A�傫������Ɗg�U���キ�Ȃ�܂�",
	IDC_GOSARATIO	,	"�g�U����덷�̉E����(X����)�Ɖ�����(Y����)�̔䗦���w�肵�܂��BX:Y",
	IDC_INTER		,	"MSX�̃C���^�[���[�X���[�h���g���Đ��������̉𑜓x���Q�{�ɂ��܂�",
	IDC_RESIZE		,	"���͉摜�̃T�C�Y���ߕ��@���g�p���邩�ۂ��w�肵�܂�",
	IDC_JKRC		,	"�Z�o���� Y �l�ɍœK�� J/K �l�� Y �����ɍČv�Z���ĕ␳���܂�",
	IDC_AUTONAME	,	"�o�̓t�@�C��������̓t�@�C��������쐬���A�t�@�C�������͗���\�����Ȃ��Ȃ�܂�",
	IDC_CMBALGO		,	"���R�挸�F�̕��@��I�����܂�",
	IDC_SELCOL		,	"��������g���Ă���F��D��I�ɑI�т܂��i�֊s���Ȃǂ̐F���I�΂�Ȃ��\��������܂��j",
	IDC_PAL			,	"�p���b�g�̐F�������������܂���",
	IDC_CHKZERO		,	"0�Ԃ̐F���g�킸�A�ʂ̐F�ɒu�������܂��iTPSET�p�j",
	IDC_TILE		,	"�����߂��F�� 1:1 �䗦�̖Ԃ����͗l��ʏ�̐F�Ƃ��Ĉ����[���I�ɐF�𑝂₵�܂�",
	IDC_ENREVS		,	"���ɐݒ肵�Ă���F�̎�������ݒ��S�Ẵp���b�g�ɓK�p���܂�",
	IDC_PALEN		,	"�F�������I�Ɍ��肷�邩�A�l���w�肷�邩�I�����܂�",
	IDC_FZERO		,	"����̐F�������I�� 0�Ԃ̐F�ɂ��܂��iTPSET�p�j�A���̐F�w��̕��@��I�����܂�",
	IDC_FZCOLOR		,	"�����I�� 0�Ԃɂ���F���w�肷��{�^���ł�",
	IDC_FZC			,	"�����I�� 0�Ԃɂ���F�ł�",
	IDC_FZ_X		,	"�����I�� 0�Ԃɂ���F���摜����̎悷��ꍇ�̍��W�ł�",
	IDC_FZ_Y		,	"�����I�� 0�Ԃɂ���F���摜����̎悷��ꍇ�̍��W�ł�",
	IDC_CMBPLT		,	"�p���b�g�̏o�͕��@���w�肵�܂�",
	IDC_LOADPLT		,	"�p���b�g�t�@�C����ǂݍ��݂܂�",
	IDC_MSX1COLOR	,	"�p���b�g���l�r�w�P���̐F�ɂ��܂�",
	IDC_MSX2COLOR	,	"�p���b�g���l�r�w�Q�̏����l�̐F�ɂ��܂�",
	IDC_192			,	"�`�F�b�N��������192���C���i�C���^�[���[�X����384���C���j�p�̉摜���o�͂��܂�",
	IDC_SORT		,	"�p���b�g�̐F�𐮗񂳂�����@���w�肵�܂�",
	IDC_SAVEAS		,	"���ݕ\�����Ă���ݒ���t�@�C���ɕۑ����܂�",
	IDC_LOAD		,	"�ȑO�ۑ������ݒ�t�@�C����ǂݍ��݂܂��A���݂̕\�����e�͏����Ă��܂��̂ł����ӂ�������",
	IDC_DEFAULT		,	"���ݕ\�����Ă���ݒ���e�� �f�t�H���g�̐ݒ� �Ƃ��č̗p���܂�",
	IDC_CHKDEF		,	"�`�F�b�N������ƁA�ĕϊ����s�����Ƃ��̐ݒ肪�������܃f�t�H���g�ݒ�ɃR�s�[����܂�",
	IDC_CMBCNV		,	"�V�����摜�t�@�C����ϊ�����Ƃ��ɍ̗p����ݒ���w�肵�܂�",
	IDC_FILLC		,	"�摜�̑��݂��Ȃ����������̐F�Ŗ��߂܂��B�p���b�g����ɂ��e������ꍇ������̂Œ���",
	IDC_SIZEMODE	,	"�c����̌�����@���w�肵�܂�",
	IDC_RESAMPLE	,	"�T�C�Y���߂̍ۂ́A�F�̃T���v�����O���@���w�肵�܂��B��ʓI�Ƀm�[�}���̏ꍇ�A�k���摜�͌������򉻂��܂��B",
};

const char *SortMode[] = {
	"���܂���",
	"��������̂ݏ������",
	"��������̂ݍ~�����",
	"���S�������",
	"���S�~�����",
};

const char *UseSetting[] = {
	"��̫�Đݒ�",
	"�Ō�̕ϊ��ݒ�",
	"�\���摜�̐ݒ�",
};

const char *Resample[] = {
	"ɰ��",
	"�����ر�",
};

const char *SizeMode[] = {
	"�����I�ɏo�ͻ��ނɂ��킹��",
	"MSX�œ����c����Ɍ����黲��",
};

#endif

// �e���[�h�ɉ������R���g���[���̗L���E�������
static const int nCtrlCode[]={
	IDC_GOSA,IDC_GOSAVAL,IDC_GOSAERR,IDC_GOSARATIO,IDC_PAL,IDC_SELCOL,IDC_CMBPLT,IDC_COLTBL,IDC_LOADPLT,IDC_RED,IDC_GREEN,IDC_BLUE,IDC_CMBALGO,IDC_JKRC,IDC_PALEN,IDC_ENREVS,IDC_CMBERR2,IDC_TILE,IDC_SORT,IDC_INTER,IDC_MSX1COLOR, IDC_MSX2COLOR,IDC_192,IDC_SAVEPLT,
};

static const BOOL nCtrlEnbl[][ ELMCNT(nCtrlCode) ]={
	{TRUE   ,TRUE       ,TRUE       ,TRUE        ,TRUE    ,TRUE     ,TRUE       ,TRUE     ,TRUE        ,TRUE   ,TRUE     ,TRUE    ,FALSE      ,FALSE   ,TRUE     ,TRUE      ,TRUE       ,TRUE    ,TRUE    , FALSE   ,TRUE         ,TRUE   ,FALSE  ,TRUE  },   // SCREEN2/4
	{TRUE   ,TRUE       ,TRUE       ,TRUE        ,TRUE    ,TRUE     ,TRUE       ,TRUE     ,TRUE        ,TRUE   ,TRUE     ,TRUE    ,FALSE      ,FALSE   ,TRUE     ,TRUE      ,TRUE       ,TRUE    ,TRUE    , FALSE   ,TRUE         ,TRUE   ,FALSE  ,TRUE  },   // SCREEN3
	{TRUE   ,TRUE       ,TRUE       ,TRUE        ,TRUE    ,TRUE     ,TRUE       ,TRUE     ,TRUE        ,TRUE   ,TRUE     ,TRUE    ,FALSE      ,FALSE   ,TRUE     ,TRUE      ,TRUE       ,TRUE    ,TRUE    , TRUE    ,TRUE         ,TRUE   ,TRUE   ,TRUE  },   // SCREEN5
	{TRUE   ,TRUE       ,TRUE       ,TRUE        ,TRUE    ,TRUE     ,TRUE       ,TRUE     ,TRUE        ,TRUE   ,TRUE     ,TRUE    ,FALSE      ,FALSE   ,TRUE     ,TRUE      ,TRUE       ,TRUE    ,TRUE    , TRUE    ,TRUE         ,TRUE   ,TRUE   ,TRUE  },   // SCREEN6
	{TRUE   ,TRUE       ,TRUE       ,TRUE        ,TRUE    ,TRUE     ,TRUE       ,TRUE     ,TRUE        ,TRUE   ,TRUE     ,TRUE    ,FALSE      ,FALSE   ,TRUE     ,TRUE      ,TRUE       ,TRUE    ,TRUE    , TRUE    ,TRUE         ,TRUE   ,TRUE   ,TRUE  },   // SCREEN7
	{TRUE   ,TRUE       ,TRUE       ,TRUE        ,FALSE   ,FALSE    ,FALSE      ,FALSE    ,FALSE       ,FALSE  ,FALSE    ,FALSE   ,FALSE      ,FALSE   ,FALSE    ,FALSE     ,TRUE       ,TRUE    ,FALSE   , TRUE    ,FALSE        ,FALSE  ,TRUE   ,FALSE },   // SCREEN8
	{TRUE   ,TRUE       ,TRUE       ,TRUE        ,FALSE   ,FALSE    ,FALSE      ,FALSE    ,FALSE       ,FALSE  ,FALSE    ,FALSE   ,TRUE       ,TRUE    ,FALSE    ,FALSE     ,FALSE      ,FALSE   ,FALSE   , TRUE    ,FALSE        ,FALSE  ,TRUE   ,FALSE },   // SCREEN10/11
	{TRUE   ,TRUE       ,TRUE       ,TRUE        ,FALSE   ,FALSE    ,FALSE      ,FALSE    ,FALSE       ,FALSE  ,FALSE    ,FALSE   ,TRUE       ,TRUE    ,FALSE    ,FALSE     ,FALSE      ,FALSE   ,FALSE   , TRUE    ,FALSE        ,FALSE  ,TRUE   ,FALSE },   // SCREEN12
	{TRUE   ,TRUE       ,TRUE       ,TRUE        ,TRUE    ,TRUE     ,TRUE       ,TRUE     ,TRUE        ,TRUE   ,TRUE     ,TRUE    ,FALSE      ,FALSE   ,TRUE     ,TRUE      ,TRUE       ,TRUE    ,TRUE    , TRUE    ,TRUE         ,TRUE   ,TRUE   ,TRUE  },   // SCREEN5 (256 lines)
	{TRUE   ,TRUE       ,TRUE       ,TRUE        ,TRUE    ,TRUE     ,TRUE       ,TRUE     ,TRUE        ,TRUE   ,TRUE     ,TRUE    ,FALSE      ,FALSE   ,TRUE     ,TRUE      ,TRUE       ,TRUE    ,TRUE    , TRUE    ,TRUE         ,TRUE   ,TRUE   ,TRUE  },   // SCREEN6 (256 lines)
	{TRUE   ,TRUE       ,TRUE       ,TRUE        ,TRUE    ,TRUE     ,TRUE       ,TRUE     ,TRUE        ,TRUE   ,TRUE     ,TRUE    ,FALSE      ,FALSE   ,TRUE     ,TRUE      ,TRUE       ,TRUE    ,TRUE    , TRUE    ,TRUE         ,TRUE   ,TRUE   ,TRUE  },   // SCREEN7 (256 lines)
	{TRUE   ,TRUE       ,TRUE       ,TRUE        ,FALSE   ,FALSE    ,FALSE      ,FALSE    ,FALSE       ,FALSE  ,FALSE    ,FALSE   ,FALSE      ,FALSE   ,FALSE    ,FALSE     ,TRUE       ,TRUE    ,FALSE   , TRUE    ,FALSE        ,FALSE  ,TRUE   ,FALSE },   // SCREEN8 (256 lines)
	{TRUE   ,TRUE       ,TRUE       ,TRUE        ,FALSE   ,FALSE    ,FALSE      ,FALSE    ,FALSE       ,FALSE  ,FALSE    ,FALSE   ,TRUE       ,TRUE    ,FALSE    ,FALSE     ,FALSE      ,FALSE   ,FALSE   , TRUE    ,FALSE        ,FALSE  ,TRUE   ,FALSE },   // SCREEN10/11 (256 lines)
	{TRUE   ,TRUE       ,TRUE       ,TRUE        ,FALSE   ,FALSE    ,FALSE      ,FALSE    ,FALSE       ,FALSE  ,FALSE    ,FALSE   ,TRUE       ,TRUE    ,FALSE    ,FALSE     ,FALSE      ,FALSE   ,FALSE   , TRUE    ,FALSE        ,FALSE  ,TRUE   ,FALSE },   // SCREEN12 (256 lines)
};

SETTING EdtMode;
extern SETTING NowMode;
extern bool bSave;		// WndProc.cpp

// -------------------------------------------------------------
// �v���g�^�C�v
// -------------------------------------------------------------
EVENT( onSetClose		);
EVENT( onSetCommand		);
EVENT( onSetHScroll		);
EVENT( onSetInitDlg		);
EVENT( onSetInter		);
EVENT( onSetLoadPlt		);
EVENT( onSetMode		);
EVENT( onSetOk			);
EVENT( onSetGosa		);
EVENT( onSetPal			);
EVENT( onSetPltMode		);
EVENT( onSetResize		);
EVENT( onSetResample	);
EVENT( onSetSizeMode	);
EVENT( onSetSelCol		);
EVENT( onSetAutoName	);
EVENT( onSetAlgo		);
EVENT( onSetJKrc		);
EVENT( onSetErr			);
EVENT( onSetPreView		);
EVENT( onSetPalEnable	);
EVENT( onSetEnRevs		);
EVENT( onSetSaveAs		);
EVENT( onSetLoad		);
EVENT( onSetSeido		);
EVENT( onSetErrAdd		);
EVENT( onSetNonZero		);
EVENT( onSetFZero		);
EVENT( onSetFZColor		);
EVENT( onSetFCColor		);
EVENT( onSetFZX			);
EVENT( onSetFZY			);
EVENT( onSetTile		);
EVENT( onSetSort		);
EVENT( onSetDefault		);
EVENT( onSetChkDef		);
EVENT( onSetCmbCnv		);
EVENT( onSetMSX1		);
EVENT( onSetMSX2		);
EVENT( onSet192			);
EVENT( onSetSavePlt		);

EVENT( onColRButtonDown	);
EVENT( onColLButtonDown	);
EVENT( onColPaint		);
EVENT( onColMouseMove	);
EVENT( onColLButtonUp	);

EVENT( onFZPaint		);
EVENT( onFCPaint		);

static void UpdateColor( int c );
static void SetEditColor( HWND hWnd,int c,bool s );
static void ChangeColor( int p1, int p2 );

static void LoadPLFile( const char *szFileName );
static void SetCtlEnable( HWND hWnd );
static void UpdateAll( HWND hWnd );
static void _enddlg( HWND hWnd,int exitcode );

// -------------------------------------------------------------
// �O���[�o���ϐ�
// -------------------------------------------------------------
static HDC		hMemDC = NULL;			// �J���[�e�[�u���i�[�p�f�o�C�X
static HBITMAP	hMemBM = NULL;			// �J���[�e�[�u���i�[�p�r�b�g�}�b�v
static int w,h;							// �J���[�e�[�u���̃T�C�Y
static int NowCol=0;					// ���ݕҏW���̐F
static WNDPROC	ColProc = NULL;			// �p���b�g�̃f�t�H���g�E�B���h�E�֐�
static WNDPROC	FZProc = NULL;			// �����[�����F�̃f�t�H���g�E�B���h�E�֐�
static WNDPROC	FCProc = NULL;			// �w�i�F�̃f�t�H���g�E�B���h�E�֐�
static bool bDraging = false;			// �p���b�g���h���b�O�����ۂ�
extern HINSTANCE	hIns;				// �C���X�^���X�n���h��( main.cpp )
bool bRedo			= false;			// �ĕϊ�����̫�Đݒ肩
char szCaption[ 256 ]="";				// �^�C�g��
static COLORREF	CustColors[16]={		// �F�I���_�C�A���O�Ŏg�p����J�X�^���J���[
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
};

// -------------------------------------------------------------
// �C�x���g����

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�E�B���h�E�֐�
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		msg		...	(I)	���b�Z�[�W
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		0
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
WINPROC( SetProc )
{
	switch( msg )
	{
		ONEVENT( WM_CLOSE		, onSetClose	);
		ONEVENT( WM_COMMAND		, onSetCommand	);
		ONEVENT( WM_HSCROLL		, onSetHScroll	);
		ONEVENT( WM_INITDIALOG	, onSetInitDlg	);
	}
	return FALSE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�J���[�p���b�g�̃T�u�N���X�֐�
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		msg		...	(I)	���b�Z�[�W
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		0
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
WINPROC( SubColProc )
{
	switch( msg )
	{
		ONEVENT( WM_RBUTTONDOWN	, onColRButtonDown	);
		ONEVENT( WM_LBUTTONDOWN	, onColLButtonDown	);
		ONEVENT( WM_PAINT		, onColPaint		);
		ONEVENT( WM_MOUSEMOVE	, onColMouseMove	);
		ONEVENT( WM_LBUTTONUP	, onColLButtonUp	);
	}
	return CallWindowProc( ColProc, hWnd, msg, wp, lp );
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�����[�����F�\�������̃T�u�N���X�֐�
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		msg		...	(I)	���b�Z�[�W
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		0
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
WINPROC( SubFZProc )
{
	switch( msg )
	{
		ONEVENT( WM_PAINT		, onFZPaint			);
	}
	return CallWindowProc( FZProc, hWnd, msg, wp, lp );
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�w�i�F�\�����̃T�u�N���X�֐�
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h���v
//		msg		...	(I)	���b�Z�[�W
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		0
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
WINPROC( SubFCProc )
{
	switch( msg )
	{
		ONEVENT( WM_PAINT		, onFCPaint			);
	}
	return CallWindowProc( FCProc, hWnd, msg, wp, lp );
}

// -------------------------------------------------------------
// ��ʃC�x���g

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�_�C�A���O�̏�����
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSetInitDlg )
{
	HDC		hDC;
	RECT	r;
	HWND	hCol;
	HWND	hTip;
	int		i;

	// �^�C�g��
	SetWindowText( hWnd,szCaption );
	SetDlgItemText( hWnd, IDOK, szCaption );

	//	�����[�����F�̃T�u�N���X��
	hCol = GetDlgItem( hWnd, IDC_FZC );
	FZProc = (WNDPROC) GetWindowLong( hCol, GWL_WNDPROC );
	SetWindowLong( hCol, GWL_WNDPROC, (LONG)SubFZProc );

	//	�w�i�F�̃T�u�N���X��
	hCol = GetDlgItem( hWnd, IDC_FILLCOLOR );
	FCProc = (WNDPROC) GetWindowLong( hCol, GWL_WNDPROC );
	SetWindowLong( hCol, GWL_WNDPROC, (LONG)SubFCProc );

	//	�J���[�p���b�g�̃T�u�N���X���ƃr�b�g�}�b�v�̍쐬
	hCol = GetDlgItem( hWnd, IDC_COLTBL );
	ColProc = (WNDPROC) GetWindowLong( hCol, GWL_WNDPROC );
	SetWindowLong( hCol, GWL_WNDPROC, (LONG)SubColProc );

	hDC = GetDC( hWnd );
	GetClientRect( hCol, &r );
	w = r.right;
	h = r.bottom;
	hMemDC = CreateCompatibleDC( hDC );
	hMemBM = CreateCompatibleBitmap( hDC, w, h );
	if( hMemDC == NULL || hMemBM == NULL ) return TRUE;
	SelectBrush( hMemDC,GetStockBrush( NULL_BRUSH ) );
	SelectBitmap( hMemDC,hMemBM );
	UpdateAll( hWnd );

	//	�c�[���`�b�v��t����
	hTip = wu_ttip_create( hWnd );
	for( i = 0; i < ELMCNT( ttips ); ++i ) {
		wu_ttip_set_item( hTip, ttips[i].nId, ttips[i].sStr );
	}

	return TRUE;	// �����ōŏ��̃t�H�[�J�X�𓖂ĂĂ��炤
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�E�B���h�E���� / �{�^���F�L�����Z��
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSetClose )
{
	_enddlg( hWnd,-1 );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�R�}���h����
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSetCommand )
{
	switch( LOWORD( wp ) )
	{
	// �e�L�X�g�{�b�N�X
	ONEVENT( IDC_FZ_X		, onSetFZX		);		// �����[�����w���W
	ONEVENT( IDC_FZ_Y		, onSetFZY		);		// �����[�����x���W
	// �{�^��
	ONEVENT( IDOK			, onSetOk		);		// OK
	ONEVENT( IDCANCEL		, onSetClose	);		// ��ݾ�
	ONEVENT( IDC_LOADPLT	, onSetLoadPlt	);		// ��گēǂݍ���
	ONEVENT( IDC_ENREVS		, onSetEnRevs	);		// �S�ē�������
	ONEVENT( IDC_SAVEAS		, onSetSaveAs	);		// �t�@�C���֕ۑ�
	ONEVENT( IDC_LOAD		, onSetLoad		);		// �t�@�C������ǂݏo��
	ONEVENT( IDC_FZCOLOR	, onSetFZColor	);		// ������ۉ��F�w��
	ONEVENT( IDC_FILLC		, onSetFCColor	);		// �w�i�F�w��
	ONEVENT( IDC_DEFAULT	, onSetDefault	);		// ��̫�Ăֺ�߰
	ONEVENT( IDC_MSX1COLOR	, onSetMSX1		);		// MSX1�F
	ONEVENT( IDC_MSX2COLOR	, onSetMSX2		);		// MSX2�f�t�H���g�F
	ONEVENT( IDC_SAVEPLT	, onSetSavePlt	);		// ��گĕۑ�
	// �`�F�b�N�{�b�N�X
	ONEVENT( IDC_INTER		, onSetInter	);		// �C���^�[���[�X
	ONEVENT( IDC_PAL		, onSetPal		);		// �Œ�p���b�g
	ONEVENT( IDC_SELCOL		, onSetSelCol	);		// �I�F���[�h
	ONEVENT( IDC_AUTONAME	, onSetAutoName	);		// �����t�@�C��������
	ONEVENT( IDC_JKRC		, onSetJKrc		);		// �F���̍Čv�Z
	ONEVENT( IDC_GOSA		, onSetGosa		);		// �덷�g�U
	ONEVENT( IDC_CHKZERO	, onSetNonZero	);		// �O�Ԃ��g��Ȃ�
	ONEVENT( IDC_TILE		, onSetTile		);		// �Ԃ����^�C��
	ONEVENT( IDC_CHKDEF		, onSetChkDef	);		// �����̫�Ăֺ�߰
	ONEVENT( IDC_192		, onSet192		);		// 192���C���o��
	ONEVENT( IDC_RESIZE		, onSetResize	);		// ���ޒ���
	// �R���{�{�b�N�X
	ONEVENT( IDC_RESAMPLE	, onSetResample	);		// ���ޒ���ػ����
	ONEVENT( IDC_CMBSCREEN	, onSetMode		);		// Screen5
	ONEVENT( IDC_CMBPLT		, onSetPltMode	);		// �p���b�g���[�h
	ONEVENT( IDC_CMBALGO	, onSetAlgo		);		// ���R�搶���A���S���Y��
	ONEVENT( IDC_CMBERR		, onSetErr		);		// �f�B�U�p�^�[��
	ONEVENT( IDC_PREVIEW		, onSetPreView	);		// �o�͐�
	ONEVENT( IDC_SEIDO		, onSetSeido	);		// ���x
	ONEVENT( IDC_CMBERR2	, onSetErrAdd	);		// �f�B�U���Z���@
	ONEVENT( IDC_PALEN		, onSetPalEnable);		// �p���b�g�g�p���E������
	ONEVENT( IDC_FZERO		, onSetFZero	);		// ������ۉ�
	ONEVENT( IDC_SORT		, onSetSort		);		// �\�[�g
	ONEVENT( IDC_CMBCNV		, onSetCmbCnv	);		// �ϊ��ݒ�
	ONEVENT( IDC_SIZEMODE	, onSetSizeMode	);		// �T�C�Y�����T�C�Y���[�h
	}
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�J���[�p���b�g�ŉE�N���b�N����
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		0
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onColRButtonDown )
{
	POINT	pos;
	int		col;
	HWND	hPWnd;

	// �_�C�A���O�̃n���h��
	hPWnd = GetParent( hWnd );

	// �}�E�X�J�[�\���̈ʒu�𓾂�
	pos.x = GET_X_LPARAM( lp );
	pos.y = GET_Y_LPARAM( lp );

	// �w�肵�Ă���F�ԍ��𓾂�
	col = (((pos.x*8)/w) & 7) + (((pos.y*2)/h) & 1)*8;

	// �p���b�g�̗L�������ݒ��ύX����
	EdtMode.PalEn[ col ] = ( EdtMode.PalEn[ col ] + 1 ) % ELMCNT( PalEnName );
	SetEditColor( hPWnd, col, true );

	InvalidateRect( hWnd,NULL,FALSE );

	return 0;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�J���[�p���b�g�ō��N���b�N����
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		0
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onColLButtonDown )
{
	POINT	pos;
	int		c;

	// �}�E�X�J�[�\���̈ʒu�𓾂�
	pos.x = GET_X_LPARAM( lp );
	pos.y = GET_Y_LPARAM( lp );

	// �w�肵�Ă���F�ԍ��𓾂�
	c = (( ( pos.x * 8 ) / w ) & 7 ) + (( ( pos.y * 2 ) / h ) & 1 ) * 8;
	if( (EdtMode.Mode == MD_SC6 || EdtMode.Mode == MD_SC6_256L) && (c > 3) ) return 0;
	SetEditColor( GetParent( hWnd ), c, true );
	InvalidateRect( hWnd, NULL, FALSE );

	//	�}�E�X���L
	SetCapture( hWnd );
	bDraging = true;

	return 0;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�J���[�p���b�g�̍ĕ`��
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		0
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onColPaint )
{
	HDC			hDC;
	PAINTSTRUCT	ps;

	hDC = BeginPaint( hWnd,&ps );

	BitBlt( hDC, 0, 0, w, h, hMemDC, 0, 0, SRCCOPY );

	EndPaint( hWnd,&ps );

	return 0;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�J���[�p���b�g�ō��{�^���𗣂���
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		0
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onColLButtonUp )
{
	POINT	pos;
	int		p;

	if( !bDraging ) goto skip;
	//	�}�E�X�����
	ReleaseCapture();
	bDraging = false;

	// �}�E�X�J�[�\���̈ʒu�𓾂�
	pos.x = GET_X_LPARAM( lp );
	pos.y = GET_Y_LPARAM( lp );

	//	�͈͓�����
	if( pos.x < 0 || pos.y < 0 || pos.x >= w || pos.y >= h ) goto skip;

	// �I��F�ƃJ�[�\���ʒu�̐F�����ւ���
	p = (( ( pos.x * 8 ) / w ) & 7 ) + (( ( pos.y * 2 ) / h ) & 1 ) * 8;
	if( p < 0 || p > 15 ) goto skip;
	if( (EdtMode.Mode == MD_SC6 || EdtMode.Mode == MD_SC6_256L) && (p > 3) ) goto skip;
	ChangeColor( NowCol, p );

	//	�I���̍X�V
	SetEditColor( hWnd, p,true );
	InvalidateRect( hWnd, NULL, FALSE );
skip:
	return 0;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�J���[�p���b�g�ŃJ�[�\�����ړ�����
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		0
//	4.	���l
//		�J�[�\���̌`���ς���
// -------------------------------------------------------------
EVENT( onColMouseMove )
{
	if( bDraging ) {
		SetCursor( LoadCursor( hIns, MAKEINTRESOURCE( IDC_CHGPAL ) ) );
	} else {
		SetCursor( LoadCursor( NULL, IDC_ARROW ) );
	}

	return 0;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�����[�����̍ĕ`��
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		0
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onFZPaint )
{
	HDC			hDC;
	PAINTSTRUCT	ps;
	RECT		r;
	HBRUSH		hBr;

	hDC = BeginPaint( hWnd,&ps );

	GetClientRect( hWnd, &r );
	hBr = CreateSolidBrush( EdtMode.FZColor );
	if( hBr != NULL ) {
		FillRect( hDC, &r, hBr );
		DeleteBrush( hBr );
	}

	EndPaint( hWnd,&ps );

	return 0;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�w�i�F�̍ĕ`��
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		0
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onFCPaint )
{
	HDC			hDC;
	PAINTSTRUCT	ps;
	RECT		r;
	HBRUSH		hBr;

	hDC = BeginPaint( hWnd,&ps );

	GetClientRect( hWnd, &r );
	hBr = CreateSolidBrush( EdtMode.FCColor );
	if( hBr != NULL ) {
		FillRect( hDC, &r, hBr );
		DeleteBrush( hBr );
	}

	EndPaint( hWnd,&ps );

	return 0;
}

// -------------------------------------------------------------
// �e�L�X�g�{�b�N�X

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�����[�����@�w���W
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSetFZX )
{
	EdtMode.FZX = 0;
	EdtMode.FZX = GetDlgItemInt( hWnd,IDC_FZ_X,NULL,FALSE );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�����[�����@�x���W
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSetFZY )
{
	EdtMode.FZY = 0;
	EdtMode.FZY = GetDlgItemInt( hWnd,IDC_FZ_Y,NULL,FALSE );
	return TRUE;
}

// -------------------------------------------------------------
// �{�^���C�x���g

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�{�^���F��Ƀf�t�H���g�փR�s�[
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSetOk )
{
	if( EdtMode.Mode == MD_SC2 || EdtMode.Mode == MD_SC3 ) {
		EdtMode.Inter = false;
	}

	if( NowMode.bDefault ) {
		CALLEVENT( onSetDefault );
	}

	_enddlg( hWnd,0 );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		������ۉ��̐F�w��C�x���g
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSetFZColor )
{
	CHOOSECOLOR	cc;
	// �F�I���_�C�A���O�̐ݒ�
	memset( &cc, 0, sizeof( cc ) );
	cc.lStructSize	= sizeof( cc );
	cc.hwndOwner	= hWnd;
	cc.lpCustColors	= CustColors;
	cc.Flags		= CC_FULLOPEN | CC_RGBINIT;
	cc.rgbResult	= EdtMode.FZColor;
	if( !ChooseColor( &cc ) ) return TRUE;
	EdtMode.FZColor	= cc.rgbResult;
	InvalidateRect( hWnd, NULL, FALSE );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		������ۉ��̐F�w��C�x���g
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSetFCColor )
{
	CHOOSECOLOR	cc;
	// �F�I���_�C�A���O�̐ݒ�
	memset( &cc, 0, sizeof( cc ) );
	cc.lStructSize	= sizeof( cc );
	cc.hwndOwner	= hWnd;
	cc.lpCustColors	= CustColors;
	cc.Flags		= CC_FULLOPEN | CC_RGBINIT;
	cc.rgbResult	= EdtMode.FCColor;
	if( !ChooseColor( &cc ) ) return TRUE;
	EdtMode.FCColor	= cc.rgbResult;
	InvalidateRect( hWnd, NULL, FALSE );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�S�ē������C�x���g
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSetEnRevs		)
{
	int i, j;

	if( EdtMode.Mode == MD_SC6 || EdtMode.Mode == MD_SC6_256L ) {
		j = 4;
	} else {
		j = 16;
	}

	for( i = 0; i < j; ++i ){
		EdtMode.PalEn[ i ] = EdtMode.PalEn[ NowCol ];
		UpdateColor( i );
	}
	InvalidateRect( hWnd, NULL, FALSE );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		��گēǂݍ��݃C�x���g
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSetLoadPlt )
{
	char			szFileName[ MAX_PATH ];
	OPENFILENAME	ofn;
	int				i;
	// �t�@�C���������
	szFileName[ 0 ] = '\0';
	ZeroMemory( &ofn, sizeof( ofn ) );
	ofn.lStructSize		= sizeof( ofn );
	ofn.hInstance		= hIns;
	ofn.hwndOwner		= hWnd;
	ofn.lpstrFilter		= cszDefPLL;
	ofn.lpstrCustomFilter= NULL;
	ofn.nFilterIndex	= 1;
	ofn.Flags			= OFN_LONGNAMES | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_EXPLORER;
	ofn.lpstrFile		= szFileName;
	ofn.nMaxFile		= MAX_PATH;
	ofn.lpstrFileTitle	= NULL;
	ofn.lpstrInitialDir	= NULL;	// �J�����g
	ofn.lpstrTitle		= PLLoad;
	if( !GetOpenFileName( &ofn ) ) return TRUE;
	// �ǂݍ���
	i = strlen( szFileName );
	if( (i > 4) && ( toupper(szFileName[ i-3 ]) == 'P' ) ) {	//	*.P?? ���ǂ�������
		LoadPLFile( szFileName );
	} else {
		bsvLoadPLBsaveFile( &EdtMode, szFileName );
	}
	// ��ʂ̍X�V
	for( i = 0; i < 16; ++i ){
		UpdateColor( i );
	}
	SetEditColor( hWnd, NowCol, true );
	InvalidateRect( hWnd, NULL, FALSE );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		��گĕۑ��C�x���g
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSetSavePlt )
{
	char			szFileName[ MAX_PATH ];
	OPENFILENAME	ofn;
	// �t�@�C���������
	szFileName[ 0 ] = '\0';
	ZeroMemory( &ofn, sizeof( ofn ) );
	ofn.lStructSize		= sizeof( ofn );
	ofn.hInstance		= hIns;
	ofn.hwndOwner		= hWnd;
	ofn.lpstrFilter		= cszDefPL;
	ofn.lpstrCustomFilter= NULL;
	ofn.nFilterIndex	= 1;
	ofn.Flags			= OFN_LONGNAMES | OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_OVERWRITEPROMPT | OFN_EXPLORER;
	ofn.lpstrFile		= szFileName;
	ofn.nMaxFile		= MAX_PATH;
	ofn.lpstrFileTitle	= NULL;
	ofn.lpstrInitialDir	= NULL;	// �J�����g
	ofn.lpstrTitle		= PLSave;
	if( !GetSaveFileName( &ofn ) ) return TRUE;
	// �ǂݍ���
	bsvSavePLFile( hWnd, szFileName, &EdtMode );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�ݒ��ۑ��C�x���g
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSetSaveAs )
{
	char szFileName[ MAX_PATH ];
	// ̧�ٖ������
	if( !GetName( hWnd, szFileName, MAX_PATH, NULL, cszDefCfg, CfgExt ) ) return TRUE;
	// �ۑ�����
	SaveCfgFile( &EdtMode, szFileName );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�ݒ�̓ǂݏo���C�x���g
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSetLoad )
{
	char szFileName[ MAX_PATH ];
	OPENFILENAME	ofn;
	// �t�@�C�����J��
	szFileName[0]='\0';
	ZeroMemory( &ofn, sizeof( ofn ) );
	ofn.lStructSize		= sizeof( ofn );
	ofn.hInstance		= hIns;
	ofn.hwndOwner		= hWnd;
	ofn.lpstrFilter		= cszDefCfg;
	ofn.lpstrCustomFilter= NULL;
	ofn.nFilterIndex	= 1;
	ofn.Flags			= OFN_LONGNAMES | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	ofn.lpstrFile		= szFileName;
	ofn.nMaxFile		= MAX_PATH;
	ofn.lpstrFileTitle	= NULL;
	ofn.lpstrInitialDir	= NULL;	// �J�����g
	ofn.lpstrTitle		= cOpenFile;
	if( !GetOpenFileName( &ofn ) ) return TRUE;
	// �ݒ�𔽉f
	GetCfgFile( &EdtMode, szFileName );
	// ��ʂ̍X�V
	UpdateAll( hWnd );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		��̫�Ăֺ�߰
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSetDefault )
{
	bool	bDefault;

	//	�ĕϊ��̎��̂ݏ�������
	if( !bRedo ) return TRUE;

	bDefault = NowMode.bDefault;
	NowMode = EdtMode;
	NowMode.bDefault = bDefault;
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�l�r�w�P�F
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSetMSX1 )
{
	set_msx1_palette( EdtMode.Col );
	UpdateAll( hWnd );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�l�r�w�Q�f�t�H���g�F
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSetMSX2 ){
	set_msx2_palette( EdtMode.Col );
	UpdateAll( hWnd );
	return TRUE;
}

// -------------------------------------------------------------
// �`�F�b�N�{�b�N�X

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�O�Ԃ��g��Ȃ��`�F�b�N�C�x���g
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSetNonZero )
{
	EdtMode.NonZero = !EdtMode.NonZero;
	UpdateColor( 0 );
	InvalidateRect( hWnd, NULL, FALSE );
	return TRUE;
}


// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�덷�g�U�`�F�b�N�C�x���g
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSetGosa )
{
	EdtMode.diffusion_error_enable = !EdtMode.diffusion_error_enable;
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�덷�g�U�`�F�b�N�C�x���g
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSetPalEnable )
{
	EdtMode.PalEn[ NowCol ] = ComboBox_GetCurSel( GetDlgItem( hWnd, IDC_PALEN ) );
	UpdateColor( NowCol );
	InvalidateRect( hWnd, NULL, FALSE );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�C���^�[���[�X�`�F�b�N�C�x���g
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSetInter )
{
	EdtMode.Inter = !EdtMode.Inter;
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�����t�@�C��������`�F�b�N�C�x���g
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSetAutoName )
{
	EdtMode.AutoName = !EdtMode.AutoName;
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�F���Čv�Z�`�F�b�N�C�x���g
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSetJKrc )
{
	EdtMode.JKrc = !EdtMode.JKrc;
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�S�Œ�p���b�g�`�F�b�N�C�x���g
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSetPal )
{
	int i;
	EdtMode.Pal = !EdtMode.Pal;
	for( i=0; i<16; ++i ){
		UpdateColor( i );
	}
	InvalidateRect( hWnd,NULL,FALSE );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�I�F���[�h�`�F�b�N�C�x���g
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSetSelCol )
{
	EdtMode.SelCol = !EdtMode.SelCol;
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�Ԃ����^�C���`�F�b�N�C�x���g
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSetTile )
{
	EdtMode.Tile = !EdtMode.Tile;
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		��Ƀf�t�H���g��
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSetChkDef )
{
	EdtMode.bDefault = !EdtMode.bDefault;
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		192���C�����[�h
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSet192 )
{
	EdtMode.b192 = !EdtMode.b192;
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�T�C�Y���߃C�x���g
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSetResize )
{
	EdtMode.Resize = !EdtMode.Resize;
	SetCtlEnable( hWnd );
	return TRUE;
}

// -------------------------------------------------------------
// �X���C�_

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�X���C�_�C�x���g
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSetHScroll )
{
	HWND	hCol = (HWND)lp;
	int		pos  = SendMessage( hCol, TBM_GETPOS, 0, 0 );
	char	szBuf[32];

	switch( GetDlgCtrlID( hCol ) )
	{
	case IDC_GOSAVAL:	// �덷�g�U�W��
		EdtMode.diffusion_error_coef = float( pos ) / 1000;
		wsprintf( szBuf, "0.%03d", pos );
		SetDlgItemText( hWnd, IDC_GOSAVALS, szBuf );
		return TRUE;
	case IDC_GOSAERR:	// �덷�؂�̂Ēl
		EdtMode.err = pos;
		wsprintf( szBuf, "%d", pos );
		SetDlgItemText( hWnd, IDC_GOSAERRS, szBuf );
		return TRUE;
	case IDC_GOSARATIO:	// �덷�䗦
		EdtMode.diffusion_error_x_weight = float( pos ) / 100;
		wsprintf( szBuf, "%3d%% : %3d%%", pos, 100 - pos );
		SetDlgItemText( hWnd, IDC_GOSARATIOS, szBuf );
		return TRUE;
	case IDC_RED:		// ��
		EdtMode.Col[ NowCol ].red   = pos;
		break;
	case IDC_GREEN:		// ��
		EdtMode.Col[ NowCol ].green = pos;
		break;
	case IDC_BLUE:		// ��
		EdtMode.Col[ NowCol ].blue  = pos;
		break;
	default:
		return FALSE;
	}
	SetEditColor( hWnd, NowCol, false );
	InvalidateRect( hWnd, NULL, FALSE );
	return TRUE;
}

// -------------------------------------------------------------
// �R���{�{�b�N�X

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�T�C�Y���߃��T���v���C�x���g
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSetResample )
{
	EdtMode.Resample = ComboBox_GetCurSel( GetDlgItem( hWnd, IDC_RESAMPLE ) );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�T�C�Y���߃T�C�Y���[�h
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSetSizeMode )
{
	EdtMode.SizeMode = ComboBox_GetCurSel( GetDlgItem( hWnd, IDC_SIZEMODE ) );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		���[�h�ݒ�R���{�{�b�N�X�C�x���g
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSetFZero )
{
	EdtMode.FourceZero = ComboBox_GetCurSel( GetDlgItem( hWnd, IDC_FZERO ) );
	// �R���g���[���� ON/OFF
	SetCtlEnable( hWnd );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		���[�h�ݒ�R���{�{�b�N�X�C�x���g
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSetMode )
{
	if( HIWORD( wp ) != CBN_SELCHANGE ) return TRUE;
	EdtMode.Mode = ComboBox_GetCurSel( GetDlgItem( hWnd, IDC_CMBSCREEN ) );
	// �R���g���[���� ON/OFF
	SetCtlEnable( hWnd );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		���R�搶���A���S���Y���C�x���g
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSetAlgo )
{
	EdtMode.AlgoMode = ComboBox_GetCurSel( GetDlgItem( hWnd, IDC_CMBALGO ) );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�f�B�U�p�^�[���C�x���g
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSetErr )
{
	EdtMode.ErrAlgo = ComboBox_GetCurSel( GetDlgItem( hWnd,IDC_CMBERR ) );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�f�B�U���Z���@�C�x���g
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSetErrAdd )
{
	EdtMode.ErrAdd = ComboBox_GetCurSel( GetDlgItem( hWnd, IDC_CMBERR2 ) );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�o�͐�C�x���g
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSetPreView )
{
	EdtMode.PreView = ComboBox_GetCurSel( GetDlgItem( hWnd, IDC_PREVIEW ) ) + 1;
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		���x�C�x���g
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSetSeido )
{
	EdtMode.Seido = ComboBox_GetCurSel( GetDlgItem( hWnd, IDC_SEIDO ) );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�p���b�g���[�h�C�x���g
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSetPltMode )
{
	EdtMode.PltMode = ComboBox_GetCurSel( GetDlgItem( hWnd,IDC_CMBPLT ) );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�\�[�g���[�h�C�x���g
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSetSort )
{
	EdtMode.SortMode = ComboBox_GetCurSel( GetDlgItem( hWnd,IDC_SORT ) );
	return TRUE;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�ϊ��ݒ�
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//		wp		...	(I)	�v�p�����[�^
//		lp		...	(I)	�k�p�����[�^
//	3.	�Ԓl
//		TRUE
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
EVENT( onSetCmbCnv )
{
	EdtMode.CnvMode = ComboBox_GetCurSel( GetDlgItem( hWnd, IDC_CMBCNV ) );
	return TRUE;
}

// -------------------------------------------------------------
// ���[�e�B���e�B

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		��ʂ��X�V����
//	2.	����
//		hWnd	..	(I)	�_�C�A���O�̃n���h��
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static void UpdateAll( HWND hWnd )
{
	int		i;
	char	szBuf[ 32 ];
	// �J���[�e�[�u����`�悷��
	for( i = 0; i < 16; i++ ) UpdateColor( i );
	// �o�̓��[�h
	Button_SetCheck( GetDlgItem( hWnd, IDC_GOSA		), EdtMode.diffusion_error_enable		? BST_CHECKED : BST_UNCHECKED );
	Button_SetCheck( GetDlgItem( hWnd, IDC_INTER	), EdtMode.Inter	? BST_CHECKED : BST_UNCHECKED );
	Button_SetCheck( GetDlgItem( hWnd, IDC_PAL		), EdtMode.Pal		? BST_CHECKED : BST_UNCHECKED );
	Button_SetCheck( GetDlgItem( hWnd, IDC_SELCOL	), EdtMode.SelCol	? BST_CHECKED : BST_UNCHECKED );
	Button_SetCheck( GetDlgItem( hWnd, IDC_RESIZE	), EdtMode.Resize	? BST_CHECKED : BST_UNCHECKED );
	Button_SetCheck( GetDlgItem( hWnd, IDC_AUTONAME	), EdtMode.AutoName	? BST_CHECKED : BST_UNCHECKED );
	Button_SetCheck( GetDlgItem( hWnd, IDC_JKRC		), EdtMode.JKrc		? BST_CHECKED : BST_UNCHECKED );
	Button_SetCheck( GetDlgItem( hWnd, IDC_CHKZERO	), EdtMode.NonZero	? BST_CHECKED : BST_UNCHECKED );
	Button_SetCheck( GetDlgItem( hWnd, IDC_TILE		), EdtMode.Tile		? BST_CHECKED : BST_UNCHECKED );
	Button_SetCheck( GetDlgItem( hWnd, IDC_CHKDEF	), EdtMode.bDefault	? BST_CHECKED : BST_UNCHECKED );
	Button_SetCheck( GetDlgItem( hWnd, IDC_192		), EdtMode.b192		? BST_CHECKED : BST_UNCHECKED );
	// �X���C�_�͈̔͂�ݒ�
	SetSliderRange( GetDlgItem( hWnd, IDC_GOSAVAL   ), 1, 500 );
	SetSliderRange( GetDlgItem( hWnd, IDC_GOSAERR   ), 0, 255 );
	SetSliderRange( GetDlgItem( hWnd, IDC_GOSARATIO ), 0, 100 );
	SetSliderRange( GetDlgItem( hWnd, IDC_RED       ), 0, 7   );
	SetSliderRange( GetDlgItem( hWnd, IDC_GREEN     ), 0, 7   );
	SetSliderRange( GetDlgItem( hWnd, IDC_BLUE      ), 0, 7   );
	// �X���C�_�̈ʒu�𒲐�
	SetSliderValue( GetDlgItem( hWnd, IDC_GOSAVAL   ), (short int)(EdtMode.diffusion_error_coef * 1000) );
	SetSliderValue( GetDlgItem( hWnd, IDC_GOSAERR   ), (short int)(EdtMode.err) );
	SetSliderValue( GetDlgItem( hWnd, IDC_GOSARATIO ), (short int)(EdtMode.diffusion_error_x_weight * 100) );
	wsprintf( szBuf, "0.%03d", int(EdtMode.diffusion_error_coef * 1000) );
	SetDlgItemText( hWnd, IDC_GOSAVALS, szBuf );
	wsprintf( szBuf, "%d", EdtMode.err );
	SetDlgItemText( hWnd, IDC_GOSAERRS, szBuf );
	wsprintf( szBuf, "%3d%% : %3d%%", int(EdtMode.diffusion_error_x_weight * 100), 100 - int(EdtMode.diffusion_error_x_weight * 100) );
	SetDlgItemText( hWnd, IDC_GOSARATIOS, szBuf );
	SetEditColor( hWnd, 0, true );
	InvalidateRect( hWnd, NULL, FALSE );
	// �R���{�{�b�N�X
	ComboBox_Init( hWnd, IDC_CMBSCREEN	,ScrModeName,	ELMCNT( ScrModeName ),	EdtMode.Mode );
	ComboBox_Init( hWnd, IDC_CMBPLT		,PltMode,		PLT_COUNT,				EdtMode.PltMode );
	ComboBox_Init( hWnd, IDC_CMBALGO		,AlgoName,		ELMCNT( AlgoName ),		EdtMode.AlgoMode );
	ComboBox_Init( hWnd, IDC_CMBERR		,ErrAlgoName,	ELMCNT( ErrAlgoName ),	EdtMode.ErrAlgo );
	ComboBox_Init( hWnd, IDC_PREVIEW		,PreViewName,	ELMCNT( PreViewName ),	EdtMode.PreView - 1 );
	ComboBox_Init( hWnd, IDC_SEIDO		,ColorSeido,	ELMCNT( ColorSeido ),	EdtMode.Seido );
	ComboBox_Init( hWnd, IDC_CMBERR2		,ErrAlgo2Name,	ELMCNT( ErrAlgo2Name ),	EdtMode.ErrAdd );
	ComboBox_Init( hWnd, IDC_PALEN		,PalEnName,		ELMCNT( PalEnName ),	EdtMode.PalEn[ NowCol ] );
	ComboBox_Init( hWnd, IDC_FZERO		,FZColorName,	ELMCNT( FZColorName ),	EdtMode.FourceZero );
	ComboBox_Init( hWnd, IDC_SORT		,SortMode,		ELMCNT( SortMode ),		EdtMode.SortMode );
	ComboBox_Init( hWnd, IDC_CMBCNV		,UseSetting,	ELMCNT( UseSetting ),	EdtMode.CnvMode );
	ComboBox_Init( hWnd, IDC_RESAMPLE	,Resample,		ELMCNT( Resample ),		EdtMode.Resample );
	ComboBox_Init( hWnd, IDC_SIZEMODE	,SizeMode,		ELMCNT( SizeMode ),		EdtMode.SizeMode );
	// �e�L�X�g�{�b�N�X
	SetDlgItemInt( hWnd, IDC_FZ_X, EdtMode.FZX, FALSE );
	SetDlgItemInt( hWnd, IDC_FZ_Y, EdtMode.FZY, FALSE );
	// �R���g���[���� ON/OFF
	SetCtlEnable( hWnd );
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�J���[�e�[�u���̐F���X�V����
//	2.	����
//		c	...	(I)	�X�V����F�ԍ�
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static void UpdateColor( int c )
{
	RECT	r;
	HBRUSH	hBr;
	HPEN	hOld, hPen1, hPen2;
	bool	bDisable;

	//	SCREEN 6 �΍�
	bDisable = ((EdtMode.Mode == MD_SC6 || EdtMode.Mode == MD_SC6_256L) && c > 3);

	r.left		= w * (  c & 7 )	   / 8;
	r.right		= w * (( c & 7 ) + 1 ) / 8;
	if( c & 8 ){
		r.top		= h / 2;
		r.bottom	= h;
	}else{
		r.top		= 0;
		r.bottom	= h / 2;
	}
	if( bDisable ) {
		hBr = CreateSolidBrush( GetSysColor( COLOR_APPWORKSPACE ) );
	} else {
		hBr = CreateSolidBrush( RGB( convert7to255_r[ EdtMode.Col[ c ].red   ],
									 convert7to255_g[ EdtMode.Col[ c ].green ],
									 convert7to255_b[ EdtMode.Col[ c ].blue  ] ) );
	}
	FillRect( hMemDC, &r, hBr );
	DeleteBrush( hBr );
	if( !bDisable ) {
		hPen1 = CreatePen( PS_SOLID, 0, COLCUR1 );
		hPen2 = CreatePen( PS_SOLID, 0, COLCUR2 );
		if( EdtMode.PalEn[ c ] == PALEN_NONE || ( c == 0 && EdtMode.NonZero ) ){
			// �g�p�����̏ꍇ�͎ΐ�������
			DrawIcon( hMemDC, r.left, r.top, LoadIcon( wu_get_instance(), MAKEINTRESOURCE(IDI_NOUSE) ) );
		}
		if( EdtMode.PalEn[ c ] == PALEN_USE || EdtMode.Pal ){
			// �Œ�Ȃ牡��������
			DrawIcon( hMemDC, r.left, r.top, LoadIcon( wu_get_instance(), MAKEINTRESOURCE(IDI_HOLD) ) );
		}
		if( c == NowCol ){
			// ���݂̒��ړ_�Ȃ�֊s�̕\��
			hOld = SelectPen( hMemDC, hPen1 );
			Rectangle( hMemDC, r.left + 1, r.top + 1, r.right - 1, r.bottom - 1 );
			SelectPen( hMemDC, hPen2 );
			Rectangle( hMemDC, r.left, r.top, r.right, r.bottom );
			SelectPen( hMemDC, hOld );
		}
		DeletePen( hPen2 );
		DeletePen( hPen1 );
	}
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�Q�̃p���b�g�̓��e����������
//	2.	����
//		p1		...	(I)	�p���b�g�ԍ��P
//		p2		...	(I)	�p���b�g�ԍ��Q
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static void ChangeColor( int p1, int p2 )
{
	int	t;

	//	SCREEN6 �΍�
	if( (EdtMode.Mode == MD_SC6 || EdtMode.Mode == MD_SC6_256L) && ((p1 > 3) || (p2 > 3)) ) return;

	//	�p���b�g�l�̌���
	t = EdtMode.Col[ p1 ].red;		EdtMode.Col[ p1 ].red	= EdtMode.Col[ p2 ].red;	EdtMode.Col[ p2 ].red	= t;
	t = EdtMode.Col[ p1 ].green;	EdtMode.Col[ p1 ].green	= EdtMode.Col[ p2 ].green;	EdtMode.Col[ p2 ].green	= t;
	t = EdtMode.Col[ p1 ].blue;		EdtMode.Col[ p1 ].blue	= EdtMode.Col[ p2 ].blue;	EdtMode.Col[ p2 ].blue	= t;

	//	����Ԃ̌���
	t = EdtMode.PalEn[ p1 ];		EdtMode.PalEn[ p1 ]		= EdtMode.PalEn[ p2 ];		EdtMode.PalEn[ p2 ]		= t;
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�ҏW�F��ύX����
//	2.	����
//		hWnd	...	(I)	�_�C�A���O�n���h��
//		c		...	(I)	�ҏW�������F
//		s		...	(I)	�X���C�_���X�V���邩�ۂ�
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static void SetEditColor( HWND hWnd, int c, bool s )
{
	char szBuf[ 32 ];
	int back;
	back	= NowCol;
	if( (EdtMode.Mode == MD_SC6 || EdtMode.Mode == MD_SC6_256L) && c > 3) return;
	NowCol	= c;
	UpdateColor( back );
	if( s ){
		SetSliderValue( GetDlgItem( hWnd,IDC_RED     ),EdtMode.Col[c].red   );
		SetSliderValue( GetDlgItem( hWnd,IDC_GREEN   ),EdtMode.Col[c].green );
		SetSliderValue( GetDlgItem( hWnd,IDC_BLUE    ),EdtMode.Col[c].blue  );
	}
	wsprintf( szBuf, "COLOR=%d", c );
	SetDlgItemText( hWnd, IDC_COL	, szBuf );
	wsprintf( szBuf, "R=%d", EdtMode.Col[ c ].red   );
	SetDlgItemText( hWnd, IDC_NRED	, szBuf );
	wsprintf( szBuf, "G=%d", EdtMode.Col[ c ].green );
	SetDlgItemText( hWnd, IDC_NGREEN, szBuf );
	wsprintf( szBuf, "B=%d", EdtMode.Col[ c ].blue  );
	SetDlgItemText( hWnd, IDC_NBLUE	, szBuf );
	UpdateColor( NowCol );
	ComboBox_SetCurSel( GetDlgItem( hWnd, IDC_PALEN	), EdtMode.PalEn[ NowCol ] );
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		PL? �`���̃p���b�g��ǂݍ���
//	2.	����
//		szFileName	...	(I)	�t�@�C����
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static void LoadPLFile( const char *szFileName )
{
	FU_FILE	*hFile;
	BYTE	Buf[ 32 ];
	char	szBuf[ MAX_PATH + 256 ];
	int		i;

	// �t�@�C�����J��
	hFile = fu_open( szFileName, "rb" );
	if( hFile == FU_INVALID_HANDLE ){
#ifdef _ENGLISH
		wsprintf( szBuf,"Read error (%s)",szFileName );
#else
		wsprintf( szBuf,"%s �̓ǂݍ��݂Ɏ��s���܂����B",szFileName );
#endif
		MessageBox( NULL, szBuf, MsgCap, MB_OK | MB_ICONWARNING );
		return;		// �t�@�C�����J���Ȃ�
	}
	// �f�[�^����͂���
	memset( Buf, 0,sizeof( Buf ) );
	fu_read( hFile, Buf, sizeof( Buf ) );
	// ���̓f�[�^�𔽉f����
	for( i = 0; i < 16; ++i ){
		EdtMode.Col[ i ].red  = (Buf[ i * 2 + 0 ] >> 4 ) & 0x07;
		EdtMode.Col[ i ].blue =  Buf[ i * 2 + 0 ]        & 0x07;
		EdtMode.Col[ i ].green=  Buf[ i * 2 + 1 ]        & 0x07;
	}
	// �I��
	fu_close( hFile );
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		���[�h�ɍ��킹���e�R���g���[���� ON/OFF
//	2.	����
//		hWnd	...	(I)	�E�B���h�E�n���h��
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		�Ȃ�
// -------------------------------------------------------------
static void SetCtlEnable( HWND hWnd )
{
	int i;
	//	��ʂ̃R���g���[��
	for( i = 0; i < ELMCNT( nCtrlCode ); ++i ){
		EnableWindow( GetDlgItem( hWnd,nCtrlCode[ i ] ), nCtrlEnbl[ EdtMode.Mode ][ i ] );
	}
	// ������ۉ�
	if( EdtMode.Mode < MD_SC10 || (EdtMode.Mode >= MD_SC5_256L && EdtMode.Mode < MD_SC10_256L) ) {
		EnableWindow( GetDlgItem( hWnd, IDC_FZERO ), TRUE );
		switch( EdtMode.FourceZero ){
		case FZ_NONE:
			EnableWindow( GetDlgItem( hWnd, IDC_FZCOLOR	), FALSE );
			EnableWindow( GetDlgItem( hWnd, IDC_FZ_X	), FALSE );
			EnableWindow( GetDlgItem( hWnd, IDC_FZ_Y	), FALSE );
			break;
		case FZ_COLOR:
			EnableWindow( GetDlgItem( hWnd, IDC_FZCOLOR	), TRUE  );
			EnableWindow( GetDlgItem( hWnd, IDC_FZ_X	), FALSE );
			EnableWindow( GetDlgItem( hWnd, IDC_FZ_Y	), FALSE );
			break;
		case FZ_MATRIX:
			EnableWindow( GetDlgItem( hWnd, IDC_FZCOLOR	), FALSE );
			EnableWindow( GetDlgItem( hWnd, IDC_FZ_X	), TRUE  );
			EnableWindow( GetDlgItem( hWnd, IDC_FZ_Y	), TRUE  );
			break;
		}
	}else{
		EnableWindow( GetDlgItem( hWnd, IDC_FZERO	), FALSE );
		EnableWindow( GetDlgItem( hWnd, IDC_FZCOLOR	), FALSE );
		EnableWindow( GetDlgItem( hWnd, IDC_FZ_X	), FALSE );
		EnableWindow( GetDlgItem( hWnd, IDC_FZ_Y	), FALSE );
	}
	//	�p���b�g�\��
	if( (EdtMode.Mode == MD_SC6 || EdtMode.Mode == MD_SC6_256L) && (NowCol > 3) ) {
		SetEditColor( hWnd, 3, true );
	}
	if( EdtMode.Mode < MD_SC8 || (EdtMode.Mode >= MD_SC5_256L && EdtMode.Mode < MD_SC8_256L) ) {
		for( i = 0; i < 16; ++i ){
			UpdateColor( i );
		}
		InvalidateRect( hWnd, NULL, FALSE );
		ShowWindow( GetDlgItem( hWnd, IDC_COLTBL ), SW_SHOW );
	} else {
		ShowWindow( GetDlgItem( hWnd, IDC_COLTBL ), SW_HIDE );
	}
	//	�ĕϊ�����̫�Đݒ�̈Ⴂ
	if( bRedo ) {
		EnableWindow( GetDlgItem( hWnd, IDC_DEFAULT ), EdtMode.bDefault ? FALSE : TRUE );
		ShowWindow( GetDlgItem( hWnd, IDC_DEFAULT ), SW_SHOW );
		ShowWindow( GetDlgItem( hWnd, IDC_CHKDEF  ), SW_HIDE );
		ShowWindow( GetDlgItem( hWnd, IDC_CMBCNV  ), SW_HIDE );
		ShowWindow( GetDlgItem( hWnd, IDC_LBLSSET ), SW_HIDE );
	} else {
		ShowWindow( GetDlgItem( hWnd, IDC_DEFAULT ), SW_HIDE );
		ShowWindow( GetDlgItem( hWnd, IDC_CHKDEF  ), SW_SHOW );
		ShowWindow( GetDlgItem( hWnd, IDC_CMBCNV  ), SW_SHOW );
		ShowWindow( GetDlgItem( hWnd, IDC_LBLSSET ), SW_SHOW );
	}
	//	�T�C�Y����
	if( EdtMode.Resize ) {
		EnableWindow( GetDlgItem( hWnd, IDC_RESAMPLE ), TRUE );
		EnableWindow( GetDlgItem( hWnd, IDC_SIZEMODE ), TRUE );
	} else {
		EnableWindow( GetDlgItem( hWnd, IDC_RESAMPLE ), FALSE );
		EnableWindow( GetDlgItem( hWnd, IDC_SIZEMODE ), FALSE );
	}
}

// -------------------------------------------------------------
//	1.	���{�ꖼ
//		�I��
//	2.	����
//		hWnd		...	(I)	�E�B���h�E�n���h��
//		exitcode	...	(I)	�_�C�A���O�I���R�[�h
//	3.	�Ԓl
//		�Ȃ�
//	4.	���l
//		���\�[�X�̉�����s���B
// -------------------------------------------------------------
static void _enddlg( HWND hWnd, int exitcode )
{
	// ���\�[�X�̉��
	if( hMemDC!=NULL ) DeleteDC( hMemDC );
	if( hMemBM!=NULL ) DeleteBitmap( hMemBM );
	hMemDC=NULL;
	hMemBM=NULL;
	// �_�C�A���O�̏I��
	EndDialog( hWnd,exitcode );
}
