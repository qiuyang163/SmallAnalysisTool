//
// Created by qiuyang on 2018/5/22.
//

#ifndef SAMLLANALYSIS_STRUTIL_H
#define SAMLLANALYSIS_STRUTIL_H

#include <string>
#include <iostream>
#include <cmath>

class StringUtil{
public:
    inline static std::string &trim(std::string &string){
        if(string.empty()) return string;
        string.erase(0,string.find_first_not_of(" "));
        string.erase(string.find_last_not_of(" ") + 1);
        string.erase(0,string.find_first_not_of("\r"));
        string.erase(0,string.find_first_not_of("\t"));
        string.erase(string.find_last_not_of("\t") + 1);
        string.erase(string.find_last_not_of("\r") + 1);
        return string;
    }

    inline static std::vector<std::string> split(std::string string, char sep = ' '){
        std::vector<std::string> result;
        if(string.empty()) return result;
        std::string temp("");
        for(auto i = 0;i < string.size();++i){
            if(string[i] == sep) {
                result.push_back(temp);
                temp = "";
            }else
                temp += string[i];
        }

        if(!temp.empty())
            result.push_back(temp);
        return result;
    }

    inline static std::string delBrackets(const std::string &str){
        return str.substr(str.find_first_not_of('('),str.find_last_of(')'));
    }

    inline static std::string delfirstNotAlpha(const std::string &str){
        for(int i = 0;i < str.size();i++){
            if(std::isalpha(str[i]))
                return str.substr(i);
        }

        return str;
    }

    inline static std::string delFirstStar(const std::string &str){
        if(str[0] == '*') {
            return str.substr(1);
        }

        return str;
    }

    inline static std::string dellastSemicolon(const std::string &str){
        return str.substr(0,str.find_last_of(";"));
    }

    inline static std::string getDelMiddleBracket(const std::string &str){
        int startIndex = str.find("[");
        int endIndex = str.find("]");
        int length = endIndex - startIndex - 1;
        return str.substr(startIndex + 1,length);
    }

    inline static void showMemory(unsigned char *pMsg,int len,char *name){
        int i;
        if(pMsg == nullptr)
            throw "null pointer exception";
        if(name != nullptr){
            std::cout<<"The memory "<<name<<" len is "<<len<<std::endl;
        }else{
            std::cout<<"The memory len is "<<len<<std::endl;
        }

        for(i = 0;i < len;i++){
            printf("%02x ",*(pMsg + i));
            if((i + 1) % 24 == 0) std::cout<<std::endl;
        }

        printf("\n");
    }

    inline static int CharPointerToInt(unsigned char *pMsg,int len){
        std::vector<unsigned char> cVec;
        int offset = 0;

        while(offset < len){
            cVec.push_back(*(pMsg + offset));
            offset ++;
        }

        int result = 0;
        int exp = 0;
        std::for_each(cVec.cbegin(),cVec.cend(),[&](const unsigned char c){
            result += c * std::pow(2,exp);
            exp += 8;
        });

        return result;
    }
};

#endif //SAMLLANALYSIS_STRUTIL_H
