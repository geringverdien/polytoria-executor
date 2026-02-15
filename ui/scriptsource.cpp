#include <ui/scriptsource.h>


// +--------------------------------------------------------+
// |                       Variables                        |
// +--------------------------------------------------------+
std::vector<ScriptSourceUI::ScriptDecompileTab> ScriptSourceUI::openTabs;

// +--------------------------------------------------------+
// |                     User Interface                     |
// +--------------------------------------------------------+
#include <ptoria/scriptservice.h>
#include <core/filesys.h>

void ScriptSourceUI::DrawTab(ScriptDecompileTab *tab)
{
    if (!tab->isEditorReady)
    {
        tab->editor = new TextEditor();
        std::string source = ScriptService::ScriptSource(tab->instance);
        tab->editor->SetText(source);
        tab->editor->SetLanguageDefinition(TextEditor::LanguageDefinitionId::Lua);
        tab->isEditorReady = true;
    }
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open Script..."))
            {
                auto filters = std::vector<filesys::FileSelectFilters>{
                    {"Lua Scripts", "lua"},
                    {"Text Files", "txt"},
                };
                auto result = filesys::OpenDialog(filters);
                if (result)
                {
                    std::string scriptContent = filesys::ReadFileAsString(*result).value_or("");
                    tab->editor->SetText(scriptContent);
                }
            }
            if (ImGui::MenuItem("Save Script..."))
            {
                auto filters = std::vector<filesys::FileSelectFilters>{
                    {"Lua Scripts", "lua"},
                    {"Text Files", "txt"},
                };
                auto result = filesys::SaveDialog(filters);
                if (result)
                {
                    std::string scriptContent = tab->editor->GetText();
                    filesys::WriteStringToFile(*result, scriptContent);
                }
            }

            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    if (tab->instance)
    {
        ImGui::Text(("Script Name: " + tab->instance->Name()->ToString()).c_str());
        ImGui::Text(("Running: " + std::string(tab->instance->Running() ? "Yes" : "No")).c_str());
        ImGui::Text("Requested Run: %s", tab->instance->RequestedRun() ? "Yes" : "No");
    }
    ImGui::Text("[WARNING] Can't save script to game yet idk why it doesnt work");
    if (ImGui::Button("Save Changes To Game"))
    {
        if (tab->instance)
        {
            if (!tab->isEditorReady)
            {
                std::cout << "Editor not ready, cannot save changes" << std::endl;
                return;
            }
            if (tab->instance->Running())
            {
                tab->instance->SetRunning(false);
            }

            std::string updatedSource = tab->editor->GetText();
            tab->instance->SetSource(UnityString::New(updatedSource));
            //polytoria::ScriptService::RunScript(tab->instance);

        }
    }
    ImGui::Separator();
    tab->editor->Render(("Decompiled Script: " + tab->instance->Name()->ToString()).c_str());
}

bool ScriptSourceUI::IsTabAlreadyOpen(BaseScript *instance)
{
    for (const auto &tab : openTabs)
    {
        if (tab.instance == instance)
            return true;
    }
    return false;
}

void ScriptSourceUI::OpenNewScriptDecompileTab(BaseScript *instance)
{
    openTabs.push_back({instance});
}
