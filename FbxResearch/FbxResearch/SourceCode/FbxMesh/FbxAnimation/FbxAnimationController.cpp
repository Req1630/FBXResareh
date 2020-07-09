#include "FbxAnimationController.h"

CFbxAnimationController::CFbxAnimationController()
	: m_AnimSpeed		( 0.0 )
	, m_NowAnimNumber	( 0 )
	, m_AnimDataList	()
	, m_NowAnimation	()
{
}

CFbxAnimationController::~CFbxAnimationController()
{
	m_AnimDataList.clear();
}

///////////////////////////////////////////////////.
// フレームの更新.
///////////////////////////////////////////////////.
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

///////////////////////////////////////////////////.
// アニメーションデータの設定.
///////////////////////////////////////////////////.
void CFbxAnimationController::SetAnimDataList( const SAnimationDataList& animDataList )
{
	// アニメーションデータが空だったら終了.
	if( animDataList.AnimList.empty() == true ) return;
	m_AnimDataList = animDataList.AnimList;			// アニメーションデータの設定.
	m_NowAnimation = m_AnimDataList[0];		// アニメーションの最初のデータを現在のデータに設定.
	m_AnimSpeed = m_NowAnimation.AnimSpeed;	// アニメーション速度の設定.
}

///////////////////////////////////////////////////.
// アニメーションデータの追加.
///////////////////////////////////////////////////.
void CFbxAnimationController::AddAnimationData( const SAnimationDataList& animationData )
{
	// アニメーションリスト分のアニメーション追加.
	for( auto& a : animationData.AnimList ) m_AnimDataList.emplace_back( a );
	// アニメーションデータが空なら終了.
	if( m_AnimDataList.empty() == true ) return;
	m_NowAnimation = m_AnimDataList[0];		// アニメーションの最初のデータを現在のデータに設定.
	m_AnimSpeed = m_NowAnimation.AnimSpeed;	// アニメーション速度の設定.
}

///////////////////////////////////////////////////.
// 次のアニメーションに変更.
///////////////////////////////////////////////////.
void CFbxAnimationController::ChangeNextAnimation()
{
	m_NowAnimNumber++;	// アニメーション番号の加算.
	ChangeAnimation( m_NowAnimNumber );	// アニメーションの変更.
}

///////////////////////////////////////////////////.
// 指定のアニメーションに変更.
///////////////////////////////////////////////////.
void CFbxAnimationController::ChangeAnimation( int& animNumber )
{
	// アニメーションデータが空なら終了.
	if( m_AnimDataList.empty() == true ) return;
	// アニメーション番号がアニメーションリストより多ければ.
	// アニメーションリストの初めから再生.
	if( animNumber >= (int)m_AnimDataList.size() ) animNumber = 0;
	m_NowAnimation = m_AnimDataList[animNumber];
	m_AnimSpeed = m_NowAnimation.AnimSpeed;
}

///////////////////////////////////////////////////.
// フレーム時の行列取得.
///////////////////////////////////////////////////.
bool CFbxAnimationController::GetFrameLinkMatrix( const int& meshNo, const int& i, FbxMatrix* outMatrix )
{
	return m_NowAnimation.GetFrameLinkMatrix( meshNo, i, m_NowAnimation.NowTime, outMatrix );
}