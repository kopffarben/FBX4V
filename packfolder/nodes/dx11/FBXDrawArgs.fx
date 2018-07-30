

RWByteAddressBuffer Outbuf : BACKBUFFER;

cbuffer cbuf
{
	uint DataEnd = 100;
	uint Count = 100;
	uint id = 0;
}

struct csin
{
	uint3 DTID : SV_DispatchThreadID;
	uint3 GTID : SV_GroupThreadID;
	uint3 GID : SV_GroupID;
};

[numthreads(1, 1, 1)]
void CSargs(csin input)
{
	Outbuf.Store(DataEnd + 0, Count);
	Outbuf.Store(DataEnd + 4, 1);
	Outbuf.Store(DataEnd + 8, 0);
	Outbuf.Store(DataEnd + 12, 0);
	Outbuf.Store(DataEnd + 16, 0);
}
[numthreads(1, 1, 1)]
void CSoffs(csin input)
{
	Outbuf.Store(DataEnd + id * 4, Count);
}
technique11 args { pass P0{SetComputeShader( CompileShader( cs_5_0, CSargs() ) );} }
technique11 offs { pass P0{SetComputeShader( CompileShader( cs_5_0, CSoffs() ) );} }