# BMP2MSX — EPAL (SCREEN2〜7) パレットエディタ 5bit 化 実装メモ

2026-11-07 / Muse 実装分。対象は前回交付（EPAL モード追加）時点のツリー
（`epal_work/` および `phase5wx/`）に対する差分のみ。
SCREEN8 EPAL (MD_SC8P) は今回の対象外（従来どおり）。

## 従来のエディタの振る舞い（変更前の調査結果）

- 設定ダイアログのパレット部はモードによらず常に 3bit 前提だった。
  スライダ範囲は 0〜7 固定（Win: UpdateAll で設定、wx: 生成時に 0〜7）、
  編集対象は `color_palette[16]`、色見本も `color_palette` を
  convert7to255 で展開して描画していた。
- `color_palette_epal[256]` はダイアログからは一切触られていなかった。
- **重要な点**: 変換側 (CreatePal の EPAL 分岐) は、変換のたびに
  `color_palette_epal` を `color_palette` から導出して**上書き**していた
  （最終段: `epal = (convert7to255[cp] * 31 + 127) / 255`）。
  したがって、仮にエディタが epal へ 5bit 値を書いても、変換時に
  3bit へ丸めた値へ戻ってしまう状態だった。本実装では、この導出を
  「編集済みエントリについては行わない」ように変更し、エディタの値を
  変換でも権威化した（詳細は「変換側の変更」）。

## 設計

### 編集ファミリの判定

`cnvIsEpalMode(mode)`（MD_SC2P〜MD_SC7P_256L、SC8P を含まない。
前回交付で CreatePal 用に追加された述語）をそのまま使う。
ダイアログ側では Win: `IsEpalEdit()`、wx: `cnvIsEpalMode(m_edit.screen_mode)`。

### color_palette と color_palette_epal のペア不変条件

変換コードが従来から維持してきた換算だけを使う（新しい換算は作らない）。

- 導出 (3bit→5bit): `derive(v3) = (convert7to255[v3] * 31 + 127) / 255`
  → {0,4,9,13,18,22,27,31}
- 反射 (5bit→3bit): `reflect(v5) = convert_rgb_to_palette(convert7to255, 8, cnvEpalTo255(v5))`
  （BSAVE の EPAL パレット読み込みが color_palette へ書く式と同一）
- 見本・プレビューの 5bit→8bit 展開は `cnvEpalTo255`（= floor(v*255/31) の
  テーブル。プログラム内で既に使われているもの）。

エディタが EPAL モードで値を書くときは、**epal に 5bit 値を書き、
cp には reflect(epal) を書く**（ペアは常に「5bit 主体」の整合状態に保つ）。
非 EPAL モードでは従来どおり cp のみを書く。

### 5bit 主体の判定 (EpalIsEdited)

チャンネル単位の述語。次の両方を満たすとき、そのチャンネルは
「エディタで 5bit 編集された値」とみなす。

1. `epal != derive(cp)`（3bit から導出した値ではない）
2. `cp == reflect(epal)`（3bit 側が 5bit 値の反射と一致する＝ペアが整合）

性質（実コードのテーブルで全数検証済み）:

- 従来ビルドが生成する状態（変換後・保存後・EPAL 出力ファイル経由）は
  常に `epal == derive(cp)` なので、**従来の設定では決して真にならない**。
  → 従来設定に対する変換結果はバイト単位で不変（回帰なし）。
- epal が未保守（全 0 等）の古い設定も、条件 2 を満たさないため
  従来経路（導出）に入り、自動的に回復する。
- 値 31 のように 3bit で表現可能な値は `epal == derive(cp)` となり
  両解釈で同一出力になる（パレットバイト 31、Pal 展開 255）ため、
  判定が偽でも結果は変わらない。

### CreatePal での権威化（変換側の変更はここだけ）

EPAL 分岐の 2 箇所のみ変更。どちらも「そのエントリがユーザ指定
(`fixed_palette` または `PalEn == PALEN_USE`) のとき」だけ
EpalIsEdited を見る。**自動選択 (PALEN_AUTO) エントリは常に従来経路**。

1. 固定色リスト（cnvGetPaletteEpalN へ渡す Pal[pp]）: 編集済み
   チャンネルは convert7to255[cp] の代わりに cnvEpalTo255(epal) を使う
   （最終 Pal と同じ色を固定色として提示するため）。
2. 最終段: 編集済みチャンネルは epal を上書きせず保持し、
   Pal[i] は保持した epal の展開で作る。それ以外は従来どおり導出。

### ダイアログ側の追従処理

- **開く／モード切替**: EPAL ファミリに入るとき、ペア不整合
  （`cp != reflect(epal)` のチャンネル）があれば cp を正として
  epal を再導出する (EpalHealPalette)。整合ペア（3bit 主体・5bit 主体
  どちらも）は変更しない。古い .B2M を EPAL モードで開いても
  黒つぶれ等にならないための処理。
- **プリセット (MSX1色/MSX2デフォルト色)**: 3bit で定義された色なので、
  EPAL モードでは押下時に epal = derive(cp) を強制する
  (EpalDerivePalette)。
- **パレット読込**:
  - *.PL? ファイル: EPAL モードでは EPAL 形式（色数×3 バイト、
    先頭ブロックのみ）として読み、epal へ格納 + cp へ反射する。
    非 EPAL モードでは従来の 2 バイト形式のまま。
  - BSAVE ファイル: 従来形式（拡張子 2/3/4/5/6/7）のものを EPAL
    モードで読み込んだ場合のみ、読込後に EpalDerivePalette で
    5bit 側を追従させる。EPAL 形式（B/D/F/G/H）の場合は
    bsvLoadPLBsaveFile が両側を整合に書くので何もしない。
- **パレット保存**: bsvSavePLFile は前回交付時点で EPAL 形式を
  epal から直接書く実装のため変更なし（エディタの値がそのまま出る）。
- **ドラッグ交換 (ChangeColor / wx OnLeftUp)**: EPAL モードでは
  color_palette_epal も一緒に交換する（非 EPAL では従来どおり
  交換しないので、保存される .B2M のバイトも従来と同一）。
- **ソート**: 変換時の cnvSortPalette が前回交付で epal も同時に
  並べ替えるため、ダイアログ側の変更は不要。
- **強制ゼロ化 (FourceZero)**: FZColor は 8bit 色でパレット配列と
  無関係。有効条件への EPAL 追加は前回交付で済んでおり変更なし。

### スライダ範囲の切替位置

- Win: `UpdatePalSliderRange()`（新規）を UpdateAll と onSetMode から
  呼ぶ。範囲は `IsEpalEdit() ? 0〜31 : 0〜7`。onSetMode では範囲変更の
  後に SetEditColor で表示値を更新する。
- wx: `ApplyPaletteRange()`（新規）を LoadAll と OnModeChanged から
  呼ぶ。OnModeChanged は m_edit.screen_mode をコンボと同期させる
  ようにした（Win 版 EdtMode と同じくライブ編集）。
  これによりパネル描画・スライダ・読込判定がすべて現在の選択モードで
  動作する。

### 色見本の描画

- Win: `UpdateColor()` に EPAL 分岐を追加し、cnvEpalTo255(epal) で描く。
  SCREEN6 系の 4 色ガード（c>3 無効表示）は SC6P/SC6P_256L も対象に
  なっており（前回交付）、EPAL でもそのまま効く。
- wx: `PalettePanel::OnPaint` に EPAL 分岐を追加し、同様に描く。
  なお wx 版のパネルにはベース SCREEN6 でも 4 色ガードが存在しない
  （全色表示・選択可、変換で 4 色使用）ため、EPAL でもベース仕様に
  合わせてガードは入れていない。

## ファイル別変更

### source/WndProc.cpp（phase5wx/source/ も同一）
- CreatePal 前にヘルパ追加: EpalDerive5 / EpalReflect3 / EpalIsEdited /
  EpalFixedPal8。
- EPAL 分岐の固定色リストと最終段を上記のとおり変更（2 箇所）。

### linux/src/convert_run.cpp、linux/wx/gui_convert.cpp
- 同じヘルパ追加と EPAL 分岐 2 箇所の変更（コメントは英語）。

### source/Setting.cpp（phase5wx/source/ も同一）
- ヘルパ追加: IsEpalEdit / EpalDerive5 / EpalReflect3 / EpalHealPalette /
  EpalDerivePalette / UpdatePalSliderRange / BsavePaletteKind。
- onSetHScroll: RGB 3 ケースに EPAL 分岐（epal へ書き cp へ反射）。
- onSetMode: モード切替時に heal + 範囲切替 + 表示値更新。
- UpdateAll: heal 追加、範囲設定を
...[truncated 3889 chars]