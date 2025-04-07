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

	cbuffer FontRNGConstants : register(b10)
	{
		float time;
		float bottomY;
	};

	Texture2D diffuseTexture : register(t0);
	Texture2D noiseTexture : register(t1);
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

	float SmoothStop2( float t )
	{
		float s = 1.f - t;
		return 1.f - (s * s);
	}
	
	float SmoothStart2( float t )
	{
		return t * t;
	}

	float4 PixelMain(v2p_t input) : SV_Target0
	{
		float4 textureColor = diffuseTexture.Sample(diffuseSampler, input.uv);
		float timePeriod = 10.f;
		float halfTimePeriod = timePeriod * 0.5f;
		float modTime1 = (abs(fmod(time * 0.04f, 1.f) - 0.5f));
		float modTime2 = (abs(fmod(time * 0.04f, 1.f) - 0.5f)) * 2.f - 0.5f;
		float4 noiseColor = noiseTexture.Sample(diffuseSampler, float2( input.uv.x , modTime1 + input.uv.y * 0.5f));
		float modTime = fmod(time, timePeriod);
		modTime = abs(modTime - halfTimePeriod) / halfTimePeriod;
		modTime = SmoothStop2(modTime) * 0.7f + 0.3f;
		float inputHeight = (400.f - input.position.y) * 0.04f;
		inputHeight = 1.f - clamp(inputHeight, 0.f, 1.f);
		inputHeight = SmoothStart2(inputHeight);
		if(textureColor.a){
			textureColor.a = modTime * textureColor.a + (1.f - modTime) * textureColor.a * inputHeight;
			textureColor.a = clamp(textureColor.a, 0.f, 1.f);
		}
		if(textureColor.a){
			textureColor.a = textureColor.a + (noiseColor.r - 0.5f) * 2.f * (1.f - textureColor.a);
			textureColor.a = clamp(textureColor.a, 0.f, 1.f);
		}
		
		clip(textureColor.a - 0.3f);
		if(textureColor.a < 0.70f){
			textureColor = float4(255, 0, 0, 255);
		}
		else{
			textureColor = float4(255, 255, 0, 255);
		}
		float4 color = textureColor;
		//color = pow(color, 1.f / 2.2f);
		return color;
	}