# MistTrainGirlsPlayer
某ゲームのアレのシーンを再生します。
## 動作要件
- Windows 10
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
ウィンドウはEscキーで閉じます。
## マウス操作
| 入力  | 動作  |
| --- | --- |
| マウスホイール | 拡大・縮小。 |
| 左ボタンクリック | 次場面移行。 |
| 左ボタンドラッグ | 視点移動。 |
| 中央ボタン | 拡縮・速度・視点初期化。 |
| 右ボタン + マウスホイール | 音声送り・戻し。 |
## Libraries
- [SFML-2.6.1](https://www.sfml-dev.org/download/sfml/2.6.1/)
- [spine-cpp-3.8](https://github.com/EsotericSoftware/spine-runtimes/tree/3.8)
## 補足説明
### 乗算済みアルファ
`spine-sfml.cpp`はそのままではうまく表示できないので毎度算出するようにしています。
```cpp
usePremultipliedAlpha = r == 255 && g == 255 && b == 255 && a == 255;
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
それでも吐息など一部うまく表示されません。
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
