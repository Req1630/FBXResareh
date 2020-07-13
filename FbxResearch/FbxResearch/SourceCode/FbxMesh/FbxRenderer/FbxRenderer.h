#pragma once

#include "..\FbxInclude\FbxInclude.h"
#include "..\FbxMeshData.h"

class CFbxModel;
class CFbxAnimationController;
class CCamera;
class CLight;

class CFbxRenderer
{
	// シェーダー名.
	const wchar_t* VS_SHADER_NAME		= L"Data\\Shader\\FbxModelVS.hlsl";
	const wchar_t* VS_ANIM_SHADER_NAME	= L"Data\\Shader\\FbxAnimationVS.hlsl";
	const wchar_t* PS_SHADER_NAME		= L"Data\\Shader\\FbxModelPS.hlsl";

	// 最大ボーン数.
	static const int BONE_COUNT_MAX = 255;

	/***************************************
	*			↓	構造体　↓.
	***************************************/
	//======================================.
	//	コンスタントバッファ(メッシュ毎).
	//======================================.
	struct CBUFFER_PER_MESH
	{
		DirectX::XMMATRIX mW;			// ワールド行列.
		DirectX::XMMATRIX mWVP;			// World,View,Proj の合成変換行列.
		DirectX::XMMATRIX mLightWVP;
		DirectX::XMFLOAT4 CameraPos;
		DirectX::XMFLOAT4 LightDir;
	};
	//======================================.
	// コンスタントバッファ(マテリアル毎).
	//======================================.
	struct CBUFFER_PER_MATERIAL
	{
		DirectX::XMFLOAT4	Ambient;	// アンビエント.
		DirectX::XMFLOAT4	Diffuse;	// ディヒューズ.
		DirectX::XMFLOAT4	Specular;	// スペキュラ.
	};
	//======================================.
	// コンスタントバッファ(ボーン毎).
	//======================================.
	struct CBUFFER_PER_BONE
	{
		DirectX::XMMATRIX Bone[BONE_COUNT_MAX];	// ボーン行列.
		CBUFFER_PER_BONE()
		{
			// 行列の初期化.
			for( int i = 0; i < BONE_COUNT_MAX; i++ ){
				Bone[i] = DirectX::XMMatrixIdentity();
			}
		}
	};
public:
	CFbxRenderer();
	~CFbxRenderer();

	//-----------------------------------------.
	//				作成.
	//-----------------------------------------.
	HRESULT Create( ID3D11DeviceContext* pContext11 );

	//-----------------------------------------.
	//				破壊.
	//-----------------------------------------.
	void Destroy();

	//-----------------------------------------.
	//				描画.
	//-----------------------------------------.
	void Render(
		CFbxModel& mdoel,
		CCamera& camera,
		CLight& light,
		CFbxAnimationController* pAc = nullptr );

private:
	// アニメーションフレームの更新.
	void AnimationFrameUpdate( CFbxModel& mdoel, CFbxAnimationController* pAc );

	// アニメーション用の行列計算.
	void AnimMatrixCalculation( 
		CFbxModel& mdoel,
		const int& meahNo, 
		FBXMeshData& meshData, 
		CFbxAnimationController* pAc );

	// ボーンの定数バッファの取得.
	bool GetBoneConstBuffer(
		const SkinData& skinData,
		const int& meahNo,
		CFbxAnimationController* pAc,
		CBUFFER_PER_BONE* pOutCB );

	//-----------------------------------------.
	//		描画用データの作成系.
	//-----------------------------------------.

	// 定数バッファの作成.
	HRESULT CreateBuffer();
	// サンプラー作成.
	HRESULT CreateSampler();
	// 定数バッファ作成.
	HRESULT CreateConstantBuffer( const size_t& byte, ID3D11Buffer** buffer );

	// シェーダーの作成.
	HRESULT CreateShader();

private:
	/***************************************
	*			DirectX11.
	***************************************/
	ID3D11Device*			m_pDevice11;	// デバイス11.
	ID3D11DeviceContext*	m_pContext11;	// コンテキスト11.

	/***************************************
	*			シェーダー.
	***************************************/
	ID3D11VertexShader*	m_pVertexShader;		// 頂点シェーダー.
	ID3D11VertexShader*	m_pVertexAnimShader;	// 頂点シェーダー(アニメーション).
	ID3D11PixelShader*	m_pPixelShader;			// ピクセルシェーダー.

	/***************************************
	*			描画用データ.
	***************************************/
	ID3D11InputLayout*	m_pVertexLayout;		// 頂点レイアウト.
	ID3D11Buffer*		m_pCBufferPerMesh;		// コンスタントバッファ.
	ID3D11Buffer*		m_pCBufferPerMaterial;	// コンスタントバッファ.
	ID3D11Buffer*		m_pCBufferPerBone;		// コンスタントバッファ.
	ID3D11SamplerState*	m_pSampleLinear;		// サンプラ:テクスチャに各種フィルタをかける.
};