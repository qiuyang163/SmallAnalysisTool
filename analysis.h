//
// Created by qiuyang on 2018/5/22.
//

#ifndef SAMLLANALYSIS_ANALYSIS_H
#define SAMLLANALYSIS_ANALYSIS_H

#include "structinfo.h"
#include <set>

class AnalysisUtil{
private:
    static std::map<std::string,int> typeMap;
    //all struct info
    static std::vector<StructInfo> structInfos;
    //all enum info
    static std::vector<EnumInfo> enumInfos;
    //all macro info
    static std::set<MacroInfo> macroInfos;
    static std::vector<std::string> filePaths;
    static std::vector<std::string> contents;
    static std::map<std::string,std::string> notCertainMacroMap;

    AnalysisUtil(const AnalysisUtil &analysisUtil) = delete;
    AnalysisUtil &operator=(const AnalysisUtil *analysisUtil) = delete;
    /**
     * pre process c language header file,delete annotation and space
     */
    static void preProcessFiles();
    static int preProcessFileCore(std::string filePath);
    static void processRowText(std::string text);

    /**
     * deal enum info in header file
     * @param text
     * @param inEnumFlag
     * @param enumInfo
     */
    static void processEnum(const std::string &text,bool &inEnumFlag,
                            EnumInfo &enumInfo);
    /**
     * deal struct info in header file
     * @param text
     * @param inStructFlag
     * @param structInfo
     * @param inX86Flag
     */
    static void processStruct(const std::string &text,bool &inStructFlag,
                              StructInfo &structInfo,bool &inX86Flag);
    /**
     * deal macro info in header file
     * @param text
     * @return
     */
    static bool processMacro(const std::string &text);
    /**
     * state machine manage,manage if current context in a enum、macro or a struct
     * @param text
     * @param inEnumFlag
     * @param inStructFlag
     * @param enumInfo
     * @param structInfo
     */
    static void processTypedef(const std::string &text,
                               bool &inEnumFlag, bool &inStructFlag,
                               EnumInfo &enumInfo,StructInfo &structInfo);
    /**
     * detail analysis an struct info
     * @param text
     * @param inStructFlag
     * @param structInfo
     */
    static void analysisStructInfo(const std::string &text,bool &inStructFlag,
                                   StructInfo &structInfo);
    /**
     * according recored not certain macro info and struct info,
     * deduce this not certain information
     */
    static void afterProcessFiles();
    static int findMacroLength(const std::string key);
    static StructInfo *findStructInfo(const std::string &structName);
    static int findEnumLength(const std::string key);
    static void printMacroInfos();
    static void printStructInfos();
    static void printEnumInfos();
    /**
     * deal not certain macro infos
     */
    static void dealNotCertainMacroInfos();
    /**
     * deal not certain struct infos
     */
    static void dealNotCertainStructInfos();

public:
    /**
     * initial read header file path,
     * maybe add a file folder path and search all header file in future
     * @param filePath
     */
    static void addFilePath(std::string filePath){filePaths.push_back(filePath);}
    /**
     * analysis header file all enum、macro and struct info
     */
    static void analysisFile();
    /**
     * according recorded struct info,calculate every variable value with bytes stream
     * @param outFilePath
     * @param structName
     * @param stream
     * @param length
     */
    static int printInfoToFile(std::string outFilePath,std::string structName,
                                unsigned char *stream, unsigned int length);
    /**
     * deduce variable value in struct info by stream
     * @param structInfo
     * @param stream
     * @param length
     */
    static void deduceVarValue(StructInfo &structInfo,unsigned char *stream,int length);
};

#endif //SAMLLANALYSIS_ANALYSIS_H
