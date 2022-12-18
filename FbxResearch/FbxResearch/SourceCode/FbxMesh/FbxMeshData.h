#pragma once
#include "FbxInclude/FbxInclude.h"

#include <vector>

//======================================.
//				���_���.
//======================================.
struct VERTEX
{
	DirectX::XMFLOAT3	Pos;		// ���_���W.
	DirectX::XMFLOAT3	Normal;		// �@��.
	DirectX::XMFLOAT4	Color;		// ���_�J���[.
	DirectX::XMFLOAT2	UV;			// UV���W.
	DirectX::XMUINT4	BoneIndex;	// �{�[���ԍ�.
	DirectX::XMFLOAT4	BoneWeight;	// �{�[���̏d��.

	VERTEX()
		: Pos			( 0.0f, 0.0f, 0.0f )
		, Normal		( 0.0f, 0.0f, 0.0f )
		, Color			( 0.0f, 0.0f, 0.0f, 0.0f )
		, UV			( 0.0f, 0.0f )
		, BoneIndex		( 0, 0, 0, 0 )
		, BoneWeight	( 0.0f, 0.0f, 0.0f, 0.0f )
	{}
};
//======================================.
//			�}�e���A�����.
//======================================.
struct MATERIAL
{
	DirectX::XMFLOAT4	Ambient;	// �A���r�G���g.
	DirectX::XMFLOAT4	Diffuse;	// �f�B�q���[�Y.
	DirectX::XMFLOAT4	Specular;	// �X�y�L����.
	std::string			Name;		// �}�e���A���̖��O.
};
//======================================.
//			�X�L���f�[�^.
//======================================.
struct SkinData
{
	bool HasSkins;
	std::vector<FbxMatrix> InitBonePositions;	// �����{�[�����W.
	std::vector<std::string> BoneName;			// �{�[�����̎擾.
};
//======================================.
//			���b�V���f�[�^.
//======================================.
struct FBXMeshData
{
	MATERIAL			Material;			// �}�e���A��.
	std::vector<VERTEX>	Vertices;			// ���_���.
	std::vector<UINT>	Indices;			// ���_�C���f�b�N�X.

	ID3D11Buffer*		pVertexBuffer;		// ���_�o�b�t�@���X�g.
	ID3D11Buffer*		pIndexBuffer;		// �C���f�b�N�X�o�b�t�@���X�g.

	SkinData			Skin;				// �X�L�����.

	int					PolygonVertexCount;	// �|���S�����_�C���f�b�N�X��.

	FBXMeshData()
		: Material				()
		, Vertices				()
		, Indices				()
		, pVertexBuffer			( nullptr )
		, pIndexBuffer			( nullptr )
		, PolygonVertexCount	( 0 )
	{}
	// �������.
	void Release()
	{
		SAFE_RELEASE( pVertexBuffer );
		SAFE_RELEASE( pIndexBuffer );
	}
};