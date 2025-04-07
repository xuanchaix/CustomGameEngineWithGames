

Texture2D sourceTexture : register(t0);
SamplerState sourceSampler : register(s0);

struct vs_input_t
{
	float3 localPosition : POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
};

struct v2p_t
{
	float4 position : SV_Position;
	float2 uv : TEXCOORD;
};

struct BlurSample{
	float2 offset;
	float weight;
	float padding;
};

#define MAX_SAMPLES 64
cbuffer BlurConstants : register(b5){
	float2 texelSize;
	float lerpT;
	int numOfSamples;
	BlurSample samples[MAX_SAMPLES];
};

v2p_t VertexMain(vs_input_t input)
{
	v2p_t v2p;
	v2p.position = float4(input.localPosition, 1.f);
	v2p.uv = input.uv;
	return v2p;
}

float4 PixelMain(v2p_t input) : SV_Target0
{
	float4 color = float4(0.f, 0.f, 0.f, 1.f);
	for(int i = 0; i < numOfSamples; i++){
		color.rgb += sourceTexture.Sample(sourceSampler, input.uv + samples[i].offset * texelSize).rgb * samples[i].weight;
	}

	return color;
}