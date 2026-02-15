#include <ui/scriptsource.h>


// +--------------------------------------------------------+
// |                       Variables                        |
// +--------------------------------------------------------+
std::vector<ScriptSourceUI::ScriptDecompileTab> ScriptSourceUI::openTabs;

// +--------------------------------------------------------+
// |                     User Interface                     |
// +--------------------------------------------------------+

void ScriptSourceUI::DrawTab(ScriptDecompileTab *tab)
{
    if (!tab->isEditorReady)
    {
        std::string source = polytoria::ScriptService::DecompileScript(tab->instance);
        tab->editor.SetText(source);
        tab->editor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Lua);
        tab->isEditorReady = true;
    }
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open Script..."))
            {
                auto filters = std::vector<FileSelectFilters>{
                    {"Lua Scripts", "lua"},
                    {"Text Files", "txt"},
                };
                auto result = OpenDialog(filters);
                if (result)
                {
                    std::string scriptContent = ReadFileAsString(*result).value_or("");
                    tab->editor.SetText(scriptContent);
                }
            }
            if (ImGui::MenuItem("Save Script..."))
            {
                auto filters = std::vector<FileSelectFilters>{
                    {"Lua Scripts", "lua"},
                    {"Text Files", "txt"},
                };
                auto result = SaveDialog(filters);
                if (result)
                {
                    std::string scriptContent = tab->editor.GetText();
                    WriteStringToFile(*result, scriptContent);
                }
            }

            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    if (tab->instance)
    {
        ImGui::Text(("Script Name: " + tab->instance->GetName()->ToString()).c_str());
        ImGui::Text(("Running: " + std::string(tab->instance->IsRunning() ? "Yes" : "No")).c_str());
        ImGui::Text("Requested Run: %s", tab->instance->GetRequestedRun() ? "Yes" : "No");
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
            if (tab->instance->IsRunning())
            {
                tab->instance->SetRunning(false);
            }

            std::string updatedSource = tab->editor.GetText();
            tab->instance->SetSource(US::New(updatedSource));
            //polytoria::ScriptService::RunScript(tab->instance);

        }
    }
    ImGui::Separator();
    tab->editor.Render(("Decompiled Script: " + tab->instance->GetName()->ToString()).c_str());
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
