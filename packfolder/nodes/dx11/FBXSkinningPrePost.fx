
#include <packs/mp.fxh/CSThreadDefines.fxh>

RWByteAddressBuffer Outbuf : BACKBUFFER;

cbuffer uni : register(b0)
{
	int OutVertSize : OVERTSIZE;
	int InstSize : INSTSIZE;
	int secoffs : SECONDARYOFFS;
	int tanoffs : VOA_TANGENT;
	int binoffs : VOA_BINORMAL;
}
cbuffer vari : register(b1)
{
	uint OutVertOffs = 0;
	uint VertCount = 100;
	bool HasTangents = false;
	bool HasSkinning;
}

struct csin
{
	uint3 DTID : SV_DispatchThreadID;
	uint3 GTID : SV_GroupThreadID;
	uint3 GID : SV_GroupID;
};

[numthreads(XTHREADS, YTHREADS, ZTHREADS)]
void CSpre(csin input)
{
	if(!HasSkinning) return;
	if(input.DTID.x > VertCount) return;
	uint ova = OutVertOffs + input.DTID.x * OutVertSize + input.DTID.z * InstSize;
	uint3 ipos = Outbuf.Load3(ova + 0);
	uint3 inorm = Outbuf.Load3(ova + 12);
	Outbuf.Store3(ova + secoffs + 0, ipos);
	Outbuf.Store3(ova + secoffs + 12, inorm);
	Outbuf.Store3(ova + 0, asuint(float3(0,0,0)));
	Outbuf.Store3(ova + 12, asuint(float3(0,0,0)));
	#if defined(PREVPOS)
	uint3 ippos = Outbuf.Load3(ova + 24);
	Outbuf.Store3(ova + secoffs + 24, ippos);
	Outbuf.Store3(ova + 24, asuint(float3(0,0,0)));
	#endif
}
[numthreads(XTHREADS, YTHREADS, ZTHREADS)]
void CSpost(csin input)
{
	if(!HasSkinning) return;
	if(input.DTID.x > VertCount) return;
	uint ova = OutVertOffs + input.DTID.x * OutVertSize + input.DTID.z * InstSize;
	float3 norm = normalize(asfloat(Outbuf.Load3(ova + 12)));
	Outbuf.Store3(ova + 12, asuint(norm));
	if(HasTangents)
	{
		float3 tang = normalize(asfloat(Outbuf.Load3(ova + tanoffs)));
		float3 binorm = normalize(asfloat(Outbuf.Load3(ova + binoffs)));
		Outbuf.Store3(ova + tanoffs, asuint(tang));
		Outbuf.Store3(ova + binoffs, asuint(binorm));
	}
}
technique11 pre { pass P0{SetComputeShader( CompileShader( cs_5_0, CSpre() ) );} }
technique11 post { pass P0{SetComputeShader( CompileShader( cs_5_0, CSpost() ) );} }