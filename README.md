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
├ ...
├ s_29340107_001.mp3
└ ...
</pre>
## マウス操作
| 入力  | 機能  |
| --- | --- |
| マウスホイール | 拡大・縮小。 |
| 左ボタン + マウスホイール | 動作加速・減速。 |
| 左ボタンクリック | 動作移行。 |
| 左ボタンドラッグ | 視点移動。 |
| 中ボタン | 拡縮・速度・視点初期化。 |
| 右ボタン + マウスホイール | 音声送り・戻し。 |
| 右ボタン + 左ボタンクリック | 窓移動。 |
## キー操作
| 入力  | 機能  |
| --- | --- |
| Esc | 終了。 |
| Up | 前のフォルダに移動。 |
| Down | 次のフォルダに移動。 |
| A | 乗算済アルファ有効・無効切り替え。 |
| B | 指定混成法無視/尊重切り替え。 |
| T | 再生音声トラック番号表示・非表示切り替え。 |

例えば次の階層構造にて`26430105`を再生していた場合、キー入力で`26420105`や`26430205`に移動できます。
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
## 外部ライブラリ
- [SFML-2.6.1](https://www.sfml-dev.org/download/sfml/2.6.1/)
- [spine-cpp-3.8](https://github.com/EsotericSoftware/spine-runtimes/tree/3.8)

## 構築

1. `deps/CMakeLists.txt`を実行
2. `MistTrainGirlsPlayer.sln`を開く

`deps`階層は以下のような構造になります。
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
## 補足説明

### 視点
背景に合わせているので、ゲーム中の表示より遠のいています。拡縮で調整して下さい。

### 描画処理
公式の実行時環境`spine-sfml.cpp`ではPMA処理に問題があったので、`v0.6`からは大きく変更しました。  
乗算済アルファ有効時には原色に透過度を乗算するようにしています。
``` cpp
for (int ii = 0; ii < indicesCount; ++ii)
{
	sf::Vertex sfmlVertex;
	sfmlVertex.position.x = (*pVertices)[(*pIndices)[ii] * 2LL];
	sfmlVertex.position.y = (*pVertices)[(*pIndices)[ii] * 2LL + 1];
	sfmlVertex.color.r = (sf::Uint8)(tint.r * 255.f * (m_bAlphaPremultiplied ? tint.a : 1.f));
	sfmlVertex.color.g = (sf::Uint8)(tint.g * 255.f * (m_bAlphaPremultiplied ? tint.a : 1.f));
	sfmlVertex.color.b = (sf::Uint8)(tint.b * 255.f * (m_bAlphaPremultiplied ? tint.a : 1.f));
	sfmlVertex.color.a = (sf::Uint8)(tint.a * 255.f);
	sfmlVertex.texCoords.x = (*pAttachmentUvs)[(*pIndices)[ii] * 2LL] * sfmlSize.x;
	sfmlVertex.texCoords.y = (*pAttachmentUvs)[(*pIndices)[ii] * 2LL + 1] * sfmlSize.y;
	sfmlVertices.append(sfmlVertex);
}
```
