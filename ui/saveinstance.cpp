#include <ui/saveinstance.h>


// +--------------------------------------------------------+
// |                     User Interface                     |
// +--------------------------------------------------------+
#include <imgui.h>
#include <core/filesys.h>
#include <spdlog/spdlog.h>
#include <ptoria/game.h>

void SaveInstanceUI::Init()
{
    
}

void SaveInstanceUI::DrawTab()
{
    ImGui::Text("Save Instance");

    if (ImGui::Button("Save To File"))
    {
        std::vector<filesys::FileSelectFilters> filters = {
            {"Polytoria Instance Files", "poly"},
        };
        auto savePathOpt = filesys::SaveDialog(filters);
        if (savePathOpt)
        {
            std::string savePath = *savePathOpt;
            spdlog::info("Saving instance to {}", savePath);
            GameIO::GetSingleton()->SaveToFile(savePath.c_str());
        }
        else
        {
            spdlog::info("Save cancelled or failed");
        }
    }
}