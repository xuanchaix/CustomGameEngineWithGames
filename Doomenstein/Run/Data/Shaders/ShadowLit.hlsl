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
	float4 lightSpacePosition : L_Position;
	float3 lightRay : L_Ray;
	float3 cameraRay : C_Ray;
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
SamplerState diffuseSampler : register(s0);

//------------------------------------------------------------------------------------------------
SamplerComparisonState shadowSampler : register(s1);
Texture2D shadowMap : register(t1);

//------------------------------------------------------------------------------------------------
v2p_t VertexMain(vs_input_t input)
{
	float4 localPosition = float4(input.localPosition, 1);
	float4 worldPosition = mul(ModelMatrix, localPosition);
	float4 viewPosition = mul(ViewMatrix, worldPosition);
	float4 clipPosition = mul(ProjectionMatrix, viewPosition);
	float4 localNormal = float4(input.localNormal, 0);
	float4 worldNormal = mul(ModelMatrix, localNormal);
	float4 lightViewPosition = mul(LightViewMatrix, worldPosition);
    float4 lightClipPosition =  mul(LightProjectionMatrix, lightViewPosition);

	v2p_t v2p;
	v2p.position = clipPosition;
	v2p.color = input.color;
	v2p.uv = input.uv;
	v2p.tangent = float4(0, 0, 0, 0);
	v2p.bitangent = float4(0, 0, 0, 0);
	v2p.normal = worldNormal;
	v2p.lightSpacePosition = lightClipPosition;
	v2p.lightRay = LightPosition - worldPosition.xyz;
	v2p.cameraRay = float3(0, 0, 0);
	return v2p;
}

//------------------------------------------------------------------------------------------------
float4 PixelMain(v2p_t input) : SV_Target0
{
	// Compute texture coordinates for the current point's location on the shadow map.
	float2 shadowTexCoords;
	shadowTexCoords.x = 0.5f + (input.lightSpacePosition.x / input.lightSpacePosition.w * 0.5f);
	shadowTexCoords.y = 0.5f - (input.lightSpacePosition.y / input.lightSpacePosition.w * 0.5f);
	float pixelDepth = input.lightSpacePosition.z / input.lightSpacePosition.w;
	
	float lighting = 1.f;
	
	float4 lightColor;
	float NdotL = saturate(dot(normalize(input.lightRay), input.normal.xyz));
	// Check if the pixel texture coordinate is in the view frustum of the 
	// light before doing any shadow work.
	if ((saturate(shadowTexCoords.x) == shadowTexCoords.x) &&
	    (saturate(shadowTexCoords.y) == shadowTexCoords.y) &&
	    (pixelDepth > 0))
	{
		// Use an offset value to mitigate shadow artifacts due to imprecise 
		// floating-point values (shadow acne).
		//
		// This is an approximation of epsilon * tan(acos(saturate(NdotL))):
		float margin = acos(NdotL);
		//#ifdef LINEAR
		// The offset can be slightly smaller with smoother shadow edges.
		//float epsilon = 0.0005 / margin;
		//#else
		float epsilon = 0.001 / margin;
		//#endif
		// Clamp epsilon to a fixed range so it doesn't go overboard.
		epsilon = clamp(epsilon, 0.0, 0.01);
		
		// Use the SampleCmpLevelZero Texture2D method (or SampleCmp) to sample from 
		// the shadow map, just as you would with Direct3D feature level 10_0 and
		// higher.  Feature level 9_1 only supports LessOrEqual, which returns 0 if
		// the pixel is in the shadow.  + epsilon
		lighting = float(shadowMap.SampleCmpLevelZero(shadowSampler, shadowTexCoords, pixelDepth - epsilon));
		float w; float h;
		shadowMap.GetDimensions(w, h);
		lighting += float(shadowMap.SampleCmpLevelZero(shadowSampler, float2(clamp(shadowTexCoords.x + 1.0 / w, 0.0, 1.0), shadowTexCoords.y), pixelDepth - epsilon));
		lighting += float(shadowMap.SampleCmpLevelZero(shadowSampler, float2(clamp(shadowTexCoords.x - 1.0 / w, 0.0, 1.0), shadowTexCoords.y), pixelDepth - epsilon));
		lighting += float(shadowMap.SampleCmpLevelZero(shadowSampler, float2(shadowTexCoords.x, clamp(shadowTexCoords.y + 1.0 / h, 0.0, 1.0)), pixelDepth - epsilon));
		lighting += float(shadowMap.SampleCmpLevelZero(shadowSampler, float2(shadowTexCoords.x, clamp(shadowTexCoords.y - 1.0 / h, 0.0, 1.0)), pixelDepth - epsilon));
		//lighting += float(shadowMap.SampleCmpLevelZero(shadowSampler, float2(clamp(shadowTexCoords.x + 1.0 / w, 0.0, 1.0), clamp(shadowTexCoords.y + 1.0 / h, 0.0, 1.0)), pixelDepth - epsilon));
		//lighting += float(shadowMap.SampleCmpLevelZero(shadowSampler, float2(clamp(shadowTexCoords.x - 1.0 / w, 0.0, 1.0), clamp(shadowTexCoords.y + 1.0 / h, 0.0, 1.0)), pixelDepth - epsilon));
		//lighting += float(shadowMap.SampleCmpLevelZero(shadowSampler, float2(clamp(shadowTexCoords.x + 1.0 / w, 0.0, 1.0), clamp(shadowTexCoords.y - 1.0 / h, 0.0, 1.0)), pixelDepth - epsilon));
		//lighting += float(shadowMap.SampleCmpLevelZero(shadowSampler, float2(clamp(shadowTexCoords.x - 1.0 / w, 0.0, 1.0), clamp(shadowTexCoords.y - 1.0 / h, 0.0, 1.0)), pixelDepth - epsilon));
		lighting = lighting / 5.0;
		//float3 light = lighting * (ambient + saturate(NdotL));
		//float3 shadow = (1.f - lighting) * ambient;
		lightColor = float4((ambient + lighting * NdotL).xxx, 1.f);
	}
	else {
		lightColor = float4((ambient).xxx, 1);
	}

	float4 textureColor = diffuseTexture.Sample(diffuseSampler, input.uv);
	float4 vertexColor = input.color;
	float4 modelColor = ModelColor;
	float4 color = lightColor * textureColor * vertexColor * modelColor;
	clip(color.a - 0.01f);
	return color;
}
