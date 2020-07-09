#pragma once

#include "FbxAnimationData.h"

/***************************************************************************
	アニメーションを操作するクラス.
----------------------------------------------------------------------------
*	↓   主に使用する関数   ↓.
*	次のアニメーションに変更.
*	void ChangeNextAnimation().
*
----------------------------------------------------------------------------
*	指定のアニメーションに変更.
*	indexがアニメーションリストより多ければ、
*	リストの初めから再生.
*	void ChangeAnimation( int& index ).
*
----------------------------------------------------------------------------
*	アニメーション速度の設定.
*	void SetAnimSpeed( const double& speed ).
*
----------------------------------------------------------------------------
*	現在のアニメーション速度の取得.
*	double GetAnimSpeed().
*
***/
class CFbxAnimationController
{
public:
	CFbxAnimationController();
	~CFbxAnimationController();

	//-----------------------------------------.
	// フレームの更新.
	//-----------------------------------------.
	void FrameUpdate();

	//-----------------------------------------.
	//	アニメーションデータの設定.
	//-----------------------------------------.

	// アニメーションデータの設定.
	void SetAnimDataList( const SAnimationDataList& animDataList );
	// アニメーションデータの追加.
	void AddAnimationData( const SAnimationDataList& animationData );

	//-----------------------------------------.
	//	アニメーションのパラメータの設定・取得.
	//-----------------------------------------.

	// 次のアニメーションに変更.
	void ChangeNextAnimation();
	// 指定のアニメーションに変更.
	void ChangeAnimation( int& animNumber );

	// アニメーション速度の設定.
	void SetAnimSpeed( const double& speed ){ m_AnimSpeed = speed; }
	// アニメーション速度の取得.
	double GetAnimSpeed() const { return m_AnimSpeed; }

	// フレーム時の行列取得.
	bool GetFrameLinkMatrix( const int& meshNo, const int& i, FbxMatrix* outMatrix );

private:
	double m_AnimSpeed;							// アニメーション速度.
	int m_NowAnimNumber;						// 現在のアニメーション番号.
	std::vector<SAnimationData>	m_AnimDataList;	// アニメーションデータリスト.
	SAnimationData	m_NowAnimation;				// 現在のアニメーションデータ.
};