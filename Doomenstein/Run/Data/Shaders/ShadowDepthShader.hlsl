//------------------------------------------------------------------------------------------------
struct vs_input_t
{
	float3 localPosition : POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
	float3 localTangent : TANGENT;
	float3 localBitangent : BITANGENT;
	float3 localNormal : NORMAL;
};

//------------------------------------------------------------------------------------------------
struct v2p_t
{
	float4 position : SV_Position;
};

//------------------------------------------------------------------------------------------------
cbuffer LightConstants : register(b0)
{
	float3 LightPosition;
	float ambient;
	float4x4 LightViewMatrix;
	float4x4 LightProjectionMatrix;
};

//------------------------------------------------------------------------------------------------
cbuffer ModelConstants : register(b3)
{
	float4x4 ModelMatrix;
	float4 ModelColor;
};

//------------------------------------------------------------------------------------------------
v2p_t VertexMain(vs_input_t input)
{
	float4 localPosition = float4(input.localPosition, 1);
	float4 worldPosition = mul(ModelMatrix, localPosition);
	float4 viewPosition = mul(LightViewMatrix, worldPosition);
	float4 clipPosition = mul(LightProjectionMatrix, viewPosition);

	v2p_t v2p;
	v2p.position = clipPosition;
	return v2p;
}



//------------------------------------------------------------------------------------------------
float4 PixelMain(v2p_t input) : SV_Target0
{
	float depth = input.position.z / input.position.w;
	float4 color = float4(depth, depth, depth, 1.f);
	return color;
}
