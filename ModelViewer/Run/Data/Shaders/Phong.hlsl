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
	float4 color : COLOR;
	float2 uv : TEXCOORD;
	float4 tangent : TANGENT;
	float4 bitangent : BITANGENT;
	float4 normal : NORMAL;
};

struct ps_output_t{
	float4 color : SV_Target0;
	float4 emissive: SV_Target1;
};

//------------------------------------------------------------------------------------------------
cbuffer LightConstants : register(b1)
{
	float3 SunDirection;
	float SunIntensity;
	float AmbientIntensity;
	float3 WorldEyePosition;

	float MinFalloff;
	float MaxFalloff;
	float MinFalloffMultiplier;
	float MaxFalloffMultiplier;
	int RenderAmbient;
	int RenderDiffuse;
	int RenderSpecular;
	int RenderEmissive;
	int UseDiffuseMap;
	int UseNormalMap;
	int UseSpecularMap;
	int UseGlossinessMap;
	int UseEmissiveMap;
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

//------------------------------------------------------------------------------------------------
Texture2D diffuseTexture : register(t0);
//------------------------------------------------------------------------------------------------
Texture2D normalTexture : register(t1);
//------------------------------------------------------------------------------------------------
Texture2D specGlossEmitTexture : register(t2);

//------------------------------------------------------------------------------------------------
SamplerState diffuseSampler : register(s0);

//------------------------------------------------------------------------------------------------
v2p_t VertexMain(vs_input_t input)
{
	float4 localPosition = float4(input.localPosition, 1);
	float4 worldPosition = mul(ModelMatrix, localPosition);
	float4 viewPosition = mul(ViewMatrix, worldPosition);
	float4 clipPosition = mul(ProjectionMatrix, viewPosition);
	float4 localNormal = float4(input.localNormal, 0);
	float4 worldNormal = mul(ModelMatrix, localNormal);
	float4 localTangent = float4(input.localTangent, 0);
	float4 worldTangent = mul(ModelMatrix, localTangent);
	float4 localBitangent = float4(input.localBitangent, 0);
	float4 worldBitangent = mul(ModelMatrix, localBitangent);

	v2p_t v2p;
	v2p.position = clipPosition;
	v2p.color = input.color;
	v2p.uv = input.uv;
	v2p.tangent = worldTangent;
	v2p.bitangent = worldBitangent;
	v2p.normal = worldNormal;
	return v2p;
}

//------------------------------------------------------------------------------------------------
ps_output_t PixelMain(v2p_t input)
{
	float3 L = normalize(-SunDirection);
	float3 specGlossEmitColor = specGlossEmitTexture.Sample(diffuseSampler, input.uv).xyz;
	float3 normalColor = normalTexture.Sample(diffuseSampler, input.uv).xyz;
	normalColor = (normalColor - float3(0.5f, 0.5f, 0.5f) ) * 2.f;
	float3 N;
	if(UseNormalMap > 0){
		N = normalize(input.normal * normalColor.z + input.tangent * normalColor.x + input.bitangent * normalColor.y).xyz;
	}
	else{
		N = normalize(input.normal).xyz;
	}
	float lambertian = max(dot(L, N), 0.f);
	float specular = 0.f;
	float specularPower = 0.f; 

	specularPower = specGlossEmitColor.g * UseGlossinessMap * 31.f + 1.f;

	float specularIntensity = specGlossEmitColor.r * UseSpecularMap;
	if(lambertian > 0.f && RenderSpecular > 0){
		float3 V = normalize(WorldEyePosition - input.position.xyz);
		float3 H = normalize(L + V);
		float specAngle = max(dot(N, H), 0.f);
		float specMultiplier = specAngle > 0.1f ? 1.f : specAngle * 10.f;
		specular = specularIntensity * pow(specAngle, specularPower) * specMultiplier;
	}

	float4 emissive;
	if(UseEmissiveMap){
		emissive = float4(specGlossEmitColor.bbb, 1);
	}
	else{
		emissive = float4(1, 1, 1, 1);
	}

	float ambient =  AmbientIntensity * RenderAmbient;
	emissive = emissive * RenderEmissive;
	
	float directionalNdotL = saturate(dot(N, L));
	float directionalMultiplier = directionalNdotL > 0.1f ? 1.f : directionalNdotL * 10.f;
	float directional = RenderDiffuse * SunIntensity * directionalNdotL * directionalMultiplier;

	float4 lightColor = float4((saturate(emissive + ambient + directional + specular)).xxx, 1);
	float4 textureColor = diffuseTexture.Sample(diffuseSampler, input.uv) * UseDiffuseMap;
	float4 vertexColor = input.color;
	float4 modelColor = ModelColor;
	float4 color = lightColor * textureColor * vertexColor * modelColor;
	clip(color.a - 0.01f);
	ps_output_t output;
	output.color = color;
	output.emissive = emissive *  textureColor * vertexColor * modelColor;
	return output;
}
