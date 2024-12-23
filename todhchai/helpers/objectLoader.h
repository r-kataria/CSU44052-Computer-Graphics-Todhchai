// objectLoader.h

#ifndef OBJECTLOADER_H
#define OBJECTLOADER_H

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <unordered_map>

// Structure to hold material information
struct Material {
    std::string name;
    std::string diffuse_texname;
    // Add other material properties if needed
};

// A class to handle OBJ loading using tinyobjloader's ObjReader
class ObjectLoader {
public:
    bool LoadOBJ(const std::string& filename,
                std::vector<glm::vec3>& vertices,
                std::vector<glm::vec2>& uvs,
                std::vector<glm::vec3>& normals,
                std::vector<unsigned int>& indices,
                std::vector<Material>& materials,
                std::unordered_map<int, std::vector<unsigned int>>& materialToIndices);
};

#endif // OBJECTLOADER_H
