

#include "Components.h"
#include "Entity.h"


class Shader {
    public:


        void setModel(vec3 trans, float rotZ, float rotY, float rotX, float sc);
        void setModel(std::shared_ptr<MatrixStack>M);
        void setModel(Entity entity);

        void initShader(const std::string &v, const std::string &f);
        void initTexVars();

        void addTexture(const std::string &f);

        void setMaterial(materials material);

        void flip(int f);

        void setTexture(int i);
        void unbindTexture(int i);

        
        Shader(const std::string &v, const std::string &f, bool has_tex);
        Shader();
        ~Shader();


        std::shared_ptr<Program> prog;
        bool has_texture;
	    std::vector<shared_ptr<Texture>> textures;

};
