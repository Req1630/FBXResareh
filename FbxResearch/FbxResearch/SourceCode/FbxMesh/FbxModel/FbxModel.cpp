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
// ���b�V���f�[�^�̎擾.
//-------------------------------------.
std::vector<FBXMeshData>& CFbxModel::GetMeshData()
{ 
	return m_MeshData;
}

//-------------------------------------.
// ���b�V���f�[�^�̃��T�C�Y.
//-------------------------------------.
void CFbxModel::ReSizeMeshData( const int& size )
{ 
	m_MeshData.clear();
	m_MeshData.resize(size);
}

//-------------------------------------.
// �e�N�X�`���̎擾.
//-------------------------------------.
std::unordered_map<std::string, ID3D11ShaderResourceView*>& 
CFbxModel::GetTextures()
{ 
	return m_Textures; 
}

//-------------------------------------.
// �A�j���[�V�����R���g���[���[�̎擾.
//-------------------------------------.
CFbxAnimationController* CFbxModel::GetPtrAC()
{ 
	return m_pAc; 
}

//-------------------------------------.
// �A�j���[�V�����f�[�^�̐ݒ�.
//-------------------------------------.
void CFbxModel::SetAnimationData( const std::vector<SAnimationData>& animDataList )
{
	m_pAc = new CFbxAnimationController;
	m_pAc->SetAnimDataList( animDataList );
}


//-------------------------------------.
// ���[���h�s����擾.
//-------------------------------------.
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

//-------------------------------------.
// �A�j���[�V�����R���g���[���[�̎擾.
//-------------------------------------.
CFbxAnimationController CFbxModel::GetAnimationController()
{
	if( m_pAc != nullptr ) return *m_pAc;
	return CFbxAnimationController();
}

//-------------------------------------.
// ���W�̐ݒ�.
//-------------------------------------.
void CFbxModel::SetPosition( const DirectX::XMFLOAT3& pos )
{ 
	m_Position = pos; 
}

//-------------------------------------.
// ��]�̐ݒ�.
//-------------------------------------.
void CFbxModel::SetRotation( const DirectX::XMFLOAT3& rot )
{ 
	m_Rotation = rot;
}

//-------------------------------------.
// �傫���̐ݒ�.
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
// �A�j���[�V�������x�̐ݒ�.
//-------------------------------------.
void CFbxModel::SetAnimSpeed( const double& speed )
{
	if( m_pAc == nullptr ) return;
	m_pAc->SetAnimSpeed( speed );
}

//-------------------------------------.
// �A�j���[�V�����̕ύX.
//-------------------------------------.
void CFbxModel::ChangeAnimation( int& index )
{
	if( m_pAc == nullptr ) return;
	m_pAc->ChangeAnimation( index );
}