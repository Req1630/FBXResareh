#include "FbxAnimationController.h"

CFbxAnimationController::CFbxAnimationController()
{
}

CFbxAnimationController::~CFbxAnimationController()
{
}

// フレームの更新.
void CFbxAnimationController::FrameUpdate()
{
	// アニメーションデータが無ければ終了.
	if( m_AnimDataList.empty() == true ) return;
	if( m_AnimSpeed >= 0.0 ){
		// 経過時間を加算.
		m_NowAnimation.NowTime += m_AnimSpeed;
		// 経過時間が終了していれば.
		if( m_NowAnimation.NowTime >= m_NowAnimation.EndTime )
			m_NowAnimation.NowTime = m_NowAnimation.StartTime;	// 開始時間を設定.
	} else {
		// 経過時間を加算.
		m_NowAnimation.NowTime += m_AnimSpeed;
		// 経過時間が開始時間なら.
		if( m_NowAnimation.NowTime <= m_NowAnimation.StartTime )
			m_NowAnimation.NowTime = m_NowAnimation.EndTime;	// 終了時間を設定.
	}
}

// アニメーションデータの設定.
void CFbxAnimationController::SetAnimDataList( const std::vector<SAnimationData>& animDataList )
{
	m_AnimDataList = animDataList;
	m_NowAnimation = m_AnimDataList[0];
	m_AnimSpeed = m_NowAnimation.AnimSpeed;
}

// 次のアニメーションに変更.
void CFbxAnimationController::ChangeNextAnimation()
{
	m_NowAnimNumber++;	// アニメーション番号の加算.
	ChangeAnimation( m_NowAnimNumber );
}

// 指定のアニメーションに変更.
void CFbxAnimationController::ChangeAnimation( int& animNumber )
{
	if( m_AnimDataList.empty() == true ) return;
	// アニメーション番号がアニメーションリストより多ければ.
	// アニメーションリストの初めから再生.
	if( animNumber >= (int)m_AnimDataList.size() ) animNumber = 0;
	m_NowAnimation = m_AnimDataList[animNumber];
	m_AnimSpeed = m_NowAnimation.AnimSpeed;
}

FbxMatrix CFbxAnimationController::GetFrameLinkMatrix( const int& meshNo, const int& i )
{
	return m_NowAnimation.GetFrameLinkMatrix( meshNo, i, m_NowAnimation.NowTime );
}

FbxMatrix CFbxAnimationController::GetFrameMatrix( const int& meshNo )
{
	return m_NowAnimation.GetFrameMatrix( meshNo, m_NowAnimation.NowTime );
}