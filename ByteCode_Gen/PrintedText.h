#ifndef PRINTEDTEXT_H
#define PRINTEDTEXT_H

#include <string>

class PrintedText {
public:
    PrintedText(const std::string& text){

    };

    const std::string& getText() const {return text_;};

private:
    std::string text_;
};

#endif