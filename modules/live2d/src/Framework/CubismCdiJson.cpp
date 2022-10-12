/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#include "CubismCdiJson.hpp"

//--------- LIVE2D NAMESPACE ------------
namespace Live2D {  namespace Cubism {  namespace Framework {

namespace Cdi {
// JSON keys
const csmChar* Version = "Version";
const csmChar* Parameters = "Parameters";
const csmChar* ParameterGroups = "ParameterGroups";
const csmChar* Parts = "Parts";
const csmChar* Id = "Id";
const csmChar* GroupId = "GroupId";
const csmChar* Name = "Name";
}

CubismCdiJson::CubismCdiJson(const csmByte* buffer, csmSizeInt size)
{
    CreateCubismJson(buffer, size);
}

CubismCdiJson::~CubismCdiJson()
{
    DeleteCubismJson();
}

// キーが存在するかどうかのチェック
csmBool CubismCdiJson::IsExistParameters() const
{
    Utils::Value& node = (_json->GetRoot()[Cdi::Parameters]);
    return !node.IsNull() && !node.IsError();
}

csmBool CubismCdiJson::IsExistParameterGroups() const
{
    Utils::Value& node = (_json->GetRoot()[Cdi::ParameterGroups]);
    return !node.IsNull() && !node.IsError();
}

csmBool CubismCdiJson::IsExistParts() const
{
    Utils::Value& node = (_json->GetRoot()[Cdi::Parts]);
    return !node.IsNull() && !node.IsError();
}

// パラメータについて
csmInt32 CubismCdiJson::GetParametersCount()
{
    if (!IsExistParameters()) return 0;
    return _json->GetRoot()[Cdi::Parameters].GetSize();
}

const csmChar* CubismCdiJson::GetParametersId(csmInt32 index)
{
    return _json->GetRoot()[Cdi::Parameters][index][Cdi::Id].GetRawString();
}

const csmChar* CubismCdiJson::GetParametersGroupId(csmInt32 index)
{
    return _json->GetRoot()[Cdi::Parameters][index][Cdi::GroupId].GetRawString();
}

const csmChar* CubismCdiJson::GetParametersName(csmInt32 index)
{
    return _json->GetRoot()[Cdi::Parameters][index][Cdi::Name].GetRawString();
}

// パラメータグループについて
csmInt32 CubismCdiJson::GetParameterGroupsCount()
{
    if (!IsExistParameterGroups()) return 0;
    return _json->GetRoot()[Cdi::ParameterGroups].GetSize();
}

const csmChar* CubismCdiJson::GetParameterGroupsId(csmInt32 index)
{
    return _json->GetRoot()[Cdi::ParameterGroups][index][Cdi::Id].GetRawString();
}

const csmChar* CubismCdiJson::GetParameterGroupsGroupId(csmInt32 index)
{
    return _json->GetRoot()[Cdi::ParameterGroups][index][Cdi::GroupId].GetRawString();
}

const csmChar* CubismCdiJson::GetParameterGroupsName(csmInt32 index)
{
    return _json->GetRoot()[Cdi::ParameterGroups][index][Cdi::Name].GetRawString();
}

// パーツについて
csmInt32 CubismCdiJson::GetPartsCount()
{
    if (!IsExistParts()) return 0;
    return _json->GetRoot()[Cdi::Parts].GetSize();
}

const csmChar* CubismCdiJson::GetPartsId(csmInt32 index)
{
    return _json->GetRoot()[Cdi::Parts][index][Cdi::Id].GetRawString();
}

const csmChar* CubismCdiJson::GetPartsName(csmInt32 index)
{
    return _json->GetRoot()[Cdi::Parts][index][Cdi::Name].GetRawString();
}

}}}
