
#include <packs/mp.fxh/CSThreadDefines.fxh>

#define VERTCOUNT ((uint)InstSize / (uint)OutVertSize)

RWByteAddressBuffer Outbuf : BACKBUFFER;
ByteAddressBuffer VertexData;

cbuffer uni : register(b0)
{
	int OutVertSize : OVERTSIZE;
	int InstSize : INSTSIZE;
	int TanOffs : VOA_TANGENT;
	int BinOffs : VOA_BINORMAL;
	int secoffs : SECONDARYOFFS;
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
	uint iva = input.DTID.x * OutVertSize;
	uint ova = iva + input.DTID.z * InstSize;
	uint3 pos = VertexData.Load3(iva + 0);
	uint3 norm = VertexData.Load3(iva + 12);
	Outbuf.Store3(ova + 0, pos);
	Outbuf.Store3(ova + secoffs + 0, pos);
	Outbuf.Store3(ova + 12, norm);
	Outbuf.Store3(ova + secoffs + 12, norm);
	#if defined(HAS_PREVPOS)
	Outbuf.Store3(ova + 24, pos);
	Outbuf.Store3(ova + secoffs + 24, pos);
	#endif
	#if defined(HAS_TANGENT)
	uint3 tang = VertexData.Load3(iva + TanOffs);
	uint3 binorm = VertexData.Load3(iva + BinOffs);
	Outbuf.Store3(ova + TanOffs, pos);
	Outbuf.Store3(ova + secoffs + TanOffs, pos);
	Outbuf.Store3(ova + BinOffs, binorm);
	Outbuf.Store3(ova + secoffs + BinOffs, binorm);
	#endif
}
technique11 cst { pass P0{SetComputeShader( CompileShader( cs_5_0, CS() ) );} }