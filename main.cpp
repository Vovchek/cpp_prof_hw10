/**
 * @file main.cpp
 * @brief network version of command batches processor.
 * @author Vladimir Chekal
 * @date April-June 2023
 * @details Otis c++ professional course homework #10.
 * Adds network server features to command processon introduces in hw #8
 **/

#include "server.h"

/**
 * @brief main function.
 * @param argc [in] Number of a program parameters strings.
 * @param argv [in] Array of pointers to c-style char arrays, storing program parameters
 * @returns Exit status - 0 (EXIT_SUCCESS) on success, non-zero (EXIT_FAILURE) on failure.
 * @details Business logic implementation has been moved to CommandProcessor class methods.
 */
int main(int argc, char const *argv[])
{
    try
    {
        size_t N{3};
        if (argc < 3 || (N = std::stoi(argv[2])) <= 0)
        {
            std::cerr << "Usage:  # bulk_server <port> <bulk_size>\n";
            return EXIT_FAILURE;
        }

        boost::asio::io_context io_context;

        server server(io_context, std::atoi(argv[1]), N);

        io_context.run();
    }
    catch (const std::exception &e)
    {
        std::cout << "Exception :" << e.what() << std::endl;
    }
}
