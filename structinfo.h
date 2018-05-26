//
// Created by qiuyang on 2018/5/22.
//
#ifndef SAMLLANALYSIS_STRUCTINFO_H
#define SAMLLANALYSIS_STRUCTINFO_H

#include <vector>
#include <string>
#include <map>
#include <iostream>

class VarInfo{
private:
    std::string varName;
    //typeName may be a base type or not
    std::string typeName;
    //if the variable is an array,type length is an array
    //element length,you should calculate varLength is typeLength * arrayLength
    int typeLength;
    bool isArray;
    int arrayLength;
    int varLength;
    //the struct dynamic value
    std::vector<long long> values;
public:
    bool operator<(const VarInfo &varInfo) const{
        return varName < varInfo.varName;
    }

    std::ostream &operator<<(std::ostream &os) const{
        if(isAnArray()){
            os<<varName<<"["<<arrayLength<<"] ";
            for(auto value:values){
                os<<value<<" ";
            }

            os<<std::endl;
        }else{
            os<<varName<<" "<<values[0]<<std::endl;
        }

        return os;
    }

    bool isAnArray() const {return isArray;}
    int getTypeLength() const { return typeLength; }
    int getVarLength() const {return varLength;}

    int getArrayLength() const {
        return arrayLength;
    }

    const std::string &getVarName() const { return varName; }
    const std::string &getTypeName() const { return typeName; }

    void addValue(const long long value){this->values.push_back(value);}

    void setVarName(const std::string &varName) {
        VarInfo::varName = varName;
    }

    void setTypeName(const std::string &typeName) {
        VarInfo::typeName = typeName;
    }

    void setTypeLength(int typeLength) {
        VarInfo::typeLength = typeLength;
    }

    void setIsArray(bool isArray) {
        VarInfo::isArray = isArray;
    }

    void setArrayLength(int arrayLength) {
        VarInfo::arrayLength = arrayLength;
    }

    void setVarLength(int varLength) {
        VarInfo::varLength = varLength;
    }

    void setVarInfo(const std::string &typeName,const std::string &varName,
                    int typeLength, bool isArray,int arrayLength,int varLength){
        this->typeName = typeName;
        this->varName = varName;
        this->typeLength = typeLength;
        this->isArray = isArray;
        this->arrayLength = arrayLength;
        this->varLength = varLength;
    }
};

class StructInfo{
private:
    //all variable info in a struct
    std::vector<VarInfo> varInfos;
    //the struct type length
    int structTypeLength;
    std::vector<std::string> structNames;
public:
    StructInfo() {}

    std::ostream &operator<<(std::ostream &os) const{
        os<<"struct name is "<<structNames[0]<<std::endl;
        os<<"struct length is "<<structTypeLength<<std::endl;
        for(auto &varInfo:varInfos){
            varInfo.operator<<(os);
        }

        os<<std::endl;
        return os;
    }

    std::vector<VarInfo> &getVarInfos() {
        return varInfos;
    }

    bool operator < (const StructInfo &structInfo) const {
        return this->structTypeLength < structInfo.structTypeLength;
    }

    const std::vector<std::string> &getStructNames() const {
        return structNames;
    }

    int getStructTypeLength() const {
        return structTypeLength;
    }

    void addName(const std::string &name){structNames.push_back(name);}

    void calStructLength(){
        int maxLength = 0;
        std::for_each(varInfos.cbegin(),varInfos.cend(),[&](const VarInfo &varInfo){
            structTypeLength += varInfo.getVarLength();
            maxLength = std::max(varInfo.getTypeLength(),maxLength);
        });

        //calculate 4 bytes alignment
        if(structTypeLength < maxLength){
//            std::cout<<structNames[0]<<" "<<maxLength<<" "
//                     <<structTypeLength<<std::endl;
            structTypeLength = maxLength;
        }
    }

    void clearAll(){
        varInfos.clear();
        structTypeLength = 0;
        structNames.clear();
    }

    void addVarInfo(const VarInfo varInfo){
        varInfos.push_back(varInfo);
    }

};

class MacroInfo{
private:
    std::pair<std::string,int> keyValue;
    std::string macroName;
public:
    bool operator< (const MacroInfo &macroInfo) const{
        return this->macroName < macroInfo.macroName;
    }

    void addMacro(const std::string &key,const int value){
        keyValue.first = key;
        keyValue.second = value;
        macroName = key;
    }

    const std::pair<std::string, int> &getKeyValue() const {
        return keyValue;
    }

    const std::string &getMacroName() const {
        return macroName;
    }
};

class EnumInfo{
private:
    std::map<std::string,int> enumMap;
    std::string enumName;
    int curValue;
public:
    EnumInfo(int curValue) : curValue(curValue) {}

    bool operator< (const EnumInfo & enumInfo) const {
        return this->enumName < enumInfo.enumName;
    }

    std::ostream &operator<<(std::ostream &os) const{
        os<<enumName<<std::endl;
        auto it = enumMap.cbegin();
        while(it != enumMap.cend()){
            os<<it->first<<" "<<it->second<<std::endl;
            it++;
        }

        os<<std::endl;
        return os;
    }

    const std::map<std::string, int> &getEnumMap() const {
        return enumMap;
    }

    const std::string &getEnumName() const {
        return enumName;
    }

    void addEnum(const std::string &key,const int value){
        enumMap.insert({key,value});
        curValue = value;
    }

    void setEnumName(const std::string &enumName) {
        EnumInfo::enumName = enumName;
    }

    int getCurValue() const {
        return curValue;
    }

    void setCurValue(int curValue) {
        EnumInfo::curValue = curValue;
    }

    void clearAll(){
        enumMap.clear();
        enumName = "";
        curValue = 0;
    }

};
#endif //SAMLLANALYSIS_STRUCTINFO_H
