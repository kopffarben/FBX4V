
#include <packs/mp.fxh/CSThreadDefines.fxh>
#include <packs/mp.fxh/RWByteAddressBufferUtils.fxh>

#define VERTCOUNT ((uint)InstSize / (uint)OutVertSize)

RWByteAddressBuffer Outbuf : BACKBUFFER;
ByteAddressBuffer ShapeData;
#define SHAPEVERTSIZE 24
ByteAddressBuffer ShapeMeta;
#define SHAPEMETASIZE 16
ByteAddressBuffer Weights;
/*  weights:
 *  	weight[shapecount]: (8)
 *			float current
 *			float previous
 */

cbuffer uni : register(b0)
{
	int OutVertSize : OVERTSIZE;
	int InstSize : INSTSIZE;
	int SSMIDOffs : VOA_SUBSETMATID;
	int tanoffs : VOA_TANGENT;
	int binoffs : VOA_BINORMAL;
	int SSVID : VOA_SUBSETVERTEXID;
	int secoffs : SECONDARYOFFS;
}
float tempw = 0;

struct csin
{
	uint3 DTID : SV_DispatchThreadID;
	uint3 GTID : SV_GroupThreadID;
	uint3 GID : SV_GroupID;
};

[numthreads(XTHREADS, YTHREADS, ZTHREADS)]
void CS(csin input)
{
	if(input.DTID.x > VERTCOUNT) return;
	uint iva = input.DTID.x * OutVertSize;
	uint ova = iva + input.DTID.z * InstSize;
	uint ssid = Outbuf.Load(ova + SSMIDOffs) & 0x0000FFFF;
	uint4 smeta = ShapeMeta.Load4(ssid * SHAPEMETASIZE);
	if(smeta.z == 0)
	{
		if(smeta.w == 0) return;
		
		Outbuf.Store3(ova + 0, asuint(float3(0,0,0)));
		Outbuf.Store3(ova + 12, asuint(float3(0,0,0)));
		#if defined(HAS_PREVPOS)
		Outbuf.Store3(ova + 24, asuint(float3(0,0,0)));
		#endif
		#if defined(HAS_TANGENT)
		Outbuf.Store3(ova + tanoffs, asuint(float3(0,0,0)));
		Outbuf.Store3(ova + binoffs, asuint(float3(0,0,0)));
		#endif
		return;
	}
	
	
	float3 ipos = asfloat(Outbuf.Load3(ova + 0));
	float3 inorm = asfloat(Outbuf.Load3(ova + 12));
	
	float3 opos = ipos;
	float3 onorm = inorm;
	#if defined(HAS_PREVPOS)
	float3 oppos = ipos;
	#endif
	
	uint spvmi = Outbuf.Load(ova + SSVID) * SHAPEVERTSIZE + smeta.x;
	for(uint i=0; i<smeta.z; i++)
	{
		uint sva = spvmi + i * smeta.y;
		uint wa = i * 8 + input.DTID.z * smeta.z * 8;
		
		float weight = asfloat(Weights.Load(wa));
		//float weight = tempw;
		float3 posdelta = asfloat(ShapeData.Load3(sva + 0)) - ipos;
		float3 normdelta = asfloat(ShapeData.Load3(sva + 12)) - inorm;
		opos += posdelta * weight;
		onorm += normdelta * weight;
		
		#if defined(HAS_PREVPOS)
		float pweight = asfloat(Weights.Load(wa + 4));
		oppos += posdelta * pweight;
		#endif
	}
	
	Outbuf.Store3(ova + 0 + secoffs, asuint(opos));
	Outbuf.Store3(ova + 12 + secoffs, asuint(onorm));
	#if defined(HAS_PREVPOS)
	Outbuf.Store3(ova + 24 + secoffs, asuint(oppos));
	#endif
	if(smeta.w != 0)
	{
		Outbuf.Store3(ova + 0, asuint(float3(0,0,0)));
		Outbuf.Store3(ova + 12, asuint(float3(0,0,0)));
		#if defined(HAS_PREVPOS)
		Outbuf.Store3(ova + 24, asuint(float3(0,0,0)));
		#endif
		#if defined(HAS_TANGENT)
		Outbuf.Store3(ova + tanoffs, asuint(float3(0,0,0)));
		Outbuf.Store3(ova + binoffs, asuint(float3(0,0,0)));
		#endif
	}
	else
	{
		Outbuf.Store3(ova + 0, asuint(opos));
		Outbuf.Store3(ova + 12, asuint(onorm));
		#if defined(HAS_PREVPOS)
		Outbuf.Store3(ova + 24, asuint(oppos));
		#endif
	}
}
technique11 cst { pass P0{SetComputeShader( CompileShader( cs_5_0, CS() ) );} }