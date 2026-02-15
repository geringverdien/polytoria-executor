#ifndef SCRIPTSOURCE_H
#define SCRIPTSOURCE_H

#include <ui/texteditor.h>
#include <ptoria/basescript.h>

class ScriptSourceUI
{
public:
    struct ScriptDecompileTab
    {
        BaseScript *instance;
        TextEditor* editor;
        bool isEditorReady = false;
    };
public:
    static void Init();
    static void DrawTab(ScriptSourceUI::ScriptDecompileTab* tab);

private:
    static std::vector<ScriptDecompileTab> openTabs;

    void OpenNewScriptDecompileTab(BaseScript *instance);
    bool IsTabAlreadyOpen(BaseScript *instance);
};

#endif /* SCRIPTSOURCE_H */
