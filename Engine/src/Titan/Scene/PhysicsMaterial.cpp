#include "PhysicsMaterial.h"


#include <yaml-cpp/yaml.h>

namespace Titan{

    void Physics2DMaterial::Save() {
        if(SourcePath.empty()){
            TI_CORE_WARN("Couldnt save Physics Material!");
            return;
        }
        try{
            YAML::Emitter out;
            out << YAML::BeginMap;
            out << YAML::Key << "Density" << YAML::Value << Density;
            out << YAML::Key << "Friction" << YAML::Value << Friction;
            out << YAML::Key << "Restitution" << YAML::Value << Restitution;
            out << YAML::Key << "RestitutionThreshold" << YAML::Value << RestitutionThreshold;
            out << YAML::EndMap;

                
            std::ofstream fout(SourcePath);        
            fout << out.c_str();

            TI_CORE_TRACE("Saved Physics Material {}", SourcePath.c_str());
        }catch (YAML::ParserException e){
            TI_CORE_ERROR("Couldnt save Physics Material: {}", e.msg.c_str());
        }
    }

    Ref<Physics2DMaterial> Physics2DMaterial::Create(const std::string& path)
    {
        auto mat = CreateRef<Physics2DMaterial>();
        if(path.empty()){
            TI_CORE_WARN("Couldnt load Physics Material!");
            return mat;
        }

        try{
            YAML::Node data = YAML::LoadFile(path);
            mat->Density = data["Density"].as<float>();
            mat->Friction =  data["Friction"].as<float>();
            mat->Restitution =  data["Restitution"].as<float>();
            mat->RestitutionThreshold =  data["RestitutionThreshold"].as<float>();

            mat->SourcePath = path;
            TI_CORE_TRACE("Loaded Physics Material {}", path.c_str());
        }catch (YAML::ParserException e){
            TI_CORE_ERROR("Couldnt load Physics Material: {}", e.msg.c_str());
        }
        return mat;
    }

}