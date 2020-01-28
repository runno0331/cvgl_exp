# cvgl_exp

3年後期実験　OpenCV/OpenGL実験の成果物  

作品名：シンプルなシューティングゲーム

作品紹介:  
OpenCVとOpenGLを用いて手を動かして遊べるシンプルなシューティングゲーム
を作りました．サウンドや画面などできる限りゲームっぽくなるように工夫を
しました．また，カメラが使えないといった環境でも遊べるようにマウス操作に
も対応できるようにしました．

実行環境：  
OS:Ubuntu 18.04.3 LTS  
コンパイラ:7.4.0

実行方法：  
make main  
./main () ()  
第1引数に使用するカメラ  
第2引数に使用するモードを選択する(省略時0)  
0:通常の手を中心とする操作  
1:マウスを中心とする操作（このとき第一引数は適当に）  
2:手を中心とする操作+前処理結果の表示  

操作説明：  
手で操作する場合  
1.手を左右に動かすことでゲームを始められます．  
2.手の位置を変化させることで照準を合わせて的を撃ちます．  
　このときの撃つ動作は手の部分の上下方向の重心に移動を検知するので
　揺らすか，指先を上にあげるかします．
3.一定時間肌色部分が認識されないと一時停止し，また手を出すと再開します．  

マウスで操作操作する場合  
1.基本的な操作は手で操作する場合と同じです．照準移動はマウス移動，
　弾の発射はクリックで行います．  
2.一時停止はPで行います．  
  
注意  
カメラによる認識を行いますが、画素値によって判断しているため照明環境によってはうまく動作しない場合があります．
