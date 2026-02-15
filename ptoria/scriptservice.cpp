#include <ptoria/scriptservice.h>
#include <ptoria/scriptinstance.h>
#include <hooking/hookmanager.h>
#include <moonsharp/cinterface.h>
#include <ptoria/networkevent.h>
#include <ptoria/player.h>
#include <ptoria/tool.h>
#include <ptoria/chatservice.h>

DynValue* ScriptService::callback = nullptr;
Script *ScriptService::exec = nullptr;

ScriptService *ScriptService::GetInstance()
{
    static UnityMethod *method = nullptr;
    if (!method)
        method = Unity::GetMethod<"get_Instance">(StaticClass<ScriptService>());
    nasec::Assert(method != nullptr, "Failed to get ScriptService::get_Instance method");
    return method->Invoke<ScriptService *>();
}

void ScriptService::RunScript(BaseScript *script)
{
    Unity::GetMethod<"RunScript">(StaticClass<ScriptService>())->Invoke<void, void *, BaseScript *>(GetInstance(), script);
}

std::string ScriptService::ScriptSource(BaseScript *script)
{
    nasec::Assert(script != nullptr, "Script was nullptr");
    return script->Source()->ToString();
}


void InstallEnvironnement(Script *script);
DynValue *ScriptService::ExecuteScriptInstanceHook(ScriptService *self, Script *script, BaseScript *instance)
{
    nasec::Assert(self != nullptr, "ScriptService instance was nullptr");
    nasec::Assert(script != nullptr, "Script was nullptr");
    nasec::Assert(instance != nullptr, "BaseScript instance was nullptr");

    /*
    Inject our custom lua environnement
    */
    InstallEnvironnement(script);

    return HookManager::Call(ScriptService::ExecuteScriptInstanceHook, self, script, instance);
}


DynValue* poop(void*, ScriptExecutionContext* context, CallbackArguments* args) {
    return DynValue::FromString("Hello from C++!");
}

DynValue* hookinvokeserver(void*, ScriptExecutionContext* ctx, CallbackArguments* args)
{
    std::printf("InvokeServer called!\n");

    // Get count - this should work
    int count = args->Count();
    std::cout << "count = " << count << std::endl;
	
	if (count < 2) {
		std::cout << "Not enough arguments passed to InvokeServer hook" << std::endl;
		return DynValue::FromString("Not enough arguments");
	}

	DynValue* net_event = args->RawGet(0, false);
	if (net_event == nullptr || net_event->Type() != DynValue::DataType::UserData)
	{
		std::cout << "fAAAAAAA" << std::endl;
		return DynValue::FromString("Invalid argument...");
	}
	
	NetworkEvent* obj = (NetworkEvent*)(net_event->Cast(StaticClass<NetworkEvent>()->GetType()));
	std::cout << obj->Name()->ToString() << std::endl;

	DynValue* callback = args->RawGet(1, false);
	if (callback == nullptr || callback->Type() != DynValue::DataType::Function)
	{
		std::cout << "sorry not a function wowie" << std::endl;
		return DynValue::FromString("Invalid arg, expected function...");
	}
	// WTFF IS WRONG WITH TS ????
	// InvokeServerHook(game["Hidden"]["DraggerPlace"], function(msg) print(msg) end)
	ScriptService::callback = callback;
	ctx->OwnerScript()->Registry()->Set((UnityObject*)UnityString::New("InvokeServerCallback"), (UnityObject*)callback);
	ScriptService::exec = ctx->OwnerScript();
	

	// print(InvokeServerHook(game["Hidden"]["CookTool"], function() print('hi') end))
	
    // // Get the args list - cast from IList to List if needed
    // auto argList = args->GetArgs();
    
    // // Try accessing size directly (it's a field in List<Type>)
    // // The IList interface might have Count property instead
    // int listSize = argList->size;  // If cast to List* worked
    // std::cout << "listSize = " << listSize << std::endl;
    
    // // // Iterate through arguments
    // // for (int i = 0; i < count; i++) {
    // //     DynValue* arg = (*argList)[i];  // Use operator[]
    // //     std::cout << "Arg " << i << std::endl;
    // // }
    
    return DynValue::FromString("Hooked InvokeServer successfully!");
}

DynValue* activatetool(void*, ScriptExecutionContext*, CallbackArguments* args)
{
    int count = args->Count();
    if (count < 1) {
        return DynValue::FromString("Not enough arguments passed to activatetool");
    }

    DynValue* tool = args->RawGet(0, false);
    if (tool == nullptr || tool->Type() != DynValue::DataType::UserData) {
        return DynValue::FromString("Invalid argument, expected Tool");
    }

    Tool* toolobj = (Tool*)(tool->Cast(StaticClass<Tool>()->GetType()));
    if (toolobj == nullptr) {
        return DynValue::FromString("Failed to cast Tool");
    }

    toolobj->CmdActivate();
    return DynValue::FromNil();
}

DynValue* unequip(void*, ScriptExecutionContext*, CallbackArguments* args)
{
    int count = args->Count();
    if (count < 1) {
        return DynValue::FromString("Not enough arguments passed to unequiptool");
    }

    DynValue* tool = args->RawGet(0, false);
    if (tool == nullptr || tool->Type() != DynValue::DataType::UserData) {
        return DynValue::FromString("Invalid argument, expected Tool");
    }

    Tool* toolobj = (Tool*)(tool->Cast(StaticClass<Tool>()->GetType()));
    if (toolobj == nullptr) {
        return DynValue::FromString("Failed to cast Tool");
    }

    toolobj->CmdUnequip();
    return DynValue::FromNil();
}

DynValue* equip(void*, ScriptExecutionContext*, CallbackArguments* args)
{
    int count = args->Count();
    if (count < 1) {
        return DynValue::FromString("Not enough arguments passed to equiptool");
    }

    DynValue* tool = args->RawGet(0, false);
    if (tool == nullptr || tool->Type() != DynValue::DataType::UserData) {
        return DynValue::FromString("Invalid argument, expected Tool");
    }

    Tool* toolobj = (Tool*)(tool->Cast(StaticClass<Tool>()->GetType()));
    if (toolobj == nullptr) {
        return DynValue::FromString("Failed to cast Tool");
    }

    toolobj->CmdEquip();
    return DynValue::FromNil();
}

DynValue* loadstring(void*, ScriptExecutionContext* ctx, CallbackArguments* args)
{
    int count = args->Count();
    if (count < 1) {
        return DynValue::FromString("Not enough arguments passed to LoadString");
    }

    DynValue* source = args->RawGet(0, false);
    if (source == nullptr || source->Type() != DynValue::DataType::String) {
        return DynValue::FromString("Invalid argument, expected string");
    }

    UnityClass* stringClass = Unity::GetClass<"String", "mscorlib.dll", "System">();
    UnityString* code = (UnityString*)source->Cast(stringClass->GetType());
    if (code == nullptr) {
        return DynValue::FromString("Failed to cast string");
    }

    Script* script = ctx->OwnerScript();
    if (script == nullptr) {
        return DynValue::FromString("Owner script not available");
    }

    return script->loadstring(code, script->Globals(), UnityString::New("LoadString"));
}

DynValue* serverequiptool(void*, ScriptExecutionContext*, CallbackArguments* args)
{
    int count = args->Count();
    if (count < 2) {
        return DynValue::FromString("Not enough arguments passed to serverequiptool");
    }

    DynValue* player = args->RawGet(0, false);
    if (player == nullptr || player->Type() != DynValue::DataType::UserData) {
        return DynValue::FromString("Invalid argument, expected Player");
    }

    DynValue* tool = args->RawGet(1, false);
    if (tool == nullptr || tool->Type() != DynValue::DataType::UserData) {
        return DynValue::FromString("Invalid argument, expected Tool");
    }

    Player* playerobj = (Player*)(player->Cast(StaticClass<Player>()->GetType()));
    if (playerobj == nullptr) {
        return DynValue::FromString("Failed to cast Player");
    }

    Tool* toolobj = (Tool*)(tool->Cast(StaticClass<Tool>()->GetType()));
    if (toolobj == nullptr) {
        return DynValue::FromString("Failed to cast Tool");
    }

    playerobj->EquipTool(toolobj);
    return DynValue::FromNil();
}

DynValue* sendchat(void*, ScriptExecutionContext* ctx, CallbackArguments* args)
{
    int count = args->Count();
    if (count < 1) {
        return DynValue::FromString("Not enough arguments passed to SendChat");
    }

    DynValue* message = args->RawGet(0, false);
    if (message == nullptr || message->Type() != DynValue::DataType::String) {
        return DynValue::FromString("Invalid argument, expected string");
    }

    UnityClass* stringClass = Unity::GetClass<"String", "mscorlib.dll", "System">();
    UnityString* msg = (UnityString*)message->Cast(stringClass->GetType());
    if (msg == nullptr) {
        return DynValue::FromString("Failed to cast string");
    }

    UnityObject* ChatServiceInstance = Unity::GetStaticFieldValue<UnityString*, "Instance">(StaticClass<ChatService>());

    nasec::Assert(ChatServiceInstance != nullptr, "Failed to get ChatService instance");

    UnityClass* chatClass = StaticClass<ChatService>();
    Unity::GetMethod<"SendChat", "System.String">(chatClass)->Invoke<void, void*, UnityString*>(ChatServiceInstance, msg);

    return DynValue::FromNil();
}

DynValue* fireclickdetector(void*, ScriptExecutionContext* ctx, CallbackArguments* args)
{
    int count = args->Count();
    if (count < 1) {
        return DynValue::FromString("Not enough arguments passed to FireClickDetector");
    }

    DynValue* instance = args->RawGet(0, false);
    if (instance == nullptr || instance->Type() != DynValue::DataType::UserData) {
        return DynValue::FromString("Invalid argument, expected Instance");

    }

    Instance* instanceobj = (Instance*)(instance->Cast(StaticClass<Instance>()->GetType()));
    if (instanceobj == nullptr) {
        return DynValue::FromString("Failed to cast Instance");
    }

    instanceobj->CmdClicked();

    return DynValue::FromNil();
}

void InstallEnvironnement(Script *script)
{
    RegisterCallback(script->Globals(), "poop", poop);
    RegisterCallback(script->Globals(), "InvokeServerHook", hookinvokeserver);
    RegisterCallback(script->Globals(), "activatetool", activatetool);
    RegisterCallback(script->Globals(), "unequiptool", unequip);
    RegisterCallback(script->Globals(), "equiptool", equip);
    RegisterCallback(script->Globals(), "loadstring", loadstring);
    RegisterCallback(script->Globals(), "serverequiptool", serverequiptool);
    RegisterCallback(script->Globals(), "sendchat", sendchat);
    RegisterCallback(script->Globals(), "fireclickdetector", fireclickdetector);
}

void ScriptService::InstallHooks()
{
    HookManager::Install(Unity::GetMethod<"ExecuteScriptInstance">(StaticClass<ScriptService>())->Cast<DynValue*, ScriptService*, Script*, BaseScript*>(), ExecuteScriptInstanceHook);
}