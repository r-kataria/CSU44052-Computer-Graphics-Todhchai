// objectLoader.cpp

#define TINYOBJLOADER_IMPLEMENTATION
#include "objectLoader.h"
#include "tiny_obj_loader.h"
#include <iostream>
#include <unordered_map>

bool ObjectLoader::LoadOBJ(const std::string& filename,
                           std::vector<glm::vec3>& vertices,
                           std::vector<glm::vec2>& uvs,
                           std::vector<glm::vec3>& normals,
                           std::vector<unsigned int>& indices,
                           std::vector<Material>& materials,
                           std::unordered_map<int, std::vector<unsigned int>>& materialToIndices)
{
    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(filename)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
        return false;
    }

    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& mats = reader.GetMaterials();

    std::cout << "Successfully loaded OBJ file: " << filename << std::endl;

    // Load materials
    for (size_t m = 0; m < mats.size(); ++m) {
        Material material;
        material.name = mats[m].name;
        material.diffuse_texname = mats[m].diffuse_texname;
        // Load other material properties if needed
        materials.push_back(material);
    }

    std::cout << "Number of materials loaded: " << materials.size() << std::endl;

    // Iterate over shapes
    for (const auto& shape : shapes) {
        size_t index_offset = 0;

        // Loop over faces(polygon)
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
            size_t fv = size_t(shape.mesh.num_face_vertices[f]);

            // Only handle triangles
            if (fv != 3) {
                std::cerr << "Non-triangulated face detected. Skipping..." << std::endl;
                // Implement triangulation if needed or skip
                index_offset += fv;
                continue;
            }

            // Get the material id for this face
            int material_id = shape.mesh.material_ids[f];
            if (material_id < 0 || material_id >= static_cast<int>(materials.size())) {
                material_id = 0; // Default to first material if invalid
            }

            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++) {
                tinyobj::index_t idx = shape.mesh.indices[index_offset + v];

                // Vertex positions
                glm::vec3 vertex;
                vertex.x = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                vertex.y = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                vertex.z = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
                vertices.push_back(vertex);

                // UVs
                if (idx.texcoord_index >= 0) {
                    glm::vec2 uv;
                    uv.x = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                    uv.y = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
                    uvs.push_back(uv);
                } else {
                    uvs.emplace_back(0.0f, 0.0f); // Default UV
                }

                // Normals
                if (idx.normal_index >= 0) {
                    glm::vec3 normal;
                    normal.x = attrib.normals[3 * size_t(idx.normal_index) + 0];
                    normal.y = attrib.normals[3 * size_t(idx.normal_index) + 1];
                    normal.z = attrib.normals[3 * size_t(idx.normal_index) + 2];
                    normals.push_back(normal);
                } else {
                    normals.emplace_back(0.0f, 0.0f, 0.0f); // Default normal
                }

                // Indices
                unsigned int currentIndex = static_cast<unsigned int>(indices.size());
                indices.push_back(currentIndex);

                // Map index to material
                materialToIndices[material_id].push_back(currentIndex);
            }
            index_offset += fv;
        }
    }

    // Debug: Print loaded data counts
    std::cout << "Loaded " << vertices.size() << " vertices." << std::endl;
    std::cout << "Loaded " << uvs.size() << " UV coordinates." << std::endl;
    std::cout << "Loaded " << normals.size() << " normals." << std::endl;
    std::cout << "Loaded " << indices.size() << " indices." << std::endl;

    return true;
}
