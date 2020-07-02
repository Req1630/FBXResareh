#include "FbxModelLoader.h"
#include "..\FbxRenderer\FbxRenderer.h"
#include "..\FbxModel\FbxModel.h"

CFbxModelLoader::CFbxModelLoader()
{
}

CFbxModelLoader::~CFbxModelLoader()
{
}

// �쐬.
HRESULT CFbxModelLoader::Create( ID3D11DeviceContext* pContext11 )
{
	if( pContext11 == nullptr ) return E_FAIL;
	// �R���e�L�X�g�̎擾.
	m_pContext11 = pContext11;
	// �f�o�C�X�̎擾.
	m_pContext11->GetDevice( &m_pDevice11 );
	if( m_pDevice11 == nullptr ) return E_FAIL;

	//------------------------------.
	// �}�l�[�W���[�̍쐬.
	//------------------------------.
	m_pFbxManager = FbxManager::Create();
	if( m_pFbxManager == nullptr ){
		ERROR_MESSAGE( "FbxManager Create Failure." );
		return E_FAIL;
	}

	return S_OK;
}

// �j��.
void CFbxModelLoader::Destroy()
{
	SAFE_DESTROY( m_pFbxScene );
	SAFE_DESTROY( m_pFbxManager );

	m_pDevice11 = nullptr;
	m_pContext11 = nullptr;
}

//-----------------------------------------.
//			���f���̓ǂݍ���.
//-----------------------------------------.
HRESULT CFbxModelLoader::LoadModel( CFbxModel* pModelData, const char* fileName )
{
	//------------------------------.
	// �C���|�[�^�[�̍쐬.
	//------------------------------.
	FbxImporter* pFbxImpoter = FbxImporter::Create( m_pFbxManager, "imp" );
	if( pFbxImpoter == nullptr ){
		ERROR_MESSAGE( "FbxImpoter Create Failure." );
		return E_FAIL;
	}

	//------------------------------.
	// FBX�t�@�C���̓ǂݍ���.
	//------------------------------.
	// �t�@�C�����̐ݒ�.
	FbxString fbxFileName( fileName );
	if( pFbxImpoter->Initialize( fbxFileName.Buffer() ) == false ){
		ERROR_MESSAGE( "FbxFile Loading Failure." );
		return E_FAIL;
	}

	//------------------------------.
	// �V�[���I�u�W�F�N�g�̍쐬.
	//------------------------------.
	m_pFbxScene = FbxScene::Create( m_pFbxManager, "fbxScene" );
	if( m_pFbxScene == nullptr ){
		ERROR_MESSAGE( "FbxScene Create Failure." );
		return E_FAIL;
	}

	//------------------------------.
	// �C���|�[�^�[�ƃV�[���I�u�W�F�N�g�̊֘A�t��.
	//------------------------------.
	if( pFbxImpoter->Import( m_pFbxScene ) == false ){
		SAFE_DESTROY( m_pFbxManager );
		SAFE_DESTROY( m_pFbxScene );
		SAFE_DESTROY( pFbxImpoter );
		ERROR_MESSAGE( "FbxScene Create Failure." );
		return E_FAIL;
	}
	SAFE_DESTROY( pFbxImpoter );

	bool convertReslut = false;
	FbxGeometryConverter geometryConverter( m_pFbxManager );
	// �|���S�����O�p�`�ɂ���.
	// ���p�`�|���S��������΍��Ȃ����̂Ŏ��Ԃ�������.
	convertReslut = geometryConverter.Triangulate( m_pFbxScene, true );
	if( convertReslut == false ){
		ERROR_MESSAGE( "Triangulate Failure." );
		return E_FAIL;
	}
	geometryConverter.RemoveBadPolygonsFromMeshes( m_pFbxScene );
	// ���b�V�����}�e���A�����Ƃɕ�������.
	convertReslut = geometryConverter.SplitMeshesPerMaterial( m_pFbxScene, true );
	if( convertReslut == false ){
		ERROR_MESSAGE( "SplitMeshesPerMaterial Failure." );
		return E_FAIL;
	}


	CFbxModel* pModel = pModelData;

	//----------------------------.
	// FbxSkeleton�̐����擾.
	//----------------------------.
	int skeletonNum = m_pFbxScene->GetSrcObjectCount<FbxSkeleton>();
	int skeletonNo = 0;
	m_Skeletons.resize( skeletonNum );
	for( auto& s : m_Skeletons ){
		s = m_pFbxScene->GetSrcObject<FbxSkeleton>(skeletonNo);
		skeletonNo++;
	}

	//----------------------------.
	// FbxMesh�̐����擾.
	//----------------------------.
	int meshNum = m_pFbxScene->GetSrcObjectCount<FbxMesh>();
	int meshNo = 0;
	pModel->ReSizeMeshData( meshNum );
	for( auto& m : pModel->GetMeshData() ){
		// ���b�V���f�[�^�̎擾.
		FbxMesh* pMesh = m_pFbxScene->GetSrcObject<FbxMesh>(meshNo);
		// �}�e���A���̎擾.
		GetMaterial( pMesh, m, fileName, pModel->GetTextures() );
		// ���b�V���f�[�^�̓ǂݍ���.
		LoadMesh( pMesh, m );
		// ���_�o�b�t�@�̍쐬.
		if( FAILED( CreateVertexBuffers( m ) )) return E_FAIL;
		// �C���f�b�N�X�o�b�t�@�̍쐬.
		if( FAILED( CreateIndexBuffers( m ) )) return E_FAIL;
		meshNo++;
	}

	//----------------------------.
	//	�A�j���[�V�����̓ǂݍ���.
	//----------------------------.
	m_pAnimLoader = std::make_unique<CFbxAnimationLoader>();
	std::vector<SAnimationData>	animDataList;
	m_pAnimLoader->LoadAnimationData( m_pFbxScene, m_MeshClusterData, m_Skeletons, &animDataList );
	if( animDataList.empty() == false ){
		// ��Őݒ肵���A�j���[�V�����f�[�^�������.
		// �A�j���[�V�����R���g���[���[���쐬����.
		// �A�j���[�V�����f�[�^��ǉ�.
		pModel->SetAnimationData( animDataList );
	}

	return S_OK;
}

// �}�e���A���擾.
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

		MATERIAL tmpMat;	// ���}�e���A����p��.

							// �A���r�G���g�擾.
		if( ambient.IsValid() == true ){
			tmpMat.Ambient.x = (float)ambient.Get<FbxDouble4>()[0];
			tmpMat.Ambient.y = (float)ambient.Get<FbxDouble4>()[1];
			tmpMat.Ambient.z = (float)ambient.Get<FbxDouble4>()[2];
			tmpMat.Ambient.w = (float)ambient.Get<FbxDouble4>()[3];
		} else {
			tmpMat.Ambient = { 1.0f, 1.0f, 1.0f, 1.0f };
		}

		// �A���r�G���g�擾.
		if( diffuse.IsValid() == true ){
			tmpMat.Diffuse.x = (float)diffuse.Get<FbxDouble4>()[0];
			tmpMat.Diffuse.y = (float)diffuse.Get<FbxDouble4>()[1];
			tmpMat.Diffuse.z = (float)diffuse.Get<FbxDouble4>()[2];
			tmpMat.Diffuse.w = (float)diffuse.Get<FbxDouble4>()[3];
		} else {
			tmpMat.Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
		}

		// �A���r�G���g�擾.
		if( specular.IsValid() == true ){
			tmpMat.Specular.x = (float)specular.Get<FbxDouble4>()[0];
			tmpMat.Specular.y = (float)specular.Get<FbxDouble4>()[1];
			tmpMat.Specular.z = (float)specular.Get<FbxDouble4>()[2];
			tmpMat.Specular.w = (float)specular.Get<FbxDouble4>()[3];
		} else {
			tmpMat.Specular = { 1.0f, 1.0f, 1.0f, 1.0f };
		}
		// �}�e���A�����擾.
		tmpMat.Name = pMat->GetName();

		// �}�e���A�����X�g�ɒǉ�.
		mesh.Material = tmpMat;

		//-----------------------.
		// �e�N�X�`���쐬.
		//-----------------------.
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

//-----------------------------------------.
// �e�N�X�`���̓ǂݍ���.
//-----------------------------------------.
HRESULT CFbxModelLoader::LoadTexture( FbxFileTexture* texture, const char* fileName, const char* keyName, std::unordered_map<std::string, ID3D11ShaderResourceView*>& textures )
{
	// �t�@�C�������擾.
	std::string texturePath = texture->GetRelativeFileName();
	std::string path = fileName;

	// �t�@�C���p�X��ݒ�.
	int pos = path.find_last_of('\\')+1;
	path = path.substr( 0, pos );
	path += texturePath;

	// wchar_t �ɕϊ�.
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> cv;
	std::wstring wstr_file_name = cv.from_bytes(path);

	// �e�N�X�`�����\�[�X�쐬.
	if( FAILED( DirectX::CreateWICTextureFromFile(
		m_pDevice11,
		wstr_file_name.c_str(),
		nullptr,
		&textures[keyName] ))){
		return E_FAIL;
	}
	return S_OK;
}

//-----------------------------------------.
// ���b�V���̓ǂݍ���.
//-----------------------------------------.
void CFbxModelLoader::LoadMesh( FbxMesh* pMesh, FBXMeshData& meshData )
{
	LoadIndices( pMesh, meshData );	// �C���f�b�N�X���.

	int ctrlCount = pMesh->GetControlPointsCount();
	// �E�F�C�g.
	std::vector<std::vector<float>> weights( ctrlCount );
	// �{�[���C���f�b�N�X.
	std::vector<std::vector<int>> bones( ctrlCount );
	// �X�L�����̎擾.
	LoadSkin( pMesh, meshData, weights, bones );

	int polygonCount = pMesh->GetPolygonCount();
	// ���_�̐�.
	int vertexCounter = 0;
	// �|���S���̐���.
	for( int i = 0; i < polygonCount; i++ ){
		// �|���S���̃T�C�Y.
		int polygonSize = pMesh->GetPolygonSize(i);
		if( polygonSize != 3 ){
			// �O�p�|���S������Ȃ�.
			ERROR_MESSAGE("Not a triangular polygon");
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

//-----------------------------------------.
// �C���f�b�N�X���ǂݍ���.
//-----------------------------------------.
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

//-----------------------------------------.
// ���_���ǂݍ���.
//-----------------------------------------.
void CFbxModelLoader::LoadVertices( FbxMesh* pMesh, VERTEX& vertex, int ctrlPointIndex )
{
	vertex.Pos.x = (float)pMesh->GetControlPoints()[ctrlPointIndex][0];
	vertex.Pos.y = (float)pMesh->GetControlPoints()[ctrlPointIndex][1];
	vertex.Pos.z = (float)pMesh->GetControlPoints()[ctrlPointIndex][2];
}

//-----------------------------------------.
// �@�����ǂݍ���.
//-----------------------------------------.
void CFbxModelLoader::LoadNormals( FbxMesh* pMesh, VERTEX& vertex, int ctrlPointIndex, int vertexCounter )
{
	if( pMesh->GetElementNormalCount() < 1 ) return;

	FbxGeometryElementNormal* leNormal = pMesh->GetElementNormal(0);
	switch( leNormal->GetMappingMode() )
	{
	case FbxGeometryElement::eByControlPoint:
	{
		switch( leNormal->GetReferenceMode() )
		{
		case FbxGeometryElement::eDirect:
		{
			vertex.Normal.x = -(float)leNormal->GetDirectArray().GetAt(ctrlPointIndex)[0];
			vertex.Normal.y = (float)leNormal->GetDirectArray().GetAt(ctrlPointIndex)[1];
			vertex.Normal.z = (float)leNormal->GetDirectArray().GetAt(ctrlPointIndex)[2];
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int id = leNormal->GetIndexArray().GetAt(ctrlPointIndex);
			vertex.Normal.x = -(float)leNormal->GetDirectArray().GetAt(id)[0];
			vertex.Normal.y = (float)leNormal->GetDirectArray().GetAt(id)[1];
			vertex.Normal.z = (float)leNormal->GetDirectArray().GetAt(id)[2];
		}
		break;

		default:
			break;
		}
	}
	break;

	case FbxGeometryElement::eByPolygonVertex:
	{
		switch (leNormal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			vertex.Normal.x = -(float)leNormal->GetDirectArray().GetAt(vertexCounter)[0];
			vertex.Normal.y = (float)leNormal->GetDirectArray().GetAt(vertexCounter)[1];
			vertex.Normal.z = (float)leNormal->GetDirectArray().GetAt(vertexCounter)[2];
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int id = leNormal->GetIndexArray().GetAt(vertexCounter);
			vertex.Normal.x = -(float)leNormal->GetDirectArray().GetAt(id)[0];
			vertex.Normal.y = (float)leNormal->GetDirectArray().GetAt(id)[1];
			vertex.Normal.z = (float)leNormal->GetDirectArray().GetAt(id)[2];
		}
		break;

		default:
			break;
		}
	}
	break;
	}
}

//-----------------------------------------.
// ���_�J���[�ǂݍ���.
//-----------------------------------------.
void CFbxModelLoader::LoadColors( FbxMesh* pMesh, VERTEX& vertex, int ctrlPointIndex, int vertexCounter )
{
	// ���_�J���[�f�[�^�̐����m�F.
	int colorCount = pMesh->GetElementVertexColorCount();
	if( colorCount == 0 ) return;

	FbxGeometryElementVertexColor* vertexColors = pMesh->GetElementVertexColor(0);
	switch( vertexColors->GetMappingMode() )
	{
	case FbxGeometryElement::eByControlPoint:
	{
		switch( vertexColors->GetReferenceMode() )
		{
		case FbxGeometryElement::eDirect:
		{
			vertex.Color.x = -(float)vertexColors->GetDirectArray().GetAt(ctrlPointIndex)[0];
			vertex.Color.y = (float)vertexColors->GetDirectArray().GetAt(ctrlPointIndex)[1];
			vertex.Color.z = (float)vertexColors->GetDirectArray().GetAt(ctrlPointIndex)[2];
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int id = vertexColors->GetIndexArray().GetAt(ctrlPointIndex);
			vertex.Color.x = -(float)vertexColors->GetDirectArray().GetAt(id)[0];
			vertex.Color.y = (float)vertexColors->GetDirectArray().GetAt(id)[1];
			vertex.Color.z = (float)vertexColors->GetDirectArray().GetAt(id)[2];
		}
		break;

		default:
			break;
		}
	}
	break;

	case FbxGeometryElement::eByPolygonVertex:
	{
		switch (vertexColors->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			vertex.Color.x = (float)vertexColors->GetDirectArray().GetAt(vertexCounter)[0];
			vertex.Color.y = (float)vertexColors->GetDirectArray().GetAt(vertexCounter)[1];
			vertex.Color.z = (float)vertexColors->GetDirectArray().GetAt(vertexCounter)[2];
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int id = vertexColors->GetIndexArray().GetAt(vertexCounter);
			vertex.Color.x = (float)vertexColors->GetDirectArray().GetAt(id)[0];
			vertex.Color.y = (float)vertexColors->GetDirectArray().GetAt(id)[1];
			vertex.Color.z = (float)vertexColors->GetDirectArray().GetAt(id)[2];
		}
		break;

		default:
			break;
		}
	}
	break;
	}
}

//-----------------------------------------.
// UV���ǂݍ���.
//-----------------------------------------.
void CFbxModelLoader::LoadUV( FbxMesh* pMesh, VERTEX& vertex, int ctrlPointIndex, int texUVIndex, int uvLayer )
{
	if( uvLayer >= 2 || pMesh->GetElementUVCount() <= uvLayer ) return;

	FbxGeometryElementUV* pVertexUV = pMesh->GetElementUV( uvLayer );

	switch( pVertexUV->GetMappingMode() )
	{
	case FbxGeometryElement::eByControlPoint:
	{
		switch( pVertexUV->GetReferenceMode() )
		{
		case FbxGeometryElement::eDirect:
		{
			vertex.UV.x = (float)pVertexUV->GetDirectArray().GetAt(ctrlPointIndex)[0];
			vertex.UV.y = -(float)pVertexUV->GetDirectArray().GetAt(ctrlPointIndex)[1];
		}
		break;
		case FbxGeometryElement::eIndexToDirect:
		{
			int id = pVertexUV->GetIndexArray().GetAt(ctrlPointIndex);
			vertex.UV.x = (float)pVertexUV->GetDirectArray().GetAt(id)[0];
			vertex.UV.y = -(float)pVertexUV->GetDirectArray().GetAt(id)[1];
		}
		break;
		default:
			break;
		}
	}
	break;

	case FbxGeometryElement::eByPolygonVertex:
	{
		switch (pVertexUV->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		case FbxGeometryElement::eIndexToDirect:
		{
			vertex.UV.x = (float)pVertexUV->GetDirectArray().GetAt(texUVIndex)[0];
			vertex.UV.y = -(float)pVertexUV->GetDirectArray().GetAt(texUVIndex)[1];
		}
		break;
		default:
			break;
		}
	}
	break;
	}
}

//-----------------------------------------.
// �X�L�����ǂݍ���.
//-----------------------------------------.
void CFbxModelLoader::LoadSkin( FbxMesh* pMesh, FBXMeshData& meshData, std::vector<std::vector<float>>& weights, std::vector<std::vector<int>>& bones )
{
	// �_�E���L���X�g���ăX�L�������擾.
	FbxSkin* pSkin = (FbxSkin*)pMesh->GetDeformer( 0, FbxDeformer::eSkin );
	if( pSkin == nullptr ) return;

	m_MeshClusterData.emplace_back();

	// �{�[���̐�.
	int boneCount = pSkin->GetClusterCount();
	for( int boneIndex = 0; boneIndex < boneCount; boneIndex++ ){
		// �{�[�����擾.
		FbxCluster* pCluster = pSkin->GetCluster( boneIndex );
		FbxNode* pNode = pCluster->GetLink();

		m_MeshClusterData.back().ClusterKey[pNode->GetName()] = pNode;
		m_MeshClusterData.back().ClusterName.emplace_back( pNode->GetName() );

		FbxAMatrix bindPoseMatrix;
		FbxAMatrix newbindpose;

		pCluster->GetTransformLinkMatrix( bindPoseMatrix );
		pCluster->GetTransformMatrix( newbindpose );

		newbindpose = bindPoseMatrix.Inverse() * newbindpose;
		// �����{�[�����W�̐ݒ�.
		meshData.Skin.InitBonePositions.emplace_back( newbindpose );

		// �{�[���C���f�b�N�X�ƃE�F�C�g�̐ݒ�.
		int*	boneVertexIndices = pCluster->GetControlPointIndices();
		double* boneVertexWeights = pCluster->GetControlPointWeights();
		int	numBoneVertexIndices = pCluster->GetControlPointIndicesCount();
		for( int boneVertexIndex = 0; boneVertexIndex < numBoneVertexIndices; boneVertexIndex++ ){
			int cpIndex = boneVertexIndices[boneVertexIndex];
			float boneWeight = (float)boneVertexWeights[boneVertexIndex];
			weights[cpIndex].emplace_back(boneWeight);
			bones[cpIndex].emplace_back(boneIndex);
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

//-----------------------------------------.
// �E�F�C�g�ƃ{�[���̐ݒ�.
//-----------------------------------------.
void CFbxModelLoader::SetBoneWeight( VERTEX& vertex, const std::vector<float>& weight, const std::vector<int>& bone )
{
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

// ���_�o�b�t�@�쐬.
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
	if( FAILED( m_pDevice11->CreateBuffer(
		&bd, &data, &meshData.pVertexBuffer ))){
		return E_FAIL;
	}
	return S_OK;
}

// �C���f�b�N�X�o�b�t�@�쐬.
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
	if( FAILED( m_pDevice11->CreateBuffer(
		&bd, &data, &meshData.pIndexBuffer ))){
		return E_FAIL;
	}
	return S_OK;
}