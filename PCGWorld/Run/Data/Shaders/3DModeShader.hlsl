
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
	float3 normal: NORMAL;
};

struct v2p_t
{
	float4 position : SV_Position;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
};

v2p_t VertexMain(vs_input_t input)
{
	float4 localPosition = float4(input.localPosition, 1);
	float4 clipPosition = mul(projectionMatrix, mul(viewMatrix, mul(modelMatrix, localPosition)));

	v2p_t v2p;
	v2p.position = clipPosition;
	v2p.color = float4(input.color) * modelColor;
	v2p.uv = input.uv;
	v2p.uv.y = 1 - v2p.uv.y;
	v2p.normal = input.normal;
	return v2p;
}

float4 PixelMain(v2p_t input) : SV_Target0
{
	float4 textureColor = diffuseTexture.Sample(diffuseSampler, input.uv);
	float blendFactor = (textureColor.r + textureColor.g + textureColor.b + textureColor.a) / 4;

// 	float NdotL = dot(input.normal, float3( 0.4472, 0.4, -0.8)); // normal dot light
// 	float remappedNDotL = (NdotL + 1.0) / 2.0 / 0.49;
// 	//float floored = floor(remappedNDotL);
// 	float res = remappedNDotL / 2.04 * 0.7 + 0.3;
// 	
// 	float _Scale = 2;
// 	float halfScaleFloor = floor(_Scale * 0.5);
// 	float halfScaleCeil = ceil(_Scale * 0.5);
// 
// 	float2 bottomLeftUV = input.uv - float2(_MainTex_TexelSize.x, _MainTex_TexelSize.y) * halfScaleFloor;
// 	float2 topRightUV = input.uv + float2(_MainTex_TexelSize.x, _MainTex_TexelSize.y) * halfScaleCeil;  
// 	float2 bottomRightUV = input.uv + float2(_MainTex_TexelSize.x * halfScaleCeil, -_MainTex_TexelSize.y * halfScaleFloor);
// 	float2 topLeftUV = input.uv + float2(-_MainTex_TexelSize.x * halfScaleFloor, _MainTex_TexelSize.y * halfScaleCeil);
// 
// 	float depth0 = SAMPLE_DEPTH_TEXTURE(_CameraDepthTexture, sampler_CameraDepthTexture, bottomLeftUV).r;
// 	float depth1 = SAMPLE_DEPTH_TEXTURE(_CameraDepthTexture, sampler_CameraDepthTexture, topRightUV).r;
// 	float depth2 = SAMPLE_DEPTH_TEXTURE(_CameraDepthTexture, sampler_CameraDepthTexture, bottomRightUV).r;
// 	float depth3 = SAMPLE_DEPTH_TEXTURE(_CameraDepthTexture, sampler_CameraDepthTexture, topLeftUV).r;
// 
// 	float depthFiniteDifference0 = depth1 - depth0;
// 	float depthFiniteDifference1 = depth3 - depth2;


 	float4 color = input.color * blendFactor + textureColor * (1 - blendFactor);
 	clip(color.a - 0.01f);
 	return color;
}

