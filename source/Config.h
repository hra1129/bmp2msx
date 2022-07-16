// -------------------------------------------------------------
// BMP to MSX
//											(C)2000 HRA!
// -------------------------------------------------------------

#ifndef _CONFIG_H_
#define _CONFIG_H_

// -------------------------------------------------------------
// 設定
// -------------------------------------------------------------
#define	CFG_VERSION		"ver 2022_07_17 (C)2000-2022 HRA!"	//	バージョン番号

#define	COLCUR1			RGB(255,255,240)				//	カラーテーブル現在位置カーソル色１（内側）
#define	COLCUR2			RGB(15,15,0)					//	カラーテーブル現在位置カーソル色２（外側）
#define PV_MAX			50								//	プレビュー記憶最大数
#define cnDataBk1		RGB(0,0,120)					//	設定表示の背景（設定、変換結果画像）
#define cnDataBk2		RGB(120,0,0)					//	設定表示の背景（変換元画像）
#define cnDataText		RGB(255,255,255)				//	設定表示の背景
#define	cnWidth			512								//	幅
#define	cnHeight		424								//	高さ
#define	cnZoomMax		4								//	拡大率最大

//	ロゴイメージのサイズ
#define cnBGWidth		256
#define cnBGHeight		212

//	最大画像サイズ
#define	cnMAXWidth		512
#define	cnMAXHeight		424

//	ヘルプファイル名
#define	cszHelpName		"bmp2msx.chm"

#ifdef _ENGLISH

#define CNWS			(WS_CAPTION | WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX | WS_BORDER)
#define CNWSS			(WS_CAPTION | WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX | WS_BORDER | WS_SIZEBOX )
#define	CSZCLASSNAME	"BMP2MSX"
#define	CSZCLASSNAMESUB	"BMP2MSXSUB"
#define	CSZTITLE		"BMP to MSX"
#define	CSZTITLESUB		"Original File"

#define	BgColor			RGB(10,0,127)
#define	FrColor			RGB(255,255,255)				//	文字の色
#define CfgFile			"BMP2MSX.B2M"					//	設定ファイル
#define PathFile		"BMP2MSX.CFG"					//	設定ファイル
#define	CfgExt			"B2M"							//	設定ファイルのデフォルト拡張子
#define	CfgBmp			"BMP"							//	BMPファイルの拡張子
#define	CfgCsv			"CSV"							//	CSVファイルの拡張子
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

#define cszSetting		"Default"						// 設定ダイアログのタイトル
#define cszRedo			"Reconvert"						// 再変換ダイアログのタイトル
#define cszScrSetting	"Setting(Script mode)"			// スクリプト命令による設定ダイアログのタイトル

// -------------------------------------------------------------
// エラー/警告メッセージ
// -------------------------------------------------------------
#define MsgCap			"Error"							// エラーメッセージのタイトル
#define WriteErr		"Write error"					// ファイル書き込みエラー
#define cszNotEnoughMem	"Not enough memory"				// メモリ不足
#define cszPalCnt		"Cannot create palette.\nNeed two palettes or more."	// パレット不足
#define MsgCapWrn		"Warning"						// 警告（確認）メッセージのタイトル
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

// スクリプトエラー/警告
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
#define	CSZTITLESUB		"ｵﾘｼﾞﾅﾙ画像"

#define	BgColor			RGB(10,0,127)
#define	FrColor			RGB(255,255,255)			//	文字の色
#define CfgFile			"BMP2MSX.B2M"				//	設定ファイル
#define PathFile		"BMP2MSX.CFG"				//	設定ファイル
#define	CfgExt			"B2M"						//	設定ファイルのデフォルト拡張子
#define	CfgBmp			"BMP"						//	BMPファイルの拡張子
#define	CfgCsv			"CSV"						//	CSVファイルの拡張子
#define	CfgSpi			"SPI"						//	プラグインの拡張子
#define PLSave			"名前を付けて保存(*.PL?形式)"
#define	PLLoad			"ﾊﾟﾚｯﾄﾌｧｲﾙ読み込み"
#define ScrptLoad		"ｽｸﾘﾌﾟﾄﾌｧｲﾙ読み込み"
#define	CSVSave			"名前を付けて保存(CSV形式)"
#define cszDefExp		"MSX画像ﾌｧｲﾙ(*.SC?/*.SR?)\0*.SC?;*.SR?\0すべてのﾌｧｲﾙ(*.*)\0*.*\0"
#define	cszDefPL		"MSXﾊﾟﾚｯﾄﾌｧｲﾙ(*.PL?)\0*.PL?\0"
#define	cszDefPLL		"MSXﾊﾟﾚｯﾄﾌｧｲﾙ(*.PL?/*.Sxx)\0*.PL?;*.S??\0"
#define	cszDefCSV		"CSVﾌｧｲﾙ(*.CSV/*.TXT)\0*.CSV;*.TXT\0"
#define cszDefScrpt		"ｽｸﾘﾌﾟﾄﾌｧｲﾙ(*.TXT)\0*.TXT\0"
#define cszDefPlug		"SPIﾌﾟﾗｸﾞｲﾝ(*.SPI)\0*.SPI\0"
#define cszDefCfg		"BMPtoMSX設定ﾌｧｲﾙ(*.B2M)\0*.B2M\0"
#define	cszCSVHead		";MSX PALETTE DATA(&Hrb,&H0g)\x0D\x0A"
#define cszClipName		"クリップボード内画像"
#define cszClipFile		"bmpmsx.bmp"
#define	cszBmpSave		"BMPで保存"
#define	cszBmpFilter	"BMPﾌｧｲﾙ(*.BMP)\0*.BMP\0"

#define cszSetting		"ﾃﾞﾌｫﾙﾄ設定"			// 設定ダイアログのタイトル
#define cszRedo			"再変換"				// 再変換ダイアログのタイトル
#define cszScrSetting	"変換設定(ｽｸﾘﾌﾟﾄﾓｰﾄﾞ)"	// スクリプト命令による設定ダイアログのタイトル

// -------------------------------------------------------------
// エラー/警告メッセージ
// -------------------------------------------------------------
#define MsgCap			"Error"						// エラーメッセージのタイトル
#define WriteErr		"書き込みに失敗しました。"	// ファイル書き込みエラー
#define cszNotEnoughMem	"メモリ不足です。"		// メモリ不足
#define cszPalCnt		"画像を生成できません。\nパレットは２つ以上使用可能にしてください。"	// パレット不足
#define MsgCapWrn		"Warning"					// 警告（確認）メッセージのタイトル
#define cszDel			"表示中のプレビューを削除します。\nよろしいですか？"
#define cszAllDel		"記憶しているプレビューをすべて削除します。\nよろしいですか？"
#define cszCannotAddPV	"これ以上プレビューを記憶できません。"
#define cszClipErr		"クリップボードから生成した画像は再変換できません。"
#define cszBDErr		"指定されたファイルが読み込めないか、これ以上画像を記憶できません。"
#define cszNoSupp		"読み込めません。非対応の形式です。"
#define cszSmall		"画像が小さすぎます。"
#define cszOpenErr		"ファイルが開けません。"
#define cszNoText		"異常なファイルです。読み込めません。"
#define cszMainClk		"まだ画像を開いていません（設定はメニューから選択してください）"

// スクリプトエラー/警告
#define cszSyntax		"文法が不正です。"
#define cszNoString		"文字列のあるべきところに文字列がありません。"
#define cszNoDelim		"文字列の終端を示す \" が見つかりません。"
#define cszNoCfg		"設定ファイルの読み込みに失敗しました。"
#define cszNoFile		"ファイル名が指定されていません。"
#define cszExit			"スクリプトを終了しました。"
#define cszScrMode		"画面モードの指定が異常です。"
#define cszNoFlag		"フラグ名の指定が異常です。"
#define	cszNoBool		"フラグ値の指定が異常です。"
#define cszWrnClear		"編集中のスクリプトがクリアされます。よろしいですか？"
#define cszPal			"パレット設定が異常です。"
#define cszPMode		"パレットの動作モード指定が異常です。"
#define cszCutErr		"切り捨て誤差の値指定が異常です。"
#define cszGosaVal		"誤差拡散係数の指定値が異常です。"
#define cszSelMode		"選色モードの指定値が異常です。"

#endif

#endif	// CONFIG_H_
