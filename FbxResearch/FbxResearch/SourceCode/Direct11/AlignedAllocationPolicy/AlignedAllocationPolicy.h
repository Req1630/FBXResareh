#pragma once

/*************************************************************************
VisualStudioが提供する標準の new/delete は8バイト境界
このため、このXMMATRIX使うクラスも16バイト単位のメモリアライメントで
生成しなければならない。
つまり、new/deleteをオーバーライドする必要がある。

DirectXMath.hのDirectX::XMMATRIXは、
SSE命令を使い行列を高速に処理するために、
16バイトメモリアライメントになっている。
****************************************/

// コンパイラ警告 C4316 を回避するため.
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