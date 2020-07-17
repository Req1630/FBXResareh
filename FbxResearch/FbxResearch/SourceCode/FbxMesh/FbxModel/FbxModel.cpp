#include "FbxModel.h"
#include "..\FbxAnimation\FbxAnimationController.h"

#include <iostream>
#include <fstream>

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

//////////////////////////////////////////////////////.
// メッシュデータの取得.
//////////////////////////////////////////////////////.
std::vector<FBXMeshData>& CFbxModel::GetMeshData()
{ 
	return m_MeshData;
}

//////////////////////////////////////////////////////.
// メッシュデータのリサイズ.
//////////////////////////////////////////////////////.
void CFbxModel::ReSizeMeshData( const int& size )
{ 
	m_MeshData.clear();
	m_MeshData.resize(size);
}

//////////////////////////////////////////////////////.
// テクスチャの取得.
//////////////////////////////////////////////////////.
std::unordered_map<std::string, ID3D11ShaderResourceView*>& 
CFbxModel::GetTextures()
{ 
	return m_Textures; 
}

//////////////////////////////////////////////////////.
// アニメーションコントローラーの取得.
//////////////////////////////////////////////////////.
CFbxAnimationController* CFbxModel::GetPtrAC()
{ 
	return m_pAc; 
}

//////////////////////////////////////////////////////.
// アニメーションデータの設定.
//////////////////////////////////////////////////////.
void CFbxModel::SetAnimationData( const SAnimationDataList& animDataList )
{
	m_pAc = new CFbxAnimationController;
	m_pAc->SetAnimDataList( animDataList );
}

//////////////////////////////////////////////////////.
// ボーン名データの作成.
//////////////////////////////////////////////////////.
void CFbxModel::SetBoneNameData( std::map<std::string, std::pair<int, int>>& boneNumberList )
{
	m_BoneNumberList = boneNumberList;
}

//////////////////////////////////////////////////////.
// ワールド行列を取得.
//////////////////////////////////////////////////////.
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

//////////////////////////////////////////////////////.
// アニメーションコントローラーの取得.
//////////////////////////////////////////////////////.
CFbxAnimationController CFbxModel::GetAnimationController()
{
	if( m_pAc != nullptr ) return *m_pAc;
	return CFbxAnimationController();
}

//////////////////////////////////////////////////////.
// ボーン座標の取得.
//////////////////////////////////////////////////////.
DirectX::XMFLOAT3 CFbxModel::GetBonePosition( const char* boneName, CFbxAnimationController* pAC )
{
	DirectX::XMFLOAT4X4 localBonePos;
	// ボーン行列を取得して、Float4x4に変換.
	DirectX::XMStoreFloat4x4( &localBonePos, GetBoneMatrix( boneName, pAC ) );

	DirectX::XMMATRIX mTarn, mRot, mScale;
	
	// 拡大縮小行列.
	mScale = DirectX::XMMatrixScaling(
		m_Scale.x, m_Scale.y, m_Scale.z );
	// 回転行列.
	mRot = DirectX::XMMatrixRotationRollPitchYaw(
		m_Rotation.x, m_Rotation.y, m_Rotation.z );
	// 平行移動行列.
	// 行列のオフセット値を取得.
	// _41:x, _42:y. _43:z.
	mTarn = DirectX::XMMatrixTranslation(
		localBonePos._41, localBonePos._42, localBonePos._43 );
	
	// ワールド行列作成.
	DirectX::XMFLOAT4X4 worldBonePos;
	DirectX::XMStoreFloat4x4( &worldBonePos, mTarn * mScale * mRot );

	return
	{
		// 取得したボーン座標を自分の座標と足して返す.
		worldBonePos._41 + m_Position.x,
		worldBonePos._42 + m_Position.y,
		worldBonePos._43 + m_Position.z,
	};

}

//////////////////////////////////////////////////////.
// ボーン行列の取得.
//////////////////////////////////////////////////////.
DirectX::XMMATRIX CFbxModel::GetBoneMatrix( const char* boneName, CFbxAnimationController* pAC )
{
	int meshNo = 0;	// メッシュ番号.
	int boneNo = 0;	// ボーン番号.
	try {
		meshNo = m_BoneNumberList.at(boneName).first;
		boneNo = m_BoneNumberList.at(boneName).second;
	} catch( std::out_of_range& ) {
		_ASSERT_EXPR( false, "指定したボーン名がありません" );
		MessageBox( nullptr, "指定したボーン名がありません", "Warning", MB_OK );
	}
	FbxMatrix frameMatrix;
	if( pAC == nullptr ){
		if( m_pAc == nullptr ) return FbxMatrixConvertDXMMatrix( frameMatrix );
		if( m_pAc->GetFrameLinkMatrix( meshNo, boneNo, &frameMatrix ) == false ){
			_ASSERT_EXPR( false, "メッシュ番号かボーン番号が合いません" );
			MessageBox( nullptr, "メッシュ番号かボーン番号が合いません", "Warning", MB_OK );
		}
	} else {
		if( pAC->GetFrameLinkMatrix( meshNo, boneNo, &frameMatrix ) == false ){
			_ASSERT_EXPR( false, "メッシュ番号かボーン番号が合いません" );
			MessageBox( nullptr, "メッシュ番号かボーン番号が合いません", "Warning", MB_OK );
		}
	}
	// FbxMatrixをDirectXMatrixに変換して返す.
	return FbxMatrixConvertDXMMatrix( frameMatrix );
}

//////////////////////////////////////////////////////.
// 座標の設定.
//////////////////////////////////////////////////////.
void CFbxModel::SetPosition( const DirectX::XMFLOAT3& pos )
{ 
	m_Position = pos; 
}

//////////////////////////////////////////////////////.
// 回転の設定.
//////////////////////////////////////////////////////.
void CFbxModel::SetRotation( const DirectX::XMFLOAT3& rot )
{ 
	m_Rotation = rot;
}

//////////////////////////////////////////////////////.
// 大きさの設定.
//////////////////////////////////////////////////////.
void CFbxModel::SetScale( const DirectX::XMFLOAT3& scale )
{ 
	m_Scale = scale;
}
void CFbxModel::SetScale( const float& scale )
{ 
	m_Scale = { scale, scale, scale };
}

//////////////////////////////////////////////////////.
// アニメーション速度の設定.
//////////////////////////////////////////////////////.
void CFbxModel::SetAnimSpeed( const double& speed )
{
	if( m_pAc == nullptr ) return;
	m_pAc->SetAnimSpeed( speed );
}

//////////////////////////////////////////////////////.
// アニメーションの変更.
//////////////////////////////////////////////////////.
void CFbxModel::ChangeAnimation( int& index )
{
	if( m_pAc == nullptr ) return;
	m_pAc->ChangeAnimation( index );
}