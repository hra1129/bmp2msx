# BMP2MSX — SCREEN2〜7 の EPAL (V9968) モード対応 実装メモ

2026-11-07 / Muse 実装分。対象は修正済みマスタ（repo HEAD〜1 + SC8 青修正1行、
Convert.cpp md5 `6ca6636e20d24b0bd9e6de3619cf610b`、2,511行）に対する差分のみ。

## 概要

従来 SCREEN8 専用だった EPAL（パレット RGB555 化）を SCREEN2/3/5/6/7 にも追加した。
表示色数はベースモードと不変（SCREEN6系=4色、その他=16色）で、変わるのは
パレット生成の精度（333→555）とパレット出力形式（2バイト/色→3バイト/色）のみ。
ピクセル/インデックス変換処理そのものはベースモードと同一コードを共用する。

## SCREEN4 について（調査結果）

SCREEN4 専用のモードは存在しない。MD_SC2 が SCREEN2/4 を兼ねている
（screen_size コメント "screen 2/4"、GUI 名 "SCREEN2/4"、スクリプト SCREEN 4 → MD_SC2）。
したがって EPAL も **MD_SC2P が SCREEN2/4 EPAL** を兼ねる。実装した EPAL モードは
2/3/5/6/7 の5系統（+256ライン版3種）。

## 追加モード（Convert.h、列挙の末尾に追加）

既存値との互換（.B2M 設定ファイルの保存値）を守るため、SC8P と同じく末尾追加とした。
SETTING 構造体のレイアウトは変更していない（MD_MAX は 16 → 24）。

| 値 | モード | 色数 | CLI トークン | 拡張子 |
|----|--------|------|--------------|--------|
| 16 | MD_SC2P (SCREEN2/4 EPAL) | 16 | `2p` | .SRB |
| 17 | MD_SC3P (SCREEN3 EPAL) | 16 | `3p` | .SRD |
| 18 | MD_SC5P (SCREEN5 EPAL) | 16 | `5p` | .SRF |
| 19 | MD_SC6P (SCREEN6 EPAL) | 4 | `6p` | .SRG |
| 20 | MD_SC7P (SCREEN7 EPAL) | 16 | `7p` | .SRH |
| 21 | MD_SC5P_256L | 16 | `5p-256` | .SRF |
| 22 | MD_SC6P_256L | 4 | `6p-256` | .SRG |
| 23 | MD_SC7P_256L | 16 | `7p-256` | .SRH |

表示名は SC8P の書式に倣い "SCREEN5 EPAL(V9968)" / "SCREEN5 EPAL(V9968)(256lines)" 等
（Windows Setting.cpp の ScrModeName、Linux wx の kModeNames、CLI の mode_names に追加）。

## パレット生成（CreatePal に EPAL 分岐を追加：WndProc.cpp / linux/src/convert_run.cpp / linux/wx/gui_convert.cpp の3写し全て）

16色分岐と同じ構成で、精度だけを 555 にした：

- 固定色（PALEN_USE）は従来どおり color_palette[16]（GUI/CLI で編集可能な 3bit パレット）
  から取り、最終段で 8→5 丸め `(v*31+127)/255`（SC8P と同じ丸め）で color_palette_epal へ記録。
- 自動選択は **555 精度** で行う新規関数 `cnvGetPaletteEpalN()`（Convert.cpp）。
  cnvGetPalette256 の方式（32768-bin ヒストグラム、上限以下なら頻度順に全採用、
  超過時はメディアンカット）を少数色向けに一般化したもの。固定色と量子化後に重なる
  bin はヒストグラムから除外し、FourceZero 色のピクセルも除外する（SC8P と同方式）。
- 最終パレットは color_palette_epal[0..色数) に 5bit で記録し、color_palette には
  最も近い 3bit 値を反映（ソート表示・タイル生成など 3bit 前提の処理との整合用）、
  変換用 Pal[] は 5bit→8bit 展開（floor(v*255/31)、cnvEpalTo255）で作り直す。
- cnvSortPalette を拡張し、EPAL 時は color_palette_epal も一緒に並べ替え、
  比較は 5bit 版 cnvCompare5（マスク 16,8,4,2,1）で行う。
- SelCol（色選択アルゴリズム）は EPAL 自動選択では SC8P と同様に不使用。

## 出力形式（bsave.cpp / bsave.h）

- **BSAVE 埋め込み**: PalTblAdr にベースモードと同じアドレスを割り当て
  （SC2P=1B80h, SC3P=2020h, SC5P/6P=7680h, SC7P=FA80h）、非256ライン版のみ
  PltMode==PLT_BSAVE 時に 色数×3 バイト（R,G,B の順、各 5bit 値）を書き込む
  （_SavePaletteTableEpal）。ヘッダ end は paltbl + 色数*3 - 1（SC2P は従来どおり 37FFh 固定）。
  256ライン版はベースモード同様に埋め込まない。
- **BASIC ローダ/DATA**: SC8P の仕組みを色数で一般化。ローダは SCREEN 設定後に
  R#20=16（EPAL=1）、R#16=0、RESTORE 60000 から READ R,G,B → OUT &H9A（Port#2）を
  色数分繰り返す。DATA は 60000 番から 4色/行で、**パレットのバイト列そのものを
  10進数で R,G,B の順に並べたもの**（作者確認済みの形式）。BSAVE 埋め込みバイト列と
  DATA の値列が一致することを検証済み。
- **PL? / CSV**: EPAL 形式（色数×3 バイト、R,G,B 各 5bit）で出力。
  PL はそのブロックを従来通り 8 回繰り返す（16色時 384 バイト、4色時 96 バイト）。
  CSV は 1色につき "%02X,%02X,%02X"。
- **BSAVE からのパレット読み込み**（bsvLoadPLBsaveFile）も EPAL 拡張子に対応し、
  3バイト形式を読んで color_palette_epal と color_palette（3bit 近似）へ反映する。
- ファイル名のモード記号（smode）は新規文字 B/D/F/G/H を割り当てた
  （既存 2,3,5,6,7,8,A,C,P と重ならない文字。インターレース時は .SB0/.SB1 等）。

## その他の追随箇所

- cnvRecolor: SC2P→cnvSC5toSC2、SC3P→cnvSC5toSC3 の後変換を追加。
  cnvRecolor5 / cnvCreateTail4 の色数判定に SC6P 系（4色）を追加。
- フロー条件（ソート実行、タイル、cnvRecolor/cnvNtcolor 振り分け、インターレース時の
  コピー方式、RM_MSX のアスペクト）へ EPAL モードを追加（WndProc / convert_run / gui_convert）。
- `screen_mode >= MD_SC5` という数値比較で「SCREEN5以上」を判定していた箇所
  （192ライン判定、計5箇所）は、EPAL モード（値 16〜）で誤作動するため
  screen_mode[] テーブルの SCREEN 番号で判定するよう変更。既存モードでは等価。
- utils.cpp: 設定読み込み時のインターレース禁止（SC2/SC3）に SC2P/SC3P を追加。
- プレビュー描画: winConvert.cpp に _draw_screen2p/3p/5p/6p/7p、
  linux/wx/preview_render.cpp に同等関数を追加（色源を color_palette_epal に差し替えた複製）。
- Windows GUI (Setting.cpp): モード名追加、コントロール有効/無効行はベースモードの行を
  複製（SC8P と違いパレット関連コントロールは有効のまま）、SC6 用の 4色ガード・
  FZERO 有効条件・パレット表示条件へ EPAL を追加。wx 側 settings_dlg.cpp も同様
  （IsPaletteMode / IsTileMode へ追加、192ライン有効条件もテーブル判定へ）。
- スクリプト: Windows 版 ScriptProc の SCREEN コマンドは SC8P と同じ扱い
  （数値指定のみ、EPAL は LOADSETTING 経由）で変更なし。Linux 独自拡張の
  SCREEN 8P に倣い、Linux script には 2P/3P/5P/6P/7P/5P256/6P256/7P256 を追加。
  Linux CLI トークンも 8p の書式に倣って追加した。

## 検証（Linux CLI、phase5wx ツリーで実施）

- **回帰**: 変更前 CLI を先にビルドし、3画像（photo / 333厳密色ブロック / グラデーション）
  × 15モード（2,3,5,6,7,8,10,12,各256ライン,8p,8p-256）× パレット出力3種（bsave/pl/csv）
  + BAS の **519 ファイル**を採取。変更後と全てバイト一致（SC8P・非EPAL への影響なし）。
- **新規モード機能チェック 47 項目すべて PASS**（/tmp/epaltest/verify/verify.py）:
  - 埋め込みパレットが 色数×3 バイト・全値 ≦31・R,G,B 順であること、
    ヘッダ end フィールドが paltbl+色数*3-1（SC2P は 37FFh）であること。
  - 誤差拡散 OFF の自動変換で、画像本体バイト列がベースモードと完全一致
    （SC2 のカラーテーブル含む）。256ライン版はファイル全体がベースと一致。
  - 固定パレット変換で、埋め込みバイト列が指定色の 8→5 丸めと完全一致。
  - BAS の DATA 値列が埋め込みバイト列と完全一致。ローダのループ回数・R#20=16 を確認。
  - FourceZero（固定色として置いた色に一致したピクセルの 0 番強制＝ベースと同一の
    cnvRecolor5 共有動作、および FZ 色の自動選択からの除外）、NonZero、ソート
    （BAS と BSAVE のパレット一致＝ソートが EPAL パレットを追随）、タイル、PL/CSV、
    インターレース、スクリプト（SCREEN 5P / 7P256）の動作。
- wx GUI（bmp2msx-wx）もビルド・リンク成功。

### 解釈上の注意（検証で判明したベースプログラムの性質）

- 誤差拡散 ON の自動変換では、拡散誤差がパレット値そのものに依存するため、
  パレット値が変わる EPAL ではインデックスがベースと変わり得る（パレットを変えれば
  ベース同士でも変わるのと同じ）。インデックス同一性は拡散 OFF で証明した。
- SCREEN6 の自動選択は、333 側（SelCol による削減）と 555 側（メディアンカット）で
  アルゴリズム自体が異なるため、多色画像では選択結果が一致しないことがある
  （4色以下の画像と固定パレットでは一致を証明済み）。これは SC8P で採用された
  「EPAL の選択は 555 で行う」方針の帰結。
- FourceZero の 0 番強制は「FZ 色と一致するパレットエントリにマッチしたピクセル」
  に対して働く（cnvRecolor5 の cc はマッチしたパレット色）。ベースと同一コードのため
  挙動も同一。

## 作者確認事項（オープンポイント）

1. **マニュアル §7 の追記**: SCREEN2〜7 EPAL のモード追加、出力ファイル拡張子
   （.SRB/.SRD/.SRF/.SRG/.SRH）の記載。拡張子文字 B/D/F/G/H は重複回避で機械的に
   選んだものなので、改名する場合は bsave.cpp の smode 文字列と
   bsvLoadPLBsaveFile の対応を変更すればよい。
2. **PL ファイル形式**: EPAL ブロック（色数×3 バイト）を 8 回繰り返す形にした
   （16 色で 384 バイト）。GraphSaurus 形式の素直な拡張と言い切れないため要確認。
3. **CSV のヘッダ文字列** ";MSX PALETTE DATA(&Hrb,&H0g)" は従来形式の説明のまま
   （共通文字列のため、非 EPAL の CSV をバイト同一に保つ目的で未変更）。
4. **5bit パレットの直接編集 UI は未実装**（SC8P と同じ制約）。ユーザーパレットは
   3bit の color_palette で指定し 5bit へ丸める。.B2M に color_palette_epal が
   保存されていても変換のたびに再生成される。555 のまま指定したい場合は別途 UI が必要。
5. **Windows 版のビルド確認は未実施**（この環境では VS ビルド不可）。
   Linux CLI / wx GUI でのビルド・動作検証は完了。WndProc.cpp / Setting.cpp /
   winConvert.cpp は Linux 側と同一パターンの変更だが、実ビルドでの確認をお願いしたい。
