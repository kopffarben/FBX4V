
#include <packs/mp.fxh/CSThreadDefines.fxh>
#include <packs/mp.fxh/ByteAddressBufferUtils.fxh>
#include <packs/mp.fxh/RWByteAddressBufferUtils.fxh>

RWByteAddressBuffer Outbuf : BACKBUFFER;
ByteAddressBuffer VertexData;
ByteAddressBuffer CompSrcDst;

cbuffer cbuf
{
    uint SubsetID = 0;
	uint VertSize = 100;
	uint VertOffs = 0;
	uint VertCount = 100;
	bool HasSkinning;
	bool HasBlendShapes;
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
	if(input.DTID.x > VertCount) return;
}
technique11 cst { pass P0{SetComputeShader( CompileShader( cs_5_0, CS() ) );} }