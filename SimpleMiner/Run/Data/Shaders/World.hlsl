//------------------------------------------------------------------------------------------------
struct vs_input_t
{
	float3 localPosition : POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
};

//------------------------------------------------------------------------------------------------
struct v2p_t
{
	float4 position : SV_Position;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
	float4 worldPosition: WORLD_POSITION;
};

//------------------------------------------------------------------------------------------------
cbuffer CameraConstants : register(b2)
{
	float4x4 ViewMatrix;
	float4x4 ProjectionMatrix;
};

//------------------------------------------------------------------------------------------------
cbuffer ModelConstants : register(b3)
{
	float4x4 ModelMatrix;
	float4 ModelColor;
};

cbuffer SimplerMinerConstants :  register(b8)
{
	float4 CameraWorldPos;
	float4 IndoorLightColor;
	float4 OutdoorLightColor;
	float4 SkyColor;
	float FogNearDist;
	float FogFarDist;
	float pad0;
	float pad1;
};

float4 DiminishingAdd( float4 a, float4 b ){
	return float4(1, 1, 1, 1) - (float4(1, 1, 1, 1) - a) * (float4(1, 1, 1, 1) - b);
}

//------------------------------------------------------------------------------------------------
Texture2D diffuseTexture : register(t0);

//------------------------------------------------------------------------------------------------
SamplerState diffuseSampler : register(s0);

//------------------------------------------------------------------------------------------------
v2p_t VertexMain(vs_input_t input)
{
	float4 localPosition = float4(input.localPosition, 1);
	float4 worldPosition = mul(ModelMatrix, localPosition);
	float4 viewPosition = mul(ViewMatrix, worldPosition);
	float4 clipPosition = mul(ProjectionMatrix, viewPosition);

	v2p_t v2p;
	v2p.position = clipPosition;
	v2p.color = input.color;
	v2p.uv = input.uv;
	v2p.worldPosition = worldPosition;
	return v2p;
}

//------------------------------------------------------------------------------------------------
float4 PixelMain(v2p_t input) : SV_Target0
{
	float4 textureColor = diffuseTexture.Sample(diffuseSampler, input.uv);
	float4 modelColor = ModelColor;
	
	float4 pixelOutdoorLightColor = input.color.r * OutdoorLightColor;
	float4 pixelIndoorLightColor = input.color.g * IndoorLightColor;

	float4 color = textureColor * modelColor * DiminishingAdd(pixelOutdoorLightColor, pixelIndoorLightColor);
	
	float dist = distance(input.worldPosition.xyz, CameraWorldPos.xyz);
	float fraction = saturate((dist - FogNearDist) / (FogFarDist - FogNearDist));
	color.rgb = lerp(color.rgb, SkyColor.rgb, fraction);
	color.a = color.a + fraction;
	color.a = saturate(color.a);

	return color;
}
