

/*
 * File: main.cpp
 * Description: PWM test application (entry point).
 * Created on: 25 February 2024
 */

#include <iostream>
#include <stdint.h>
#include <csignal>
#include <atomic>
#include "config.h"

std::atomic<bool> termination_requested = false;


static void OnProcessSignal(int signal)
{
    switch (signal)
    {
        case SIGINT:
        case SIGTERM:
        {
            termination_requested = true;
            break;
        }
    }
}


/**
 * \brief Application entry point.
 */
int main(int argc, char** argv)
{
    (void) (argc);
    (void) (argv);

    std::cout << "Starting " << PROJECT_NAME <<" application" << std::endl;
    std::cout << "Version : " << VERSION_MAJOR << "." << VERSION_MINOR << "." << VERSION_PATCH << std::endl;

    try
    {
        // Set handler for the process's signals.
        signal(SIGINT, OnProcessSignal);
        signal(SIGTERM, OnProcessSignal);

        while (!termination_requested)
        {

        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    std::cout << PROJECT_NAME <<" application ended" << std::endl;
    return 0;
}
