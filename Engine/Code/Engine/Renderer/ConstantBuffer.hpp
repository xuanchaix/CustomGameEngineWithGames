#pragma once
#include "Engine/Core/EngineCommon.hpp"

struct ID3D11Buffer;
struct ID3D12Resource;
struct D3D12_CONSTANT_BUFFER_VIEW_DESC;

class ConstantBuffer {
	friend class DX11Renderer;
	friend class DX12Renderer;
public:
	ConstantBuffer( size_t size );
	ConstantBuffer( ConstantBuffer const& copy ) = delete;
	virtual ~ConstantBuffer();

private:

#ifdef ENGINE_DX11_RENDERER_INTERFACE
	ID3D11Buffer* m_buffer = nullptr;
#endif

#ifdef ENGINE_DX12_RENDERER_INTERFACE
	ID3D12Resource* m_dx12ConstantBuffer = nullptr;
	D3D12_CONSTANT_BUFFER_VIEW_DESC* m_constantBufferView = nullptr;
#endif
	size_t m_size = 0;
};