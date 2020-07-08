#include "FbxModel.h"
#include "..\FbxAnimation\FbxAnimationController.h"

CFbxModel::CFbxModel()
	: m_MeshData	()
	, m_Textures	()
	, m_pAc			( nullptr )
	, m_Position	( 0.0f, 0.0f, 0.0f )
	, m_Rotation	( 0.0f, 0.0f, 0.0f )
	, m_Scale		( 1.0f, 1.0f, 1.0f )
{
}

CFbxModel::~CFbxModel()
{
	SAFE_DELETE( m_pAc );
	for( auto& m : m_MeshData ) m.Release();
	for( auto& t : m_Textures ) SAFE_RELEASE( t.second )
}

//-------------------------------------.
// メッシュデータの取得.
//-------------------------------------.
std::vector<FBXMeshData>& CFbxModel::GetMeshData()
{ 
	return m_MeshData;
}

//-------------------------------------.
// メッシュデータのリサイズ.
//-------------------------------------.
void CFbxModel::ReSizeMeshData( const int& size )
{ 
	m_MeshData.clear();
	m_MeshData.resize(size);
}

//-------------------------------------.
// テクスチャの取得.
//-------------------------------------.
std::unordered_map<std::string, ID3D11ShaderResourceView*>& 
CFbxModel::GetTextures()
{ 
	return m_Textures; 
}

//-------------------------------------.
// アニメーションコントローラーの取得.
//-------------------------------------.
CFbxAnimationController* CFbxModel::GetPtrAC()
{ 
	return m_pAc; 
}

//-------------------------------------.
// アニメーションデータの設定.
//-------------------------------------.
void CFbxModel::SetAnimationData( const std::vector<SAnimationData>& animDataList )
{
	m_pAc = new CFbxAnimationController;
	m_pAc->SetAnimDataList( animDataList );
}


//-------------------------------------.
// ワールド行列を取得.
//-------------------------------------.
DirectX::XMMATRIX CFbxModel::GetWorldMatrix()
{
	DirectX::XMMATRIX mTarn, mRot, mScale;
	// 平行移動行列.
	mTarn = DirectX::XMMatrixTranslation(
		m_Position.x, m_Position.y, m_Position.z );
	// 回転行列.
	mRot = DirectX::XMMatrixRotationRollPitchYaw(
		m_Rotation.x, m_Rotation.y, m_Rotation.z );
	// 拡大縮小行列.
	mScale = DirectX::XMMatrixScaling(
		m_Scale.x, m_Scale.y, m_Scale.z );

	// ワールド行列作成.
	// 拡縮*回転*移動.
	return mScale * mRot * mTarn;
}

//-------------------------------------.
// アニメーションコントローラーの取得.
//-------------------------------------.
CFbxAnimationController CFbxModel::GetAnimationController()
{
	if( m_pAc != nullptr ) return *m_pAc;
	return CFbxAnimationController();
}

//-------------------------------------.
// 座標の設定.
//-------------------------------------.
void CFbxModel::SetPosition( const DirectX::XMFLOAT3& pos )
{ 
	m_Position = pos; 
}

//-------------------------------------.
// 回転の設定.
//-------------------------------------.
void CFbxModel::SetRotation( const DirectX::XMFLOAT3& rot )
{ 
	m_Rotation = rot;
}

//-------------------------------------.
// 大きさの設定.
//-------------------------------------.
void CFbxModel::SetScale( const DirectX::XMFLOAT3& scale )
{ 
	m_Scale = scale;
}
void CFbxModel::SetScale( const float& scale )
{ 
	m_Scale = { scale, scale, scale };
}

//-------------------------------------.
// アニメーション速度の設定.
//-------------------------------------.
void CFbxModel::SetAnimSpeed( const double& speed )
{
	if( m_pAc == nullptr ) return;
	m_pAc->SetAnimSpeed( speed );
}

//-------------------------------------.
// アニメーションの変更.
//-------------------------------------.
void CFbxModel::ChangeAnimation( int& index )
{
	if( m_pAc == nullptr ) return;
	m_pAc->ChangeAnimation( index );
}