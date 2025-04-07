#pragma once
#include "Engine/Core/EngineCommon.hpp"

struct ID3D11Buffer;
struct ID3D12Resource;
struct D3D12_INDEX_BUFFER_VIEW;

class IndexBuffer {
	friend class DX11Renderer;
	friend class DX12Renderer;
public:
	IndexBuffer( size_t size );
	IndexBuffer( IndexBuffer const& copy ) = delete;
	size_t GetSize() const;
	int GetIndexCount() const;
	virtual ~IndexBuffer();

protected:

#ifdef ENGINE_DX11_RENDERER_INTERFACE
	ID3D11Buffer* m_indexBuffer = nullptr;
#endif

#ifdef ENGINE_DX12_RENDERER_INTERFACE
	ID3D12Resource* m_dx12IndexBuffer = nullptr;
	D3D12_INDEX_BUFFER_VIEW* m_indexBufferView = nullptr;
#endif

	size_t m_size = 0;
	int m_indexCount = 0;
};