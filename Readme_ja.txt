================================================================================
	IPC8 Library (Readme_ja.txt)

						2014年12月17日
						株式会社デンソーウェーブ
================================================================================

このたびは 'IPC8 Library' をダウンロードいただき，誠にありがとうございます．

----------
1. 各種ライブラリの説明
1-1. IPC8ドライバ
I/OやFRAMなどのIPC8機能を利用するためのドライバです．
詳細な説明は，driver_dkms/IPC8Driver_UsersGuide_ja.docをご参照ください．

1-2. TPCommライブラリ
ティーチングペンダントと通信するためのライブラリです．
ライブラリをコンパイルするには以下のコマンドを実行します．

$ cd library/src/TPComm
$ make -f Makefile.Linux
$ make -f Makefile.Linux install

1-3. b-CAPクライアントライブラリ
RC8と通信するためのライブラリです．
ライブラリをコンパイルするには以下のコマンドを実行します．

$ cd library/src/bCAPClient
$ make -f Makefile.Linux
$ make -f Makefile.Linux install

----------
2. 各種サンプルの説明
2-1. IPC8ドライバ
IPC8ドライバを使用するサンプルプログラムです．
サンプルプログラムを実行するには以下のコマンドを実行します．

$ cd sample/Driver
$ make
$ make run

2-2. TPCommライブラリ
TPCommライブラリを使用するサンプルプログラムです．
サンプルプログラムを実行するには以下のコマンドを実行します．

$ cd sample/TPComm
$ make
$ make run

2-3. b-CAPクライアントライブラリ
b-CAPクライアントライブラリを使用するサンプルプログラムです．
サンプルプログラムを実行するには以下のコマンドを実行します．

$ cd sample/bCAPClient
$ make
$ make run

[注意1]
Makefileは必要に応じて修正してください．

----------
3. ライセンス
3-1. GNU General Public License
driver_dkms以下のすべてのプログラムはdriver_dkms/License.txtに示すGNU General Public Licenseの元で公開されています。

3-2. MIT License
library以下のすべてのプログラムはlibrary/License.txtに示すMIT Licenseの元で公開されています。

----------
4. リリースノート

下記に前バージョンからの修正点が列挙されています．ご確認の上，本製品を
ご活用いただきますよう宜しくお願い致します．

[v1.0.0] 初版
