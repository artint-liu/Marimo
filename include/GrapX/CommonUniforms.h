float4x4 MARIMO_MATRIX_M; // Object to world
float4x4 MARIMO_MATRIX_V; // view matrix
float4x4 MARIMO_MATRIX_P; // projection matrix

float4x4 MARIMO_MATRIX_MVP;
float4x4 MARIMO_MATRIX_VP;
float4x4 MARIMO_MATRIX_I_VP;

float4 _CameraWorldPos; // 相机世界坐标(float3)
float4 _CameraWorldDir; // 相机方向(float3)

float4 _Time;