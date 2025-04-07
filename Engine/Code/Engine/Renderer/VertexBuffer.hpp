#pragma once
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/EngineCommon.hpp"
struct ID3D11Buffer;
struct D3D12_VERTEX_BUFFER_VIEW;
struct ID3D12Resource;

class VertexBuffer {
	friend class DX11Renderer;
	friend class DX12Renderer;
public:
	VertexBuffer( size_t size, unsigned int stride = sizeof(Vertex_PCU) );
	VertexBuffer( VertexBuffer const& copy ) = delete;
	unsigned int GetStride() const;
	size_t GetSize() const;
	int GetVertexCount() const;
	virtual ~VertexBuffer();

	void SetAsLinePrimitive( bool value );

protected:

#ifdef ENGINE_DX11_RENDERER_INTERFACE
	ID3D11Buffer* m_vertexBuffer = nullptr;
#endif

#ifdef ENGINE_DX12_RENDERER_INTERFACE
	ID3D12Resource* m_dx12VertexBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW* m_vertexBufferView = nullptr;
#endif

	size_t m_size = 0;
	unsigned int m_stride = 0;
	int m_vertexCount = 0;
	bool m_isLinePrimitive = false;
};