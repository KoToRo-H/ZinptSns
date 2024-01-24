# ZinptSns
■説明<br>
X68000 Zに接続したキーボード／ジョイスティック／マウスの入力データを画面に表示します。

非純正のUSBジョイパッドのA/Bボタン配置やUSBキーボードのどのキーがXF1キーに割り当たっているのかなどを確認する時に使います。<br>
[ESC]キーを押すと終了します。

※本家X68000で実行しても動くと思いますが、純正環境でキー割り当てが不明という事態になることはあまり無さそうなので……

■コンパイル方法<br>
・XCの場合 : cc /Y ZinptSns.c<br>
・GCCの場合 : gcc -ldos -liocs ZinptSns.c<br>
