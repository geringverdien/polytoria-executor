#ifndef ENVIROMENT
#define ENVIROMENT

#include <HookManager.hpp>
#include <polytoria/polytoria.hpp>
using namespace polytoria;
#include <unity/il2cpp.hpp>

#include <unity/u.hpp>
#include <format>

using CF = DynValue*(*)(ScriptExecutionContext*, CallbackArguments*);

DynValue* CF_Poop(ScriptExecutionContext* ctx, CallbackArguments* args)
{
	std::printf("Yas\n");
;
	return DynValue::NewString(US::New("Poop"));
}

DynValue* hookinvokeserver(ScriptExecutionContext* ctx, CallbackArguments* args)
{
	std::printf("InvokeServer called!\n");
	std::string message = std::format("InvokeServer called with {} arguments", args->GetCount());
	std::cout << message << std::endl;
	return DynValue::NewString(US::New(message.c_str()));
}

void RegisterCFunction(const std::string& name, CF func, Table* table)
{
	// ~TODO: uhh maybe add some "type caching" feature to avoid doing lookups every time~ / Solved by ElCapor :wink:
	auto dynValueKlass = DynValue::GetClass();
	auto scriptExecutionContext = ScriptExecutionContext::GetClass();
	auto callbackArguments = CallbackArguments::GetClass();
	
	if (!dynValueKlass || !scriptExecutionContext || !callbackArguments) return;
			
	auto mi = il2cpp::CreateCustomMethodInfo("huge anus",
		{
			(il2cpp::Il2CppType*)scriptExecutionContext->GetType(),
			(il2cpp::Il2CppType*)callbackArguments->GetType()
		},
		(il2cpp::Il2CppType*)dynValueKlass->GetType(),
		(il2cpp::Il2CppMethodPointer)func
	);


	const auto mscorelibAsm = U::Get("mscorlib.dll");
	if (!mscorelibAsm) {
		std::cout << "[ERROR] Failed to find assembly: mscorlib.dll" << std::endl;
		return;
	}

	const auto typeKlass = mscorelibAsm->Get("Func`3", "System");
	if (!typeKlass) {
		std::cout << "[ERROR] Failed to find class: Func`3" << std::endl;
		return;
	}

	const auto type = typeKlass->GetType();

	auto delegates = U::Invoke<UO*, uintptr_t>("il2cpp_object_new", *(uintptr_t*)type);
	il2cpp::BindDelegate((il2cpp::Il2CppDelegate*)delegates, mi);

	auto finalCallbackMethod = dynValueKlass->Get<UM>("NewCallback");
	if (!finalCallbackMethod) {
		std::cout << "[ERROR] Failed to find method: DynValue::NewCallback" << std::endl;
		return;
	}
	auto finalCallback = finalCallbackMethod->Cast<UO*, UO*, US*>()((UO*)delegates, 0);

	table->SetItem(US::New(name.c_str()), (UO*)finalCallback);
}

void InstallEnviromentFunctions(Table* table)
{
	RegisterCFunction("Poop", CF_Poop, table);
	RegisterCFunction("InvokeServerHook", hookinvokeserver, table);
}

#endif /* ENVIROMENT */
