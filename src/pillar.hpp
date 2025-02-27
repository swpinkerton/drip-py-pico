#ifndef PILLAR_HPP
#define PILLAR_HPP

#include <string>

class Pillar
{
    public:
        Pillar(std::string identifer = " ");
        auto ToString() -> std::string;
        bool selected;

    private:
        std::string identifer;
};
#endif
