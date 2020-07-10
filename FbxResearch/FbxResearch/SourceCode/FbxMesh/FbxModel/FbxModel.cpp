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

//////////////////////////////////////////////////////.
// ���b�V���f�[�^�̎擾.
//////////////////////////////////////////////////////.
std::vector<FBXMeshData>& CFbxModel::GetMeshData()
{ 
	return m_MeshData;
}

//////////////////////////////////////////////////////.
// ���b�V���f�[�^�̃��T�C�Y.
//////////////////////////////////////////////////////.
void CFbxModel::ReSizeMeshData( const int& size )
{ 
	m_MeshData.clear();
	m_MeshData.resize(size);
}

//////////////////////////////////////////////////////.
// �e�N�X�`���̎擾.
//////////////////////////////////////////////////////.
std::unordered_map<std::string, ID3D11ShaderResourceView*>& 
CFbxModel::GetTextures()
{ 
	return m_Textures; 
}

//////////////////////////////////////////////////////.
// �A�j���[�V�����R���g���[���[�̎擾.
//////////////////////////////////////////////////////.
CFbxAnimationController* CFbxModel::GetPtrAC()
{ 
	return m_pAc; 
}

//////////////////////////////////////////////////////.
// �A�j���[�V�����f�[�^�̐ݒ�.
//////////////////////////////////////////////////////.
void CFbxModel::SetAnimationData( const SAnimationDataList& animDataList )
{
	m_pAc = new CFbxAnimationController;
	m_pAc->SetAnimDataList( animDataList );
}

//////////////////////////////////////////////////////.
// �{�[�����f�[�^�̍쐬.
//////////////////////////////////////////////////////.
void CFbxModel::BoneNameDataCreate()
{
	int meshNo = 0;
	int boneNo = 0;
	for( auto& m : m_MeshData ){
		boneNo = 0;
		for( auto& s : m.Skin.BoneName ){
			m_BoneNumberList.try_emplace( s, meshNo, boneNo );
			boneNo++;
		}
		meshNo++;
	}
}

//////////////////////////////////////////////////////.
// ���[���h�s����擾.
//////////////////////////////////////////////////////.
DirectX::XMMATRIX CFbxModel::GetWorldMatrix()
{
	DirectX::XMMATRIX mTarn, mRot, mScale;
	// ���s�ړ��s��.
	mTarn = DirectX::XMMatrixTranslation(
		m_Position.x, m_Position.y, m_Position.z );
	// ��]�s��.
	mRot = DirectX::XMMatrixRotationRollPitchYaw(
		m_Rotation.x, m_Rotation.y, m_Rotation.z );
	// �g��k���s��.
	mScale = DirectX::XMMatrixScaling(
		m_Scale.x, m_Scale.y, m_Scale.z );

	// ���[���h�s��쐬.
	// �g�k*��]*�ړ�.
	return mScale * mRot * mTarn;
}

//////////////////////////////////////////////////////.
// �A�j���[�V�����R���g���[���[�̎擾.
//////////////////////////////////////////////////////.
CFbxAnimationController CFbxModel::GetAnimationController()
{
	if( m_pAc != nullptr ) return *m_pAc;
	return CFbxAnimationController();
}

//////////////////////////////////////////////////////.
// �{�[�����W�̎擾.
//////////////////////////////////////////////////////.
DirectX::XMFLOAT3 CFbxModel::GetBonePosition( const char* boneName )
{
	int meshNo = m_BoneNumberList[boneName].first;
	int boneNo = m_BoneNumberList[boneName].second;

	FbxMatrix frameMatrix;
	if( m_pAc->GetFrameLinkMatrix( meshNo, boneNo, &frameMatrix ) == false ){
		_ASSERT_EXPR( false, "���f���ƃA�j���[�V�����̃{�[���̐��������܂���" );
		MessageBox( nullptr, "���f���ƃA�j���[�V�����̃{�[���̐��������܂���", "Warning", MB_OK );
	}
	DirectX::XMMATRIX outMat = FbxMatrixConvertDXMMatrix( frameMatrix );
	DirectX::XMFLOAT4X4 float4x4;
	DirectX::XMStoreFloat4x4( &float4x4, outMat );

	DirectX::XMMATRIX world;
	DirectX::XMMATRIX mTarn, mRot, mScale;

	// �g��k���s��.
	mScale = DirectX::XMMatrixScaling(
		m_Scale.x, m_Scale.y, m_Scale.z );
	// ��]�s��.
	mRot = DirectX::XMMatrixRotationRollPitchYaw(
		m_Rotation.x, m_Rotation.y, m_Rotation.z );
	// ���s�ړ��s��.
	mTarn = DirectX::XMMatrixTranslation(
		float4x4._41, float4x4._42, float4x4._43 );
	world = mTarn * mScale * mRot;

	// ���[���h�s��쐬.
	DirectX::XMFLOAT4X4 world4x4;
	DirectX::XMStoreFloat4x4( &world4x4, world );

	DirectX::XMFLOAT3 pos;
	pos.x = world4x4._41 + m_Position.x;
	pos.y = world4x4._42 + m_Position.y;
	pos.z = world4x4._43 + m_Position.z;

	return pos;
}

//////////////////////////////////////////////////////.
// ���W�̐ݒ�.
//////////////////////////////////////////////////////.
void CFbxModel::SetPosition( const DirectX::XMFLOAT3& pos )
{ 
	m_Position = pos; 
}

//////////////////////////////////////////////////////.
// ��]�̐ݒ�.
//////////////////////////////////////////////////////.
void CFbxModel::SetRotation( const DirectX::XMFLOAT3& rot )
{ 
	m_Rotation = rot;
}

//////////////////////////////////////////////////////.
// �傫���̐ݒ�.
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
// �A�j���[�V�������x�̐ݒ�.
//////////////////////////////////////////////////////.
void CFbxModel::SetAnimSpeed( const double& speed )
{
	if( m_pAc == nullptr ) return;
	m_pAc->SetAnimSpeed( speed );
}

//////////////////////////////////////////////////////.
// �A�j���[�V�����̕ύX.
//////////////////////////////////////////////////////.
void CFbxModel::ChangeAnimation( int& index )
{
	if( m_pAc == nullptr ) return;
	m_pAc->ChangeAnimation( index );
}