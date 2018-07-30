//@author: vux
//@help: standard constant shader
//@tags: color
//@credits: 

struct vsInput
{
	uint vid : SV_VertexID;
};

struct psInput
{
    float4 posScreen : SV_Position;
};

ByteAddressBuffer MeshData;

cbuffer cbPerDraw : register(b0)
{
	float4x4 tVP : VIEWPROJECTION;
};

cbuffer cbPerObj : register( b1 )
{
	float4x4 tW : WORLD;
	uint VertSize = 100;
};

cbuffer cbTextureData : register(b2)
{
	float4x4 tTex <string uiname="Texture Transform"; bool uvspace=true; >;
};

psInput VS(vsInput input)
{
	psInput output;
	float3 pos = asfloat(MeshData.Load3(input.vid * VertSize));
	
	output.posScreen = mul(float4(pos, 1),mul(tW,tVP));
	return output;
}


float4 PS(psInput input): SV_Target
{
    float4 col = 1;
    return col;
}


technique11 ConstantNoTexture
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_4_0, VS() ) );
		SetPixelShader( CompileShader( ps_4_0, PS() ) );
	}
}





