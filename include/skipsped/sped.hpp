#ifndef SPED
#define SPED

#include <skipsped/state_machine.hpp>
#include <imgui/TextEditor.h>
#include <d3d11.h>
#include <vector>
#include <string>
#include <optional>

namespace polytoria
{
    struct Instance;
    // sorry daddy nasec
    class BaseScript;
}

namespace sped
{
    void main_thread();

    /**
     * @brief Represents the cheat
     *
     */
    class Sped
    {
    public:
        SpedState state;
        polytoria::Instance *selectedInstance = nullptr;

        Sped() : state(SpedState::X) {}

        static Sped &GetInstance()
        {
            static Sped instance;
            return instance;
        }

        ~Sped() = default;
        Sped(const Sped &) = delete;
        Sped &operator=(const Sped &) = delete;

        void Init();
        /**
         * @brief Render function, called every frame by the hooked Present function
         *
         * @param swap
         * @param swapInterval
         * @param flags
         * @return true
         * @return false do not render
         */
        bool Render(IDXGISwapChain *swap, UINT swapInterval, UINT flags);

    private:
        void LoadUnityAPI();

        void LoadImGui();
        static HRESULT HookedPresent(IDXGISwapChain *swap, UINT swapInterval, UINT flags);

        void RenderExplorerTab();
        void RenderSaveInstanceTab();
        void RenderExecutorTab();

    private:
        struct ScriptDecompileTab
        {
            polytoria::BaseScript *instance;
            TextEditor editor;
            bool isEditorReady = false;
        };

        void RenderDecompileTab(ScriptDecompileTab* tab);
        void OpenNewScriptDecompileTab(polytoria::BaseScript *instance);
        bool IsTabAlreadyOpen(polytoria::BaseScript *instance);

        void RenderNetMessageLoggerTab();

        void LoadConsole();

    private:
        bool isEditorSetup = false;
        TextEditor *mScriptEditor;

        // TBD
        // for decompiled scripts through properties window
        std::vector<ScriptDecompileTab> mDecompileTabs;

    public:
        struct FileSelectFilters {
            std::string description;
            std::string extensions; // e.g. "txt,log"
        };

        std::optional<std::string> SaveDialog(std::vector<FileSelectFilters> filters);
        std::optional<std::string> OpenDialog(std::vector<FileSelectFilters> filters);

        std::optional<std::string> ReadFileAsString(const std::string& filePath);
        void WriteStringToFile(const std::string& filePath, const std::string& content);
    };
}

#endif /* SPED */
