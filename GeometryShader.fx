struct INPUT {
    float4 Position   : SV_POSITION;
    float4 Color      : COLOR;
    float  Size	      : PSIZE;
};
[maxvertexcount(1)]
void GS( point INPUT particle[1], inout PointStream<INPUT> pStream )
{
	 INPUT v1;
	 v1.Position = particle[0].Position;
	 v1.Color = particle[0].Color;
	 v1.Size = particle[0].Size;
	 pStream.Append(v1);
}
