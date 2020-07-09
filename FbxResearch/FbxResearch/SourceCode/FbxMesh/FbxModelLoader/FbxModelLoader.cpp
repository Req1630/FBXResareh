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
// �쐬.
//////////////////////////////////////////////////////////////////////.
HRESULT CFbxModelLoader::Create( ID3D11Device* pDevice )
{
	//---------------------------------------------.
	// �f�o�C�X�̎擾.
	//---------------------------------------------.
	if( pDevice == nullptr ){
		_ASSERT_EXPR( false, "�f�o�C�X�̎擾���s" );
		MessageBox( nullptr, "�f�o�C�X�̎擾���s", "Warning", MB_OK );
		return E_FAIL;
	}
	m_pDevice11 = pDevice;

	//---------------------------------------------.
	// �}�l�[�W���[�̍쐬.
	//---------------------------------------------.
	m_pFbxManager = FbxManager::Create();
	if( m_pFbxManager == nullptr ){
		_ASSERT_EXPR( false, "FbxManager�쐬���s" );
		MessageBox( nullptr, "FbxManager�쐬���s", "Warning", MB_OK );
		return E_FAIL;
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////.
// �j��.
//////////////////////////////////////////////////////////////////////.
void CFbxModelLoader::Destroy()
{
	SAFE_DESTROY( m_pFbxScene );
	SAFE_DESTROY( m_pFbxManager );

	m_pDevice11 = nullptr;
}

//////////////////////////////////////////////////////////////////////.
//			���f���̓ǂݍ���.
//////////////////////////////////////////////////////////////////////.
HRESULT CFbxModelLoader::LoadModel( CFbxModel* pModelData, const char* fileName )
{
	//---------------------------------------------.
	// �C���|�[�^�[�̍쐬.
	//---------------------------------------------.
	FbxImporter* pFbxImpoter = FbxImporter::Create( m_pFbxManager, "imp" );
	if( pFbxImpoter == nullptr ){
		_ASSERT_EXPR( false, "pFbxImpoter�쐬���s" );
		MessageBox( nullptr, "pFbxImpoter�쐬���s", "Warning", MB_OK );
		return E_FAIL;
	}

	//---------------------------------------------.
	// FBX�t�@�C���̓ǂݍ���.
	//---------------------------------------------.
	// �t�@�C�����̐ݒ�.
	FbxString fbxFileName( fileName );
	if( pFbxImpoter->Initialize( fbxFileName.Buffer() ) == false ){
		_ASSERT_EXPR( false, "Fbx�t�@�C���̓ǂݍ���" );
		MessageBox( nullptr, "Fbx�t�@�C���̓ǂݍ���", "Warning", MB_OK );
		return E_FAIL;
	}

	//---------------------------------------------.
	// �V�[���I�u�W�F�N�g�̍쐬.
	//---------------------------------------------.
	m_pFbxScene = FbxScene::Create( m_pFbxManager, "fbxScene" );
	if( m_pFbxScene == nullptr ){
		_ASSERT_EXPR( false, "FbxScene�̍쐬���s" );
		MessageBox( nullptr, "FbxScene�̍쐬���s", "Warning", MB_OK );
		return E_FAIL;
	}

	//---------------------------------------------.
	// �C���|�[�^�[�ƃV�[���I�u�W�F�N�g�̊֘A�t��.
	//---------------------------------------------.
	if( pFbxImpoter->Import( m_pFbxScene ) == false ){
		SAFE_DESTROY( m_pFbxManager );
		SAFE_DESTROY( m_pFbxScene );
		SAFE_DESTROY( pFbxImpoter );
		_ASSERT_EXPR( false, "FbxScene��FbxImpoter�̊֘A�t�����s" );
		MessageBox( nullptr, "FbxScene��FbxImpoter�̊֘A�t�����s", "Warning", MB_OK );
		return E_FAIL;
	}
	SAFE_DESTROY( pFbxImpoter );


	//---------------------------------------------.
	// �|���S���̐ݒ�.
	//---------------------------------------------.
	bool convertReslut = false;
	FbxGeometryConverter geometryConverter( m_pFbxManager );
	// �|���S�����O�p�`�ɂ���.
	// ���p�`�|���S��������΍��Ȃ����̂Ŏ��Ԃ�������.
	convertReslut = geometryConverter.Triangulate( m_pFbxScene, true );
	if( convertReslut == false ){
		_ASSERT_EXPR( false, "�|���S���̎O�p�����s" );
		MessageBox( nullptr, "�|���S���̎O�p�����s", "Warning", MB_OK );
		return E_FAIL;
	}
	geometryConverter.RemoveBadPolygonsFromMeshes( m_pFbxScene );
	// ���b�V�����}�e���A�����Ƃɕ�������.
	convertReslut = geometryConverter.SplitMeshesPerMaterial( m_pFbxScene, true );
	if( convertReslut == false ){
		_ASSERT_EXPR( false, "�}�e���A�����Ƃ̃��b�V���������s" );
		MessageBox( nullptr, "�}�e���A�����Ƃ̃��b�V���������s", "Warning", MB_OK );
		return E_FAIL;
	}

	//---------------------------------------------.
	// FbxSkeleton�̎擾.
	//---------------------------------------------.
	// FbxSkeleton�̐����擾.
	int skeletonNum = m_pFbxScene->GetSrcObjectCount<FbxSkeleton>();
	int skeletonNo = 0;
	m_Skeletons.clear();
	m_Skeletons.resize( skeletonNum );
	for( auto& s : m_Skeletons ){
		// FbxSkeleton�̎擾.
		s = m_pFbxScene->GetSrcObject<FbxSkeleton>(skeletonNo);
		skeletonNo++;
	}

	//---------------------------------------------.
	// FbxMesh�̎擾.
	//---------------------------------------------.
	// FbxMesh�̐����擾.
	int meshNum = m_pFbxScene->GetSrcObjectCount<FbxMesh>();
	int meshNo = 0;
	pModelData->ReSizeMeshData( meshNum );
	for( auto& m : pModelData->GetMeshData() ){
		// ���b�V���f�[�^�̎擾.
		FbxMesh* pMesh = m_pFbxScene->GetSrcObject<FbxMesh>(meshNo);
		// �}�e���A���̎擾.
		GetMaterial( pMesh, m, fileName, pModelData->GetTextures() );
		// ���b�V���f�[�^�̓ǂݍ���.
		LoadMesh( pMesh, m );
		// ���_�o�b�t�@�̍쐬.
		if( FAILED( CreateVertexBuffers( m ) )) return E_FAIL;
		// �C���f�b�N�X�o�b�t�@�̍쐬.
		if( FAILED( CreateIndexBuffers( m ) )) return E_FAIL;
		meshNo++;
	}

	//---------------------------------------------.
	//	�A�j���[�V�����̓ǂݍ���.
	//---------------------------------------------.
	CFbxAnimationLoader animLoader;				// �A�j���[�V�����ǂݍ��݃N���X.
	std::vector<SAnimationData>	animDataList;	// �A�j���[�V�����f�[�^.
	animLoader.LoadAnimationData( m_pFbxScene, m_MeshClusterData, m_Skeletons, &animDataList );
	if( animDataList.empty() == false ){
		// ��Őݒ肵���A�j���[�V�����f�[�^�������.
		// �A�j���[�V�����R���g���[���[���쐬����.
		// �A�j���[�V�����f�[�^��ǉ�.
		pModelData->SetAnimationData( animDataList );
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////.
// �}�e���A���擾.
//////////////////////////////////////////////////////////////////////.
void CFbxModelLoader::GetMaterial( FbxMesh* pMesh, FBXMeshData& mesh, const char* fileName, std::unordered_map<std::string, ID3D11ShaderResourceView*>& textures )
{
	// �}�e���A����񂪖����̂ŏI��.
	if( pMesh->GetElementMaterialCount() == 0 ) return;

	// Mesh���̃}�e���A�������擾.
	FbxLayerElementMaterial* material = pMesh->GetElementMaterial(0);
	int index = material->GetIndexArray().GetAt(0);

	// FBX�̃}�e���A�����擾.
	FbxSurfaceMaterial* pMat = pMesh->GetNode()->GetSrcObject<FbxSurfaceMaterial>(index);

	// Lambert��Phong��.
	if( pMat->GetClassId().Is( FbxSurfaceLambert::ClassId )){
		// Ambient�̏����擾.
		FbxProperty ambient = pMat->FindProperty( FbxSurfaceMaterial::sAmbient );
		// diffuse�̏����擾.
		FbxProperty diffuse = pMat->FindProperty( FbxSurfaceMaterial::sDiffuse );
		// Specular�̏����擾.
		FbxProperty specular = pMat->FindProperty( FbxSurfaceMaterial::sSpecular );
		
		// �A���r�G���g�擾.
		if( ambient.IsValid() == true ){
			mesh.Material.Ambient.x = (float)ambient.Get<FbxDouble4>()[0];
			mesh.Material.Ambient.y = (float)ambient.Get<FbxDouble4>()[1];
			mesh.Material.Ambient.z = (float)ambient.Get<FbxDouble4>()[2];
			mesh.Material.Ambient.w = (float)ambient.Get<FbxDouble4>()[3];
		} else {
			mesh.Material.Ambient = { 1.0f, 1.0f, 1.0f, 1.0f };
		}

		// �f�B�t�[�Y�擾.
		if( diffuse.IsValid() == true ){
			mesh.Material.Diffuse.x = (float)diffuse.Get<FbxDouble4>()[0];
			mesh.Material.Diffuse.y = (float)diffuse.Get<FbxDouble4>()[1];
			mesh.Material.Diffuse.z = (float)diffuse.Get<FbxDouble4>()[2];
			mesh.Material.Diffuse.w = (float)diffuse.Get<FbxDouble4>()[3];
		} else {
			mesh.Material.Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
		}

		// �X�y�L�����擾.
		if( specular.IsValid() == true ){
			mesh.Material.Specular.x = (float)specular.Get<FbxDouble4>()[0];
			mesh.Material.Specular.y = (float)specular.Get<FbxDouble4>()[1];
			mesh.Material.Specular.z = (float)specular.Get<FbxDouble4>()[2];
			mesh.Material.Specular.w = (float)specular.Get<FbxDouble4>()[3];
		} else {
			mesh.Material.Specular = { 1.0f, 1.0f, 1.0f, 1.0f };
		}
		// �}�e���A�����擾.
		mesh.Material.Name = pMat->GetName();

		//---------------------------------------------.
		// �e�N�X�`���쐬.
		//---------------------------------------------.
		FbxFileTexture* texture = nullptr;

		// FbxFileTexture == �V���O���e�N�X�`��.
		int textureCount = diffuse.GetSrcObjectCount<FbxFileTexture>();
		if( textureCount > 0 ){
			texture = diffuse.GetSrcObject<FbxFileTexture>(0);
			mesh.UVSetCount = textureCount;
		} else {
			// �}���`�e�N�X�`��(���C���[�e�N�X�`��)�̉\��������̂Ō���.
			int layerCount = diffuse.GetSrcObjectCount<FbxLayeredTexture>();
			if( layerCount > 0 ){
				/******************************
				�}���`�e�N�X�`���̎����͌�.
				*******************************/
				texture = diffuse.GetSrcObject<FbxFileTexture>(0);
				mesh.UVSetCount = layerCount;
			}
		}
		if( texture == nullptr ) return;
		// �e�N�X�`���̓ǂݍ���.
		if( FAILED( LoadTexture( texture, fileName, pMat->GetName(), textures ))) return;
	}
}

//////////////////////////////////////////////////////////////////////.
// �e�N�X�`���̓ǂݍ���.
//////////////////////////////////////////////////////////////////////.
HRESULT CFbxModelLoader::LoadTexture( FbxFileTexture* texture, const char* fileName, const char* keyName, std::unordered_map<std::string, ID3D11ShaderResourceView*>& textures )
{
	//---------------------------------------------.
	// �t�@�C�������擾.
	//---------------------------------------------.
	std::string textureName = texture->GetRelativeFileName();
	std::string path = fileName;

	// ���΃p�X����̏ꍇ�A��΃p�X���󂯎��.
	if( textureName.length() == 0 ) textureName = texture->GetFileName();

	// ���f���̃p�X���g�p���A���f���t�@�C�������폜���Ƃ�.
	int pos = path.find_last_of('\\')+1;
	path = path.substr( 0, pos );

	// �e�N�X�`���̃t�@�C�������擾����.
	int texNamePos = textureName.find_last_of('\\')+1;
	textureName = textureName.substr( texNamePos, textureName.length() );
	// �t�@�C���p�X�ƁA�e�N�X�`���������킹��.
	path += textureName;

	// wchar_t �ɕϊ�.
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> cv;
	std::wstring wstr_file_name = cv.from_bytes(path);

	//---------------------------------------------.
	// �e�N�X�`�����\�[�X�쐬.
	//---------------------------------------------.
	if( FAILED( 
		DirectX::CreateWICTextureFromFile(
		m_pDevice11,
		wstr_file_name.c_str(),
		nullptr,
		&textures[keyName] ))){
		_ASSERT_EXPR( false, "�e�N�X�`���擾���s" );
		MessageBox( nullptr, "�e�N�X�`���擾���s", "Warning", MB_OK );
		return E_FAIL;
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////.
// ���b�V���̓ǂݍ���.
//////////////////////////////////////////////////////////////////////.
void CFbxModelLoader::LoadMesh( FbxMesh* pMesh, FBXMeshData& meshData )
{
	// �C���f�b�N�X���̎擾.
	LoadIndices( pMesh, meshData );

	// ���_���̎擾.
	int ctrlCount = pMesh->GetControlPointsCount();

	// �E�F�C�g.
	std::vector<std::vector<float>> weights( ctrlCount );
	// �{�[���C���f�b�N�X.
	std::vector<std::vector<int>> bones( ctrlCount );
	// �X�L�����̎擾.
	LoadSkin( pMesh, meshData, weights, bones );

	// �|���S�����̎擾.
	int polygonCount = pMesh->GetPolygonCount();
	// ���_�̃J�E���^�[.
	int vertexCounter = 0;

	// �|���S���̐���.
	for( int i = 0; i < polygonCount; i++ ){
		// �|���S���̃T�C�Y.
		int polygonSize = pMesh->GetPolygonSize(i);
		if( polygonSize != 3 ){
			// �O�p�|���S������Ȃ�.
			_ASSERT_EXPR( false, "���b�V�����O�p����Ȃ��ł�" );
			MessageBox( nullptr, "���b�V�����O�p����Ȃ��ł�", "Warning", MB_OK );
			return;
		}

		// ��̃|���S���̒��_3��.
		for( int j = 0; j < polygonSize; j++ ){
			// ���_�̃C���f�b�N�X���擾.
			int ctrlPointIndex = pMesh->GetPolygonVertex( i, j );
			// ���_����p��.
			VERTEX vertex = {};

			// ���_���W���擾.
			LoadVertices( pMesh, vertex, ctrlPointIndex );
			// �@���̎擾.
			LoadNormals( pMesh, vertex, ctrlPointIndex, vertexCounter );
			// ���_�J���[�̎擾.
			LoadColors( pMesh, vertex, ctrlPointIndex, vertexCounter );
			// UV���W�̎擾.
			for( int k = 0; k < 2; k++ ){
				LoadUV( pMesh, vertex, ctrlPointIndex, pMesh->GetTextureUVIndex( i, j ), k );
			}
			// �E�F�C�g�ƃ{�[���C���f�b�N�X�̐ݒ�.
			SetBoneWeight( vertex, weights[ctrlPointIndex], bones[ctrlPointIndex] );

			// ���_����ǉ�.
			meshData.Vertices.emplace_back( vertex );

			// ���_�������Z.
			vertexCounter++;
		}
	}
}

//////////////////////////////////////////////////////////////////////.
// �C���f�b�N�X���ǂݍ���.
//////////////////////////////////////////////////////////////////////.
void CFbxModelLoader::LoadIndices( FbxMesh* pMesh, FBXMeshData& meshData )
{
	// �|���S�����̎擾.
	int polygonCount = pMesh->GetPolygonCount();
	meshData.PolygonVertexCount = pMesh->GetPolygonVertexCount();
	// �|���S���̐������A�ԂƂ��ĕۑ�.
	for( int i = 0; i < polygonCount; i++ ){
		meshData.Indices.emplace_back( i * 3 + 2 );
		meshData.Indices.emplace_back( i * 3 + 1 );
		meshData.Indices.emplace_back( i * 3 + 0 );
	}
}

//////////////////////////////////////////////////////////////////////.
// ���_���ǂݍ���.
//////////////////////////////////////////////////////////////////////.
void CFbxModelLoader::LoadVertices( FbxMesh* pMesh, VERTEX& vertex, int ctrlPointIndex )
{
	vertex.Pos.x = (float)pMesh->GetControlPoints()[ctrlPointIndex][0];
	vertex.Pos.y = (float)pMesh->GetControlPoints()[ctrlPointIndex][1];
	vertex.Pos.z = (float)pMesh->GetControlPoints()[ctrlPointIndex][2];
}

//////////////////////////////////////////////////////////////////////.
// �@�����ǂݍ���.
//////////////////////////////////////////////////////////////////////.
void CFbxModelLoader::LoadNormals( FbxMesh* pMesh, VERTEX& vertex, int ctrlPointIndex, int vertexCounter )
{
	if( pMesh->GetElementNormalCount() < 1 ) return;

	/************************************************
	* DirectX�̏ꍇx�l�𔽓]������K�v����.
	**/

	// �@���̎擾.
	FbxGeometryElementNormal* pNormal = pMesh->GetElementNormal(0);

	// �}�b�s���O���[�h�͖@�����ǂ̂悤�Ƀ��f���\�ʂɒ�`����Ă��邩��\��.
	switch( pNormal->GetMappingMode() )
	{
	// eByControlPoint : ���_�ɑ΂��Ă̖@��.
	case FbxGeometryElement::eByControlPoint:
	{
		// �C���f�b�N�X�̎擾.
		int index = pNormal->GetIndexArray().GetAt(ctrlPointIndex);

		// ���t�@�����X���[�h�͖@�����ǂ̂悤�ɔz��Ɋi�[����Ă��邩��\��.
		switch( pNormal->GetReferenceMode() )
		{
		// eDIRECT : ���ԂɊi�[���Ă���.
		case FbxGeometryElement::eDirect:
			vertex.Normal.x = -(float)pNormal->GetDirectArray().GetAt(ctrlPointIndex)[0];
			vertex.Normal.y =  (float)pNormal->GetDirectArray().GetAt(ctrlPointIndex)[1];
			vertex.Normal.z =  (float)pNormal->GetDirectArray().GetAt(ctrlPointIndex)[2];
			break;

		// eIndexToDirect : �C���f�b�N�X�ɑΉ���������.
		case FbxGeometryElement::eIndexToDirect:
			vertex.Normal.x = -(float)pNormal->GetDirectArray().GetAt(index)[0];
			vertex.Normal.y =  (float)pNormal->GetDirectArray().GetAt(index)[1];
			vertex.Normal.z =  (float)pNormal->GetDirectArray().GetAt(index)[2];
			break;
		}
	}
	// eByPolygonVertex : �|���S���ɑ΂��Ă̖@��.
	case FbxGeometryElement::eByPolygonVertex:
	{
		// �C���f�b�N�X�̎擾.
		int index = pNormal->GetIndexArray().GetAt(vertexCounter);

		// ���t�@�����X���[�h�͖@�����ǂ̂悤�ɔz��Ɋi�[����Ă��邩��\��.
		switch( pNormal->GetReferenceMode() )
		{
		// eDIRECT : ���ԂɊi�[���Ă���.
		case FbxGeometryElement::eDirect:
			vertex.Normal.x = -(float)pNormal->GetDirectArray().GetAt(vertexCounter)[0];
			vertex.Normal.y =  (float)pNormal->GetDirectArray().GetAt(vertexCounter)[1];
			vertex.Normal.z =  (float)pNormal->GetDirectArray().GetAt(vertexCounter)[2];
			break;

		// eIndexToDirect : �C���f�b�N�X�ɑΉ���������.
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
// ���_�J���[�ǂݍ���.
//////////////////////////////////////////////////////////////////////.
void CFbxModelLoader::LoadColors( FbxMesh* pMesh, VERTEX& vertex, int ctrlPointIndex, int vertexCounter )
{
	// ���_�J���[�f�[�^�̐����m�F.
	if( pMesh->GetElementVertexColorCount() == 0 ) return;

	FbxGeometryElementVertexColor* pVertexColor = pMesh->GetElementVertexColor(0);

	// �}�b�s���O���[�h�͒��_�J���[���ǂ̂悤�Ƀ��f���\�ʂɒ�`����Ă��邩��\��.
	switch( pVertexColor->GetMappingMode() )
	{
	// eByControlPoint : ���_�ɑ΂��Ă̖@��.
	case FbxGeometryElement::eByControlPoint:
	{
		// �C���f�b�N�X�̎擾.
		int index = pVertexColor->GetIndexArray().GetAt(ctrlPointIndex);

		// ���t�@�����X���[�h�͖@�����ǂ̂悤�ɔz��Ɋi�[����Ă��邩��\��.
		switch( pVertexColor->GetReferenceMode() )
		{
		// eDIRECT : ���ԂɊi�[���Ă���.
		case FbxGeometryElement::eDirect:
			vertex.Color.x = (float)pVertexColor->GetDirectArray().GetAt(ctrlPointIndex)[0];
			vertex.Color.y = (float)pVertexColor->GetDirectArray().GetAt(ctrlPointIndex)[1];
			vertex.Color.z = (float)pVertexColor->GetDirectArray().GetAt(ctrlPointIndex)[2];
			vertex.Color.w = (float)pVertexColor->GetDirectArray().GetAt(ctrlPointIndex)[3];
			break;

		// eIndexToDirect : �C���f�b�N�X�ɑΉ���������.
		case FbxGeometryElement::eIndexToDirect:
			vertex.Color.x = (float)pVertexColor->GetDirectArray().GetAt(index)[0];
			vertex.Color.y = (float)pVertexColor->GetDirectArray().GetAt(index)[1];
			vertex.Color.z = (float)pVertexColor->GetDirectArray().GetAt(index)[2];
			vertex.Color.w = (float)pVertexColor->GetDirectArray().GetAt(index)[3];
			break;
		}
	}
	// eByPolygonVertex : �|���S���ɑ΂��Ă̖@��.
	case FbxGeometryElement::eByPolygonVertex:
	{
		// �C���f�b�N�X�̎擾.
		int index = pVertexColor->GetIndexArray().GetAt(vertexCounter);

		// ���t�@�����X���[�h�͖@�����ǂ̂悤�ɔz��Ɋi�[����Ă��邩��\��.
		switch( pVertexColor->GetReferenceMode() )
		{
		// eDIRECT : ���ԂɊi�[���Ă���.
		case FbxGeometryElement::eDirect:
			vertex.Color.x = (float)pVertexColor->GetDirectArray().GetAt(vertexCounter)[0];
			vertex.Color.y = (float)pVertexColor->GetDirectArray().GetAt(vertexCounter)[1];
			vertex.Color.z = (float)pVertexColor->GetDirectArray().GetAt(vertexCounter)[2];
			vertex.Color.w = (float)pVertexColor->GetDirectArray().GetAt(vertexCounter)[3];
			break;

		// eIndexToDirect : �C���f�b�N�X�ɑΉ���������.
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
// UV���ǂݍ���.
//////////////////////////////////////////////////////////////////////.
void CFbxModelLoader::LoadUV( FbxMesh* pMesh, VERTEX& vertex, int ctrlPointIndex, int texUVIndex, int uvLayer )
{
	if( uvLayer >= 2 || pMesh->GetElementUVCount() <= uvLayer ) return;

	/************************************************
	* DirectX�̏ꍇUV���W��V�l�𔽓]������K�v����.
	**/

	FbxGeometryElementUV* pVertexUV = pMesh->GetElementUV( uvLayer );
	// �}�b�s���O���[�h��UV���ǂ̂悤�Ƀ��f���\�ʂɒ�`����Ă��邩��\��.
	switch( pVertexUV->GetMappingMode() )
	{
	// eByControlPoint : ���_�ɑ΂��Ă̖@��.
	case FbxGeometryElement::eByControlPoint:
	{
		// �C���f�b�N�X�̎擾.
		int index = pVertexUV->GetIndexArray().GetAt(ctrlPointIndex);

		// ���t�@�����X���[�h�͖@�����ǂ̂悤�ɔz��Ɋi�[����Ă��邩��\��.
		switch( pVertexUV->GetReferenceMode() )
		{
			// eDIRECT : ���ԂɊi�[���Ă���.
		case FbxGeometryElement::eDirect:
			vertex.UV.x =  (float)pVertexUV->GetDirectArray().GetAt(ctrlPointIndex)[0];
			vertex.UV.y = -(float)pVertexUV->GetDirectArray().GetAt(ctrlPointIndex)[1];
			break;

			// eIndexToDirect : �C���f�b�N�X�ɑΉ���������.
		case FbxGeometryElement::eIndexToDirect:
			vertex.UV.x =  (float)pVertexUV->GetDirectArray().GetAt(index)[0];
			vertex.UV.y = -(float)pVertexUV->GetDirectArray().GetAt(index)[1];
			break;
		}
	}
	// eByPolygonVertex : �|���S���ɑ΂��Ă̖@��.
	case FbxGeometryElement::eByPolygonVertex:
	{
		// ���t�@�����X���[�h�͖@�����ǂ̂悤�ɔz��Ɋi�[����Ă��邩��\��.
		switch( pVertexUV->GetReferenceMode() )
		{
			// eDIRECT : ���ԂɊi�[���Ă���.
		case FbxGeometryElement::eDirect:
			// eIndexToDirect : �C���f�b�N�X�ɑΉ���������.
		case FbxGeometryElement::eIndexToDirect:
			vertex.UV.x =  (float)pVertexUV->GetDirectArray().GetAt(texUVIndex)[0];
			vertex.UV.y = -(float)pVertexUV->GetDirectArray().GetAt(texUVIndex)[1];
			break;
		}
	}
	}
}

//////////////////////////////////////////////////////////////////////.
// �X�L�����ǂݍ���.
//////////////////////////////////////////////////////////////////////.
void CFbxModelLoader::LoadSkin( FbxMesh* pMesh, FBXMeshData& meshData, std::vector<std::vector<float>>& weights, std::vector<std::vector<int>>& bones )
{
	// �_�E���L���X�g���ăX�L�������擾.
	FbxSkin* pSkin = (FbxSkin*)pMesh->GetDeformer( 0, FbxDeformer::eSkin );
	// �X�L����񂪖�����ΏI��.
	if( pSkin == nullptr ) return;

	m_MeshClusterData.emplace_back();

	// ��Cluster == Bone.
	// �{�[���̐�.
	int boneCount = pSkin->GetClusterCount();
	for( int boneIndex = 0; boneIndex < boneCount; boneIndex++ ){
		// �{�[�����擾.
		FbxCluster* pCluster = pSkin->GetCluster( boneIndex );
		FbxNode* pNode = pCluster->GetLink();

		m_MeshClusterData.back().ClusterName.emplace_back( pNode->GetName() );

		FbxAMatrix bindPoseMatrix;	// �����{�[���s��.
		FbxAMatrix newbindpose;		// �����|�[�Y.

		pCluster->GetTransformLinkMatrix( bindPoseMatrix );
		pCluster->GetTransformMatrix( newbindpose );

		newbindpose = bindPoseMatrix.Inverse() * newbindpose;
		// �����{�[�����W�̐ݒ�.
		meshData.Skin.InitBonePositions.emplace_back( newbindpose );

		// �{�[���C���f�b�N�X�ƃE�F�C�g�̐ݒ�.
		int*	boneVertexIndices = pCluster->GetControlPointIndices();
		double* boneVertexWeights = pCluster->GetControlPointWeights();
		// �{�[���ɉe������|���S���̐�.
		int	numBoneVertexIndices = pCluster->GetControlPointIndicesCount();
		for( int boneVertexIndex = 0; boneVertexIndex < numBoneVertexIndices; boneVertexIndex++ ){
			int cpIndex			= boneVertexIndices[boneVertexIndex];
			float boneWeight	= (float)boneVertexWeights[boneVertexIndex];
			weights[cpIndex].emplace_back( boneWeight );
			bones[cpIndex].emplace_back( boneIndex );
		}
	}
	// �E�F�C�g���d�����Ƀ\�[�g.
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
// �E�F�C�g�ƃ{�[���̐ݒ�.
//////////////////////////////////////////////////////////////////////.
void CFbxModelLoader::SetBoneWeight( VERTEX& vertex, const std::vector<float>& weight, const std::vector<int>& bone )
{
	// �E�F�C�g�� 0����d�����ɂȂ��Ă���͂��Ȃ̂�.
	// �z���3�ڂ������Ă���(�ő�E�F�C�g�� : 4��).
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
// ���_�o�b�t�@�쐬.
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

	// ���_�o�b�t�@�̍쐬.
	if( FAILED( m_pDevice11->CreateBuffer( &bd, &data, &meshData.pVertexBuffer ))){
		_ASSERT_EXPR( false, "���_�o�b�t�@�̍쐬���s" );
		MessageBox( nullptr, "���_�o�b�t�@�̍쐬���s", "Warning", MB_OK );
		return E_FAIL;
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////.
// �C���f�b�N�X�o�b�t�@�쐬.
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

	// �C���f�b�N�X�o�b�t�@�̍쐬.
	if( FAILED( m_pDevice11->CreateBuffer( &bd, &data, &meshData.pIndexBuffer ))){
		_ASSERT_EXPR( false, "�C���f�b�N�X�o�b�t�@�̍쐬���s" );
		MessageBox( nullptr, "�C���f�b�N�X�o�b�t�@�̍쐬���s", "Warning", MB_OK );
		return E_FAIL;
	}
	return S_OK;
}