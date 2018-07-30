
#include <packs/mp.fxh/CSThreadDefines.fxh>
#include <packs/mp.fxh/VertexFeatureFlags.fxh>
#include <packs/mp.fxh/ByteAddressBufferUtils.fxh>
#include <packs/mp.fxh/RWByteAddressBufferUtils.fxh>

#define VERTCOUNT ((uint)InstSize / (uint)OutVertSize)

RWByteAddressBuffer Outbuf : BACKBUFFER;

StructuredBuffer<float4x4> BoneMatrices;
StructuredBuffer<float4x4> PrevBoneMatrices;

ByteAddressBuffer WeightData;
#define WEIGHTSIZE 16
/*  skin:
 *	weights[vertexcount + pervertexweightdata]: (16)
 *		uint VertexID
 *		uint Mode (0: Normalize, 1: Additive, 2: TotalOne)
 *		uint BoneBindingInfoID
 *		float Weight
 */
ByteAddressBuffer BoneBindingInfo;
#define BBIELSIZE 68
/*
 *	uint BoneID
 *	float4x4 InverseBindingMatrix
 */

cbuffer uni : register(b0)
{
	int OutVertSize : OVERTSIZE;
	int InstSize : INSTSIZE;
	int secoffs : SECONDARYOFFS;
	int tanoffs : VOA_TANGENT;
	int binoffs : VOA_BINORMAL;
	int flagsoffs : VOA_FEATUREFLAGS;
	
}
cbuffer vari : register(b1)
{
	uint WeightCount = 100;
	uint BoneSetSize = 100;
}

struct csin
{
	uint3 DTID : SV_DispatchThreadID;
	uint3 GTID : SV_GroupThreadID;
	uint3 GID : SV_GroupID;
};

[numthreads(XTHREADS, YTHREADS, ZTHREADS)]
void CS(csin input)
{
	if(input.DTID.x > WeightCount) return;
	uint iwa = input.DTID.x * WEIGHTSIZE;
	uint4 wdata = WeightData.Load4(iwa);
	uint ova = wdata.x * OutVertSize + input.DTID.z * InstSize;
	float3 posin = asfloat(Outbuf.Load3(ova + secoffs + 0));
	float3 normin = asfloat(Outbuf.Load3(ova + secoffs + 12));
	float3 tangin = float3(1,0,0);
	float3 binormin = float3(0,1,0);
	bool hastans = (Outbuf.Load(ova + flagsoffs) & FLAG_TANGENT) > 0;
	if(hastans)
	{
		float3 tangin = asfloat(Outbuf.Load3(ova + secoffs + tanoffs));
		float3 binormin = asfloat(Outbuf.Load3(ova + secoffs + binoffs));
	}
	
	#if defined(HAS_PREVPOS)
	float3 pposin = asfloat(Outbuf.Load3(ova + secoffs + 24));
	#endif
	
	uint selbone = BoneBindingInfo.Load(wdata.z * BBIELSIZE);
    float4x4 invmat = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
    [unroll]
    for(uint ii=0; ii<4; ii++)
    {
        [unroll]
        for(uint jj=0; jj<4; jj++)
        {
            invmat[ii][jj] = asfloat(BoneBindingInfo.Load(wdata.z * BBIELSIZE + 4 + ii*16 + jj*4 ));
        }
    }
	float4x4 bonemat = mul(invmat, BoneMatrices[selbone + input.DTID.z * BoneSetSize]);
	
	float weight = asfloat(wdata.w);
	float3 posdelta = mul(float4(posin, 1), bonemat).xyz * weight;
	float3 normdelta = mul(float4(normin, 0), bonemat).xyz * weight;
	
	InterlockedAddFloat(Outbuf, ova + 0, posdelta);
	InterlockedAddFloat(Outbuf, ova + 12, normdelta);
	
	if(hastans)
	{
		float3 tandelta = mul(float4(tangin, 0), bonemat).xyz * weight;
		float3 bindelta = mul(float4(binormin, 0), bonemat).xyz * weight;
		InterlockedAddFloat(Outbuf, ova + tanoffs, tandelta);
		InterlockedAddFloat(Outbuf, ova + binoffs, bindelta);
	}
	
	#if defined(HAS_PREVPOS)
	float4x4 pbonemat = mul(invmat, PrevBoneMatrices[selbone + input.DTID.z * BoneSetSize]);
	float3 pposdelta = mul(float4(pposin, 1), pbonemat).xyz * weight;
	InterlockedAddFloat(Outbuf, ova + 24, pposdelta);
	#endif
}
technique11 cst { pass P0{SetComputeShader( CompileShader( cs_5_0, CS() ) );} }