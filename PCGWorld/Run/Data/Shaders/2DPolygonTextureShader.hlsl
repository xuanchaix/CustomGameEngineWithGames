
cbuffer ModelConstants : register(b3)
{
	float4 modelMatrix1;
	float4 modelMatrix2;
	float4 modelMatrix3;
	float4 dimensions;
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

struct ps_output_t{
	//float4 discardColor: SV_Target0;
	float4 color: SV_Target0;
};

v2p_t VertexMain(vs_input_t input)
{
	float4 clipPosition = float4(2 * ((input.localPosition.x / dimensions.x) - 0.5), 2 * ((input.localPosition.y / dimensions.y) -0.5), input.localPosition.z, 1);

	v2p_t v2p;
	v2p.position = clipPosition;
	if(input.color.r == modelColor.r && input.color.g == modelColor.g && input.color.b == modelColor.b){
		v2p.color = float4(0.0, 0.4, 1.0, 1.0);
	}
	else {
		v2p.color = float4(input.color);
	}
	v2p.uv = input.uv;
	return v2p;
}

ps_output_t PixelMain(v2p_t input)
{
	float4 textureColor = diffuseTexture.Sample(diffuseSampler, input.uv);
	float4 color = input.color * textureColor;

	clip(color.a - 0.01f);
	ps_output_t retValue;
	retValue.color = color;
	return retValue;
}

