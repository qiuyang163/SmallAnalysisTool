//
// Created by qiuyang on 2018/5/22.
//
#include "analysis.h"
#include "strutil.h"
#include "bitutil.h"
#include <fstream>
#include <iostream>

std::vector<StructInfo> AnalysisUtil::structInfos;
std::vector<EnumInfo> AnalysisUtil::enumInfos;
std::set<MacroInfo> AnalysisUtil::macroInfos;
std::vector<std::string> AnalysisUtil::filePaths;
std::vector<std::string> AnalysisUtil::contents;

//temporily not certain macro,certain it after
//construct struct info and enum info
std::map<std::string,std::string> AnalysisUtil::notCertainMacroMap;

std::map<std::string,int> AnalysisUtil::typeMap{{"INT8",1 * 8},{"UINT8",1 * 8},
                                                {"UINT16",2 * 8},{"UINT32",4 * 8},
                                                {"UINT64",8 * 8},{"INT16",2 * 8},
                                                {"INT32",4 * 8},{"INT64",8 * 8},
                                                {"UINT8*",8 * 8},{"char",8},
                                                {"unsigned char",8},{"short",8 * 2},
                                                {"int",8 * 4},{"unsigned int",8 * 4}};

const std::string END_ANNOTATION("*/");
const std::string HEAD_ANNOTATION("/*");
const std::string INCLUDE_STR("#include");
const std::string DEFINE_STR("#define");
const std::string TYPEDEF_STR("typedef");
const std::string ENUM_STR("enum");
const std::string ENUM_BRAKET_STR("enum{");
const std::string STRUCT_STR("struct");
const std::string STRUCT_BRAKET_STR("struct{");
const std::string IFNDEF_STR("#ifndef");
const std::string IFDEF_STR("#ifdef");
const std::string DEF_ELSE_STR("#else");
const std::string DEF_END_STR("endif");
const std::string SIZEOF_STR("sizeof");
const std::string X86_64("#ifndef __x86_64__");
const std::string NOTCERTAIN_STR("notcertain");
const std::string POINT_STR("pointer");

const int ENUM_LENGTH = 4;
const int BYTES_LENGTH = 8;

void AnalysisUtil::preProcessFiles() {
    std::cout<<"in pre process header file--------"<<std::endl;
    for(auto &file:filePaths){
        try {
            preProcessFileCore(file);
        }catch (std::exception &e){
            std::cerr<<e.what()<<std::endl;
            exit(-1);
        }
    }
}

int AnalysisUtil::preProcessFileCore(std::string filePath) {
    std::ifstream ifstream(filePath,std::ios::in);
    if(ifstream.is_open()){
        std::string str;
        while(!ifstream.eof()){
            getline(ifstream,str);
            processRowText(str);
        }

        ifstream.close();
        return 0;
    }else
        throw "open" + filePath + "failed";
}

void AnalysisUtil::processRowText(std::string text) {
    //delete space head and tail
    std::string str = StringUtil::trim(text);
    if(str.empty()) return;
    contents.push_back(str);
}

void AnalysisUtil::analysisFile() {
    AnalysisUtil::preProcessFiles();
    std::cout<<"in analysis header file--------"<<std::endl;
    //record current row is in annotation or not
    bool inAnnotationFlag = false;
    //record current row is in enum or not
    bool inEnumFlag = false;
    //record current row is in struct or not
    bool inStructFlag = false;
    //record current row is in macro or not,if all macro is one row,it is no use
    bool inMacroflag = false;
    //record ifdef X86_64
    bool inX64Flag = false;

    EnumInfo enumInfo(0);
    StructInfo structInfo;

    for(auto &text:contents){
        if(inAnnotationFlag) {
            if(text.find(END_ANNOTATION) != std::string::npos) {
                inAnnotationFlag = false;
            }
        }else{
            if(text[0] == '/' && text[1] == '*'){
                //this situation /*******************
                if(text.find("*/") == std::string::npos) {
                    inAnnotationFlag = true;
                }
            }
            else if(text[0] == '/' && text[1] == '/')
                //one row annotation
                continue;
            else{
                //#include ...
                if(text.find(INCLUDE_STR) == std::string::npos){
                    if(!inEnumFlag && !inStructFlag){
                        //#define ...
                        if(text.find(DEFINE_STR) != std::string::npos) {
                            processMacro(text);
                        }else if(text.find(TYPEDEF_STR) != std::string::npos){
                            //typedef ...
                            processTypedef(text,inEnumFlag,inStructFlag,
                                           enumInfo,structInfo);
                        }
                    }else if(inEnumFlag){
                        processEnum(text,inEnumFlag,enumInfo);
                    }else if(inStructFlag){
                        processStruct(text,inStructFlag,structInfo,inX64Flag);
                    }
                }
            }
        }
    }

    //deal not certain enum、macro、struct info
    afterProcessFiles();
}

bool AnalysisUtil::processMacro(const std::string &text) {
    std::vector<std::string> strs = StringUtil::split(text);
    //delete #define _INTEL_L1API_H_ situation
    if(strs.size() < 3) return false;

    MacroInfo macroInfo;
    //this situation #define ULSUBFR_CMNCTRL_LEN sizeof(ULSUBFRCMNCTRL)
    if(strs[2].find(SIZEOF_STR) != std::string::npos){
        bool structFindFlag = false;
        bool enumFindFlag = false;
        std::string notCertainName = strs[2].substr(strlen(SIZEOF_STR.data()) + 1);
        notCertainName = StringUtil::delBrackets(notCertainName);
        for(auto &info:structInfos){
            for(const std::string &name:info.getStructNames()){
                //can find struct name
                if(notCertainName == name){
                    macroInfo.addMacro(strs[1],info.getStructTypeLength());
                    structFindFlag = true;
                }
            }
        }

        for(auto &info:enumInfos){
            if(notCertainName == info.getEnumName()){
                macroInfo.addMacro(strs[1],ENUM_LENGTH);
                enumFindFlag = true;
            }
        }

        if(!structFindFlag && !enumFindFlag) {
            notCertainMacroMap[strs[1]] = notCertainName;
        }
    }else{
        macroInfo.addMacro(strs[1],atoi(StringUtil::trim(strs[strs.size() - 1]).data()));
        macroInfos.insert(macroInfo);
    }

    return false;
}

void AnalysisUtil::processTypedef(const std::string &text,
                                  bool &inEnumFlag, bool &inStructFlag,
                                  EnumInfo &enumInfo,StructInfo &structInfo) {
    std::vector<std::string> strs = StringUtil::split(text);
    if(strs.size() < 2) return;
    if(strs[1].find(ENUM_STR) != std::string::npos) {
        inEnumFlag = true;
    }else if(strs[1].find(STRUCT_STR) != std::string::npos) {
        inStructFlag = true;
    }
    else{
        std::cout<<"type not enum or struct"<<std::endl;
    }
}

void AnalysisUtil::processEnum(const std::string &text, bool &inEnumFlag,EnumInfo &enumInfo) {
    if(!inEnumFlag) return;
    std::vector<std::string> strs = StringUtil::split(text,',');
    if(strs.size() == 1){
        //a enum start
        if(strs[0].find("{") != std::string::npos)
            return;
        else if(strs[0].find("}") != std::string::npos){
            //a enum end,set enum name
            std::string name = strs[0].substr(1,strs[0].size() - 2);
            enumInfo.setEnumName(name);
            inEnumFlag = false;
            enumInfos.push_back(enumInfo);
//            enumInfo.operator<<(std::cout);
            enumInfo.clearAll();
            return;
        }
        else{
            //this situation PHY_TXSTART_REQUEST = 1//15/16....
            strs = StringUtil::split(text);
            std::string value = strs[2];
            value = value.substr(0, value.find_first_of('/'));
            enumInfo.addEnum(strs[0], atoi(value.data()));
            enumInfo.setCurValue(atoi(value.data()) + 1);
        }
    }
    else if(strs.size() == 2 && (text.find("//") != std::string::npos ||
            text.find("/*") != std::string::npos)){
        //this situation CARRIER_SPACING = 12,
        strs = StringUtil::split(strs[0]);
        enumInfo.addEnum(strs[0],atoi(strs[1].data()));
        enumInfo.setCurValue(atoi(strs[1].data()) + 1);
    }else{
        //this situation PRACH_RESULT = 0, SRS_RESULT, CQIRIHI_RESULT,
        //or LOCALIZED_TYPE = 0, DISTRIBUTED_TYPE this situation
        std::string tStr("");
        for(int i = 0;i < strs.size();i++){
            tStr = StringUtil::trim(strs[i]);
            if(tStr.empty()) continue;
            if(tStr.find("=") != std::string::npos){
                std::vector<std::string> tStrs = StringUtil::split(tStr);
                enumInfo.addEnum(tStrs[0],atoi(tStrs[2].data()));
                enumInfo.setCurValue(atoi(tStrs[2].data()) + 1);
            }else{
                tStr = StringUtil::trim(tStr);
                enumInfo.addEnum(tStr,enumInfo.getCurValue());
                enumInfo.setCurValue(enumInfo.getCurValue() + 1);
            }
        }
    }
}

void AnalysisUtil::processStruct(const std::string &text,
                                 bool &inStructFlag, StructInfo &structInfo, bool &inX86Flag) {
    if(!inStructFlag) return;
    //this situation {
    if(text.find("{") != std::string::npos) return;
    //this situation }PhyErrorIndication;
    if(text.find("}") != std::string::npos){
        std::vector<std::string> strs = StringUtil::split(text,',');
        std::string name("");
        if(strs.size() == 1){
            //this situation }PhyErrorIndication or } PhyErrorIndication
            name = StringUtil::delfirstNotAlpha(strs[0]);
            name = name.substr(0,name.size() - 1);
            structInfo.addName(name);
        }else{
            //this situation }MAC2PHY_QUEUE_EL,*PMAC2PHY_QUEUE_EL;
            name = StringUtil::delfirstNotAlpha(strs[0]);
            name = name.substr(0,name.size());
            structInfo.addName(name);

            for(int i = 1;i < strs.size() - 1;i++){
                name = StringUtil::trim(strs[i]);
                structInfo.addName(name);
            }

            strs[1] = StringUtil::trim(strs[1]);
            name = StringUtil::delFirstStar(strs[1]);
            name = StringUtil::dellastSemicolon(name);
            structInfo.addName(name);
        }

        inStructFlag = false;

        structInfo.calStructLength();
        structInfos.push_back(structInfo);
        structInfo.clearAll();

        return;
    }

    //delete exception situation
    std::string str = StringUtil::dellastSemicolon(text);
    if(str.find(X86_64) != std::string::npos && !inX86Flag){
        //this situation //#ifndef __x86_64__
        inX86Flag = true;
        return;
    }else if(str.find(DEF_ELSE_STR) != std::string::npos && inX86Flag){
        //this situation #else
        inX86Flag = false;
        return;
    }else if(str.find(DEF_END_STR) != std::string::npos) {
        //this situation #endif
        return;
    }

    if(inX86Flag) return;
    analysisStructInfo(str,inStructFlag,structInfo);
}

//add struct type info
void AnalysisUtil::analysisStructInfo(const std::string &str, bool &inStructFlag, StructInfo &structInfo) {
    std::vector<std::string> strs = StringUtil::split(str);
    VarInfo varInfo;
    if(strs.size() == 2){
        //this situation UINT32 msg_len:16; is a base type
        if(typeMap.count(strs[0])){
            varInfo.setTypeName(strs[0]);
            int wholeTypeLength = typeMap.find(strs[0])->second;
            varInfo.setTypeLength(wholeTypeLength);
            if(strs[1].find(":") != std::string::npos) {
                //calculate bit position
                std::vector<std::string> values = StringUtil::split(strs[1],':');
                varInfo.setVarInfo(strs[0],values[0],
                                   wholeTypeLength, false,0,atoi(values[1].data()));
            }
            else {
                if(strs[1].find("[") != std::string::npos){
                    //is an array
                    std::string varName = strs[1].substr(0,strs[1].find_first_of("["));
                    std::string sArrayLength = StringUtil::getDelMiddleBracket(strs[1]);
                    int length = atoi(sArrayLength.data()) > 0?atoi(sArrayLength.data()):
                                 findMacroLength(sArrayLength);
                    length = length > 0?length:findEnumLength(sArrayLength);
                    //is an variable point to base type
                    if(varName.find("*") != std::string::npos){
                        varInfo.setVarInfo(strs[0] + "*",varName.substr(1),8 * BYTES_LENGTH,
                                           true,length,8 * BYTES_LENGTH * length);
                    }else{
                        varInfo.setVarInfo(strs[0],varName,wholeTypeLength,true,
                                           length,wholeTypeLength * length);
                    }
                }
                else{
                    //one variable
                    varInfo.setVarInfo(strs[0],strs[1],wholeTypeLength,
                                       false,0,wholeTypeLength);
                }
            }
        }else{
            //variable type is not a base type
            StructInfo *s = findStructInfo(strs[0]);
            varInfo.setVarName(strs[1].substr(0,strs[1].find_first_of("[")));

            if(s != nullptr && s->getStructTypeLength() > 0){
                varInfo.setTypeName(s->getStructNames()[0]);
                varInfo.setTypeLength(s->getStructTypeLength());
            }else{
                //not certain type
                varInfo.setTypeName(NOTCERTAIN_STR);
                varInfo.setTypeLength(0);
            }

            if(str.find("[") != std::string::npos){
                std::string aLength = StringUtil::getDelMiddleBracket(strs[1]);
                int arrayLength = atoi(aLength.data());
                if(arrayLength == 0){
                    arrayLength = findMacroLength(aLength);
                }

                varInfo.setIsArray(true);
                if(s != nullptr)
                    varInfo.setVarLength(arrayLength * s->getStructTypeLength());
                else
                    varInfo.setVarLength(0);
                varInfo.setArrayLength(arrayLength);
            }else{
                varInfo.setIsArray(false);
                varInfo.setArrayLength(0);
                if(s != nullptr)
                    varInfo.setVarLength(s->getStructTypeLength());
                else
                    varInfo.setVarLength(0);
            }
        }
    }
    else{
        //this situation struct tMac2PhyApiQueueElem* Next
        if(str.find(STRUCT_STR) != std::string::npos &&
           str.find("*") != std::string::npos){
            std::vector<std::string> strs = StringUtil::split(str);
            varInfo.setVarInfo(POINT_STR,strs[strs.size() - 1],8 * BYTES_LENGTH,
                               false,0,8 * BYTES_LENGTH);
        }else{
            //this situation UINT32 crcLength            :6;
            std::vector<std::string> strs = StringUtil::split(str,':');
            std::vector<std::string> typeAndVar = StringUtil::split(strs[0]);
            std::string sVarLength = StringUtil::split(strs[strs.size() - 1],';')[0];
            varInfo.setVarInfo(typeAndVar[0],typeAndVar[1],typeMap[typeAndVar[0]],
                               false,0,atoi(sVarLength.data()));
        }
    }

    structInfo.addVarInfo(varInfo);
}

int AnalysisUtil::findMacroLength(const std::string key) {
    auto iterator = macroInfos.begin();
    while(iterator != macroInfos.end()){
        std::pair<std::string,int> pair = iterator->getKeyValue();
        if(pair.first == key)
            return pair.second;
        iterator++;
    }

    return -1;
}

int AnalysisUtil::findEnumLength(const std::string key) {
    auto iterator = enumInfos.cbegin();
    while(iterator != enumInfos.cend()){
        std::map<std::string,int> m = iterator->getEnumMap();
        auto mIt = m.cbegin();
        while(mIt != m.cend()){
            if(mIt->first == key)
                return mIt->second;
            mIt++;
        }

        iterator++;
    }

    return -1;

}

StructInfo *AnalysisUtil::findStructInfo(const std::string &structName) {
    auto iterator = structInfos.cbegin();
    while(iterator != structInfos.cend()){
        std::vector<std::string> names = iterator->getStructNames();
        for(auto s:names){
            if(s == structName)
                return const_cast<StructInfo *>(&(*iterator));
        }
        iterator++;
    }

    return nullptr;
}

void AnalysisUtil::printMacroInfos(){
    auto iterator = macroInfos.begin();
    while(iterator != macroInfos.end()){
        std::pair<std::string,int> pair = iterator->getKeyValue();
        std::cout<<pair.first<<" "<<pair.second<<std::endl;
        iterator++;
    }
}

void AnalysisUtil::printEnumInfos() {
    auto it = enumInfos.cbegin();
    while(it != enumInfos.cend()){
        std::cout<<it->getEnumName()<<std::endl<<std::endl;
        std::map<std::string,int> enumMap = it->getEnumMap();
        auto enumIt = enumMap.cbegin();
        while(enumIt != enumMap.cend()){
            std::cout<<enumIt->first<<" "<<enumIt->second<<std::endl;
            enumIt++;
        }

        it++;
    }
}

void AnalysisUtil::printStructInfos() {
    auto it = structInfos.cbegin();
    while(it != structInfos.cend()){
        StructInfo &structInfo = static_cast<StructInfo &>(
                const_cast<StructInfo &>(*it));
        structInfo.operator<<(std::cout);
        it++;
    }
}


void AnalysisUtil::afterProcessFiles() {
    std::cout<<"in after process header file--------"<<std::endl;
    dealNotCertainStructInfos();
    dealNotCertainMacroInfos();
}

void AnalysisUtil::dealNotCertainStructInfos() {
    auto it = structInfos.begin();
    while(it != structInfos.end()){
        StructInfo &structInfo = const_cast<StructInfo &>(*it);
        for(auto &varInfo:structInfo.getVarInfos()){
            if(varInfo.getTypeName() == NOTCERTAIN_STR){
                //deal not certain struct varInfo type,but now
                //doesn't have this situation in header file

            }
        }

        it++;
    }
}

void AnalysisUtil::dealNotCertainMacroInfos() {
    auto it = notCertainMacroMap.begin();
    while(it != notCertainMacroMap.end()){
        auto structIt = structInfos.cbegin();
        while(structIt != structInfos.cend()){
            if(structIt->getStructNames()[0] == it->second){
                MacroInfo macroInfo;
                macroInfo.addMacro(it->first,structIt->getStructTypeLength());
                macroInfos.insert(macroInfo);
            }
            structIt++;
        }

        it++;
    }
}

int AnalysisUtil::printInfoToFile(std::string outFilePath,
                                   std::string structName,
                                   unsigned char *stream, unsigned int length) {
    std::cout<<"in print info to file--------"<<std::endl;
    if(stream == nullptr)
        throw "null pointer exception";

    auto it = structInfos.cbegin();
    bool findFlag = false;

    StructInfo *pStructInfo = nullptr;
    while(it != structInfos.cend()){
        pStructInfo = const_cast<StructInfo *>(&*it);
        if(pStructInfo->getStructNames()[0] == structName){
            deduceVarValue(*pStructInfo,stream,length);
            findFlag = true;
            break;
        }

        it++;
    }

    std::ofstream ofstream(outFilePath,std::ios_base::out);
    if(!ofstream.is_open()){
        throw "open out file failed";
    }

    if(findFlag && pStructInfo != nullptr){
        ofstream<<pStructInfo->getStructNames()[0]<<std::endl;
        std::vector<VarInfo> &vec = const_cast<std::vector<VarInfo> &>(
                pStructInfo->getVarInfos());
        for(auto &var:vec){
            var.operator<<(ofstream);
        }

        ofstream<<std::endl;
    }

    ofstream.close();
    return 0;
}

void AnalysisUtil::deduceVarValue(StructInfo &structInfo, unsigned char *stream, int length) {
    if(length <= 0 || stream == nullptr) return;
    std::vector<VarInfo> &varVec = structInfo.getVarInfos();
    unsigned int offset = 0;//record offset to stream
    int bitWidth = 0;//record variable type bit width
    int bitStart = 0;//if an bit width %BYTES_LENGTH !=0,set this variable

    for(auto &var:varVec){
        if(var.isAnArray()){
            bitWidth = var.getVarLength() / var.getArrayLength();
            for(int i = 0;i < var.getArrayLength();i++){
                int value = StringUtil::CharPointerToInt(stream + offset,
                                                         bitWidth / BYTES_LENGTH);
                offset += (bitWidth / BYTES_LENGTH);
                var.addValue(value);
            }
        }else{
            bitWidth = var.getVarLength();
            //has bit area
            if(bitWidth % BYTES_LENGTH != 0){
                int bitZeroLength = var.getTypeLength();
                BitUtil &bitUtil = BitUtil::getInstance();
                if(bitStart == 0){
                    bitUtil.initailBitZeros(stream + offset,
                                            var.getTypeLength() / BYTES_LENGTH);
                }

                long value = bitUtil.getValue(bitWidth);
                bitStart += bitWidth;
                var.addValue(value);

                offset += bitStart / BYTES_LENGTH;
                //clear bit start
                if(bitStart % BYTES_LENGTH == 0)
                    bitStart = 0;
            }else{
                int value = StringUtil::CharPointerToInt(stream + offset,
                                                         bitWidth / BYTES_LENGTH);
                var.addValue(value);
                offset += (bitWidth / BYTES_LENGTH);
            }
        }
    }
}


