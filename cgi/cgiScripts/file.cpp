#include <iostream>

int main() {
    // Output HTTP header
    std::cout << "Content-Type: text/html\n\n";

    // Output HTML content
    std::cout << "<html>\n";
    std::cout << "<head><title>Hello CGI</title></head>\n";
    std::cout << "<body>\n";
    std::cout << "<h1>Hello, World!</h1>\n";
    std::cout << "</body>\n";
    std::cout << "</html>\n";

    return 0;
}
