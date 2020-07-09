#include "FbxModelLoader.h"
#include "..\FbxRenderer\FbxRenderer.h"
#include "..\FbxModel\FbxModel.h"
#include "..\..\Direct11\TextuerLoader\WICTextureLoader.h"

#include <crtdbg.h>
#include <string>
#include <codecvt>

CFbxModelLoader::CFbxModelLoader()
	: m_pDevice11		( nullptr )
	, m_pFbxManager		( nullptr )
	, m_pFbxScene		( nullptr )
	, m_MeshClusterData	()
	, m_Skeletons		()
{
}

CFbxModelLoader::~CFbxModelLoader()
{
}

//////////////////////////////////////////////////////////////////////.
// 作成.
//////////////////////////////////////////////////////////////////////.
HRESULT CFbxModelLoader::Create( ID3D11Device* pDevice )
{
	//---------------------------------------------.
	// デバイスの取得.
	//---------------------------------------------.
	if( pDevice == nullptr ){
		_ASSERT_EXPR( false, "デバイスの取得失敗" );
		MessageBox( nullptr, "デバイスの取得失敗", "Warning", MB_OK );
		return E_FAIL;
	}
	m_pDevice11 = pDevice;

	//---------------------------------------------.
	// マネージャーの作成.
	//---------------------------------------------.
	m_pFbxManager = FbxManager::Create();
	if( m_pFbxManager == nullptr ){
		_ASSERT_EXPR( false, "FbxManager作成失敗" );
		MessageBox( nullptr, "FbxManager作成失敗", "Warning", MB_OK );
		return E_FAIL;
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////.
// 破壊.
//////////////////////////////////////////////////////////////////////.
void CFbxModelLoader::Destroy()
{
	SAFE_DESTROY( m_pFbxScene );
	SAFE_DESTROY( m_pFbxManager );

	m_pDevice11 = nullptr;
}

//////////////////////////////////////////////////////////////////////.
//			モデルの読み込み.
//////////////////////////////////////////////////////////////////////.
HRESULT CFbxModelLoader::LoadModel( CFbxModel* pModelData, const char* fileName )
{
	//---------------------------------------------.
	// インポーターの作成.
	//---------------------------------------------.
	FbxImporter* pFbxImpoter = FbxImporter::Create( m_pFbxManager, "imp" );
	if( pFbxImpoter == nullptr ){
		_ASSERT_EXPR( false, "pFbxImpoter作成失敗" );
		MessageBox( nullptr, "pFbxImpoter作成失敗", "Warning", MB_OK );
		return E_FAIL;
	}

	//---------------------------------------------.
	// FBXファイルの読み込み.
	//---------------------------------------------.
	// ファイル名の設定.
	FbxString fbxFileName( fileName );
	if( pFbxImpoter->Initialize( fbxFileName.Buffer() ) == false ){
		_ASSERT_EXPR( false, "Fbxファイルの読み込み" );
		MessageBox( nullptr, "Fbxファイルの読み込み", "Warning", MB_OK );
		return E_FAIL;
	}

	//---------------------------------------------.
	// シーンオブジェクトの作成.
	//---------------------------------------------.
	m_pFbxScene = FbxScene::Create( m_pFbxManager, "fbxScene" );
	if( m_pFbxScene == nullptr ){
		_ASSERT_EXPR( false, "FbxSceneの作成失敗" );
		MessageBox( nullptr, "FbxSceneの作成失敗", "Warning", MB_OK );
		return E_FAIL;
	}

	//---------------------------------------------.
	// インポーターとシーンオブジェクトの関連付け.
	//---------------------------------------------.
	if( pFbxImpoter->Import( m_pFbxScene ) == false ){
		SAFE_DESTROY( m_pFbxManager );
		SAFE_DESTROY( m_pFbxScene );
		SAFE_DESTROY( pFbxImpoter );
		_ASSERT_EXPR( false, "FbxSceneとFbxImpoterの関連付け失敗" );
		MessageBox( nullptr, "FbxSceneとFbxImpoterの関連付け失敗", "Warning", MB_OK );
		return E_FAIL;
	}
	SAFE_DESTROY( pFbxImpoter );


	//---------------------------------------------.
	// ポリゴンの設定.
	//---------------------------------------------.
	bool convertReslut = false;
	FbxGeometryConverter geometryConverter( m_pFbxManager );
	// ポリゴンを三角形にする.
	// 多角形ポリゴンがあれば作りなおすので時間がかかる.
	convertReslut = geometryConverter.Triangulate( m_pFbxScene, true );
	if( convertReslut == false ){
		_ASSERT_EXPR( false, "ポリゴンの三角化失敗" );
		MessageBox( nullptr, "ポリゴンの三角化失敗", "Warning", MB_OK );
		return E_FAIL;
	}
	geometryConverter.RemoveBadPolygonsFromMeshes( m_pFbxScene );
	// メッシュをマテリアルごとに分割する.
	convertReslut = geometryConverter.SplitMeshesPerMaterial( m_pFbxScene, true );
	if( convertReslut == false ){
		_ASSERT_EXPR( false, "マテリアルごとのメッシュ分割失敗" );
		MessageBox( nullptr, "マテリアルごとのメッシュ分割失敗", "Warning", MB_OK );
		return E_FAIL;
	}

	//---------------------------------------------.
	// FbxSkeletonの取得.
	//---------------------------------------------.
	// FbxSkeletonの数を取得.
	int skeletonNum = m_pFbxScene->GetSrcObjectCount<FbxSkeleton>();
	int skeletonNo = 0;
	m_Skeletons.clear();
	m_Skeletons.resize( skeletonNum );
	for( auto& s : m_Skeletons ){
		// FbxSkeletonの取得.
		s = m_pFbxScene->GetSrcObject<FbxSkeleton>(skeletonNo);
		skeletonNo++;
	}

	//---------------------------------------------.
	// FbxMeshの取得.
	//---------------------------------------------.
	// FbxMeshの数を取得.
	int meshNum = m_pFbxScene->GetSrcObjectCount<FbxMesh>();
	int meshNo = 0;
	pModelData->ReSizeMeshData( meshNum );
	for( auto& m : pModelData->GetMeshData() ){
		// メッシュデータの取得.
		FbxMesh* pMesh = m_pFbxScene->GetSrcObject<FbxMesh>(meshNo);
		// マテリアルの取得.
		GetMaterial( pMesh, m, fileName, pModelData->GetTextures() );
		// メッシュデータの読み込み.
		LoadMesh( pMesh, m );
		// 頂点バッファの作成.
		if( FAILED( CreateVertexBuffers( m ) )) return E_FAIL;
		// インデックスバッファの作成.
		if( FAILED( CreateIndexBuffers( m ) )) return E_FAIL;
		meshNo++;
	}

	//---------------------------------------------.
	//	アニメーションの読み込み.
	//---------------------------------------------.
	CFbxAnimationLoader animLoader;				// アニメーション読み込みクラス.
	std::vector<SAnimationData>	animDataList;	// アニメーションデータ.
	animLoader.LoadAnimationData( m_pFbxScene, m_MeshClusterData, m_Skeletons, &animDataList );
	if( animDataList.empty() == false ){
		// 上で設定したアニメーションデータがあれば.
		// アニメーションコントローラーを作成して.
		// アニメーションデータを追加.
		pModelData->SetAnimationData( animDataList );
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////.
// マテリアル取得.
//////////////////////////////////////////////////////////////////////.
void CFbxModelLoader::GetMaterial( FbxMesh* pMesh, FBXMeshData& mesh, const char* fileName, std::unordered_map<std::string, ID3D11ShaderResourceView*>& textures )
{
	// マテリアル情報が無いので終了.
	if( pMesh->GetElementMaterialCount() == 0 ) return;

	// Mesh側のマテリアル情報を取得.
	FbxLayerElementMaterial* material = pMesh->GetElementMaterial(0);
	int index = material->GetIndexArray().GetAt(0);

	// FBXのマテリアルを取得.
	FbxSurfaceMaterial* pMat = pMesh->GetNode()->GetSrcObject<FbxSurfaceMaterial>(index);

	// LambertかPhongか.
	if( pMat->GetClassId().Is( FbxSurfaceLambert::ClassId )){
		// Ambientの情報を取得.
		FbxProperty ambient = pMat->FindProperty( FbxSurfaceMaterial::sAmbient );
		// diffuseの情報を取得.
		FbxProperty diffuse = pMat->FindProperty( FbxSurfaceMaterial::sDiffuse );
		// Specularの情報を取得.
		FbxProperty specular = pMat->FindProperty( FbxSurfaceMaterial::sSpecular );
		
		// アンビエント取得.
		if( ambient.IsValid() == true ){
			mesh.Material.Ambient.x = (float)ambient.Get<FbxDouble4>()[0];
			mesh.Material.Ambient.y = (float)ambient.Get<FbxDouble4>()[1];
			mesh.Material.Ambient.z = (float)ambient.Get<FbxDouble4>()[2];
			mesh.Material.Ambient.w = (float)ambient.Get<FbxDouble4>()[3];
		} else {
			mesh.Material.Ambient = { 1.0f, 1.0f, 1.0f, 1.0f };
		}

		// ディフーズ取得.
		if( diffuse.IsValid() == true ){
			mesh.Material.Diffuse.x = (float)diffuse.Get<FbxDouble4>()[0];
			mesh.Material.Diffuse.y = (float)diffuse.Get<FbxDouble4>()[1];
			mesh.Material.Diffuse.z = (float)diffuse.Get<FbxDouble4>()[2];
			mesh.Material.Diffuse.w = (float)diffuse.Get<FbxDouble4>()[3];
		} else {
			mesh.Material.Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
		}

		// スペキュラ取得.
		if( specular.IsValid() == true ){
			mesh.Material.Specular.x = (float)specular.Get<FbxDouble4>()[0];
			mesh.Material.Specular.y = (float)specular.Get<FbxDouble4>()[1];
			mesh.Material.Specular.z = (float)specular.Get<FbxDouble4>()[2];
			mesh.Material.Specular.w = (float)specular.Get<FbxDouble4>()[3];
		} else {
			mesh.Material.Specular = { 1.0f, 1.0f, 1.0f, 1.0f };
		}
		// マテリアル名取得.
		mesh.Material.Name = pMat->GetName();

		//---------------------------------------------.
		// テクスチャ作成.
		//---------------------------------------------.
		FbxFileTexture* texture = nullptr;

		// FbxFileTexture == シングルテクスチャ.
		int textureCount = diffuse.GetSrcObjectCount<FbxFileTexture>();
		if( textureCount > 0 ){
			texture = diffuse.GetSrcObject<FbxFileTexture>(0);
			mesh.UVSetCount = textureCount;
		} else {
			// マルチテクスチャ(レイヤーテクスチャ)の可能性があるので検索.
			int layerCount = diffuse.GetSrcObjectCount<FbxLayeredTexture>();
			if( layerCount > 0 ){
				/******************************
				マルチテクスチャの実装は後.
				*******************************/
				texture = diffuse.GetSrcObject<FbxFileTexture>(0);
				mesh.UVSetCount = layerCount;
			}
		}
		if( texture == nullptr ) return;
		// テクスチャの読み込み.
		if( FAILED( LoadTexture( texture, fileName, pMat->GetName(), textures ))) return;
	}
}

//////////////////////////////////////////////////////////////////////.
// テクスチャの読み込み.
//////////////////////////////////////////////////////////////////////.
HRESULT CFbxModelLoader::LoadTexture( FbxFileTexture* texture, const char* fileName, const char* keyName, std::unordered_map<std::string, ID3D11ShaderResourceView*>& textures )
{
	//---------------------------------------------.
	// ファイル名を取得.
	//---------------------------------------------.
	std::string textureName = texture->GetRelativeFileName();
	std::string path = fileName;

	// 相対パスが空の場合、絶対パスを受け取る.
	if( textureName.length() == 0 ) textureName = texture->GetFileName();

	// モデルのパスを使用し、モデルファイル名を削除しとく.
	int pos = path.find_last_of('\\')+1;
	path = path.substr( 0, pos );

	// テクスチャのファイル名を取得する.
	int texNamePos = textureName.find_last_of('\\')+1;
	textureName = textureName.substr( texNamePos, textureName.length() );
	// ファイルパスと、テクスチャ名を合わせる.
	path += textureName;

	// wchar_t に変換.
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> cv;
	std::wstring wstr_file_name = cv.from_bytes(path);

	//---------------------------------------------.
	// テクスチャリソース作成.
	//---------------------------------------------.
	if( FAILED( 
		DirectX::CreateWICTextureFromFile(
		m_pDevice11,
		wstr_file_name.c_str(),
		nullptr,
		&textures[keyName] ))){
		_ASSERT_EXPR( false, "テクスチャ取得失敗" );
		MessageBox( nullptr, "テクスチャ取得失敗", "Warning", MB_OK );
		return E_FAIL;
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////.
// メッシュの読み込み.
//////////////////////////////////////////////////////////////////////.
void CFbxModelLoader::LoadMesh( FbxMesh* pMesh, FBXMeshData& meshData )
{
	// インデックス情報の取得.
	LoadIndices( pMesh, meshData );

	// 頂点数の取得.
	int ctrlCount = pMesh->GetControlPointsCount();

	// ウェイト.
	std::vector<std::vector<float>> weights( ctrlCount );
	// ボーンインデックス.
	std::vector<std::vector<int>> bones( ctrlCount );
	// スキン情報の取得.
	LoadSkin( pMesh, meshData, weights, bones );

	// ポリゴン数の取得.
	int polygonCount = pMesh->GetPolygonCount();
	// 頂点のカウンター.
	int vertexCounter = 0;

	// ポリゴンの数分.
	for( int i = 0; i < polygonCount; i++ ){
		// ポリゴンのサイズ.
		int polygonSize = pMesh->GetPolygonSize(i);
		if( polygonSize != 3 ){
			// 三角ポリゴンじゃない.
			_ASSERT_EXPR( false, "メッシュが三角じゃないです" );
			MessageBox( nullptr, "メッシュが三角じゃないです", "Warning", MB_OK );
			return;
		}

		// 一つのポリゴンの頂点3つ分.
		for( int j = 0; j < polygonSize; j++ ){
			// 頂点のインデックスを取得.
			int ctrlPointIndex = pMesh->GetPolygonVertex( i, j );
			// 頂点情報を用意.
			VERTEX vertex = {};

			// 頂点座標を取得.
			LoadVertices( pMesh, vertex, ctrlPointIndex );
			// 法線の取得.
			LoadNormals( pMesh, vertex, ctrlPointIndex, vertexCounter );
			// 頂点カラーの取得.
			LoadColors( pMesh, vertex, ctrlPointIndex, vertexCounter );
			// UV座標の取得.
			for( int k = 0; k < 2; k++ ){
				LoadUV( pMesh, vertex, ctrlPointIndex, pMesh->GetTextureUVIndex( i, j ), k );
			}
			// ウェイトとボーンインデックスの設定.
			SetBoneWeight( vertex, weights[ctrlPointIndex], bones[ctrlPointIndex] );

			// 頂点情報を追加.
			meshData.Vertices.emplace_back( vertex );

			// 頂点数を加算.
			vertexCounter++;
		}
	}
}

//////////////////////////////////////////////////////////////////////.
// インデックス情報読み込み.
//////////////////////////////////////////////////////////////////////.
void CFbxModelLoader::LoadIndices( FbxMesh* pMesh, FBXMeshData& meshData )
{
	// ポリゴン数の取得.
	int polygonCount = pMesh->GetPolygonCount();
	meshData.PolygonVertexCount = pMesh->GetPolygonVertexCount();
	// ポリゴンの数だけ連番として保存.
	for( int i = 0; i < polygonCount; i++ ){
		meshData.Indices.emplace_back( i * 3 + 2 );
		meshData.Indices.emplace_back( i * 3 + 1 );
		meshData.Indices.emplace_back( i * 3 + 0 );
	}
}

//////////////////////////////////////////////////////////////////////.
// 頂点情報読み込み.
//////////////////////////////////////////////////////////////////////.
void CFbxModelLoader::LoadVertices( FbxMesh* pMesh, VERTEX& vertex, int ctrlPointIndex )
{
	vertex.Pos.x = (float)pMesh->GetControlPoints()[ctrlPointIndex][0];
	vertex.Pos.y = (float)pMesh->GetControlPoints()[ctrlPointIndex][1];
	vertex.Pos.z = (float)pMesh->GetControlPoints()[ctrlPointIndex][2];
}

//////////////////////////////////////////////////////////////////////.
// 法線情報読み込み.
//////////////////////////////////////////////////////////////////////.
void CFbxModelLoader::LoadNormals( FbxMesh* pMesh, VERTEX& vertex, int ctrlPointIndex, int vertexCounter )
{
	if( pMesh->GetElementNormalCount() < 1 ) return;

	/************************************************
	* DirectXの場合x値を反転させる必要あり.
	**/

	// 法線の取得.
	FbxGeometryElementNormal* pNormal = pMesh->GetElementNormal(0);

	// マッピングモードは法線がどのようにモデル表面に定義されているかを表す.
	switch( pNormal->GetMappingMode() )
	{
	// eByControlPoint : 頂点に対しての法線.
	case FbxGeometryElement::eByControlPoint:
	{
		// インデックスの取得.
		int index = pNormal->GetIndexArray().GetAt(ctrlPointIndex);

		// リファレンスモードは法線がどのように配列に格納されているかを表す.
		switch( pNormal->GetReferenceMode() )
		{
		// eDIRECT : 順番に格納している.
		case FbxGeometryElement::eDirect:
			vertex.Normal.x = -(float)pNormal->GetDirectArray().GetAt(ctrlPointIndex)[0];
			vertex.Normal.y =  (float)pNormal->GetDirectArray().GetAt(ctrlPointIndex)[1];
			vertex.Normal.z =  (float)pNormal->GetDirectArray().GetAt(ctrlPointIndex)[2];
			break;

		// eIndexToDirect : インデックスに対応した並び.
		case FbxGeometryElement::eIndexToDirect:
			vertex.Normal.x = -(float)pNormal->GetDirectArray().GetAt(index)[0];
			vertex.Normal.y =  (float)pNormal->GetDirectArray().GetAt(index)[1];
			vertex.Normal.z =  (float)pNormal->GetDirectArray().GetAt(index)[2];
			break;
		}
	}
	// eByPolygonVertex : ポリゴンに対しての法線.
	case FbxGeometryElement::eByPolygonVertex:
	{
		// インデックスの取得.
		int index = pNormal->GetIndexArray().GetAt(vertexCounter);

		// リファレンスモードは法線がどのように配列に格納されているかを表す.
		switch( pNormal->GetReferenceMode() )
		{
		// eDIRECT : 順番に格納している.
		case FbxGeometryElement::eDirect:
			vertex.Normal.x = -(float)pNormal->GetDirectArray().GetAt(vertexCounter)[0];
			vertex.Normal.y =  (float)pNormal->GetDirectArray().GetAt(vertexCounter)[1];
			vertex.Normal.z =  (float)pNormal->GetDirectArray().GetAt(vertexCounter)[2];
			break;

		// eIndexToDirect : インデックスに対応した並び.
		case FbxGeometryElement::eIndexToDirect:
			vertex.Normal.x = -(float)pNormal->GetDirectArray().GetAt(index)[0];
			vertex.Normal.y =  (float)pNormal->GetDirectArray().GetAt(index)[1];
			vertex.Normal.z =  (float)pNormal->GetDirectArray().GetAt(index)[2];
			break;
		}
	}
	}
}

//////////////////////////////////////////////////////////////////////.
// 頂点カラー読み込み.
//////////////////////////////////////////////////////////////////////.
void CFbxModelLoader::LoadColors( FbxMesh* pMesh, VERTEX& vertex, int ctrlPointIndex, int vertexCounter )
{
	// 頂点カラーデータの数を確認.
	if( pMesh->GetElementVertexColorCount() == 0 ) return;

	FbxGeometryElementVertexColor* pVertexColor = pMesh->GetElementVertexColor(0);

	// マッピングモードは頂点カラーがどのようにモデル表面に定義されているかを表す.
	switch( pVertexColor->GetMappingMode() )
	{
	// eByControlPoint : 頂点に対しての法線.
	case FbxGeometryElement::eByControlPoint:
	{
		// インデックスの取得.
		int index = pVertexColor->GetIndexArray().GetAt(ctrlPointIndex);

		// リファレンスモードは法線がどのように配列に格納されているかを表す.
		switch( pVertexColor->GetReferenceMode() )
		{
		// eDIRECT : 順番に格納している.
		case FbxGeometryElement::eDirect:
			vertex.Color.x = (float)pVertexColor->GetDirectArray().GetAt(ctrlPointIndex)[0];
			vertex.Color.y = (float)pVertexColor->GetDirectArray().GetAt(ctrlPointIndex)[1];
			vertex.Color.z = (float)pVertexColor->GetDirectArray().GetAt(ctrlPointIndex)[2];
			vertex.Color.w = (float)pVertexColor->GetDirectArray().GetAt(ctrlPointIndex)[3];
			break;

		// eIndexToDirect : インデックスに対応した並び.
		case FbxGeometryElement::eIndexToDirect:
			vertex.Color.x = (float)pVertexColor->GetDirectArray().GetAt(index)[0];
			vertex.Color.y = (float)pVertexColor->GetDirectArray().GetAt(index)[1];
			vertex.Color.z = (float)pVertexColor->GetDirectArray().GetAt(index)[2];
			vertex.Color.w = (float)pVertexColor->GetDirectArray().GetAt(index)[3];
			break;
		}
	}
	// eByPolygonVertex : ポリゴンに対しての法線.
	case FbxGeometryElement::eByPolygonVertex:
	{
		// インデックスの取得.
		int index = pVertexColor->GetIndexArray().GetAt(vertexCounter);

		// リファレンスモードは法線がどのように配列に格納されているかを表す.
		switch( pVertexColor->GetReferenceMode() )
		{
		// eDIRECT : 順番に格納している.
		case FbxGeometryElement::eDirect:
			vertex.Color.x = (float)pVertexColor->GetDirectArray().GetAt(vertexCounter)[0];
			vertex.Color.y = (float)pVertexColor->GetDirectArray().GetAt(vertexCounter)[1];
			vertex.Color.z = (float)pVertexColor->GetDirectArray().GetAt(vertexCounter)[2];
			vertex.Color.w = (float)pVertexColor->GetDirectArray().GetAt(vertexCounter)[3];
			break;

		// eIndexToDirect : インデックスに対応した並び.
		case FbxGeometryElement::eIndexToDirect:
			vertex.Color.x = (float)pVertexColor->GetDirectArray().GetAt(index)[0];
			vertex.Color.y = (float)pVertexColor->GetDirectArray().GetAt(index)[1];
			vertex.Color.z = (float)pVertexColor->GetDirectArray().GetAt(index)[2];
			vertex.Color.w = (float)pVertexColor->GetDirectArray().GetAt(index)[3];
			break;
		}
	}
	}
}

//////////////////////////////////////////////////////////////////////.
// UV情報読み込み.
//////////////////////////////////////////////////////////////////////.
void CFbxModelLoader::LoadUV( FbxMesh* pMesh, VERTEX& vertex, int ctrlPointIndex, int texUVIndex, int uvLayer )
{
	if( uvLayer >= 2 || pMesh->GetElementUVCount() <= uvLayer ) return;

	/************************************************
	* DirectXの場合UV座標のV値を反転させる必要あり.
	**/

	FbxGeometryElementUV* pVertexUV = pMesh->GetElementUV( uvLayer );
	// マッピングモードはUVがどのようにモデル表面に定義されているかを表す.
	switch( pVertexUV->GetMappingMode() )
	{
	// eByControlPoint : 頂点に対しての法線.
	case FbxGeometryElement::eByControlPoint:
	{
		// インデックスの取得.
		int index = pVertexUV->GetIndexArray().GetAt(ctrlPointIndex);

		// リファレンスモードは法線がどのように配列に格納されているかを表す.
		switch( pVertexUV->GetReferenceMode() )
		{
			// eDIRECT : 順番に格納している.
		case FbxGeometryElement::eDirect:
			vertex.UV.x =  (float)pVertexUV->GetDirectArray().GetAt(ctrlPointIndex)[0];
			vertex.UV.y = -(float)pVertexUV->GetDirectArray().GetAt(ctrlPointIndex)[1];
			break;

			// eIndexToDirect : インデックスに対応した並び.
		case FbxGeometryElement::eIndexToDirect:
			vertex.UV.x =  (float)pVertexUV->GetDirectArray().GetAt(index)[0];
			vertex.UV.y = -(float)pVertexUV->GetDirectArray().GetAt(index)[1];
			break;
		}
	}
	// eByPolygonVertex : ポリゴンに対しての法線.
	case FbxGeometryElement::eByPolygonVertex:
	{
		// リファレンスモードは法線がどのように配列に格納されているかを表す.
		switch( pVertexUV->GetReferenceMode() )
		{
			// eDIRECT : 順番に格納している.
		case FbxGeometryElement::eDirect:
			// eIndexToDirect : インデックスに対応した並び.
		case FbxGeometryElement::eIndexToDirect:
			vertex.UV.x =  (float)pVertexUV->GetDirectArray().GetAt(texUVIndex)[0];
			vertex.UV.y = -(float)pVertexUV->GetDirectArray().GetAt(texUVIndex)[1];
			break;
		}
	}
	}
}

//////////////////////////////////////////////////////////////////////.
// スキン情報読み込み.
//////////////////////////////////////////////////////////////////////.
void CFbxModelLoader::LoadSkin( FbxMesh* pMesh, FBXMeshData& meshData, std::vector<std::vector<float>>& weights, std::vector<std::vector<int>>& bones )
{
	// ダウンキャストしてスキン情報を取得.
	FbxSkin* pSkin = (FbxSkin*)pMesh->GetDeformer( 0, FbxDeformer::eSkin );
	// スキン情報が無ければ終了.
	if( pSkin == nullptr ) return;

	m_MeshClusterData.emplace_back();

	// ※Cluster == Bone.
	// ボーンの数.
	int boneCount = pSkin->GetClusterCount();
	for( int boneIndex = 0; boneIndex < boneCount; boneIndex++ ){
		// ボーン情報取得.
		FbxCluster* pCluster = pSkin->GetCluster( boneIndex );
		FbxNode* pNode = pCluster->GetLink();

		m_MeshClusterData.back().ClusterName.emplace_back( pNode->GetName() );

		FbxAMatrix bindPoseMatrix;	// 初期ボーン行列.
		FbxAMatrix newbindpose;		// 初期ポーズ.

		pCluster->GetTransformLinkMatrix( bindPoseMatrix );
		pCluster->GetTransformMatrix( newbindpose );

		newbindpose = bindPoseMatrix.Inverse() * newbindpose;
		// 初期ボーン座標の設定.
		meshData.Skin.InitBonePositions.emplace_back( newbindpose );

		// ボーンインデックスとウェイトの設定.
		int*	boneVertexIndices = pCluster->GetControlPointIndices();
		double* boneVertexWeights = pCluster->GetControlPointWeights();
		// ボーンに影響するポリゴンの数.
		int	numBoneVertexIndices = pCluster->GetControlPointIndicesCount();
		for( int boneVertexIndex = 0; boneVertexIndex < numBoneVertexIndices; boneVertexIndex++ ){
			int cpIndex			= boneVertexIndices[boneVertexIndex];
			float boneWeight	= (float)boneVertexWeights[boneVertexIndex];
			weights[cpIndex].emplace_back( boneWeight );
			bones[cpIndex].emplace_back( boneIndex );
		}
	}
	// ウェイトを重い順にソート.
	for( int i = 0; i < (int)weights.size(); i++ ){
		for( int m = (int)weights[i].size(); m > 1; m-- ){
			for( int n = 1; n < m; n++ ){
				if( weights[i][n-1] < weights[i][n] ){

					float tmpweight = weights[i][n-1];
					weights[i][n-1] = weights[i][n];
					weights[i][n] = tmpweight;

					int tmpbone = bones[i][n-1];
					bones[i][n-1] = bones[i][n];
					bones[i][n] = tmpbone;
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////.
// ウェイトとボーンの設定.
//////////////////////////////////////////////////////////////////////.
void CFbxModelLoader::SetBoneWeight( VERTEX& vertex, const std::vector<float>& weight, const std::vector<int>& bone )
{
	// ウェイトが 0から重い順になっているはずなので.
	// 配列の3つ目から入れていく(最大ウェイト数 : 4つ).
	switch( weight.size() )
	{
	default:
	case 4:
		vertex.BoneWeight.w = weight[3];
		vertex.BoneIndex.w = bone[3];
	case 3:
		vertex.BoneWeight.z = weight[2];
		vertex.BoneIndex.z = bone[2];
	case 2:
		vertex.BoneWeight.y = weight[1];
		vertex.BoneIndex.y = bone[1];
	case 1:
		vertex.BoneWeight.x = weight[0];
		vertex.BoneIndex.x = bone[0];
		break;
	case 0:
		break;
	}
}

//////////////////////////////////////////////////////////////////////.
// 頂点バッファ作成.
//////////////////////////////////////////////////////////////////////.
HRESULT CFbxModelLoader::CreateVertexBuffers( FBXMeshData& meshData )
{
	D3D11_BUFFER_DESC bd;
	bd.ByteWidth = sizeof(VERTEX)*meshData.Vertices.size();
	bd.Usage		= D3D11_USAGE_DYNAMIC;
	bd.BindFlags	= D3D11_BIND_VERTEX_BUFFER;
	bd.MiscFlags	= 0;
	bd.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;
	bd.StructureByteStride	= 0;
	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem			= &meshData.Vertices[0];
	data.SysMemPitch		= 0;
	data.SysMemSlicePitch	= 0;

	// 頂点バッファの作成.
	if( FAILED( m_pDevice11->CreateBuffer( &bd, &data, &meshData.pVertexBuffer ))){
		_ASSERT_EXPR( false, "頂点バッファの作成失敗" );
		MessageBox( nullptr, "頂点バッファの作成失敗", "Warning", MB_OK );
		return E_FAIL;
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////.
// インデックスバッファ作成.
//////////////////////////////////////////////////////////////////////.
HRESULT CFbxModelLoader::CreateIndexBuffers( FBXMeshData& meshData )
{
	D3D11_BUFFER_DESC bd;
	bd.ByteWidth = sizeof(int)*meshData.Indices.size();
	bd.Usage		= D3D11_USAGE_DEFAULT;
	bd.BindFlags	= D3D11_BIND_INDEX_BUFFER;
	bd.MiscFlags	= 0;
	bd.CPUAccessFlags		= 0;
	bd.StructureByteStride	= 0;
	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem			= &meshData.Indices[0];
	data.SysMemPitch		= 0;
	data.SysMemSlicePitch	= 0;

	// インデックスバッファの作成.
	if( FAILED( m_pDevice11->CreateBuffer( &bd, &data, &meshData.pIndexBuffer ))){
		_ASSERT_EXPR( false, "インデックスバッファの作成失敗" );
		MessageBox( nullptr, "インデックスバッファの作成失敗", "Warning", MB_OK );
		return E_FAIL;
	}
	return S_OK;
}