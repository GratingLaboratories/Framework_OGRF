-- Lua script.
p=tetview:new()
p:load_mesh("C:/Users/luwuy/Source/Repos/MeshCompression/MeshCompression/scene/test/tetra/horse.ele")
rnd=glvCreate(0, 0, 500, 500, "TetView")
p:plot(rnd)
glvWait()
