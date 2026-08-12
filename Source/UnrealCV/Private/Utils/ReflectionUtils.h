#pragma once

#include "CoreMinimal.h"
#include "JsonObjectBP.h"

class FProperty;
class UFunction;
class UClass;
class UObject;

namespace UnrealCV
{
namespace ReflectionUtils
{

bool IsOutputParameter(const FProperty* Property);

UClass* ResolveClass(const FString& ClassNameOrPath);

FJsonObjectBP SerializeFunctionsForObject(UObject* Object);
FJsonObjectBP SerializePropertiesForObject(UObject* Object);
FJsonObjectBP SerializeOutputParameters(UFunction* Function, const uint8* ParamsBuffer);

bool ResolvePropertyPath(UObject* RootObject, const FString& PropertyPath, void*& OutContainerPtr,
                         FProperty*& OutProperty, FString& OutError);
FJsonObjectBP SerializePropertyResult(FProperty* Property, const void* ContainerPtr);
bool SetPropertyValueFromText(FProperty* Property, void* ContainerPtr, const FString& ValueText, FString& OutError);
bool CallFunctionWithJson(UObject* TargetObject, const FString& FunctionName, const FString& ArgsJson,
                          FJsonObjectBP& OutResult, FString& OutError);

} // namespace ReflectionUtils
} // namespace UnrealCV
