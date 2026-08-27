#include "Utils/ReflectionUtils.h"

#include "Serialization/JsonSerializer.h"
#include "UObject/FieldIterator.h"
#include "UObject/TextProperty.h"
#include "UObject/UObjectIterator.h"
#include "UObject/UnrealType.h"

namespace UnrealCV
{
namespace ReflectionUtils
{
namespace
{

FJsonObjectBP MakeJsonNull()
{
    FJsonObjectBP Json;
    Json.JsonValue = MakeShareable(new FJsonValueNull());
    return Json;
}

FJsonObjectBP MakeJsonBool(bool Value)
{
    FJsonObjectBP Json;
    Json.JsonValue = MakeShareable(new FJsonValueBoolean(Value));
    return Json;
}

FJsonObjectBP MakeJsonNumber(double Value)
{
    FJsonObjectBP Json;
    Json.JsonValue = MakeShareable(new FJsonValueNumber(Value));
    return Json;
}

FJsonObjectBP MakeJsonString(const FString& Value)
{
    return FJsonObjectBP(Value);
}

FJsonObjectBP MakeJsonObject(const TMap<FString, FJsonObjectBP>& Fields)
{
    return FJsonObjectBP(Fields);
}

FProperty* FindPropertyByName(const UStruct* OwnerStruct, const FName PropertyName)
{
    if (OwnerStruct == nullptr)
    {
        return nullptr;
    }

    for (TFieldIterator<FProperty> It(OwnerStruct, EFieldIterationFlags::IncludeSuper); It; ++It)
    {
        if (It->GetFName() == PropertyName)
        {
            return *It;
        }
    }

    return nullptr;
}

void AppendPropertyFlagStrings(const FProperty* Property, TArray<FString>& OutFlags)
{
    if (Property->HasAnyPropertyFlags(CPF_BlueprintVisible))
        OutFlags.Add(TEXT("BlueprintVisible"));
    if (Property->HasAnyPropertyFlags(CPF_BlueprintReadOnly))
        OutFlags.Add(TEXT("BlueprintReadOnly"));
    if (Property->HasAnyPropertyFlags(CPF_Edit))
        OutFlags.Add(TEXT("Edit"));
    if (Property->HasAnyPropertyFlags(CPF_Parm))
        OutFlags.Add(TEXT("Parm"));
    if (Property->HasAnyPropertyFlags(CPF_OutParm))
        OutFlags.Add(TEXT("OutParm"));
    if (Property->HasAnyPropertyFlags(CPF_ReturnParm))
        OutFlags.Add(TEXT("ReturnParm"));
    if (Property->HasAnyPropertyFlags(CPF_ReferenceParm))
        OutFlags.Add(TEXT("ReferenceParm"));
    if (Property->HasAnyPropertyFlags(CPF_Transient))
        OutFlags.Add(TEXT("Transient"));
    if (Property->HasAnyPropertyFlags(CPF_Config))
        OutFlags.Add(TEXT("Config"));
}

void AppendFunctionFlagStrings(const UFunction* Function, TArray<FString>& OutFlags)
{
    if (Function->HasAnyFunctionFlags(FUNC_BlueprintCallable))
        OutFlags.Add(TEXT("BlueprintCallable"));
    if (Function->HasAnyFunctionFlags(FUNC_BlueprintPure))
        OutFlags.Add(TEXT("BlueprintPure"));
    if (Function->HasAnyFunctionFlags(FUNC_Exec))
        OutFlags.Add(TEXT("Exec"));
    if (Function->HasAnyFunctionFlags(FUNC_Static))
        OutFlags.Add(TEXT("Static"));
    if (Function->HasAnyFunctionFlags(FUNC_Public))
        OutFlags.Add(TEXT("Public"));
    if (Function->HasAnyFunctionFlags(FUNC_Native))
        OutFlags.Add(TEXT("Native"));
    if (Function->HasAnyFunctionFlags(FUNC_Event))
        OutFlags.Add(TEXT("Event"));
    if (Function->HasAnyFunctionFlags(FUNC_Const))
        OutFlags.Add(TEXT("Const"));
}

FJsonObjectBP SerializePropertyValue(FProperty* Property, const void* ValuePtr);
bool JsonValueToImportText(FProperty* Property, const TSharedPtr<FJsonValue>& JsonValue, FString& OutText,
                           FString& OutError);

FString JsonValueToCompactString(const TSharedPtr<FJsonValue>& JsonValue)
{
    FString Output;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Output);
    FJsonSerializer::Serialize(JsonValue, TEXT(""), Writer);
    return Output;
}

FJsonObjectBP SerializeStructProperty(FStructProperty* StructProperty, const void* ValuePtr)
{
    if (StructProperty->Struct == TBaseStructure<FVector>::Get())
    {
        return FJsonObjectBP(*static_cast<const FVector*>(ValuePtr));
    }

    if (StructProperty->Struct == TBaseStructure<FRotator>::Get())
    {
        return FJsonObjectBP(*static_cast<const FRotator*>(ValuePtr));
    }

    if (StructProperty->Struct == TBaseStructure<FTransform>::Get())
    {
        return FJsonObjectBP(*static_cast<const FTransform*>(ValuePtr));
    }

    if (StructProperty->Struct == TBaseStructure<FColor>::Get())
    {
        return FJsonObjectBP(*static_cast<const FColor*>(ValuePtr));
    }

    if (StructProperty->Struct == TBaseStructure<FLinearColor>::Get())
    {
        const FLinearColor& LinearColor = *static_cast<const FLinearColor*>(ValuePtr);
        TMap<FString, FJsonObjectBP> Fields;
        Fields.Add(TEXT("R"), MakeJsonNumber(LinearColor.R));
        Fields.Add(TEXT("G"), MakeJsonNumber(LinearColor.G));
        Fields.Add(TEXT("B"), MakeJsonNumber(LinearColor.B));
        Fields.Add(TEXT("A"), MakeJsonNumber(LinearColor.A));
        return MakeJsonObject(Fields);
    }

    TMap<FString, FJsonObjectBP> Fields;
    for (TFieldIterator<FProperty> It(StructProperty->Struct, EFieldIterationFlags::IncludeSuper); It; ++It)
    {
        FProperty* ChildProperty = *It;
        Fields.Add(ChildProperty->GetName(),
                   SerializePropertyValue(ChildProperty, ChildProperty->ContainerPtrToValuePtr<void>(ValuePtr)));
    }
    return MakeJsonObject(Fields);
}

FJsonObjectBP SerializeArrayProperty(FArrayProperty* ArrayProperty, const void* ValuePtr)
{
    FScriptArrayHelper ArrayHelper(ArrayProperty, ValuePtr);
    TArray<FJsonObjectBP> Elements;
    Elements.Reserve(ArrayHelper.Num());
    for (int32 Index = 0; Index < ArrayHelper.Num(); ++Index)
    {
        Elements.Add(SerializePropertyValue(ArrayProperty->Inner, ArrayHelper.GetRawPtr(Index)));
    }
    return FJsonObjectBP(Elements);
}

FJsonObjectBP SerializeSetProperty(FSetProperty* SetProperty, const void* ValuePtr)
{
    FScriptSetHelper SetHelper(SetProperty, ValuePtr);
    TArray<FJsonObjectBP> Elements;
    for (int32 Index = 0; Index < SetHelper.GetMaxIndex(); ++Index)
    {
        if (!SetHelper.IsValidIndex(Index))
        {
            continue;
        }

        Elements.Add(SerializePropertyValue(SetProperty->ElementProp, SetHelper.GetElementPtr(Index)));
    }
    return FJsonObjectBP(Elements);
}

FJsonObjectBP SerializeMapProperty(FMapProperty* MapProperty, const void* ValuePtr)
{
    FScriptMapHelper MapHelper(MapProperty, ValuePtr);
    TArray<FJsonObjectBP> Entries;
    for (int32 Index = 0; Index < MapHelper.GetMaxIndex(); ++Index)
    {
        if (!MapHelper.IsValidIndex(Index))
        {
            continue;
        }

        TMap<FString, FJsonObjectBP> EntryFields;
        EntryFields.Add(TEXT("key"), SerializePropertyValue(MapProperty->KeyProp, MapHelper.GetKeyPtr(Index)));
        EntryFields.Add(TEXT("value"), SerializePropertyValue(MapProperty->ValueProp, MapHelper.GetValuePtr(Index)));
        Entries.Add(MakeJsonObject(EntryFields));
    }
    return FJsonObjectBP(Entries);
}

FJsonObjectBP SerializeObjectReference(UObject* Object)
{
    if (Object == nullptr)
    {
        return MakeJsonNull();
    }

    TMap<FString, FJsonObjectBP> Fields;
    Fields.Add(TEXT("name"), MakeJsonString(Object->GetName()));
    Fields.Add(TEXT("class"), MakeJsonString(Object->GetClass()->GetName()));
    Fields.Add(TEXT("path"), MakeJsonString(Object->GetPathName()));
    return MakeJsonObject(Fields);
}

FJsonObjectBP SerializePropertyValue(FProperty* Property, const void* ValuePtr)
{
    if (FBoolProperty* BoolProperty = CastField<FBoolProperty>(Property))
    {
        return MakeJsonBool(BoolProperty->GetPropertyValue(ValuePtr));
    }

    if (FEnumProperty* EnumProperty = CastField<FEnumProperty>(Property))
    {
        const int64 EnumValue = EnumProperty->GetUnderlyingProperty()->GetSignedIntPropertyValue(ValuePtr);
        if (const UEnum* Enum = EnumProperty->GetEnum())
        {
            return MakeJsonString(Enum->GetNameStringByValue(EnumValue));
        }
        return MakeJsonNumber(EnumValue);
    }

    if (FByteProperty* ByteProperty = CastField<FByteProperty>(Property))
    {
        if (const UEnum* Enum = ByteProperty->Enum)
        {
            return MakeJsonString(Enum->GetNameStringByValue(ByteProperty->GetPropertyValue(ValuePtr)));
        }
        return MakeJsonNumber(ByteProperty->GetPropertyValue(ValuePtr));
    }

    if (FNumericProperty* NumericProperty = CastField<FNumericProperty>(Property))
    {
        return NumericProperty->IsFloatingPoint()
                   ? MakeJsonNumber(NumericProperty->GetFloatingPointPropertyValue(ValuePtr))
                   : MakeJsonNumber(NumericProperty->GetSignedIntPropertyValue(ValuePtr));
    }

    if (FStrProperty* StrProperty = CastField<FStrProperty>(Property))
    {
        return MakeJsonString(StrProperty->GetPropertyValue(ValuePtr));
    }

    if (FNameProperty* NameProperty = CastField<FNameProperty>(Property))
    {
        return MakeJsonString(NameProperty->GetPropertyValue(ValuePtr).ToString());
    }

    if (FTextProperty* TextProperty = CastField<FTextProperty>(Property))
    {
        return MakeJsonString(TextProperty->GetPropertyValue(ValuePtr).ToString());
    }

    if (FClassProperty* ClassProperty = CastField<FClassProperty>(Property))
    {
        if (UClass* ClassValue = Cast<UClass>(ClassProperty->GetObjectPropertyValue(ValuePtr)))
        {
            TMap<FString, FJsonObjectBP> Fields;
            Fields.Add(TEXT("name"), MakeJsonString(ClassValue->GetName()));
            Fields.Add(TEXT("path"), MakeJsonString(ClassValue->GetPathName()));
            return MakeJsonObject(Fields);
        }
        return MakeJsonNull();
    }

    if (FSoftClassProperty* SoftClassProperty = CastField<FSoftClassProperty>(Property))
    {
        return MakeJsonString(SoftClassProperty->GetPropertyValue(ValuePtr).ToString());
    }

    if (FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(Property))
    {
        return SerializeObjectReference(ObjectProperty->GetObjectPropertyValue(ValuePtr));
    }

    if (FSoftObjectProperty* SoftObjectProperty = CastField<FSoftObjectProperty>(Property))
    {
        return MakeJsonString(SoftObjectProperty->GetPropertyValue(ValuePtr).ToString());
    }

    if (FStructProperty* StructProperty = CastField<FStructProperty>(Property))
    {
        return SerializeStructProperty(StructProperty, ValuePtr);
    }

    if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
    {
        return SerializeArrayProperty(ArrayProperty, ValuePtr);
    }

    if (FSetProperty* SetProperty = CastField<FSetProperty>(Property))
    {
        return SerializeSetProperty(SetProperty, ValuePtr);
    }

    if (FMapProperty* MapProperty = CastField<FMapProperty>(Property))
    {
        return SerializeMapProperty(MapProperty, ValuePtr);
    }

    FString ExportedText;
    Property->ExportTextItem_Direct(ExportedText, ValuePtr, nullptr, nullptr, PPF_None);
    return MakeJsonString(ExportedText);
}

FJsonObjectBP SerializePropertyDescriptor(FProperty* Property)
{
    TMap<FString, FJsonObjectBP> Fields;
    Fields.Add(TEXT("name"), MakeJsonString(Property->GetName()));
    Fields.Add(TEXT("cpp_type"), MakeJsonString(Property->GetCPPType()));

    if (const UStruct* OwnerStruct = Property->GetOwnerStruct())
    {
        Fields.Add(TEXT("owner"), MakeJsonString(OwnerStruct->GetName()));
    }

    TArray<FString> FlagStrings;
    AppendPropertyFlagStrings(Property, FlagStrings);
    Fields.Add(TEXT("flags"), FJsonObjectBP(FlagStrings));

    return MakeJsonObject(Fields);
}

FJsonObjectBP SerializeFunctionDescriptor(UFunction* Function)
{
    TMap<FString, FJsonObjectBP> Fields;
    Fields.Add(TEXT("name"), MakeJsonString(Function->GetName()));
    Fields.Add(TEXT("display_name"), MakeJsonString(Function->GetName()));

    if (const UClass* OwnerClass = Function->GetOuterUClass())
    {
        Fields.Add(TEXT("owner_class"), MakeJsonString(OwnerClass->GetName()));
    }

    TArray<FString> FunctionFlags;
    AppendFunctionFlagStrings(Function, FunctionFlags);
    Fields.Add(TEXT("flags"), FJsonObjectBP(FunctionFlags));

    TArray<FJsonObjectBP> Parameters;
    for (TFieldIterator<FProperty> It(Function); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
    {
        FProperty* Parameter = *It;
        TMap<FString, FJsonObjectBP> ParameterFields;
        ParameterFields.Add(TEXT("name"), MakeJsonString(Parameter->GetName()));
        ParameterFields.Add(TEXT("cpp_type"), MakeJsonString(Parameter->GetCPPType()));

        FString Direction = TEXT("In");
        if (Parameter->HasAnyPropertyFlags(CPF_ReturnParm))
        {
            Direction = TEXT("Return");
        }
        else if (Parameter->HasAnyPropertyFlags(CPF_OutParm))
        {
            Direction = Parameter->HasAnyPropertyFlags(CPF_ReferenceParm) ? TEXT("InOut") : TEXT("Out");
        }
        else if (Parameter->HasAnyPropertyFlags(CPF_ReferenceParm) && !Parameter->HasAnyPropertyFlags(CPF_ConstParm))
        {
            Direction = TEXT("InOut");
        }

        ParameterFields.Add(TEXT("direction"), MakeJsonString(Direction));
        Parameters.Add(MakeJsonObject(ParameterFields));
    }
    Fields.Add(TEXT("parameters"), FJsonObjectBP(Parameters));

    return MakeJsonObject(Fields);
}

bool JsonArrayToImportText(FArrayProperty* ArrayProperty, const TArray<TSharedPtr<FJsonValue>>& JsonArray,
                           FString& OutText, FString& OutError)
{
    TArray<FString> Elements;
    for (const TSharedPtr<FJsonValue>& ElementValue : JsonArray)
    {
        FString ElementText;
        if (!JsonValueToImportText(ArrayProperty->Inner, ElementValue, ElementText, OutError))
        {
            return false;
        }
        Elements.Add(ElementText);
    }

    OutText = FString::Printf(TEXT("(%s)"), *FString::Join(Elements, TEXT(",")));
    return true;
}

bool JsonObjectToStructImportText(FStructProperty* StructProperty, const TSharedPtr<FJsonObject>& JsonObject,
                                  FString& OutText, FString& OutError)
{
    TArray<FString> Fields;
    for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : JsonObject->Values)
    {
        FProperty* ChildProperty = FindPropertyByName(StructProperty->Struct, FName(*Pair.Key));
        if (ChildProperty == nullptr)
        {
            OutError = FString::Printf(TEXT("Struct '%s' has no property '%s'"), *StructProperty->Struct->GetName(),
                                       *Pair.Key);
            return false;
        }

        FString ChildText;
        if (!JsonValueToImportText(ChildProperty, Pair.Value, ChildText, OutError))
        {
            return false;
        }

        Fields.Add(FString::Printf(TEXT("%s=%s"), *Pair.Key, *ChildText));
    }

    OutText = FString::Printf(TEXT("(%s)"), *FString::Join(Fields, TEXT(",")));
    return true;
}

bool JsonObjectToMapImportText(FMapProperty* MapProperty, const TSharedPtr<FJsonObject>& JsonObject, FString& OutText,
                               FString& OutError)
{
    TArray<FString> Entries;
    for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : JsonObject->Values)
    {
        FString KeyText;
        TSharedPtr<FJsonValue> KeyValue = MakeShareable(new FJsonValueString(Pair.Key));
        if (!JsonValueToImportText(MapProperty->KeyProp, KeyValue, KeyText, OutError))
        {
            return false;
        }

        FString ValueText;
        if (!JsonValueToImportText(MapProperty->ValueProp, Pair.Value, ValueText, OutError))
        {
            return false;
        }

        Entries.Add(FString::Printf(TEXT("(%s,%s)"), *KeyText, *ValueText));
    }

    OutText = FString::Printf(TEXT("(%s)"), *FString::Join(Entries, TEXT(",")));
    return true;
}

bool JsonValueToImportText(FProperty* Property, const TSharedPtr<FJsonValue>& JsonValue, FString& OutText,
                           FString& OutError)
{
    if (!JsonValue.IsValid() || JsonValue->Type == EJson::Null)
    {
        OutText = TEXT("None");
        return true;
    }

    if (CastField<FStrProperty>(Property) || CastField<FNameProperty>(Property) || CastField<FTextProperty>(Property) ||
        CastField<FObjectPropertyBase>(Property) || CastField<FSoftObjectProperty>(Property) ||
        CastField<FSoftClassProperty>(Property))
    {
        if (JsonValue->Type == EJson::String)
        {
            OutText = JsonValue->AsString();
            return true;
        }
    }

    if (CastField<FBoolProperty>(Property))
    {
        if (JsonValue->Type == EJson::Boolean)
        {
            OutText = JsonValue->AsBool() ? TEXT("true") : TEXT("false");
            return true;
        }
    }

    if (CastField<FNumericProperty>(Property) || CastField<FEnumProperty>(Property) ||
        CastField<FByteProperty>(Property))
    {
        if (JsonValue->Type == EJson::Number)
        {
            OutText = FString::SanitizeFloat(JsonValue->AsNumber());
            return true;
        }
        if (JsonValue->Type == EJson::String)
        {
            OutText = JsonValue->AsString();
            return true;
        }
    }

    if (FStructProperty* StructProperty = CastField<FStructProperty>(Property))
    {
        if (JsonValue->Type == EJson::Object)
        {
            return JsonObjectToStructImportText(StructProperty, JsonValue->AsObject(), OutText, OutError);
        }
    }

    if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
    {
        if (JsonValue->Type == EJson::Array)
        {
            return JsonArrayToImportText(ArrayProperty, JsonValue->AsArray(), OutText, OutError);
        }
    }

    if (FMapProperty* MapProperty = CastField<FMapProperty>(Property))
    {
        if (JsonValue->Type == EJson::Object)
        {
            return JsonObjectToMapImportText(MapProperty, JsonValue->AsObject(), OutText, OutError);
        }
    }

    if (JsonValue->Type == EJson::String)
    {
        OutText = JsonValue->AsString();
        return true;
    }

    OutText = JsonValueToCompactString(JsonValue);
    return true;
}

bool InitializeFunctionParams(UFunction* Function, uint8* ParamsBuffer)
{
    FMemory::Memzero(ParamsBuffer, Function->ParmsSize);
    for (TFieldIterator<FProperty> It(Function); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
    {
        FProperty* Property = *It;
        if (!Property->HasAnyPropertyFlags(CPF_ZeroConstructor))
        {
            Property->InitializeValue_InContainer(ParamsBuffer);
        }
    }
    return true;
}

void DestroyFunctionParams(UFunction* Function, uint8* ParamsBuffer)
{
    for (TFieldIterator<FProperty> It(Function); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
    {
        It->DestroyValue_InContainer(ParamsBuffer);
    }
}

} // namespace

bool IsOutputParameter(const FProperty* Property)
{
    return Property != nullptr &&
           (Property->HasAnyPropertyFlags(CPF_ReturnParm) || Property->HasAnyPropertyFlags(CPF_OutParm) ||
            (Property->HasAnyPropertyFlags(CPF_ReferenceParm) && !Property->HasAnyPropertyFlags(CPF_ConstParm)));
}

UClass* ResolveClass(const FString& ClassNameOrPath)
{
    if (ClassNameOrPath.IsEmpty())
    {
        return nullptr;
    }

    if (UClass* LoadedClass = LoadObject<UClass>(nullptr, *ClassNameOrPath))
    {
        return LoadedClass;
    }

    for (TObjectIterator<UClass> It; It; ++It)
    {
        UClass* Class = *It;
        if (Class == nullptr)
        {
            continue;
        }

        if (Class->GetName() == ClassNameOrPath || Class->GetPathName() == ClassNameOrPath)
        {
            return Class;
        }
    }

    return nullptr;
}

FJsonObjectBP SerializeFunctionsForObject(UObject* Object)
{
    TArray<FJsonObjectBP> Functions;
    TSet<FName> SeenFunctionNames;

    for (TFieldIterator<UFunction> It(Object->GetClass(), EFieldIterationFlags::IncludeSuper); It; ++It)
    {
        UFunction* Function = *It;
        if (Function == nullptr || SeenFunctionNames.Contains(Function->GetFName()))
        {
            continue;
        }

        SeenFunctionNames.Add(Function->GetFName());
        Functions.Add(SerializeFunctionDescriptor(Function));
    }

    return FJsonObjectBP(Functions);
}

FJsonObjectBP SerializePropertiesForObject(UObject* Object)
{
    TArray<FJsonObjectBP> Properties;
    TSet<FName> SeenPropertyNames;

    for (TFieldIterator<FProperty> It(Object->GetClass(), EFieldIterationFlags::IncludeSuper); It; ++It)
    {
        FProperty* Property = *It;
        if (Property == nullptr || Property->HasAnyPropertyFlags(CPF_Parm) ||
            SeenPropertyNames.Contains(Property->GetFName()))
        {
            continue;
        }

        SeenPropertyNames.Add(Property->GetFName());
        Properties.Add(SerializePropertyDescriptor(Property));
    }

    return FJsonObjectBP(Properties);
}

FJsonObjectBP SerializeOutputParameters(UFunction* Function, const uint8* ParamsBuffer)
{
    TMap<FString, FJsonObjectBP> Fields;
    for (TFieldIterator<FProperty> It(Function); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
    {
        FProperty* Property = *It;
        if (!IsOutputParameter(Property))
        {
            continue;
        }

        Fields.Add(Property->GetName(),
                   SerializePropertyValue(Property, Property->ContainerPtrToValuePtr<void>(ParamsBuffer)));
    }

    return MakeJsonObject(Fields);
}

bool ResolvePropertyPath(UObject* RootObject, const FString& PropertyPath, void*& OutContainerPtr,
                         FProperty*& OutProperty, FString& OutError)
{
    OutContainerPtr = nullptr;
    OutProperty = nullptr;
    OutError.Empty();

    if (RootObject == nullptr)
    {
        OutError = TEXT("Target object is null");
        return false;
    }

    TArray<FString> Segments;
    PropertyPath.ParseIntoArray(Segments, TEXT("."), true);
    if (Segments.Num() == 0)
    {
        OutError = TEXT("Property path is empty");
        return false;
    }

    void* CurrentContainerPtr = RootObject;
    const UStruct* CurrentStruct = RootObject->GetClass();

    for (int32 Index = 0; Index < Segments.Num(); ++Index)
    {
        const bool bIsLastSegment = Index == Segments.Num() - 1;
        const FName SegmentName(*Segments[Index]);
        FProperty* Property = FindPropertyByName(CurrentStruct, SegmentName);

        if (Property == nullptr)
        {
            OutError = FString::Printf(TEXT("Property '%s' was not found on '%s'"), *Segments[Index],
                                       *CurrentStruct->GetName());
            return false;
        }

        if (bIsLastSegment)
        {
            OutContainerPtr = CurrentContainerPtr;
            OutProperty = Property;
            return true;
        }

        if (FStructProperty* StructProperty = CastField<FStructProperty>(Property))
        {
            CurrentContainerPtr = StructProperty->ContainerPtrToValuePtr<void>(CurrentContainerPtr);
            CurrentStruct = StructProperty->Struct;
            continue;
        }

        if (FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(Property))
        {
            UObject* NextObject = ObjectProperty->GetObjectPropertyValue(
                ObjectProperty->ContainerPtrToValuePtr<void>(CurrentContainerPtr));
            if (NextObject == nullptr)
            {
                OutError = FString::Printf(TEXT("Property '%s' is null"), *Segments[Index]);
                return false;
            }

            CurrentContainerPtr = NextObject;
            CurrentStruct = NextObject->GetClass();
            continue;
        }

        OutError = FString::Printf(TEXT("Property '%s' does not support nested access"), *Segments[Index]);
        return false;
    }

    OutError = TEXT("Failed to resolve property path");
    return false;
}

FJsonObjectBP SerializePropertyResult(FProperty* Property, const void* ContainerPtr)
{
    TMap<FString, FJsonObjectBP> Fields;
    Fields.Add(TEXT("name"), MakeJsonString(Property->GetName()));
    Fields.Add(TEXT("cpp_type"), MakeJsonString(Property->GetCPPType()));
    Fields.Add(TEXT("value"), SerializePropertyValue(Property, Property->ContainerPtrToValuePtr<void>(ContainerPtr)));
    return MakeJsonObject(Fields);
}

bool SetPropertyValueFromText(FProperty* Property, void* ContainerPtr, const FString& ValueText, FString& OutError)
{
    OutError.Empty();

    const TCHAR* Result = Property->ImportText_Direct(*ValueText, Property->ContainerPtrToValuePtr<void>(ContainerPtr),
                                                      nullptr, PPF_None);
    if (Result == nullptr)
    {
        OutError =
            FString::Printf(TEXT("Failed to import value '%s' for property '%s'"), *ValueText, *Property->GetName());
        return false;
    }

    return true;
}

bool CallFunctionWithJson(UObject* TargetObject, const FString& FunctionName, const FString& ArgsJson,
                          FJsonObjectBP& OutResult, FString& OutError)
{
    OutError.Empty();

    if (TargetObject == nullptr)
    {
        OutError = TEXT("Target object is null");
        return false;
    }

    UFunction* Function = TargetObject->FindFunction(FName(*FunctionName));
    if (Function == nullptr)
    {
        OutError =
            FString::Printf(TEXT("Function '%s' was not found on '%s'"), *FunctionName, *TargetObject->GetName());
        return false;
    }

    TSharedPtr<FJsonObject> ArgsObject;
    if (!ArgsJson.IsEmpty())
    {
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ArgsJson);
        if (!FJsonSerializer::Deserialize(Reader, ArgsObject) || !ArgsObject.IsValid())
        {
            OutError = TEXT("call_json expects a JSON object for arguments");
            return false;
        }
    }
    else
    {
        ArgsObject = MakeShareable(new FJsonObject());
    }

    uint8* ParamsBuffer = static_cast<uint8*>(FMemory_Alloca(Function->ParmsSize));
    InitializeFunctionParams(Function, ParamsBuffer);

    bool bFailed = false;
    for (TFieldIterator<FProperty> It(Function); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
    {
        FProperty* Property = *It;
        if (Property->HasAnyPropertyFlags(CPF_ReturnParm) || Property->HasAnyPropertyFlags(CPF_OutParm))
        {
            continue;
        }

        const TSharedPtr<FJsonValue>* JsonValuePtr = ArgsObject->Values.Find(Property->GetName());
        if (JsonValuePtr == nullptr)
        {
#if WITH_EDITOR
            const FName DefaultPropertyKey(*(FString(TEXT("CPP_Default_")) + Property->GetName()));
            const FString& PropertyDefaultValue = Function->GetMetaData(DefaultPropertyKey);
            if (!PropertyDefaultValue.IsEmpty())
            {
                const TCHAR* Result = Property->ImportText_Direct(
                    *PropertyDefaultValue, Property->ContainerPtrToValuePtr<void>(ParamsBuffer), nullptr, PPF_None);
                if (Result != nullptr)
                {
                    continue;
                }
            }
#endif
            OutError = FString::Printf(TEXT("Missing JSON argument '%s' for function '%s'"), *Property->GetName(),
                                       *FunctionName);
            bFailed = true;
            break;
        }

        FString ImportText;
        if (!JsonValueToImportText(Property, *JsonValuePtr, ImportText, OutError))
        {
            bFailed = true;
            break;
        }

        const TCHAR* Result = Property->ImportText_Direct(
            *ImportText, Property->ContainerPtrToValuePtr<void>(ParamsBuffer), nullptr, PPF_None);
        if (Result == nullptr)
        {
            OutError = FString::Printf(TEXT("Failed to import argument '%s' as %s from '%s'"), *Property->GetName(),
                                       *Property->GetCPPType(), *ImportText);
            bFailed = true;
            break;
        }
    }

    if (!bFailed)
    {
        TargetObject->ProcessEvent(Function, ParamsBuffer);
        OutResult = SerializeOutputParameters(Function, ParamsBuffer);
    }

    DestroyFunctionParams(Function, ParamsBuffer);
    return !bFailed;
}

} // namespace ReflectionUtils
} // namespace UnrealCV
