#version 430

layout(local_size_x = 16, local_size_y = 16) in;
layout (binding = 0, rgba32f) readonly uniform image2D dispMap; 
layout (binding = 1, rgba32f) writeonly uniform image2D normMap;





