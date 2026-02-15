#include <ui/explorer.h>
#include <ptoria/game.h>

// +--------------------------------------------------------+
// |                       Variables                        |
// +--------------------------------------------------------+
Instance *ExplorerUI::selectedInstance = nullptr;



// +--------------------------------------------------------+
// |                       Functions                        |
// +--------------------------------------------------------+
void RenderInstanceTree(Instance* instance);

// +--------------------------------------------------------+
// |                   Explorer Interface                   |
// +--------------------------------------------------------+
#include <imgui.h>

void ExplorerUI::Init()
{
}

void ExplorerUI::DrawTab()
{
    static Game *gameInstance = Game::GetSingleton();
    if (!gameInstance)
        return;

    float availableHeight = ImGui::GetContentRegionAvail().y;

    // Top part: Tree Explorer
    ImGui::BeginChild("TreeRegion", ImVec2(0, availableHeight * 0.6f), true);
    RenderInstanceTree((Instance*)gameInstance);
    ImGui::EndChild();

    ImGui::Separator();

    // Bottom part: Properties Explorer
    ImGui::BeginChild("PropertiesRegion", ImVec2(0, 0), true);
    if (selectedInstance)
    {
        ImGui::TextColored(ImVec4(1, 0.8f, 0, 1), "Properties: %s", selectedInstance->Name()->ToString().c_str());
        ImGui::Separator();

        if (ImGui::BeginTable("PropertiesTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
        {
            ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();

            // Name
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Name");
            ImGui::TableSetColumnIndex(1);
            std::string nameStr = selectedInstance->Name()->ToString();
            if (ImGui::Selectable(nameStr.c_str()))
                ImGui::SetClipboardText(nameStr.c_str());
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Click to copy: %s", nameStr.c_str());

            // Type
            auto selectedInstanceObject = Unity::CastToUnityObject(selectedInstance);
            auto type = selectedInstanceObject->GetType();
            if (type)
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Class");
                ImGui::TableSetColumnIndex(1);
                std::string classStr = type->GetFullNameOrDefault()->ToString();
                if (ImGui::Selectable(classStr.c_str()))
                    ImGui::SetClipboardText(classStr.c_str());
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Click to copy: %s", classStr.c_str());

                if (type->GetFullNameOrDefault()->ToString() == "Polytoria.Datamodel.LocalScript" || type->GetFullNameOrDefault()->ToString() == "Polytoria.Datamodel.Script" || type->GetFullNameOrDefault()->ToString() == "Polytoria.Datamodel.ModuleScript" || type->GetFullNameOrDefault()->ToString() == "Polytoria.Datamodel.ScriptInstance")
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("Script Source");
                    ImGui::TableSetColumnIndex(1);
                    if (ImGui::Button("View Source"))
                    {
                       
                    }
                }
            }

            // Memory Address
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Address");
            ImGui::TableSetColumnIndex(1);
            char addrBuf[32];
            snprintf(addrBuf, sizeof(addrBuf), "0x%p", selectedInstance);
            if (ImGui::Selectable(addrBuf))
                ImGui::SetClipboardText(addrBuf);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Click to copy: %s", addrBuf);

            // Full Name
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Full Name");
            ImGui::TableSetColumnIndex(1);
            std::string fullNameStr = selectedInstance->FullName()->ToString();
            if (ImGui::Selectable(fullNameStr.c_str()))
                ImGui::SetClipboardText(fullNameStr.c_str());
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Click to copy: %s", fullNameStr.c_str());

            ImGui::EndTable();
        }
    }
    else
    {
        ImGui::TextDisabled("Select an instance to see its properties");
    }
    ImGui::EndChild();
}

// +--------------------------------------------------------+
// |                        Helpers                         |
// +--------------------------------------------------------+
void RenderInstanceTree(Instance* instance)
{
    if (!instance) return;

    // flags to make it selectable and span the full width of the tree
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

    auto children = instance->Children();
    bool hasChildren = children && children->max_length > 0;

    if (!hasChildren)
        flags |= ImGuiTreeNodeFlags_Leaf;

    bool opened = ImGui::TreeNodeEx(instance, flags, "%s", instance->Name()->ToString().c_str());

    if (ImGui::IsItemClicked())
    {
        ExplorerUI::selectedInstance = instance;
    }

    if (opened)
    {
        if (hasChildren)
        {
            for (auto child : children->ToVector())
            {
                // An InstanceBase , is basically just an instance, so we can upcast
                RenderInstanceTree((Instance*)child);
            }
        }
        ImGui::TreePop();
    }
}