# EPAL 改善選択 (EpalHGuard) の GUI チェックボックス化

2026-11-07。`epal_hguard_impl.md` §6 オープン点1 (GUI チェックボックス
未実装) の解消。土台は hguard 納品時点のツリー (Windows =
`epal_work/`、wx = `phase5wx/`)。コア (Convert.cpp 等) は無変更で、
SETTING の保持・変換への反映は hguard 納品時点で済んでいたため、
本作業は両 GUI の設定ダイアログへの配線のみである。

## 1. ラベル

ダイアログ近傍の既存文言に合わせ、次のとおりとした。

- Windows (日): 「特徴色を優先(EPAL)」(IDC_SELCOL「パレット多分布選色」
  等の並び。EPAL でのみ有効なことが分かるよう (EPAL) を付けた)
- Windows (英): "Distinctive colors (EPAL)"(配置幅 114 DU に収める
  ため短縮形)
- wx: "Prefer distinctive colors (EPAL)"(隣の "Prefer most-used
  colors" と対になる形)

## 2. Windows 側 (source/)

- `resource.h`: `IDC_EPALHG` = 1066 を追加、
  `_APS_NEXT_CONTROL_VALUE` を 1067 に更新。
- `ResScript.rc` / `ResScriptE.rc`: IDD_SETTING のパレット設定
  グループ内、右列ボタン群 (MSX2デフォルト色) の下の空き位置
  (313,374、114x11) に BS_AUTOCHECKBOX を追加。ダイアログ全体の
  サイズ変更はしていない。
- `Setting.cpp` (既存チェックボックスと同じ配線):
  - ツールチップ文字列を日英両テーブルに追加。
  - モード別有効/無効テーブル `nCtrlCode[]` の末尾に IDC_EPALHG を
    追加し、`nCtrlEnbl[][]` の各行末尾に値を追加。EPAL 系の行
    (SCREEN8 EPAL、SCREEN2〜7 EPAL、いずれも 256ライン版含む)
    のみ TRUE、それ以外は FALSE。コアが EPAL-N と SC8P の両分岐で
    EpalHGuard を見る (hguard_impl §2.2) ことに対応する。
  - `ONEVENT( IDC_EPALHG, onSetEpalHGuard )` とハンドラ
    (onSetSelCol と同じく `EdtMode.EpalHGuard` の反転) を追加。
  - `UpdateAll()` に `Button_SetCheck(... EdtMode.EpalHGuard ...)`
    を追加し、ダイアログ表示へ反映。
- 永続化: ダイアログは EdtMode を直接編集し、OK で
  `NowMode = EdtMode` (構造体全体コピー)、保存は SETTING 全体の
  B2M 書き出しという既存経路のままで、EpalHGuard は自動的に
  往復する。個別の保存コードは不要だった。

## 3. wx 側 (linux/wx/)

- `settings_dlg.h`: メンバ `m_epal_hguard` を追加。
- `settings_dlg.cpp`:
  - Palette グループに m_fixed_palette の下へチェックボックスを
    追加 (既存3項目と同じ並び)。
  - `LoadAll()`: `m_epal_hguard->SetValue( m_edit.EpalHGuard )`。
  - `StoreAll()`: `m_edit.EpalHGuard = m_epal_hguard->GetValue()`。
    OK は OnOk → StoreAll → Edited() → アプリ側の
    SetDefaultSetting + SaveCfgFile (SETTING 全体) という既存経路で
    ファイルまで届く。
  - `UpdateEnabled()`: `cnvIsEpalMode( mode ) || IsEpalMode( mode )`
    のときのみ有効化 (EPAL-N + SCREEN8 EPAL)。パレット系一括有効化
    (IsPaletteMode) には含めていない。SC8P では他のパレット項目は
    無効でも本項目は有効になるのが正しいため。

## 4. 検証

- ビルド: Linux CLI / wx GUI ともビルド・リンク成功。
- CLI 回帰スポットチェック: 変更前バイナリ (hguard 納品時点) と
  変更後で、bmp2msx.png を 5 条件 (5p OFF / 5p ON / 8p OFF /
  5 OFF / 2 OFF) 変換し、全出力バイト一致。GUI 変更は CLI の
  変換結果に影響しない。
- SETTING 永続化 (コア経路): GetDefCfg → EpalHGuard=true,
  screen_mode=MD_SC5P を設定 → SaveCfgFile → GetCfgFile の往復で
  EpalHGuard=1 が保持されることを実測 (B2M は sizeof(SETTING)=
  3400 バイト)。
- wx ダイアログ実機確認 (Xvfb + `--snapshot-settings` フック):
  - SCREEN5 EPAL + EpalHGuard=ON の B2M: チェックボックスが
    チェック済み・有効 (ラベル濃度は隣接項目と同等) で表示。
  - 同 OFF の B2M: 未チェックで表示。ON/OFF 2枚の画像差分は
    当該チェックボックスのチェックマーク部分のみだった。
  - SCREEN5 (非EPAL) + ON の B2M: チェックボックスは無効化
    (ラベルが淡色化、濃度実測でも明確に低下) して表示。
    スクリーンショット3枚を zip の shots/ に同梱する。
  - ダイアログ上のクリック操作そのものはヘッドレス環境で
    自動化できないため、ダイアログ→ファイル方向は上記の
    コード経路 (OnOk/StoreAll/Edited/SaveCfgFile) の確認と、
    ファイル→ダイアログ方向の実測で担保した。

## 5. オープン点

- Windows (Visual Studio) ビルドはこの環境では実行できないため
  未確認 (従来どおり作者側の確認待ち)。ResScript.rc の変更に
  伴う .aps は VS 側で再生成される前提で触っていない。
- チェックボックスを入れたことで、hguard の残オープン点は
  epal_hguard_impl.md §6 の 2 (ON 時の 5bit 記録の解釈) と
  3 (birds 型ケースのガード許容) の作者判断のみになる。
