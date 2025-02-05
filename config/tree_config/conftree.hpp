#ifndef CONFTREE_HPP
#define CONFTREE_HPP

#include <iostream>

class ConfTree {
    public:
        ConfTree();
        ConfTree(const ConfTree& other);
        ConfTree& operator=(const ConfTree& other);
        ~ConfTree();
        
        int GetPort();
        void SetPort(int Port);
        std::string GetAddress();
        void SetAddress(std::string Address);
            
    private:
        int Port;
        std::string Address;
};

#endif