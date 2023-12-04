/*
 * File: main.cpp
 * Description: object-detection program entry point.
 * Created on: 4 December 2023
 */

#include <iostream>

int main(int argc, char** argv)
{
    (void) (argc);
    (void) (argv);

    std::cout << "Starting object-detection program" << std::endl;
    std::cout << "Version info:" << std::endl;
    std::cout << "  Git sha1: " << VERSION_HASH << std::endl;
    std::cout << "  Build date: " << VERSION_DATE << std::endl;
    std::cout << std::endl;


    std::cout << "object-detection program ended" << std::endl;
    return 0;
}
