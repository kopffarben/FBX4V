
#include <packs/mp.fxh/CSThreadDefines.fxh>

#define VERTCOUNT ((uint)InstSize / (uint)OutVertSize)

RWByteAddressBuffer Outbuf : BACKBUFFER;
ByteAddressBuffer VertexData;

cbuffer uni : register(b0)
{
	int OutVertSize : OVERTSIZE;
	int secoffs : SECONDARYOFFS;
	int InstSize : INSTSIZE;
	int IIDOffs : VOA_INSTANCEID;
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
	if(input.DTID.x > VERTCOUNT) return;
	uint ova = input.DTID.x * OutVertSize;
	uint ca = input.DTID.y * 4;
	uint data = VertexData.Load(ova + ca);
	Outbuf.Store(ova + ca + InstSize * input.DTID.z, data);
	Outbuf.Store(ova + ca + InstSize * input.DTID.z + secoffs, data);
	
	#if defined(HAS_INSTANCEID)
	if(ca == IIDOffs)
		Outbuf.Store(ova + ca + InstSize * input.DTID.z, input.DTID.z);
		Outbuf.Store(ova + ca + InstSize * input.DTID.z + secoffs, input.DTID.z);
	#endif
}
technique11 cst { pass P0{SetComputeShader( CompileShader( cs_5_0, CS() ) );} }