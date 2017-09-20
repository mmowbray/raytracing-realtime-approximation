#ifndef RAYTRACING_REALTIME_APPROXIMATION_OBJLOADER_H
#define RAYTRACING_REALTIME_APPROXIMATION_OBJLOADER_H


#include <glm/vec3.hpp>

class OBJLoader {

public:
    static void loadOBJ(const std::string &source, std::vector<glm::vec3> & vertices, std::vector<glm::vec3> & normals);

};


#endif //RAYTRACING_REALTIME_APPROXIMATION_OBJLOADER_H
