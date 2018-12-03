//--------------------------------------------------------------------------------------
// File: Grating.fx
// Pixel Shader
//--------------------------------------------------------------------------------------

extern const float width  = 166.0f;
extern const float height = 166.0f;

//     vvvvvvv function name must match the file name
float4 Grating( float4 Pos : SV_POSITION ) : SV_Target
{
    static const float pi = 3.141592654;
    float2 origin = {960.0f, 540.0f};
    if (distance(origin, Pos) > 83.0f) discard;
    float bright = (sin((Pos.x-960.0f)*(2.0f*pi)/14.0f) + 1.0f)/2.0f;
    return float4( bright, bright, bright, width/height );
}
