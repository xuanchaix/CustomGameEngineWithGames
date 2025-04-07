
cbuffer CameraConstants : register(b2)
{
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
};

cbuffer ModelConstants : register(b3)
{
	float4x4 modelMatrix;
	float4 modelColor;
};

Texture2D diffuseTexture : register(t0);
SamplerState diffuseSampler : register(s0);

struct vs_input_t
{
	float3 localPosition : POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
};

struct v2p_t
{
	float4 position : SV_Position;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
};

v2p_t VertexMain(vs_input_t input)
{
	float4 localPosition = float4(input.localPosition, 1);
	float4 clipPosition = mul(projectionMatrix, mul(viewMatrix, mul(modelMatrix, localPosition)));

	v2p_t v2p;
	v2p.position = clipPosition;
	v2p.color = float4(input.color) * modelColor;
	v2p.uv = input.uv;
	return v2p;
}

float4 PixelMain(v2p_t input) : SV_Target0
{
	float4 textureColor = diffuseTexture.Sample(diffuseSampler, input.uv);
	float4 color = input.color * textureColor;
	clip(color.a - 0.01f);
	return color;
}

