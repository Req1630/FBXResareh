#pragma once

/*************************************************************************
VisualStudio���񋟂���W���� new/delete ��8�o�C�g���E
���̂��߁A����XMMATRIX�g���N���X��16�o�C�g�P�ʂ̃������A���C�����g��
�������Ȃ���΂Ȃ�Ȃ��B
�܂�Anew/delete���I�[�o�[���C�h����K�v������B

DirectXMath.h��DirectX::XMMATRIX�́A
SSE���߂��g���s��������ɏ������邽�߂ɁA
16�o�C�g�������A���C�����g�ɂȂ��Ă���B
****************************************/

// �R���p�C���x�� C4316 ��������邽��.
template <size_t T>
class AlignedAllocationPolicy {
public:
	static void* operator new( size_t i ) {
		return _mm_malloc(i,T);
	}
	static void operator delete( void* p ) {
		_mm_free(p);
	}
};