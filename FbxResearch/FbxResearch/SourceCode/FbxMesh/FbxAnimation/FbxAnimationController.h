#pragma once

#include "FbxAnimationData.h"

class CFbxAnimationController
{
public:
	CFbxAnimationController();
	~CFbxAnimationController();

	// フレームの更新.
	void FrameUpdate();

	// アニメーションデータの設定.
	void SetAnimDataList( const std::vector<SAnimationData>& animDataList );

	// 次のアニメーションに変更.
	void ChangeNextAnimation();
	// 指定のアニメーションに変更.
	void ChangeAnimation( int& animNumber );
	// フレーム時の行列取得.
	FbxMatrix GetFrameLinkMatrix( const int& meshNo, const int& i );
	// フレーム時の行列取得.
	FbxMatrix GetFrameMatrix( const int& meshNo );

	// アニメーション速度の設定.
	void SetAnimSpeed( const double& speed ){ m_AnimSpeed = speed; }
	// アニメーション速度の取得.
	double GetAnimSpeed() const { return m_AnimSpeed; }

	// アニメーションデータの追加.
	void AddAnimationData( const std::vector<SAnimationData>& animationData )
	{
		for( auto& a : animationData )
			m_AnimDataList.emplace_back( a ); 
	}

private:
	double m_AnimSpeed;							// アニメーション速度.
	std::vector<SAnimationData>	m_AnimDataList;	// アニメーションデータリスト.
	SAnimationData	m_NowAnimation;				// 現在のアニメーションデータ.
	int m_NowAnimNumber;						// 現在のアニメーション番号.
};