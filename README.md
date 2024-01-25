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
| 左ボタン + マウスホイール | コマ送り加速・減速 |
| 左ボタンクリック | 次場面移行。 |
| 左ボタンドラッグ | 視点移動。 |
| 中央ボタン | 拡縮・速度・視点初期化。 |
| 右ボタン + マウスホイール | 音声送り・戻し。 |
| 右ボタン + 左ボタンクリック | 窓移動。 |
## キー操作
| 入力  | 機能  |
| --- | --- |
| Esc | 再生終了。 |
| Up | 前のフォルダに移動。 |
| Down | 次のフォルダに移動。 |

例えば次の階層構造にて`26430105`を再生していた場合、キー入力で`26420105`や`26430205`に移動できます。
<pre>
Resource
├ ...
├ 26420105
│   ├ 1.atlas
│   └ ...
├ 26430105
│   ├ 1.atlas
│   └ ...
├ 26430205
│   ├ 1.atlas
│   └ ...
└ ...
</pre>
## Libraries
- [SFML-2.6.1](https://www.sfml-dev.org/download/sfml/2.6.1/)
- [spine-cpp-3.8](https://github.com/EsotericSoftware/spine-runtimes/tree/3.8)
## 補足説明
### 乗算済みアルファ
そのままではうまく表示できないので`spine-sfml.cpp`にて毎度算出し、且つ、いくつかのスロットに対して合成モードを変更しています。
```cpp
usePremultipliedAlpha = r == 255 && g == 255 && b == 255 && a == 255;
if (!usePremultipliedAlpha)
{
	if (a > 109)
	{
		for (size_t ii = 0; ii < m_blendMultiplyList.size(); ++ii)
		{
			if (strstr(slot.getData().getName().buffer(), m_blendMultiplyList.at(ii).c_str()))
			{
				slot.getData().setBlendMode(spine::BlendMode::BlendMode_Multiply);
			}
		}
	}
}
sf::BlendMode blend;
switch (slot.getData().getBlendMode())
{
case BlendMode_Additive:
    blend = sf::BlendMode(usePremultipliedAlpha ? sf::BlendMode::One: sf::BlendMode::SrcAlpha, sf::BlendMode::One);
    break;
case BlendMode_Multiply:
    blend = sf::BlendMode(sf::BlendMode::DstColor, sf::BlendMode::OneMinusSrcAlpha);
    break;
case BlendMode_Screen:
    blend = sf::BlendMode(sf::BlendMode::One, sf::BlendMode::OneMinusSrcColor);
    break;
default:
    blend = sf::BlendMode(usePremultipliedAlpha ? sf::BlendMode::One: sf::BlendMode::SrcAlpha, sf::BlendMode::OneMinusSrcAlpha);
    break;
}
```
変更するスロット名は `sfml_spine_player.cpp`にて指定しています。
```cpp
for (size_t i = 0; i < m_skeletonData.size(); ++i)
{
	/*中略*/
	drawable->SetBlendMultiplyList(blendMultiplyList);

	auto& slots = m_skeletonData.at(i).get()->getSlots();
	for (size_t ii = 0; ii < slots.size(); ++ii)
	{
		std::string strName = slots[ii]->getName().buffer();
		for (const std::string& str : blendScreenList)
		{
			if (strncmp(strName.c_str(), str.c_str(), str.size()) == 0)
			{
				slots[ii]->setBlendMode(spine::BlendMode::BlendMode_Screen);
			}
		}
	}
}
```
### 立ち絵を表示したい場合
  `usePremultipliedAlpha`を常に`false`にすれば正しく表示できます。  
  上記ライブラリは同梱していないので所定の箇所に補ってビルドして下さい。
  <pre>
    deps
    ├ SFML-2.6.1 // 上記リンクから取得
    │   ├ include
    │   │   └ SFML
    │   │       └ ...
    │   └ lib
    │       └ ...
    ├ spine-cpp-3.8 // 上記リンクから取得
    │   ├ include
    │   │   └ spine
    │   │       └ ...
    │   └ src
    │       └ spine
    │           └ ...
    └ spine-sfml-3.8 // 同梱済み
        ├ spine-sfml.cpp
        └ spine-sfml.h
  </pre>
### 視点
背景に合わせているので、ゲーム中の表示より遠のいています。拡縮で調整して下さい。
