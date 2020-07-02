# FBXResareh
FBX_SDKを使用したFBXファイルの読み込みテスト  
  
初めにFBX_SDKのインストール  
以下のサイトにアクセス  
<https://www.autodesk.com/developer-network/platform-technologies/fbx-sdk-2018-0>  
  
  赤丸をダウンロード  
<img src="https://github.com/Req1630/FBXResareh/blob/master/fbxTextuer/FBXSDK3.png" alt="FbxSDK" title="FbxSDK">  
  
  ダウンロード後 exeファイルを起動  
  `Iaccept`を押して有効にし次に進む  
    
<img src="https://github.com/Req1630/FBXResareh/blob/master/fbxTextuer/FBXSDK1.png" alt="FbxSDK" title="FbxSDK">  
  保存先を決めて(保存先を覚えとく)  
  インストールを始める  
<img src="https://github.com/Req1630/FBXResareh/blob/master/fbxTextuer/FBXSDK2.png" alt="FbxSDK" title="FbxSDK">  
  
  FBX_SDKのインストールはこれで終了  
  次にプロジェクトの設定  
  
  プロジェクトのプロパティを開く  
  VC++ ディレクトリを開く  
  構成が`すべての構成`になっていることを確認  
<img src="https://github.com/Req1630/FBXResareh/blob/master/fbxTextuer/プロパティ1.PNG" alt="FbxSDK" title="FbxSDK">  
  インクルードディレクトリを選択し 
<img src="https://github.com/Req1630/FBXResareh/blob/master/fbxTextuer/プロパティ2.png" alt="FbxSDK" title="FbxSDK">  
  先ほどインストールしたFBXSDKのIncludeフォルダまでのファイルパスを入力する  
<img src="https://github.com/Req1630/FBXResareh/blob/master/fbxTextuer/プロパティ3.png" alt="FbxSDK" title="FbxSDK">  
  次にライブラリディレクトリを選択し  
<img src="https://github.com/Req1630/FBXResareh/blob/master/fbxTextuer/プロパティ4.png" alt="FbxSDK" title="FbxSDK">  
  同じく先ほどインストールしたFBXSDKのlibフォルダ->vs2015->x86までのファイルパスを入力する  
<img src="https://github.com/Req1630/FBXResareh/blob/master/fbxTextuer/プロパティ5.png" alt="FbxSDK" title="FbxSDK">  
  
