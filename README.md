# MistTrainGirlsPlayer

某ゲームのアレのシーンを再生します。

## 動作要件

- Windows 10 以降のWindows OS
- MSVC 2015-2022 (x64)

## 再生方法

フォルダ選択ダイアログから次のようなフォルダを開くと再生を開始します。
<pre>
29340107
├ 1.atlas
├ 1.png
├ 1.skel
├ 2m.atlas
├ 2m.png
├ 2m.skel
├ 3.atlas
├ 3.png
├ 3.skel
├ ...
├ s_29340107_001.mp3
└ ...
</pre>

## マウス操作
| 入力 | 機能 |
| --- | --- |
| マウスホイール | 窓の拡大・縮小。`Ctrl`を押しながらで寄り・引き。 |
| 左ボタン + マウスホイール | 動作加速・減速。 |
| 左ボタンクリック | 動作移行。 |
| 左ボタンドラッグ | 視点移動。 |
| 中ボタン | 拡縮・速度・視点初期化。 |
| 右ボタン + マウスホイール | 音声送り・戻し。 |
| 右ボタン + 左ボタンクリック | 窓移動。 |

## キー操作

| 入力 | 機能 |
| --- | --- |
| <kbd>Esc</kbd> | 終了。 |
| <kbd>↑</kbd> | 前のフォルダに移動。 |
| <kbd>↓</kbd> | 次のフォルダに移動。 |
| <kbd>→</kbd> | 音声送り。 |
| <kbd>←</kbd> | 音声戻し。 |
| <kbd>T</kbd> | 音声トラック番号表示・非表示切り替え。 |

- 例えば次の階層構造にて`26430105`を再生していた場合、キー入力で`26420105`や`26430205`に移動できます。
<pre>
Resource
├ ...
├ 26420105
│  ├ 1.atlas
│  └ ...
├ 26430105
│  ├ 1.atlas
│  └ ...
├ 26430205
│  ├ 1.atlas
│  └ ...
└ ...
</pre>

- 以下の割り当てもありますが、恐らく不要です。

| 入力 | 機能 |
| --- | --- |
| <kbd>A</kbd> | 乗算済アルファ有効・無効切り替え。 |
| <kbd>B</kbd> | 指定混成法無視/尊重切り替え。 |

## 外部ライブラリ

- [SFML-2.6.1](https://www.sfml-dev.org/download/sfml/2.6.1/)
- [spine-cpp-3.8](https://github.com/EsotericSoftware/spine-runtimes/tree/3.8)

## 構築

Visual Studioが必要になります。
1. `MistTrainGirlsPlayer/deps`階層をファイルエクスプローラで開く。
2. 階層表示欄に`cmd`と入力
     - コマンドプロンプトが起動します。
3. コマンドプロンプト上で`start devenv .`と入力。
     - Visual Studioが起動し、CMakeの設定を開始します。
4. 設定完了後、`MistTrainGirlsPlayer.sln`を開いてビルドして下さい。
     - ビルド成功後にはCMakeの生成ファイルは不要になるので`deps`階層下の`out`フォルダと`.vs`フォルダは手動で削除して下さい。

<details><summary>出来上がるdeps階層の構成</summary>

<pre>
deps
├ SFML-2.6.1 // VC17-x64用SFML
│  ├ include
│  │  └ SFML
│  │    └ ...
│  └ lib
│    └ ...
└ spine-cpp-3.8 // Spine汎用ランタイム
   ├ include
   │  └ spine
   │    └ ...
   └ src
      └ spine
        └ ...
</pre>

</details>
