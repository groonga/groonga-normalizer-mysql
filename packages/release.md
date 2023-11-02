# リリース手順

## 準備

### Groongaのソースコード取得

パッケージのダウンロードやアップロードを実行するためにGroongaのリポジトリーを参照するため、以下のコマンドを実行します。::

  ```shell
  % git clone --recursive git@github.com:groonga/groonga.git groonga.clean
  ```

### groonga-normalizer-mysqlのソースコード取得

リリース用のクリーンな状態でソースコードを取得するために以下のコマンドを実行します。::

  ```shell
  % git clone git@github.com:groonga/groonga-normalizer-mysql.git　groonga-normalizer-mysql.clean
  ```

以後の手順は、上記でcloneした groonga-normalizer-mysql.clean 配下で実施します。

## リリース

### configureスクリプトの生成/実行

  ```shell
  % ./autogen.sh
  % ./configure \
      --prefix=/tmp/local \
      --with-launchpad-uploader-pgp-key=xxxxxx \
      --with-groonga-source-path=../groonga.clean \
      PKG_CONFIG_PATH=/tmp/local/lib/pkgconfig
  ```

### リリースノート作成

`doc/text/news.md` を編集して変更点を記載する。
翻訳はしていない。

### パッケージの変更履歴作成

  ```shell
  % cd packages
  % rake version:update
  % git add .
  % git commit
  % git push
  ```

### ソースアーカイブ作成(Ubuntu向けパッケージ Nightlyビルド用)

  ```shell
  % ./configure \
      --prefix=/tmp/local \
      --with-launchpad-uploader-pgp-key=xxxxxx \
      --with-groonga-source-path=../groonga.clean \
      PKG_CONFIG_PATH=/tmp/local/lib/pkgconfig
  % make dist
  ```

### CIの確認

以下の2つを確認する。

  * GitHub Actionsの結果
  * Ubuntu向けパッケージのビルド結果(Nightlyでビルドしたもの)

### タグの設定

  ```shell
  % make tag
  ```

### ソースアーカイブのアップロード

  ```shell
  % cd packages
  % rake source
  ```

### リリース用パッケージのダウンロードとアップロード

ソースアーカイブをアップロードしたら、以下のコマンドを実行してUbuntu向けのパッケージをビルドします ::

  ```shell
  % cd packages
  % rake ubuntu
  ```

DebianとCentOS,AlmaLinuxのリリース用パッケージはGitHub Actionsで生成されます。
タグを設定すると自動的にCIが実行され、パッケージが生成されます。

GitHub Actionsでパッケージの生成が終わったことを確認したら、以下のコマンドを実行します。

  ```shell
  % cd packages
  % rake apt
  % rake yum
  ```

### パッケージの署名、リポジトリーの更新

  ```shell
  % cd ../packages.groonga.org
  % rake apt
  % rake yum
  ```

### バージョンの更新

  ```shell
  % make update-version NEW_VERSION=x.x.x
  % git add version
  % git commit
  % git push
  ```
